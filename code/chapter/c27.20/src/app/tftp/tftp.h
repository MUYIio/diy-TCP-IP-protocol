/**
 * @file tftp.h
 * @author lishutong(527676163@qq.com)
 * @brief tftp协议的实现
 * @version 0.1
 * @date 2022-11-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef TFTP_H
#define TFTP_H

#include "netapi.h"

#define TFTP_DEF_PORT               69           // 服务器缺省的端口号
#define TFTP_DEF_BLKSIZE            512          // 默认的tftp传输块大小
#define TFTP_BLKSIZE_MAX            8192         // 最大数据块大小
#define TFTP_TMO_SEC                5            // 等待的超时值
#define TFTP_MAX_RETRY              6            // 最多重试几次重发

/**
 * @brief TFTP包类型
 */
typedef enum _tftp_op_t {
    TFTP_PKT_RRQ = 1,                 // 读请求
    TFTP_PKT_WRQ,                     // 写请求
    TFTP_PKT_DATA,                    // 数据包
    TFTP_PKT_ACK,                     // 响应包
    TFTP_PKT_ERROR,                   // 错误包
    TFTP_PKT_OACK,                    // 带选项的ack

    TFTP_PKT_REQ = (TFTP_PKT_WRQ << 8) | TFTP_PKT_RRQ,  // RRQ或者WRQ
}tftp_op_t;

/**
 * @brief TFTP错误码
 */
typedef enum _tftp_err_t {
    TFTP_ERR_OK = 0,                   // 没有错误
    TFTP_ERR_NO_FILE = 1,              // 找不到文件
    TFTP_ERR_ACC_VIO,                  // 非法访问
    TFTP_ERR_DISK_FULL,                // Disk full or allocation exceeded
    TFTP_ERR_OP,                       // Illegal TFTP operation
    TFTP_ERR_UNKNOWN_TID,              // Unknown transfer ID
    TFTP_ERR_FILE_EXIST,               // File already exists      

    TFTP_ERR_USER_NOT_EXIT,             // 模式不支持
    TFTP_ERR_OPTION,                    // 协端选项出错
    TFTP_ERR_UNKNOWN,                   // 不支持的操作

    NET_TFTP_ERR_END,
}tftp_err_t;

/**
 * @brief TFTP包结构：用于DATA、ACK、ERROR包
 */
#pragma pack(1)
typedef struct _tftp_packet_t {
    uint16_t opcode;                    // 操作码

    union {
        struct {
            uint8_t args[1];            // 用于RRQ或WRQ的数据区，放置文件名和模式
        }req;

        struct {                        // DATA
            uint16_t block;             // 块序号
            uint8_t data[TFTP_BLKSIZE_MAX];   // 文件数据区
        }data;

        struct {                        // ACK
            uint16_t block;             // 块序号
            char option[1];             // 选项数据
        }ack;

        struct {                        // ACK
            char option[1];             // 选项数据
        }oack;

        struct {                        // ERROR
            uint16_t code;              // 块序号
            char msg[1];                // 文件数据区
        }err;
    };
}tftp_pkt_t;
#pragma pack()

#define TFTP_PACKET_MIN_SIZE   4                   // 至少opcode和err_code

/**
 * tftp客户端结构
 */
typedef struct _tftp_t {
    int socket;                     // 本地所用的socket
    tftp_err_t error;               // 上一次的错误
    int block_size;                 // 传输块大小
    int file_size;                  // 文件大小
    uint16_t curr_blk;              // 当前块序号

    // 当连接服务器时，服务器可能会改变其端口，使用
    // 另一端口与本地连接
    struct x_sockaddr remote;       // 远程主机

    uint8_t tmo_retry;              // 超时重发次数
    uint8_t tmo_sec;                // 等待超时的毫秒值

    int tx_size;                    // 发送数据包长度
    tftp_pkt_t rx_packet;        // 接收包缓存
    tftp_pkt_t tx_packet;        // 发送包缓存
}tftp_t;

/**
 * @brief 请求信息
 */
#define TFTP_NAME_SIZE          32      // 请求的文件名大小

typedef struct _tftp_req_t {
    tftp_t tftp;                        // tftp连接块
    tftp_op_t op;                       // 请求类型
    struct x_sockaddr_in remote;       // 远程主机
    int option;                         // 是否有选项
    int blksize;                        // 块大小
    int filesize;                       // 文件大小
    char filename[TFTP_NAME_SIZE];      // 当前操作的文件名
}tftp_req_t;

const char* tftp_error_msg(tftp_err_t err);
int tftp_send_request(tftp_t * tftp, int is_read, const char* filename, uint32_t file_size);
int tftp_send_ack(tftp_t * tftp, uint16_t block_num);
int tftp_send_oack(tftp_t * tftp);
int tftp_send_data(tftp_t * tftp, uint16_t block_num, size_t size);
int tftp_send_error(tftp_t * tftp, uint16_t code);
int tftp_resend(tftp_t * tftp);
int tftp_wait_packet(tftp_t * tftp, tftp_op_t op, uint16_t block, size_t * pkt_size);
int tftp_parse_oack (tftp_t * tftp);

#endif // NET_ECHO_US_H