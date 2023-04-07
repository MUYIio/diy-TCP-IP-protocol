/**
 * @file httpd.c
 * @author lishutong(527676163@qq.com)
 * @brief http web服务器的简单实现
 * @version 0.1
 * @date 2022-11-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "httpd.h"
#include "sys.h"
#include <string.h>
#include <stdlib.h>

static const char * root_dir;           // 文档的根目录
static uint16_t server_port;            // 服务器端口号
static int server_socket;               // 服务器套接字
static http_client_t client_tbl[HTTPD_MAX_CLIENT];  // 客户端列表
static sys_sem_t client_sem;
static sys_mutex_t client_mutex;

/**
 * @brief MIME信息表
 */
static const http_mime_info_t mime_tbl[] = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".bmp", "image/bmp"},
    {".gif", "image/gif"},
    {".png", "image/png"},
    {".js", "application/x-javascript"},
};

/**
 * @brief 纪录日志
 */
#define httpd_log(fmt, ...)     {   \
    printf("httpd:");        \
    printf(fmt, ##__VA_ARGS__);  \
    puts("\n");          \
}

/**
 * @brief 查找指定的mime类型
 */
static const char * mime_find (const char * filename) {
    for (int i = 0; i < sizeof(mime_tbl) / sizeof(mime_tbl[0]); i++) {
        const http_mime_info_t * type = mime_tbl + i;
        if (strstr(filename, type->extension) != (char *)0) {
            return type->type;
        }
    }

    return (const char *)0;
}

/**
 * @brief 分配一个client结构
 */
static http_client_t * alloc_client (void) {
    http_client_t * c = (http_client_t *)0;

    // 等待可用的client结构
    sys_sem_wait(client_sem, 0);

    // 从中找出一个可用的结构
    sys_mutex_lock(client_mutex);
    for (int i = 0; i < HTTPD_MAX_CLIENT; i++) {
        http_client_t * curr = client_tbl + i;
        if (curr->sock < 0) {
            c = curr;
            c->sock = 0;        // 标记为已经分配出去
            break;
        }
    }
    sys_mutex_unlock(client_mutex);
    return c;
}

/**
 * @brief 释放一个client结构
 */
static void free_client (http_client_t * c) {
    sys_mutex_lock(client_mutex);
    c->sock = -1;
    sys_mutex_unlock(client_mutex);

    sys_sem_notify(client_sem);
}

/**
 * @brief 显示客户端信息
 */
static void show_client_info (char * title, http_client_t * client) {
    char ipbuf[32];

    inet_ntop(AF_INET, &client->addr.sin_addr, ipbuf, sizeof(ipbuf));
    printf("httpd[%d]: %s ip: %s, port: %d\n", 
            (int)(client - client_tbl), title, ipbuf, ntohs(client->addr.sin_port));
}

/**
 * @brief 显示请求信息
 * 仅供调式使用
 */
static void show_request (http_request_t * request) {
    // 起始行信息
    printf("method:%s, url: %s, version: %s\n", request->method, request->url, request->version);
}

/**
 * @brief 当请求不合法时，发送错误信息给客户端
 */
static void http_404_not_found (http_client_t * client) {
    // 注意不要写成const char * response，否则下面sizeof计算错误
	static const char response[] =
		"HTTP/1.1 404 File Not Found\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"
		"<html><title>File not found</title><body><h1>Not Found</h1><p>File not found</p></body></html>\r\n";
	send(client->sock, response, sizeof(response), 0);
}

/**
 * @brief 当请求不合法时，发送错误信息给客户端
 */
static void http_400_bad_request (http_client_t * client) {
    // 注意不要写成const char * response，否则下面sizeof计算错误
	static const char response[] =
		"HTTP/1.1 400 Bad Request\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"
		"<html><title>Bad Request</title><body>Bad Request from your browser</body></html>\r\n";
	send(client->sock, response, sizeof(response), 0);
}

/**
 * @brief 当请求不合法或者服务器无法服务请求时发送该错误
 */
static void http_501_not_implemented (http_client_t * client) {
    // 注意不要写成const char * response，否则下面sizeof计算错误
	static const char response[] =
		"HTTP/1.1 501 Not Implemented\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"
		"<html><title>Not Implementedt</title><body>Unsupport method from you frowser</body></html>\r\n";
	send(client->sock, response, sizeof(response), 0);
}

/**
 * @brief 服务器拒绝服务该请求t 
 */
static void http_403_forbidden (http_client_t * client) {
    // 注意不要写成const char * response，否则下面sizeof计算错误
	static const char response[] =
		"HTTP/1.1 403 Forbidden\r\n"
		"Content-Type: text/html\r\n"
		"\r\n"
		"<html><title>Forbidden</title><body>You don't have acces to this content</body></html>\r\n";
	send(client->sock, response, sizeof(response), 0);
}

#define STR(arg)        #arg
#define STR2(arg)       STR(arg)

/**
 * @brief 不断从缓冲中读取数据，然后栎建出请求结构
 */
static int request_parese (http_client_t * client, http_request_t * request) {
    char buf[2048];

    // 只读取一次，由于目前只处理GET，所以应该能一次性读取完毕
    int read_size = recv(client->sock, buf, sizeof(buf), 0);
    if (read_size < 0) {
        httpd_log("httpd: client read error");
        return -1;
    }
    buf[sizeof(buf) - 1] = '\0';

    // 先解析起始行
    char * method = strchr(buf, '\r');
    if (method) {
        *method = '\0';
    }

    char * fmt = "%"STR2(HTTP_SIZE_METHOD)"s %"STR2(HTTP_SIZE_URL)"s %"STR2(HTTP_SIZE_VERSION)"s\r\n";
    int count = sscanf(buf, fmt, request->method, request->url, request->version);       // 注意缓冲区问题
    return count == 3 ? 0 : -1;    
}

/**
 * @brief 一些首部类型
 */
static const property_info_t property_info_tbl[] = {
    [HTTP_CONTENT_LENGTH] = {.name = "Content-Length",},
    [HTTP_CONTENT_TYPE] = {.name = "Content-Type", },
    [HTTP_CONNECTION] = {.name = "Connection",},
};

/**
 * @brief 初始化响应结构
 */
static void response_init(http_response_t * response) {
	memset(response, 0, sizeof(http_response_t));
}

/**
 * @brief 设置起始信息
 */
static void response_set_start (http_response_t * response, const char * version, const char * status, const char * reason) {
    strncpy(response->version, version, sizeof(response->version) - 1);
    strncpy(response->status, status, sizeof(response->status) - 1);
    strncpy(response->reason, reason, sizeof(response->reason)- 1);
}

/**
 * @brief 添加属性
 */
static void response_set_property (http_response_t * response, property_key_t key, const char * value) {
    property_t * p = (property_t *)0;

    // 遍历找空闲项或者已经存在的项
    for (int i = 0; i < HTTP_RESPONSE_PROPERTY_MAX; i++) {
        property_t * curr = response->property + i;
        if ((curr->key == HTTP_PROPERTY_NONE) && !p) {
            p = curr;
            continue;
        } else if (curr->key == key) {
            p = curr;
            break;             
        }
    }

    // 找到则更新
    if (p) {
        p->key = key;
        strncpy(p->value, value, sizeof(p->value) - 1);
    }
}

/**
 * @brief 生成响应最终的文本信息
 */
static size_t response_text (http_response_t * response, char * buf, size_t size) {
    // 清空，似乎没有太大必要，或者写成buf[0] = '\0'比较简单
    memset(buf, 0, size);

    // 先做起始行
    sprintf(buf, "%s %s %s\r\n", response->version, response->status, response->reason);
    size_t text_size = strlen(buf);
    buf += text_size;

    // 写起始行
    size_t free_size = size - text_size;
    for (int i = 0; i < HTTP_RESPONSE_PROPERTY_MAX; i++) {
        property_t * curr = response->property + i;

        // 跳过无效的项
        if (curr->key == HTTP_PROPERTY_NONE) {
            continue;
        }

        // 写入属性值
        size_t copy_size = strlen(property_info_tbl[curr->key].name) + 2 + strlen(curr->value) + 2;        // 包含\r\n以及:号
        
        // 空间不够，退出，不能只拷贝一部分
        if (copy_size > free_size) {
            break;
        }

        // 写入属性值等
        sprintf(buf, "%s: %s\r\n", property_info_tbl[curr->key].name, curr->value);

        text_size += copy_size;
        buf += copy_size;
    }

    sprintf(buf, "%s", "\r\n");
    text_size += 2;
    
    return text_size;
}

/**
 * @brief HTTP GET方法处理
 * 目前暂时只处理普通文件的发送
 */
static void method_get_in(http_client_t * client, http_request_t * request) {
    // 确认路径中指向的是目录还是文件，如果是目录，则访问目录下的主页文件
	char buf[8192];
	if (request->url[strlen(request->url) - 1] == '/') {
        sprintf(buf, "%s%s%s", root_dir, request->url, "index.html");
	} else {
        sprintf(buf, "%s%s", root_dir, request->url);
    }
    
    // 打开要访问的文件，一定要注意文件打开的模式
    char * name = buf;
    if (buf[0] == '/') {
        name++;
    }

    FILE * file = fopen(buf, "rb");
    if (file == (FILE *)0) {
        httpd_log("httpd: [GET] file %s not found\n", buf);
        http_404_not_found(client);
        return;
    }

    // 发送起始行和首部等信息
    http_response_t response;
    response_init(&response);
    response_set_start(&response, "HTTP/1.1", "200", "OK");

    // 响应后应当关闭连接(可加可不加，不加的会使用TCP KeepAlive保持连接)
    response_set_property(&response, HTTP_CONNECTION, "close");

    // 添加mime类型，用于查看非html文件用
    const char * mime_type = mime_find(buf);
    if (!mime_type) {
        mime_type = "application/octet-stream";
    } 

    // 设置缺省类型
    response_set_property(&response, HTTP_CONTENT_TYPE, mime_type);

    size_t size = response_text(&response, buf, sizeof(buf));
    send(client->sock, buf, size, 0);

    // 不断读取文件并写入套接字
    while ((size = fread(buf, 1, sizeof(buf), file)) > 0) {
        if (send(client->sock, buf, size, 0) < 0) {
            printf("http: send error, quit\n");
            break;
        }
    }

    // 最后记得关闭文件
    printf("httpd: send %s end\n", request->url);
    fclose(file);
}

/**
 * @brief 处理HTTP请求
 */
static void process_request(http_client_t * client, http_request_t * request) {
    if (strcmp(request->method, "GET") == 0) {
        method_get_in(client, request);
    } else {
        // 不能识别的方法，发送错误信息
        http_501_not_implemented(client);
    }
}

/**
 * @brief 检查请求是否合法 
 */
static int valid_request (http_client_t * client, http_request_t * request) {
    // 方法，具体方法在以后检查
    if (request->method[0] == '\0') {
        httpd_log("method is empty");
        http_400_bad_request(client);
        return -1;
    }

    // 目录不能为空；也不能访问上层目录，以免访问到htdocs之外的其它地方
    if ((request->url[0] == '\0') || strstr(request->url, "..")) {
        httpd_log("url is not valid");
        http_403_forbidden(client);
        return -1;
    }

    // 检查版本号
    if ((request->version[0] == '\0') || (!strcmp(request->version, "HTTP/1.1") 
            && !strcmp(request->version, "HTTP/1.0"))) {
        http_400_bad_request(client);
        return -1;
    }

    return 0;
}

/**
 * @brief 处理具体客户端连接的线程
 */
static void client_thread(void* arg) {
    http_client_t * client = (http_client_t *)arg;

    // 解析请求
    http_request_t request;
    if (request_parese(client, &request) < 0) {
        httpd_log("httpd: parse request failed.");
        show_client_info("request error", client);
    } else {
        // 显示请求信息
        show_request(&request);

        // 处理请求
        if (valid_request(client, &request) < 0) {
            httpd_log("httpd: request is not valid\n");
        } else {
            process_request(client, &request);
        }
    }
    
    // 关闭套接字，并删除自己
    show_client_info("close request", client);
    close(client->sock);
    free_client(client);
    sys_thread_exit(0);
}

/**
 * @brief httpd服务器线程
 */
static void httpd_server_thread(void* arg) {
    // 循环监听连接，收到连接后交给相应的线程去处理
	for (;;) {
        http_client_t * client = alloc_client();

        // 等待客户端的连接
        int len = sizeof(struct sockaddr_in);
		client->sock = accept(server_socket, (struct sockaddr *)&client->addr, &len);
		if (client->sock < 0) {
            // 出错，释放结构并重来
            httpd_log("httpd: accept error\n");
            free_client(client);
        	continue;;
        }

        // 显示客户端连接信息
        show_client_info("new connection", client);

        // 创建客户端线程来处理请求
        // 当然，也可以在启动服务器时先将所有的线程创建好，以提升运行效率
        sys_thread_t thread = sys_thread_create(client_thread, (void *)client);
        if (thread == SYS_THREAD_INVALID) {
            httpd_log("httpd: create client thread failed\n");
            free_client(client);

            // 注意关闭套接字
            close(client->sock);
            continue;
        }
	}
}

/**
 * @brief 启动httpd服务器
 */
int httpd_start (const char* dir, uint16_t port) {
    if (!dir) {
        httpd_log("param error\n");
        return NET_ERR_PARAM;
    }

    root_dir = dir;
    server_port = port ? port : HTTPD_DEFAULT_PORT;
    client_sem = SYS_SEM_INVALID;
    client_mutex = SYS_MUTEx_INVALID;

    // 创建服务器套接字，使用IPv4，数据流传输
	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0) {
		httpd_log("httpd: create socket error\n");
		goto start_error;
	}

    // 绑定本地地址
    struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		httpd_log("httpd: bind error\n");
		goto start_error;
	}

    // 进入监听状态
	if (listen(server_socket, HTTPD_BACKLOG) < 0) {
		httpd_log("httpd: listen error\n");
		goto start_error;
	}

    // 创建客户端列表
    memset(client_tbl, -1, sizeof(client_tbl));
    client_sem = sys_sem_create(HTTPD_MAX_CLIENT); 
    if (client_sem == SYS_SEM_INVALID) {
        httpd_log("httpd: create sem failed.");
        goto start_error;
    }

    client_mutex = sys_mutex_create();
    if (client_mutex == SYS_MUTEx_INVALID) {
        httpd_log("httpd: create mutex failed.");
        goto start_error;
    }

    // 创建单独的线程来运行服务
    sys_thread_t thread = sys_thread_create(httpd_server_thread, (void *)0);
    if (thread == SYS_THREAD_INVALID) {
        httpd_log("httpd: create server thread failed\n");
		goto start_error;
    }

    printf("httpd: server is running, port=%d\n", port);
    return 0;
start_error:
    // 释放资源
    if (client_sem != SYS_SEM_INVALID) {
        sys_sem_free(client_sem);
    }
    if (client_mutex != SYS_MUTEx_INVALID) {
        sys_mutex_free(client_mutex);
    }
    if (server_socket >= 0) {
        close(server_socket);
    }
    return -1;
}
