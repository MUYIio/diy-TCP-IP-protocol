/**
 * 系统启动信息
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

#define BOOT_RAM_REGION_MAX			10		// RAM区最大数量

/**
 * 启动信息参数
 */
typedef struct _boot_info_t {
    // RAM区信息
    struct {
        uint32_t start;
        uint32_t size;
    }ram_region_cfg[BOOT_RAM_REGION_MAX];
    int ram_region_count;
}boot_info_t;

#define SECTOR_SIZE		512			// 磁盘扇区大小

// 引入网络后，其占用的内存比较大，整体将超过1MB，所以将其放到更高的地址
#define SYS_KERNEL_LOAD_ADDR		(8*1024*1024)		// 内核加载的起始地址

#endif // BOOT_INFO_H
