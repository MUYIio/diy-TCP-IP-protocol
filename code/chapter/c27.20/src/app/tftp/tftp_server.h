/**
 * @file tftp_server.h
 * @author lishutong(527676163@qq.com)
 * @brief tftp服务器的实现
 * @version 0.1
 * @date 2022-11-02
 * 
 * @copyright Copyright (c) 2022
 * 支持多客户端同时连接
 */
#ifndef TFTP_SERVER_H
#define TFTP_SERVER_H

#include "tftp.h"

net_err_t tftpd_start (const char* dir, uint16_t port);

#endif // TFTP_SERVER_H