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
#define DBG_MBLOCK		    DBG_LEVEL_ERROR			// 内存块管理器
#define DBG_QUEUE           DBG_LEVEL_ERROR          // 定长存储块
#define DBG_MSG             DBG_LEVEL_ERROR          // 消息通信
#define DBG_BUF             DBG_LEVEL_ERROR          // 数据包管理器
#define DBG_PLAT            DBG_LEVEL_ERROR          // 系统平台
#define DBG_INIT            DBG_LEVEL_ERROR          // 初始化模块
#define DBG_NETIF           DBG_LEVEL_ERROR          // 网络接口层
#define DBG_ETHER           DBG_LEVEL_ERROR          // 以太网协议层
#define DBG_TOOLS           DBG_LEVEL_ERROR          // 工具集
#define DBG_TIMER           DBG_LEVEL_ERROR         // 定时器
#define DBG_ARP             DBG_LEVEL_ERROR          // ARP协议
#define DBG_IP			    DBG_LEVEL_ERROR          // 调试开关
#define DBG_ICMP			DBG_LEVEL_ERROR			// 调试开关
#define DBG_SOCKET		    DBG_LEVEL_ERROR			// 调试开关
#define DBG_RAW			    DBG_LEVEL_ERROR         // 调试开关
#define DBG_UDP			    DBG_LEVEL_ERROR			// 调试开关
#define DBG_DNS             DBG_LEVEL_ERROR              // DNS调试开关
#define DBG_TCP			    DBG_LEVEL_INFO		// 调试开关
//#define DBG_TCP			    DBG_LEVEL_INFO      // DBG_LEVEL_ERROR		// 调试开关

#define NET_ENDIAN_LITTLE       1                   // 系统是否为小端

#define EXMSG_MSG_CNT          10                 // 消息缓冲区大小

#define PKTBUF_BLK_SIZE         1024                 // 数据包块大小
#define PKTBUF_BLK_CNT          2048                 // 数据包块的数量
#define PKTBUF_BUF_CNT          1024                 // 数据包的数量

#define NETIF_USE_INT                   0           // 网络接口使用中断保护
#define NETIF_HWADDR_SIZE               10                  // 硬件地址长度，mac地址最少6个字节
#define NETIF_NAME_SIZE                 10                  // 网络接口名称大小
#define NETIF_DEV_CNT                   4                   // 网络接口的数量
#define NETIF_INQ_SIZE                  50                  // 网卡输入队列最大容量
#define NETIF_OUTQ_SIZE                 50                  // 网卡输出队列最大容量

#define TIMER_NAME_SIZE             32              // 定时器名称长度
#define TIMER_SCAN_PERIOD           50             // 定时器列表扫描周期

#define ARP_CACHE_SIZE					2 // 50              // ARP表项大小
#define ARP_MAX_PKT_WAIT			5               // ARP表上最多等待的缓存数量
#define ARP_ENTRY_STABLE_TMO			 (20*60)		    // 已解析的ARP表项超时值，以秒计，一般20分钟
#define ARP_ENTRY_PENDING_TMO			3               // 正在解析的ARP表项超时值，以秒计， RFC1122建议每秒1次
#define ARP_ENTRY_RETRY_CNT				5               // 挂起的ARP表项请求解析的重试次数
#define ARP_TIMER_TMO               1               // ARP表扫描的周期(秒)

#define IP_FRAGS_MAX_NR               10              // 最多支持的分片控制数量
#define IP_FRAG_MAX_BUF_NR             10              // 每个IP分片最多允许停留的buf数量
#define IP_FRAG_SCAN_PERIOD         (1)             // IP分片表扫描周期，以秒为单位
#define IP_FRAG_TMO                 5               // IP分片最大超时时间，以秒为单位
#define IP_RTABLE_SIZE				    16          // 路由表项数量

#define RAW_MAX_NR                      5           // RAW控制块数量
#define RAW_MAX_RECV                    50          // RAW控制块接收队列限长

#define UDP_MAX_NR				        4		    // UDP的数量
#define UDP_MAX_RECV                    50          // RAW控制块接收队列限长

#define DNS_REQ_SIZE                    10          // DNS请求结构
#define DNS_SERVER_DEFAULT              "180.76.76.76"  // 百度的DNS服务器
#define DNS_SERVER_0                    "8.8.8.8"       // google的DNS服务器
#define DNS_SERVER_1                    "223.6.6.6"     // 阿里云的DNS服务器
#define DNS_SERVER_MAX                  4           // 最大支持的DNS服务器表项数量
#define DNS_ENTRY_SIZE                  10          // DNS表项的数量
#define DNS_DOMAIN_NAME_MAX             64          // DNS中域名的长度
#define DNS_PORT_DEFAULT                53          // 缺省的DNS端口
#define DNS_QUERY_RETRY_TMO             5           // DNS查询超时
#define DNS_QUERY_RETRY_CNT             5           // 查询超时重试次数
#define DNS_WORKING_BUF_SIZE            512         // 工作缓存大小 

#define TCP_MAX_NR					    10		    // TCP控制块的数量
#define TCP_SBUF_SIZE                   10240       // TCP发送缓冲区大小
//#define TCP_RBUF_SIZE                   64240        // 接收缓冲区大小
#define TCP_RBUF_SIZE                   10240        // 接收缓冲区大小
#define TCP_SYN_RETRIES                 5           // 发送SYN报文重试的最大次数
#define TCP_INIT_RTO                    1000        // 发送SYN的初始RTO设置
#define TCP_SYN_RTO                     3000        // 初始连接超时
#define TCP_SENDING_TMO                 1000        // 发送超时
#define TCP_RESENDING_RETRIES           20          // 最多重传的次数
#define TCP_RTO_MAX                     60000       // 最大允许的重传超时
#define TCP_RTO_MIN                     200         // 最小重传超时，RFC6289中的值
#define TCP_DUPTHRESH                   3           // 重复ACK快速重传时的次数
#define TCP_PERSIST_RETRIES             10          // 最多重传的次数
#define TCP_PERSIST_TMO                 1000        // 坚持定时器初始超时
#define TCP_KEEPALIVE_TIME              (2*60*60)   // Keepalive超时时间，RFC1122要求至少2小时，为0时禁用
#define TCP_KEEPALIVE_PROBES            10          // Keepalive超时重试次数
#define TCP_KEEPALIVE_INTVL             75           // Keepalive超时间隔
#define TCP_TMO_MSL                     5000        // TCP最大报文段生存时间，短一点调试用

#endif // NET_CFG_H

