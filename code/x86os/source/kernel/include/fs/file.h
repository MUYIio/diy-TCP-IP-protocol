/**
 * 文件管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef PFILE_H
#define PFILE_H

#include "comm/types.h"

#define FILE_TABLE_SIZE         2048        // 可打开的文件数量
#define FILE_NAME_SIZE          32          // 文件名称大小

/**
 * 文件类型
 */
typedef enum _file_type_t {
    FILE_UNKNOWN = 0,
    FILE_TTY = 1,
    FILE_NORMAL,
    FILE_DIR,
} file_type_t;

struct _fs_t;

/**
 * 文件描述符
 */
typedef struct _file_t {
    char file_name[FILE_NAME_SIZE];	// 文件名
    file_type_t type;           // 文件类型
    uint32_t size;              // 文件大小
    int ref;                    // 引用计数

    int dev_id;                 // 文件所属的设备号 

    int pos;                   	// 当前位置
    int sblk;                   // 内部起始块位置
    int cblk;                   // 当前块
    int p_index;                // 在父目录中的索引
    int mode;					// 读写模式

    struct _fs_t * fs;          // 所在的文件系统
} file_t;

file_t * file_alloc (void) ;
void file_free (file_t * file);
void file_table_init (void);
void file_inc_ref (file_t * file);

#endif // PFILE_H
