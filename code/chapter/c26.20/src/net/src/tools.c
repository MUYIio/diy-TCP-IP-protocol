
/**
 * net工具集
 * 包含字节顺序转换函数、校验和计算等功能的支持
 */
#include "tools.h"
#include "dbg.h"

/**
 * 计算伪校验和
 */
uint16_t checksum_peso(const uint8_t * src_ip, const uint8_t* dest_ip, uint8_t protocol, pktbuf_t * buf) {
    uint8_t zero_protocol[2] = { 0, protocol };
    uint16_t len = x_htons(buf->total_size);

    int offset = 0;
    uint32_t sum = checksum16(offset, (uint16_t*)src_ip, IPV4_ADDR_SIZE, 0, 0);
    offset += IPV4_ADDR_SIZE;
    sum = checksum16(offset, (uint16_t*)dest_ip, IPV4_ADDR_SIZE, sum, 0);
    offset += IPV4_ADDR_SIZE;
    sum = checksum16(offset, (uint16_t*)zero_protocol, 2, sum, 0);
    offset += 2;
    sum = checksum16(offset, (uint16_t*)&len, 2, sum, 0);

    pktbuf_reset_acc(buf);
    sum = pktbuf_checksum16(buf, buf->total_size, sum, 1);
    return sum;
}

/**
 * @brief 检查当前机器的大小端
 */
static int is_little_endian(void) {
    // 存储字节顺序，从低地址->高地址
    // 大端：0x12, 0x34;小端：0x34, 0x12
    uint16_t v = 0x1234;
    uint8_t* b = (uint8_t*)&v;

    // 取最开始第1个字节，为0x34,即为小端，否则为大端
    return b[0] == 0x34;
}

/**
 * @brief 工具集初始化
 */
net_err_t tools_init(void) {
    dbg_info(DBG_TOOLS, "init tools.");

    // 实际是小端，但配置项非小端，检查报错
    if (is_little_endian()  != NET_ENDIAN_LITTLE) {
        dbg_error(DBG_TOOLS, "check endian faild.");
        return NET_ERR_SYS;
    }
    
    dbg_info(DBG_TOOLS, "done.");
    return NET_ERR_OK;
}

/**
 * 计算16位校验和
 * @param buf 数组缓存区起始
 * @param len 数据长
 * @param 之前对其它缓存进行累加的结果计数，用于方便计算伪首部校验和
 */
uint16_t checksum16(uint32_t offset, void* buf, uint16_t len, uint32_t pre_sum, int complement) {
    uint16_t* curr_buf = (uint16_t *)buf;
    uint32_t checksum = pre_sum;

    // 起始字节不对齐, 加到高8位
    if (offset & 0x1) {
        uint8_t * buf = (uint8_t *)curr_buf;
        checksum += *buf++ << 8;
        curr_buf = (uint16_t *)buf;
        len--;
    }

    while (len > 1) {
        checksum += *curr_buf++;
        len -= 2;
    }

    if (len > 0) {
        checksum += *(uint8_t*)curr_buf;
    }

    // 注意，这里要不断累加。不然结果在某些情况下计算不正确
    uint16_t high;
    while ((high = checksum >> 16) != 0) {
        checksum = high + (checksum & 0xffff);
    }

    return complement ? (uint16_t)~checksum : (uint16_t)checksum;
}
