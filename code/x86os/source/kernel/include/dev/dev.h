/**
 * 设备接口
 *
 * 创建时间：2022年7月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 * 参考资料：https://wiki.osdev.org/Printing_To_Screen
 */
#ifndef DEV_H
#define DEV_H

#include "comm/types.h"

#define DEV_NAME_SIZE               32      // 设备名称长度

enum {
    DEV_UNKNOWN = 0,            // 未知类型
    DEV_TTY,                // TTY设备
    DEV_DISK,               // 磁盘设备
};

struct _dev_desc_t;

/**
 * @brief 设备驱动接口
 */
typedef struct _device_t {
    struct _dev_desc_t * desc;      // 设备特性描述符
    int mode;                       // 操作模式
    int minor;                      // 次设备号
    void * data;                    // 设备参数
    int open_count;                 // 打开次数
}device_t;


/**
 * @brief 设备描述结构
 */
typedef struct _dev_desc_t {
    char name[DEV_NAME_SIZE];           // 设备名称
    int major;                          // 主设备号

    int (*open) (device_t * dev) ;
    int (*read) (device_t * dev, int addr, char * buf, int size);
    int (*write) (device_t * dev, int addr, char * buf, int size);
    int (*control) (device_t * dev, int cmd, int arg0, int arg1);
    void (*close) (device_t * dev);
}dev_desc_t;

int dev_open (int major, int minor, void * data);
int dev_read (int dev_id, int addr, char * buf, int size);
int dev_write (int dev_id, int addr, char * buf, int size);
int dev_control (int dev_id, int cmd, int arg0, int arg1);
void dev_close (int dev_id);

#endif // DEV_H
