/**
 * @file net_err.h
 * @author lishutong(lishutong@qq.com)
 * @brief 错误类型及常量
 * @version 0.1
 * @date 2022-10-24
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef NET_ERR_H
#define NET_ERR_H

/**
 * 错误码及其类型
 */
typedef enum _net_err_t {
    NET_ERR_NEED_WAIT = 1,                  // 需要继续等待
    NET_ERR_OK = 0,                         // 没有错误
    NET_ERR_SYS = -1,                       // 操作系统错误
    NET_ERR_MEM = -2,                       // 存储错误
    NET_ERR_FULL = -3,                      // 缓存满
    NET_ERR_TMO = -4,                       // 超时
    NET_ERR_NONE = -5,                      // 没有资源
    NET_ERR_SIZE = -6,                      // 大小错误
    NET_ERR_PARAM = -7,                     // 参数错误
    NET_ERR_EXIST = -8,                     // 不存在
    NET_ERR_STATE = -9,                     // 状态错误
    NET_ERR_IO = -10,                       // 接口IO错误
    NET_ERR_NOT_SUPPORT = -11,              // 不支持
    NET_ERR_UNREACH = -14,                  // 目的地不可达
    NET_ERR_CHKSUM = -15,                   // 校验和错误
    NET_ERR_CONNECTED = -19,                // 已经建立连接
    NET_ERR_UNKNOW = -27,                   // 未知错误
}net_err_t;

#endif // NET_ERR_H
