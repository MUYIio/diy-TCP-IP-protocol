/**
 * FAT文件系统结构
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef FAT_H
#define FAT_H

#include "ipc/mutex.h"

#pragma pack(1)    // 千万记得加这个

#define FAT_CLUSTER_INVALID 		0xFFF8      	// 无效的簇号
#define FAT_CLUSTER_FREE          	0x00     	    // 空闲或无效的簇号

#define DIRITEM_NAME_FREE               0xE5                // 目录项空闲名标记
#define DIRITEM_NAME_END                0x00                // 目录项结束名标记

#define DIRITEM_ATTR_READ_ONLY          0x01                // 目录项属性：只读
#define DIRITEM_ATTR_HIDDEN             0x02                // 目录项属性：隐藏
#define DIRITEM_ATTR_SYSTEM             0x04                // 目录项属性：系统类型
#define DIRITEM_ATTR_VOLUME_ID          0x08                // 目录项属性：卷id
#define DIRITEM_ATTR_DIRECTORY          0x10                // 目录项属性：目录
#define DIRITEM_ATTR_ARCHIVE            0x20                // 目录项属性：归档
#define DIRITEM_ATTR_LONG_NAME          0x0F                // 目录项属性：长文件名

#define SFN_LEN                    	 	11              // sfn文件名长

/**
 * FAT目录项
 */
typedef struct _diritem_t {
    uint8_t DIR_Name[11];                   // 文件名
    uint8_t DIR_Attr;                      // 属性
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTeenth;             // 创建时间的毫秒
    uint16_t DIR_CrtTime;         // 创建时间
    uint16_t DIR_CrtDate;         // 创建日期
    uint16_t DIR_LastAccDate;     // 最后访问日期
    uint16_t DIR_FstClusHI;                // 簇号高16位
    uint16_t DIR_WrtTime;         // 修改时间
    uint16_t DIR_WrtDate;         // 修改时期
    uint16_t DIR_FstClusL0;                // 簇号低16位
    uint32_t DIR_FileSize;                 // 文件字节大小
} diritem_t;

/**
 * 完整的DBR类型
 */
typedef struct _dbr_t {
    uint8_t BS_jmpBoot[3];                 // 跳转代码
    uint8_t BS_OEMName[8];                 // OEM名称
    uint16_t BPB_BytsPerSec;               // 每扇区字节数
    uint8_t BPB_SecPerClus;                // 每簇扇区数
    uint16_t BPB_RsvdSecCnt;               // 保留区扇区数
    uint8_t BPB_NumFATs;                   // FAT表项数
    uint16_t BPB_RootEntCnt;               // 根目录项目数
    uint16_t BPB_TotSec16;                 // 总的扇区数
    uint8_t BPB_Media;                     // 媒体类型
    uint16_t BPB_FATSz16;                  // FAT表项大小
    uint16_t BPB_SecPerTrk;                // 每磁道扇区数
    uint16_t BPB_NumHeads;                 // 磁头数
    uint32_t BPB_HiddSec;                  // 隐藏扇区数
    uint32_t BPB_TotSec32;                 // 总的扇区数

	uint8_t BS_DrvNum;                     // 磁盘驱动器参数
	uint8_t BS_Reserved1;				   // 保留字节
	uint8_t BS_BootSig;                    // 扩展引导标记
	uint32_t BS_VolID;                     // 卷标序号
	uint8_t BS_VolLab[11];                 // 磁盘卷标
	uint8_t BS_FileSysType[8];             // 文件类型名称
} dbr_t;
#pragma pack()

/**
 * fat结构
 */
typedef struct _fat_t {
    // fat文件系统本身信息
    uint32_t tbl_start;                     // FAT表起始扇区号
    uint32_t tbl_cnt;                       // FAT表数量
    uint32_t tbl_sectors;                   // 每个FAT表的扇区数
    uint32_t bytes_per_sec;                 // 每扇区大小
    uint32_t sec_per_cluster;               // 每簇的扇区数
    uint32_t root_ent_cnt;                  // 根目录的项数
    uint32_t root_start;                    // 根目录起始扇区号
    uint32_t data_start;                    // 数据区起始扇区号
    uint32_t cluster_byte_size;             // 每簇字节数

    // 与文件系统读写相关信息
    uint8_t * fat_buffer;             		// FAT表项缓冲
    int curr_sector;                        // 当前缓存的扇区数

    struct _fs_t * fs;                      // 所在的文件系统
    mutex_t mutex;                          // 互斥信号量
} fat_t;

typedef uint16_t cluster_t;

#endif // FAT_H
