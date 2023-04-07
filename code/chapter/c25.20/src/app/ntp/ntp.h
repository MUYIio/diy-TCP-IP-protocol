/**
 * @file ntp.h
 * @author lishutong(527676163@qq.com)
 * @brief NTP时间客户端
 * @version 0.1
 * @date 2022-11-04
 * 
 * @copyright Copyright (c) 2022
 * 
 * 测试服务器列表请见： http://www.ntp.org.cn/pool
 */
#include <time.h>
#include "netapi.h"

#define NTP_TIME_1900_1970          2208988800ull       // 1900到1970之间的秒数
#define NTP_SERVER_PORT            123      // NTP服务器端口
#define NTP_VERSION                 3       // NTP版本3
#define NTP_MODE                    3       // 客户机模式

#define NTP_REQ_TMO                 2       // 查询等待服务器的超时
#define NTP_REQ_RETRY               3       // 查询等待重试次数

/**
 * @brief 时间戳
 * Seconds and Fractions since 01.01.1900
 */
typedef struct _ntp_ts_t {
   uint32_t seconds;        // 秒的整数部分
   uint32_t fraction;       // 小数部分
} ntp_ts_t;

/**
 * @brief NTP数据包
 */
#pragma pack(1)
typedef struct _ntp_pkt_t {
    // LI: NTP时间标尺中将要插入或删除的下一跳情况
    // VN: NTP的版本号，目前值为3
    // Mode: 工作模式, 当值为3时，表示Client，客户端模式。
    uint8_t LI_VN_Mode; 
    uint8_t stratum;            // 时钟的层数
    uint8_t poll;               // 轮询时间，即发送报文的最大间隔时间。
    uint8_t precision;          // 时钟的精度。
    uint32_t root_delay;        // 到主参考时钟的总往返延迟时间
    uint32_t root_dispersion;   // 本地时钟相对于主参考时钟的最大误差。
    uint32_t ref_id;            // 标识特定参考时钟
    ntp_ts_t ref_ts;            // 本地时钟最后一次被设定或更新的时间。
    ntp_ts_t orig_ts;           // NTP报文离开源端时的本地时间
    ntp_ts_t recv_ts;           // NTP报文到达目的端的本地时间。
    ntp_ts_t trans_ts;           // 目的端应答报文离开服务器端的本地时间。
    // Authenticator 96字节可选验证信息。这里不用
}ntp_pkt_t;
#pragma pack()

struct tm * ntp_time (void);
