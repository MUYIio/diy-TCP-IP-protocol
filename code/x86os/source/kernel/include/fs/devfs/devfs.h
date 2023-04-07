/**
 * 设备文件系统描述
 *
 * 创建时间：2022年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef DEVFS_H
#define DEVFS_H

#include "fs/fs.h"

/**
 * @brief 设备类型描述结构
 */
typedef struct _devfs_type_t {
    const char * name;
    int dev_type;
    int file_type;
}devfs_type_t;

#endif
