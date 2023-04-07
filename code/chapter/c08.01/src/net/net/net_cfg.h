/**
 * @file net_cfg.h
 * @author lishutong(lishutong@qq.com)
 * @brief 协议栈的配置文件
 * @version 0.1
 * @date 2022-10-25
 * 
 * @copyright Copyright (c) 2022
 * 
 * 所有的配置项，都使用了类似#ifndef的形式，用于实现先检查appcfg.h有没有预先定义。
 * 如果有的话，则优先使用。否则，就使用该文件中的缺省配置。
 */
#ifndef NET_CFG_H
#define NET_CFG_H

// 调试信息输出
#define DBG_MBLOCK		    DBG_LEVEL_INFO			// 内存块管理器
#define DBG_QUEUE           DBG_LEVEL_INFO          // 定长存储块

#endif // NET_CFG_H

