#ifndef NET_CFG_H
#define NET_CFG_H

#define DBG_MBLOCK      DBG_LEVEL_ERROR
#define DBG_QUEUE       DBG_LEVEL_ERROR
#define DBG_MSG         DBG_LEVEL_ERROR
#define DBG_BUF         DBG_LEVEL_ERROR
#define DBG_INIT        DBG_LEVEL_ERROR
#define DBG_PLAT        DBG_LEVEL_ERROR
#define DBG_NETIF       DBG_LEVEL_ERROR
#define DBG_ETHER       DBG_LEVEL_ERROR
#define DBG_TOOLS       DBG_LEVEL_ERROR
#define DBG_TIMER       DBG_LEVEL_ERROR
#define DBG_ARP         DBG_LEVEL_ERROR
#define DBG_IP          DBG_LEVEL_ERROR
#define DBG_ICMPv4      DBG_LEVEL_ERROR
#define DBG_SOCKET      DBG_LEVEL_INFO

#define NET_ENDIAN_LITTLE           1

#define EXMSG_MSG_CNT       10
#define EXMSG_LOCKER        NLCOKER_THREAD

#define PKTBUF_BLK_SIZE     127
#define PKTBUF_BLK_CNT      100
#define PKTBUF_BUF_CNT      100

#define NETIF_HWADDR_SIZE       10
#define NETIF_NAME_SIZE         10
#define NETIF_INQ_SIZE          50
#define NETIF_OUTQ_SIZE         50

#define NETIF_DEV_CNT           10

#define TIMER_NAME_SIZE         32

#define ARP_CACHE_SIZE          2
#define ARP_MAX_PKT_WAIT        5

#define ARP_TIMER_TMO           1
#define ARP_ENTRY_STABLE_TMO    (20*60)
#define ARP_ENTRY_PENDING_TMO   3
#define ARP_ENTRY_RETRY_CNT     5

#define IP_FRAGS_MAX_NR         5
#define IP_FRAG_MAX_BUF_NR      10
#define IP_FRAG_SCAN_PERIOD     1
#define IP_FRAG_TMO             10

#endif 