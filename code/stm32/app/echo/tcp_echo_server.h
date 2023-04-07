/**
 * @brief 简单的TCP回显服务器程序
 *
 * @author lishutong (527676163@qq.com)
 * @version 0.1
 * @date 2022-08-19
 * @copyright Copyright (c) 2022
 * @note 该程序仅限在Mac和Linux编程上编译通过，Windows由于套接字接口不同编译会失败
 */
#ifndef TCP_ECHO_SERVER_H
#define TCP_ECHO_SERVER_H

int tcp_echo_server_start(int port);

#endif // TECHO_H
