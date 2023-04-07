
/**
 * PING的实现
 */
#ifndef PING_H
#define PING_H

#include <stdint.h>
#include <time.h>
#include "netapi.h"

// 缺省值
#define PING_BUFFER_SIZE			4096		// 附带的额外数据
#define PING_DEFAULT_ID				0x200		// ping的起始ID
#define PING_INTERVAL_MS			1000		// 每次间隔时间，毫秒为单位
#define PING_DEFAULT_TMO			1000		// 每次ping的时间间隔

#pragma pack(1)

/**
 * 自定义的IP包头
 * 没有使用协议栈自带的，目的是让ping的代码与所用的协议栈无关
 */
typedef struct _ip_hdr_t {
	uint8_t shdr : 4;           // 首部长，低4字节
	uint8_t version : 4;        // 版本号
	uint8_t tos;		        // 服务类型
	uint16_t total_len;		    // 总长度、
	uint16_t id;		        // 标识符，用于区分不同的数据报,可用于ip数据报分片与重组
	uint16_t frag;				// 分片偏移与标志位
	uint8_t ttl;                // 存活时间，每台路由器转发时减1，减到0时，该包被丢弃
	uint8_t protocol;	        // 上层协议
	uint16_t hdr_checksum;      // 首部校验和
	uint8_t	src_ip[4];			// 源IP
	uint8_t dest_ip[4];			// 目标IP
}ip_hdr_t;

/**
 * 回显请求与应答的包头
 */
typedef struct _icmp_hdr_t {
	uint8_t type;           // 类型
	uint8_t code;			// 代码
	uint16_t checksum;	    // ICMP报文的校验和
	uint16_t id;            // 标识符
	uint16_t seq;           // 序号
}icmp_hdr_t;

/**
 * 请求包，不含IP包头
 */
typedef struct _echo_req_t {
	icmp_hdr_t echo_hdr;
	clock_t time;
	char buf[PING_BUFFER_SIZE];
}echo_req_t;

/**
 * 响应包，包含IP包头
 */
typedef struct _echo_reply_t {
	ip_hdr_t iphdr;
	icmp_hdr_t echo_hdr;
	clock_t time;
	char buf[PING_BUFFER_SIZE];
}echo_reply_t;
#pragma pack()

/**
 * ping结构
 */
typedef struct _ping_t {
	echo_req_t req;				// 请求包，不含IP包头
	echo_reply_t reply;			// 响应包，含IP包头
}ping_t;

void ping_run(ping_t * ping, const char * dest, int count, int size, int interval);

#endif //NET_PING_H
