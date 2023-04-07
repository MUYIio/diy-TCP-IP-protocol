/**
 * @file ipv4.h
 * @author lishutong (527676163@qq.com)
 * @brief IPv4协议支持
 * @version 0.1
 * @date 2022-10-30
 *
 * @copyright Copyright (c) 2022
 * 包含IP包输入输出处理，数据报分片与重组、路由表
 */
#ifndef IPV4_H
#define IPV4_H

#include <stdint.h>
#include "net_err.h"
#include "netif.h"
#include "pktbuf.h"

#define IPV4_ADDR_SIZE          4           // IPV4地址长度
#define NET_VERSION_IPV4        4           // IPV4版本号
#define NET_IP_DEF_TTL          64          // 缺省的IP包TTL值, RFC1122建议设为64

#pragma pack(1)

/**
 * @brief IP包头部
 * 长度可变，最少20字节，如下结构。当包含选项时，长度变大，最多60字节
 * 不过，包含选项的包比较少见，因此一般为20字节
 * 总长最大65536，实际整个pktbuf的长度可能比total_len大，因为以太网填充的缘故
 * 因此需要total_len来判断实际有效的IP数据包长
 */
typedef struct _ipv4_hdr_t {
    union {
        struct {
#if NET_ENDIAN_LITTLE
            uint16_t shdr : 4;           // 首部长，低4字节
            uint16_t version : 4;        // 版本号
            uint16_t ds : 6;
            uint16_t ecn : 2;
#else
            uint16_t ecn : 2;
            uint16_t ds : 6;
            uint16_t version : 4;        // 版本号
            uint16_t shdr : 4;           // 首部长，低4字节
#endif
        };
        uint16_t shdr_all;
    };

    uint16_t total_len;		    // 总长度、
    uint16_t id;		        // 标识符，用于区分不同的数据报,可用于ip数据报分片与重组

    union {
        struct {
#if NET_ENDIAN_LITTLE
            uint16_t offset : 13;               // 数据报分片偏移, 以8字节为单位，从0开始算
            uint16_t more : 1;                  // 不是最后一个包，还有后续
            uint16_t disable : 1;               // 1-不允许分片，0-可以分片
            uint16_t resvered : 1;              // 保留，必须为0
#else
            uint16_t resvered : 1;              // 保留，必须为0
            uint16_t offset : 13;               // 数据报分片偏移, 以8字节为单位，从0开始算
            uint16_t more : 1;                  // 不是最后一个包，还有后续
            uint16_t disable : 1;               // 1-不允许分片，0-可以分片
#endif
        };
        uint16_t frag_all;
    };

    uint8_t ttl;                // 存活时间，每台路由器转发时减1，减到0时，该包被丢弃
    uint8_t protocol;	        // 上层协议
    uint16_t hdr_checksum;      // 首部校验和
    uint8_t	src_ip[IPV4_ADDR_SIZE];        // 源IP
    uint8_t dest_ip[IPV4_ADDR_SIZE];	   // 目标IP
}ipv4_hdr_t;

/**
 * @brief IP数据包
 */
typedef struct _ipv4_pkt_t {
    ipv4_hdr_t hdr;              // 数据包头
    uint8_t data[1];            // 数据区
}ipv4_pkt_t;

#pragma pack()

/**
 * @brief IP分片控制结构
 * 由于id在每次发送IP数据包时都会自增，因此可用id和ip进行区分
 */
typedef struct _ip_frag_t {
    ipaddr_t ip;                // 分片收的源IP
    uint16_t id;                // 数据包的id
    int tmo;                    // 分片超时
    nlist_t buf_list;            // 等待重组的缓存链表
    nlist_node_t node;           // 下一分片控制结构
}ip_frag_t;

/**
 * 路由信息
 */
typedef struct _rentry_t {
    ipaddr_t net;            // 网络地址
    ipaddr_t mask;           // 子网掩码
    int mask_1_cnt;         // 掩码中1的位数
    ipaddr_t next_hop;       // 下一跳IP地址
    netif_t* netif;         // 对应的网络接口

    enum {
        NET_RT_LOCAL_NET,       // 同一网络
        NET_RT_NETIF,           // 指向接口
        NET_RT_OTHER,           // 其它主机
    }type;                      // 类型
    
    nlist_node_t node;           // 链接结点
}rentry_t;

void rt_init(void);
void rt_add(ipaddr_t* net, ipaddr_t* mask, ipaddr_t* next_hop, netif_t* netif);
void rt_remove(ipaddr_t* net, ipaddr_t* mask);
rentry_t* rt_find(ipaddr_t * ip);

net_err_t ipv4_init(void);
net_err_t ipv4_in(netif_t *netif, pktbuf_t *buf);
net_err_t ipv4_out(uint8_t protocol, ipaddr_t* dest, ipaddr_t* src, pktbuf_t* buf);

/**
 * @brief 获取IP包头部长
 */
static inline int ipv4_hdr_size(ipv4_pkt_t* pkt) {
    return pkt->hdr.shdr * 4;   // 4字节为单位
}

#endif // IPV4_H
