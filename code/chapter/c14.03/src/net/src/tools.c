
/**
 * net工具集
 * 包含字节顺序转换函数、校验和计算等功能的支持
 */
#include "tools.h"
#include "dbg.h"

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
