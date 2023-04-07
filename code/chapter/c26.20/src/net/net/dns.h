/**
 * @file dns.h
 * @author lishutong(527676163@qq.com)
 * @brief DNS客户端协议实现
 * @version 0.1
 * @date 2022-11-30
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef DNS_H
#define DNS_H

#include "nlist.h"
#include "net_cfg.h"
#include "udp.h"

#define DNS_UPDATE_PERIOID      1           // 每隔多少秒刷新一次DNS表   

#define DNS_OPCODE_STDQUERY     0           // 标准查询

// DNS错误类型
#define DNS_ERR_NONE            0           // 没有错误
#define DNS_ERR_FORMAT  	    1           // 格式错误，查询不能被解读
#define DNS_ERR_SERV_FAIL	    2           // 服务器失效，服务器的处理错误
#define DNS_ERR_NXMOMAIN		3           // 不存在的域名，引用了未知域名
#define DNS_ERR_NOTIMP  	    4           // 没有实现，请示在服务器端不被支持
#define DNS_ERR_REFUSED		    5           // 拒绝：服务器不希望提供回答

// DNS包头标志位
#pragma pack(1)

/**
 * @brief DNS包头
 */
typedef struct _dns_hdr_t {
    uint16_t id;                // 事务ID
    union {
        uint16_t all;

#if NET_ENDIAN_LITTLE
        struct {
            uint16_t rcode : 4;         // 响应码
            uint16_t cd : 1;            // 禁用安全检查（1）
            uint16_t ad : 1;            // 信息已授权（1）
            uint16_t z : 1;             // 保底为0
            uint16_t ra : 1;            // 服务器是否支持递归查询(1)
            uint16_t rd : 1;            // 告诉服务器执行递归查询(1)，0 容许迭代查询
            uint16_t tc : 1;            // 可截断(1)，即UDP长超512字节时，可只返回512字节
            uint16_t aa : 1;            // 授权回答
            uint16_t opcode : 4;        // 操作码(缺省为0)
            uint16_t qr : 1;            
        };
#else
        struct {
            uint16_t qr : 1;            
            uint16_t opcode : 4;        // 操作码(缺省为0)
            uint16_t aa : 1;            // 授权回答
            uint16_t tc : 1;            // 可截断(1)，即UDP长超512字节时，可只返回512字节
            uint16_t rd : 1;            // 告诉服务器执行递归查询(1)，0 容许迭代查询
            uint16_t ra : 1;            // 服务器是否支持递归查询(1)
            uint16_t z : 1;             // 保底为0
            uint16_t ad : 1;            // 信息已授权（1）
            uint16_t cd : 1;            // 禁用安全检查（1）
            uint16_t rcode : 5;         // 响应码
        };
#endif
    }flags;
    uint16_t qdcount;           // 查询数/区域数
    uint16_t ancount;           // 回答/先决条件数
    uint16_t nscount;           // 授权纪录数/更新数 
    uint16_t arcount;           // 额外信息数
}dns_hdr_t;

#define DNS_QUERY_CLASS_INET            1     // 查询类：1 - 表示互联网类

// DNS资源纪录类型
#define DNS_QUERY_TYPE_A                1       // IPv4地址纪录
#define DNS_QUERY_TYPE_NS               2       // 名称服务器：提供区域授权名称服务器的名称
#define DNS_QUERY_TYPE_CNAME            3       // 规范名称，域名别名
#define DNS_QUERY_TYPE_MX               15      // 邮件交换

/**
 * @brief 问题（查询）区域区段格式
 * 不含名称，因为名称是可变长的且位于开头
 */
typedef struct _dns_qfield_t {
    uint16_t type;              // 查询类型
    uint16_t class;             // 查询类
}dns_qfield_t;

/**
 * @brief 回答、授权和额外信息区段格式，不含名称
 * 不含名称，因为名称是可变长的且位于开头
 */
typedef struct _dns_afield_t {
    uint16_t type;              // 同上查询类型
    uint16_t class;             // 同上查询类
    uint32_t ttl;               // 结果可以缓存的最大秒数
    uint16_t rd_len;            // rdata中包含的字节数
    uint16_t rdata[1];          // 资源数据
}dns_afield_t;
#pragma pack() 

/**
 * @brief DNS表
 * 该表存储DNS域名和IP映射之间的相关信息
 */
typedef struct _dns_entry_t {
    int ttl;                // 该表项的生存时间
    int age;                // 表项存在的时间
    ipaddr_t ipaddr;        // 对应的IP地址
    char domain_name[DNS_DOMAIN_NAME_MAX];          //  域名最大长度
}dns_entry_t;

/**
 * @brief DNS服务器
 */
typedef struct _dns_server_t {
    ipaddr_t ipaddr;            // 服务器的IP地址
    enum {
        DNS_SERVER_FREE = 0,    // 空闲态
        DNS_SERVER_OK,          // 可以正常查询和响应
        DNS_SERVER_NO_RESP,     // 没有响应等
    }state;
}dns_server_t;

/**
 * @brief DNS查询消息
 */
typedef struct _dns_req_t {
    char domain_name[DNS_DOMAIN_NAME_MAX];           // 域名
    net_err_t err;
    int query_id;                       // 查询发包使用的id
    int server;                         // 当前查询所用的服务器
    ipaddr_t  ipaddr;                   // 查询的IP地址
    sys_sem_t wait_sem;               // 等待的信号量
    net_err_t (*done) (void * arg);     // 查询完成时的回调处理

    uint8_t retry_tmo;                  // 重试查询的秒数
    uint8_t retry_cnt;                  // 重试查询的次数
    nlist_node_t node;                   // 查询链接结点
}dns_req_t;

dns_server_t * dns_insert_server (const char * ipstr);
dns_server_t * dns_default_server (void);
net_err_t dns_set_default_server (const char * ipstr);

void dns_init (void);
void dns_in (void);
int dns_is_arrive (udp_t * udp);
dns_req_t * dns_alloc_req (void);
void dns_free_req (dns_req_t * req);

net_err_t dns_req_in (func_msg_t * msg);

#endif // DNS_H
