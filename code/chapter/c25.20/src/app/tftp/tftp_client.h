/**
 * @file tftp_client.h
 * @author lishutong(527676163@qq.com)
 * @brief tftp客户端的实现
 * @version 0.1
 * @date 2022-11-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H

#include "tftp.h"

#define TFTP_CMD_BUF_SIZE           128     // tftp客户端命令行缓存

int tftp_start (const char * ip, uint16_t port);
int tftp_get(const char * ip, uint16_t port, int block_size, const char* filename);
int tftp_put(const char* ip, uint16_t port, int block_size, const char* filename);

#endif // TFTP_CLIENT_H