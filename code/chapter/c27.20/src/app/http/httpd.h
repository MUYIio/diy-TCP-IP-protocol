/**
 * @file httpd.h
 * @author lishutong(527676163@qq.com)
 * @brief http web服务器的简单实现
 * @version 0.1
 * @date 2022-11-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef HTTPD_H
#define HTTPD_H

#include "net_err.h"
#include "net_plat.h"
#include "netapi.h"

#if 0 // windows下使用
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#define HTTP_SIZE_METHOD            10      // 方法名长度
#define HTTP_SIZE_VERSION           10      // 版本号字节量
#define HTTP_SIZE_URL               1024    // URL长度

#define HTTP_SIZE_STATUS            5       // 状态码长度
#define HTTP_SIZE_REASON            10      // 状态原因

#define HTTPD_DEFAULT_ROOT          "htdocs"     // 缺省的HTML目录
#define HTTPD_DEFAULT_PORT          80          // HTTP服务器缺省端口
#define HTTPD_BACKLOG               5           // 队列中等待处理的连接数量
#define HTTPD_MAX_CLIENT            5          // 最多允许的客户端服务数量

/**
 * @brief HTTP MIME类型
 */
typedef struct _http_mime_info_t {
    const char *extension;
    const char *type;
}http_mime_info_t;

/**
 * @brief 首部类型
 */
typedef enum _property_key_t {
    HTTP_PROPERTY_NONE = 0,
    HTTP_CONTENT_TYPE,
    HTTP_CONTENT_LENGTH,
    HTTP_CONNECTION,
}property_key_t;

/**
 * @brief HTTP首部的属性
 */
typedef struct _property_t {
    property_key_t key;
    char value[32];
}property_t;

/**
 * @brief HTTP属性信息 
 */
typedef struct _property_info_t {
    const char * name;
}property_info_t;

/**
 * @brief HTTP请求 
 */
typedef enum _http_method_t {
    HTTP_METHOD_GET,            // GET方法
}http_method_t;

/**
 * @brief HTTP客户端连接
 */
typedef struct _http_client_t {
	int sock;
	struct sockaddr_in addr;
    int len;
}http_client_t;

/**
 * @brief HTTP请求描述结构
 */
typedef struct _http_request_t {
	char method[HTTP_SIZE_METHOD];      // 方法名
	char url[HTTP_SIZE_URL];            // URL
	char version[HTTP_SIZE_VERSION];    // 版本号
}http_request_t;

#define HTTP_RESPONSE_PROPERTY_MAX      10

/**
 * @brief HTTP响应结构
 */
typedef struct _http_response_t {
    char version[HTTP_SIZE_VERSION];
    char status[HTTP_SIZE_STATUS];
    char reason[HTTP_SIZE_REASON];
	property_t property[HTTP_RESPONSE_PROPERTY_MAX];
}http_response_t;

int httpd_start (const char* dir, uint16_t port);

#endif // HTTPD_H

