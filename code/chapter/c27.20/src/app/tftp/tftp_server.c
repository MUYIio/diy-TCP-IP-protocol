/**
 * @file tftp.c
 * @author lishutong(527676163@qq.com)
 * @brief tftp协议的实现
 * @version 0.1
 * @date 2022-11-02
 * 
 * @copyright Copyright (c) 2022
 * 不支持多客户端连接，另外支持带选项和不带选项的请求
 */
#include <string.h>
#include "tftp_server.h"

static const char * server_path;
static uint16_t server_port;

/**
 * @brief 接收客户端上传的文件
 */
static int do_recv_file(tftp_req_t * req) {
    char path_buf[128];
    tftp_t * tftp = &req->tftp;

    // 生成文件名
    if (server_path) {
        snprintf(path_buf, sizeof(path_buf), "%s/%s", server_path, req->filename);
    } else {
        snprintf(path_buf, sizeof(path_buf), "%s", req->filename);
    }

    // 先创建文件，注意以二进制写入，否则以文本模式会进行换行符号的改写，即遇到0x0A时，实际写入0x0A, 0x0D
    FILE * file = fopen(path_buf, "wb");
    if (file == (FILE*)0) {
        printf("tftpd: create %s failed\n", path_buf);
        tftp_send_error(tftp, TFTP_ERR_DISK_FULL);
        return -1;
    }

    printf("tftpd: recving %s\n", path_buf);
    
    // 响应ack，表示可以写
    int err = tftp_send_oack(tftp);
    if (err < 0) {
        printf("tftpd: send oack failed.\n");
        fclose(file);
        return -1;
    }

    // 循环接收文件数据
    uint16_t curr_blk = 1;
    uint32_t total_size = 0;            // 收发的总字节量
    uint32_t total_block = 0;           // 收发的总块数量
    tftp_pkt_t * rpacket = &tftp->rx_packet;
    while (1) {
        // 获取数据包及其序号
        size_t pkt_size;
        err = tftp_wait_packet(tftp, TFTP_PKT_DATA, curr_blk, &pkt_size);
        if (err < 0) {
            // 其它错误，应当直接退出
            printf("tftpd: wait data failed.\n");
            break;
        }

        // 块序号与期望的相同，则将其写入文件
        size_t block_size = pkt_size - 4;         // 去掉block + opcode大小

        // 相等，将数据写入文件，然后继续申请下一包，减4-opcode和block_num
        size_t size = fwrite(rpacket->data.data, 1, block_size, file);
        if (size < block_size) {
            printf("tftpd: write file %s failed.\n", path_buf);
            break;
        }

        // 写成功后，以当前的块序号发送响应
        err = tftp_send_ack(tftp, curr_blk++);
        if (err < 0) {
            printf("tftpd: send ack failed.\n");
            break;
        }

        // 增加计数
        total_size += (uint32_t)size;
        total_block++;

        // 不足一块，传输结束
        if (block_size < tftp->block_size) {
            break;
        }
    }

    printf("tftpd: recv %s, %u byte, %u blocks\n", path_buf, total_size, total_block);
    if (total_size != tftp->file_size) {
        printf("tftpd: size incorrect %d = %d\n", total_size, tftp->file_size);
    }
    fclose(file);
    return err;
}

/**
 * @brief 发送文件给客户端
 */
static int do_send_file(tftp_req_t * req) {
    char path_buf[128];
    tftp_t * tftp = &req->tftp;
    tftp_pkt_t * tpacket = &tftp->tx_packet;

    // 生成文件名
    if (server_path) {
        snprintf(path_buf, sizeof(path_buf), "%s/%s", server_path, req->filename);
    } else {
        snprintf(path_buf, sizeof(path_buf), "%s", req->filename);
    }

    // 打开文件, 二进制文件打开
    FILE * file = fopen(path_buf, "rb");
    if (file == (FILE*)0) {
        // 打开失败，发送失败信息
        printf("tftpd: file %s does not exist\n", path_buf);
        tftp_send_error(tftp, TFTP_ERR_NO_FILE);
        return -1;
    }

    printf("tftp: sending file %s...\n", path_buf);

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    tftp->file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 响应oack，表示有数据
    int err = tftp_send_oack(tftp);
    if (err < 0) {
        printf("tftpd: send oack failed.\n");
        goto send_failed;
    }

    // 等待对方的确认ACK
    size_t pkt_size;
    err = tftp_wait_packet(tftp, TFTP_PKT_ACK, 0, &pkt_size);
    if (err < 0) {
        // 等待失败，退出发送过程
        printf("tftp: wait packet failed, err = %d\n", err);
        goto send_failed;
    }

    // 循环发送文件数据，直至发送完毕
    uint16_t curr_blk = 1;
    uint32_t total_size = 0;            // 收发的总字节量
    uint32_t total_block = 0;           // 收发的总块数量
    while (1) {
        // 读取文件到发送缓存
        size_t size = fread(tpacket->data.data, 1, tftp->block_size, file);
        if (size < 0) {
            // 未遇到文件末尾，但又读取量少于要求量，应当是读取发生错误, 退出
            printf("tftpd: read file %s failed\n", path_buf);
            tftp_send_error(tftp, TFTP_ERR_ACC_VIO);
            goto send_failed;
       }

        // 读取成功后，以当前的块序号发送数据块
        err = tftp_send_data(tftp, curr_blk, size);
        if (err < 0) {
            printf("tftp: send data block failed.\n");
            goto send_failed;
        }

        // 等待对方的响应
        err = tftp_wait_packet(tftp, TFTP_PKT_ACK, curr_blk++, &pkt_size);
        if (err < 0) {
            // 等待失败，退出发送过程
            printf("tftp: wait packet failed, err = %d\n", err);
            goto send_failed;
        }

        // 增加计数
        total_size += (uint32_t)size;
        total_block++;

        // 检查文件是否发送完毕，退出
        if (size < tftp->block_size) {
            break;
        }
    }

    // 打印结果
    printf("tftpd: recv %s, %u byte, %u blocks\n", path_buf, total_size, total_block);
send_failed:
    if (total_size != tftp->file_size) {
        printf("tftpd: size incorrect %d = %d\n", total_size, tftp->file_size);
    }
    fclose(file);
    return err;
}

/**
 * @brief 处理客户请求的线程
 * 
 * @param arg 
 */
static void tftp_working_thread (void * arg) {
    tftp_req_t * req = (tftp_req_t *)arg;
    tftp_t * tftp = &req->tftp;

    // 打开服务器套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        printf("tftp %s: create server socket failed\n", inet_ntoa(req->remote.sin_addr));
        return;
    }

    // 连接到远程，专门与其进行通信
    if (connect(sockfd, (struct sockaddr *)&req->remote, sizeof(req->remote)) < 0) {
        printf("tftp %s: connect error\n", inet_ntoa(req->remote.sin_addr));
        goto init_error;
    }

    // 设置超时
    struct timeval tmo;
    tmo.tv_sec = tftp->tmo_sec;
    tmo.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tmo, sizeof(tmo));

    // 设置各项缺省值
    tftp->socket = sockfd;
    tftp->error = TFTP_ERR_OK;
    tftp->tmo_retry = TFTP_MAX_RETRY;
    tftp->tmo_sec = TFTP_TMO_SEC;
    tftp->tx_size = 0;
    tftp->block_size = req->blksize;
    memcpy(&tftp->remote, &req->remote, sizeof(req->remote));

    // 根据读写类型来做不同的处理
    if (req->op == TFTP_PKT_WRQ) {
        tftp->file_size = req->filesize;
        do_recv_file(req); 
    } else {
        do_send_file(req);
    }
init_error:
    close(sockfd);
    free(req);
}

/**
 * @brief 等待任意客户端的请求
 */
static int wait_for_req(tftp_t * tftp, tftp_req_t * req) {
    tftp_pkt_t * pkt = &tftp->rx_packet;

    // 等待请求包, 读或写均可. 对于请求包，没有重传，因为没有开超时
    size_t pkt_size;
    int err = tftp_wait_packet(tftp, TFTP_PKT_REQ, 0, &pkt_size);
    if (err < 0) {
        printf("tftpd: wait RRQ/WRQ failed.\n");
        return err;
    }

    // 记录操作码，缺省数据
    req->op = ntohs(pkt->opcode);
    req->option = 0;
    req->blksize = TFTP_DEF_BLKSIZE;        // 给个缺省值
    memcpy(&req->remote, &tftp->remote, sizeof(tftp->remote));
    memset(req->filename, 0, sizeof(req->filename));

    // 显示连接信息
    struct sockaddr_in * addr = (struct sockaddr_in *)&tftp->remote;
    printf("tftpd: recv req %s from %s %d\n", 
        req->op == TFTP_PKT_RRQ ? "get": "put",
        inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));

    // 提取请求信息
    char * buf = (char *)pkt->req.args;
    char * end = (char *)pkt + pkt_size - 2;     // 不算操作码2字节

    // 先提取文件名
    strncpy(req->filename, buf, sizeof(req->filename));
    buf += strlen(req->filename) + 1;

    // 再提取模式, 出错错误的包
    if (strcmp(buf, "octet") != 0) {
        printf("tftpd: unknown transfter mode %s\n", buf);
        return -1;
    }
    buf += strlen("octet") + 1;

    // 再解析其它选项数据
    while ((buf < end) && *buf) {
        req->option = 1;        // 有选项

        if (strcmp(buf, "blksize") == 0) {
            buf += strlen(buf) + 1;

            int blksize = atoi(buf);
            if (blksize <= 0) {
                // 转换失败，发送错误
                tftp_send_error(tftp, TFTP_ERR_OPTION);
                return -1;
            } else if (blksize < TFTP_BLKSIZE_MAX) {
                // 使用客户端传来的块大小
                req->blksize = blksize;
            } else {
                req->blksize = TFTP_BLKSIZE_MAX;
            }

            buf += strlen(buf) + 1;
        } else if (strcmp(buf, "tsize") == 0) {
            buf += strlen(buf) + 1;
            req->filesize = atoi(buf);
            buf += strlen(buf) + 1;
        } else {
            buf += strlen(buf) + 1;
        }
    }

    return NET_ERR_OK;
}

/**
 * @brief TFTP服务器处理函数
 */
static void tftp_server_thread(void* arg) {
    tftp_t tftp;

    // 打开服务器套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        printf("tftpd: create server socket failed\n");
        return;
    }

    // 绑定本地和端口
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(struct sockaddr_in));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(server_port);  
    if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
        printf("tftpd: bind error, port: %d\n", server_port);
        close(sockfd);
        return;
    }
    tftp.socket = sockfd;

    // 循环等待客户连接
    while (1) {
        tftp_req_t * req = (tftp_req_t *)malloc(sizeof(tftp_req_t));

        // 等待客户端发来的请求
        int err = wait_for_req(&tftp, req);
        if (err < 0) {
            // 失败，释放资源
            free(req);
            continue;
        }

        // 创建工作线程处理该请求
        if (sys_thread_create(tftp_working_thread, req) == SYS_THREAD_INVALID) {
            free(req);
        }
    }

    close(sockfd);
}

/**
 * @brief 启动tftp启动服务器
 */
int tftpd_start (const char* dir, uint16_t port) {
    server_path = dir;
    server_port = port ? port : TFTP_DEF_PORT;

    // 创建单独的线程来运行服务
    sys_thread_t thread = sys_thread_create(tftp_server_thread, (void *)0);
    if (thread == SYS_THREAD_INVALID) {
        printf("tftpd: create server thread failed\n");
        return -1;
    }

    printf("tftpd: server is running, port=%d\n", port);
    return 0;
}

