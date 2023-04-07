/**
 * 简单的FAT文件系统结构
 * 创建时间：2022年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "fs/fs.h"
#include "fs/fatfs/fatfs.h"
#include "dev/dev.h"
#include "core/memory.h"
#include "tools/log.h"
#include "tools/klib.h"
#include <sys/fcntl.h>

/**
 * @brief 缓存读取磁盘数据，用于目录的遍历等
 */
static int bread_sector (fat_t * fat, int sector) {
    if (sector == fat->curr_sector) {
        return 0;
    }

    int cnt = dev_read(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    if (cnt == 1) {
        fat->curr_sector = sector;
        return 0;
    }
    return -1;
}

/**
 * @brief 写缓存
 */
static int bwrite_secotr (fat_t * fat, int sector) {
    int cnt = dev_write(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    return (cnt == 1) ? 0 : -1;
}

/**
 * 检查指定簇是否可用，非占用或坏簇
 */
int cluster_is_valid (cluster_t cluster) {
    return (cluster < 0xFFF8) && (cluster >= 0x2);     // 值是否正确
}

/**
 * 获取指定簇的下一个簇
 */
int cluster_get_next (fat_t * fat, cluster_t curr) {
    if (!cluster_is_valid(curr)) {
        return FAT_CLUSTER_INVALID;
    }

    // 取fat表中的扇区号和在扇区中的偏移
    int offset = curr * sizeof(cluster_t);
    int sector = offset / fat->bytes_per_sec;
    int off_sector = offset % fat->bytes_per_sec;
    if (sector >= fat->tbl_sectors) {
        log_printf("cluster too big. %d", curr);
        return FAT_CLUSTER_INVALID;
    }

    // 读扇区，然后取其中簇数据
    int err = bread_sector(fat, fat->tbl_start + sector);
    if (err < 0) {
        return FAT_CLUSTER_INVALID;
    }

    return *(cluster_t*)(fat->fat_buffer + off_sector);
}

/**
 * @brief 设置簇的下一簇
 */
int cluster_set_next (fat_t * fat, cluster_t curr, cluster_t next) {
    if (!cluster_is_valid(curr)) {
        return -1;
    }

    int offset = curr * sizeof(cluster_t);
    int sector = offset / fat->bytes_per_sec;
    int off_sector = offset % fat->bytes_per_sec;
    if (sector >= fat->tbl_sectors) {
        log_printf("cluster too big. %d", curr);
        return -1;
    }

    // 读缓存
    int err = bread_sector(fat, fat->tbl_start + sector);
    if (err < 0) {
        return -1;
    }

    // 改next
    *(cluster_t*)(fat->fat_buffer + off_sector) = next;

    // 回写到多个表中
    for (int i = 0; i < fat->tbl_cnt; i++) {
        err = bwrite_secotr(fat, fat->tbl_start + sector);
        if (err < 0) {
            log_printf("write cluster failed.");
            return -1;
        }
        sector += fat->tbl_sectors;
    }
    return 0;
}

/**
 * @brief 释放cluster链
 */
void cluster_free_chain(fat_t * fat, cluster_t start) {
    while (cluster_is_valid(start)) {
        cluster_t next = cluster_get_next(fat, start);
        cluster_set_next(fat, start, FAT_CLUSTER_FREE);
        start = next;
    }
}

/**
 * @brief 找一个空闲的cluster
 */
cluster_t cluster_alloc_free (fat_t * fat, int cnt) {
    cluster_t pre, curr, start;
    int c_total = fat->tbl_sectors * fat->bytes_per_sec / sizeof(cluster_t);

    pre = start = FAT_CLUSTER_INVALID;
    for (curr = 2; (curr< c_total) && cnt; curr++) {
        cluster_t free = cluster_get_next(fat, curr);
        if (free == FAT_CLUSTER_FREE) {
            // 记录首个簇
            if (!cluster_is_valid(start)) {
                start = curr;
            } 
        
            // 前一簇如果有效，则设置。否则忽略掉
            if (cluster_is_valid(pre)) {
                // 找到空表项，设置前一表项的链接
                int err = cluster_set_next(fat, pre, curr);
                if (err < 0) {
                    cluster_free_chain(fat, start);
                    return FAT_CLUSTER_INVALID;
                }
            }

            pre = curr;
            cnt--;
        }
    }

    // 最后的结点
    if (cnt == 0) {
        int err = cluster_set_next(fat, pre, FAT_CLUSTER_INVALID);
        if (err == 0) {
            return start;
        }
    }

    // 失败，空间不够等问题
    cluster_free_chain(fat, start);
    return FAT_CLUSTER_INVALID;
}

/**
 * @brief 转换文件名为diritem中的短文件名，如a.txt 转换成a      txt
 */
static void to_sfn(char* dest, const char* src) {
    kernel_memset(dest, ' ', SFN_LEN);

    // 不断生成直到遇到分隔符和写完缓存
    char * curr = dest;
    char * end = dest + SFN_LEN;
    while (*src && (curr < end)) {
        char c = *src++;

        switch (c) {
        case '.':       // 隔附，跳到扩展名区，不写字符
            curr = dest + 8;
            break;
        default:
            if ((c >= 'a') && (c <= 'z')) {
                c = c - 'a' + 'A';
            }
            *curr++ = c;
            break;
        }
    }
}

/**
 * @brief 判断item项是否与指定的名称相匹配
 */
int diritem_name_match (diritem_t * item, const char * path) {
    char buf[SFN_LEN];
    to_sfn(buf, path);
    return kernel_memcmp(buf, item->DIR_Name, SFN_LEN) == 0;
}

/**
 * 缺省初始化driitem
 */
int diritem_init(diritem_t * item, uint8_t attr,const char * name) {
    to_sfn((char *)item->DIR_Name, name);
    item->DIR_FstClusHI = (uint16_t )(FAT_CLUSTER_INVALID >> 16);
    item->DIR_FstClusL0 = (uint16_t )(FAT_CLUSTER_INVALID & 0xFFFF);
    item->DIR_FileSize = 0;
    item->DIR_Attr = attr;
    item->DIR_NTRes = 0;

    // 时间写固定值，简单方便
    item->DIR_CrtTime = 0;
    item->DIR_CrtDate = 0;
    item->DIR_WrtTime = item->DIR_CrtTime;
    item->DIR_WrtDate = item->DIR_CrtDate;
    item->DIR_LastAccDate = item->DIR_CrtDate;
    return 0;
}

/**
 * @brief 获取diritem中的名称，转换成合适
 */
void diritem_get_name (diritem_t * item, char * dest) {
    char * c = dest;
    char * ext = (char *)0;

    kernel_memset(dest, 0, SFN_LEN + 1);     // 最多11个字符
    for (int i = 0; i < 11; i++) {
        if (item->DIR_Name[i] != ' ') {
            *c++ = item->DIR_Name[i];
        }

        if (i == 7) {
            ext = c;
            *c++ = '.';
        }
    }

    // 没有扩展名的情况
    if (ext && (ext[1] == '\0')) {
        ext[0] = '\0';
    }
}

/**
 * @brief 获取文件类型
 */
file_type_t diritem_get_type (diritem_t * item) {
    file_type_t type = FILE_UNKNOWN;

    // 长文件名和volum id
    if (item->DIR_Attr & (DIRITEM_ATTR_VOLUME_ID | DIRITEM_ATTR_HIDDEN | DIRITEM_ATTR_SYSTEM)) {
        return FILE_UNKNOWN;
    }

    return item->DIR_Attr & DIRITEM_ATTR_DIRECTORY ? FILE_DIR : FILE_NORMAL;
}

/**
 * @brief 在root目录中读取diritem
 */
static diritem_t * read_dir_entry (fat_t * fat, int index) {
    if ((index < 0) || (index >= fat->root_ent_cnt)) {
        return (diritem_t *)0;
    }

    int offset = index * sizeof(diritem_t);
    int err = bread_sector(fat, fat->root_start + offset / fat->bytes_per_sec);
    if (err < 0) {
        return (diritem_t *)0;
    }
    return (diritem_t *)(fat->fat_buffer + offset % fat->bytes_per_sec);
}

/**
 * @brief 写dir目录项
 */
static int write_dir_entry (fat_t * fat, diritem_t * item, int index) {
    if ((index < 0) || (index >= fat->root_ent_cnt)) {
        return -1;
    }

    int offset = index * sizeof(diritem_t);
    int sector = fat->root_start + offset / fat->bytes_per_sec;
    int err = bread_sector(fat, sector);
    if (err < 0) {
        return -1;
    }
    kernel_memcpy(fat->fat_buffer + offset % fat->bytes_per_sec, item, sizeof(diritem_t));
    return bwrite_secotr(fat, sector);
}


/**
 * @brief 更改文件大小, 此时当前族号已经到达了最后的簇号
 */
static int expand_file(file_t * file, int inc_bytes) {
    fat_t * fat = (fat_t *)file->fs->data;
    
    int cluster_cnt;
    if ((file->size == 0) || (file->size % fat->cluster_byte_size == 0)) {
        // 文件为空，或者刚好达到的簇的末尾
        cluster_cnt = up2(inc_bytes, fat->cluster_byte_size) / fat->cluster_byte_size; 
    } else {
        // 文件非空，当前簇的空闲量，如果空间够增长，则直接退出了
        // 例如：大小为2048，再扩充1024,簇大小为1024
        int cfree = fat->cluster_byte_size - (file->size % fat->cluster_byte_size);
        if (cfree >= inc_bytes) {
            file->size += inc_bytes;
            return 0;
        }

        // 不够，则分配新簇用来放额外的空间
        cluster_cnt = up2(inc_bytes - cfree, fat->cluster_byte_size) / fat->cluster_byte_size; 
    }

    cluster_t start = cluster_alloc_free(fat, cluster_cnt);
    if (!cluster_is_valid(start)) {
        log_printf("no cluster for file write");
        return -1;
    }

    // 在文件关闭时，回写
    if (!cluster_is_valid(file->sblk)) {
        file->cblk = file->sblk = start;
    } else {
        // 建立链接关系
        int err = cluster_set_next(fat, file->cblk, start);
        if (err < 0) {
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 移动文件指针
 */
static int move_file_pos(file_t* file, fat_t * fat, uint32_t move_bytes, int expand) {
	uint32_t c_offset = file->pos % fat->cluster_byte_size;

    // 跨簇，则调整curr_cluster。注意，如果已经是最后一个簇了，则curr_cluster不会调整
	if (c_offset + move_bytes >= fat->cluster_byte_size) {
        cluster_t next = cluster_get_next(fat, file->cblk);
		if ((next == FAT_CLUSTER_INVALID) && expand) {
            int err = expand_file(file, fat->cluster_byte_size);
            if (err < 0) {
                return -1;
            }

            next = cluster_get_next(fat, file->cblk);
        }

        file->cblk = next;
	}

	file->pos += move_bytes;
	return 0;
}

/**
 * @brief 挂载fat文件系统
 */
int fatfs_mount (struct _fs_t * fs, int dev_major, int dev_minor) {
    // 打开设备
    int dev_id = dev_open(dev_major, dev_minor, (void *)0);
    if (dev_id < 0) {
        log_printf("open disk failed. major: %x, minor: %x", dev_major, dev_minor);
        return -1;
    }

    // 读取dbr扇区并进行检查
    dbr_t * dbr = (dbr_t *)memory_alloc_page();
    if (!dbr) {
        log_printf("mount fat failed: can't alloc buf.");
        goto mount_failed;
    }

    // 这里需要使用查询的方式来读取，因为此时多进程还没有跑起来，只在初始化阶段？
    int cnt = dev_read(dev_id, 0, (char *)dbr, 1);
    if (cnt < 1) {
        log_printf("read dbr failed.");
        goto mount_failed;
    }

    // 解析DBR参数，解析出有用的参数
    fat_t * fat = &fs->fat_data;
    fat->fat_buffer = (uint8_t *)dbr;
    fat->bytes_per_sec = dbr->BPB_BytsPerSec;
    fat->tbl_start = dbr->BPB_RsvdSecCnt;
    fat->tbl_sectors = dbr->BPB_FATSz16;
    fat->tbl_cnt = dbr->BPB_NumFATs;
    fat->root_ent_cnt = dbr->BPB_RootEntCnt;
    fat->sec_per_cluster = dbr->BPB_SecPerClus;
    fat->cluster_byte_size = fat->sec_per_cluster * dbr->BPB_BytsPerSec;
	fat->root_start = fat->tbl_start + fat->tbl_sectors * fat->tbl_cnt;
    fat->data_start = fat->root_start + fat->root_ent_cnt * 32 / SECTOR_SIZE;
    fat->curr_sector = -1;
    fat->fs = fs;
    mutex_init(&fat->mutex);
    fs->mutex = &fat->mutex;

	// 简单检查是否是fat16文件系统, 可以在下边做进一步的更多检查。此处只检查做一点点检查
	if (fat->tbl_cnt != 2) {
        log_printf("fat table num error, major: %x, minor: %x", dev_major, dev_minor);
		goto mount_failed;
	}

    if (kernel_memcmp(dbr->BS_FileSysType, "FAT16", 5) != 0) {
        log_printf("not a fat16 file system, major: %x, minor: %x", dev_major, dev_minor);
        goto mount_failed;
    }

    // 记录相关的打开信息
    fs->type = FS_FAT16;
    fs->data = &fs->fat_data;
    fs->dev_id = dev_id;
    return 0;

mount_failed:
    if (dbr) {
        memory_free_page((uint32_t)dbr);
    }
    dev_close(dev_id);
    return -1;
}

/**
 * @brief 卸载fatfs文件系统
 */
void fatfs_unmount (struct _fs_t * fs) {
    fat_t * fat = (fat_t *)fs->data;

    dev_close(fs->dev_id);
    memory_free_page((uint32_t)fat->fat_buffer);
}

/**
 * @brief 从diritem中读取相应的文件信息
 */
static void read_from_diritem (fat_t * fat, file_t * file, diritem_t * item, int index) {
    file->type = diritem_get_type(item);
    file->size = (int)item->DIR_FileSize;
    file->pos = 0;
    file->sblk = (item->DIR_FstClusHI << 16) | item->DIR_FstClusL0;
    file->cblk = file->sblk;
    file->p_index = index;
}

/**
 * @brief 打开指定的文件
 */
int fatfs_open (struct _fs_t * fs, const char * path, file_t * file) {
    fat_t * fat = (fat_t *)fs->data;
    diritem_t * file_item = (diritem_t *)0;
    int p_index = -1;

    // 遍历根目录的数据区，找到已经存在的匹配项
    for (int i = 0; i < fat->root_ent_cnt; i++) {
        diritem_t * item = read_dir_entry(fat, i);
        if (item == (diritem_t *)0) {
            return -1;
        }

         // 结束项，不需要再扫描了，同时index也不能往前走
        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            p_index = i;
            break;
        }

        // 只显示普通文件和目录，其它的不显示
        if (item->DIR_Name[0] == DIRITEM_NAME_FREE) {
            p_index = i;
            continue;
        }

        // 找到要打开的目录
        if (diritem_name_match(item, path)) {
            file_item = item;
            p_index = i;
            break;
        }
    }

    if (file_item) {
        read_from_diritem(fat, file, file_item, p_index);

        // 如果要截断，则清空
        if (file->mode & O_TRUNC) {
            cluster_free_chain(fat, file->sblk);
            file->cblk = file->sblk = FAT_CLUSTER_INVALID;
            file->size = 0;
        }
        return 0;
    } else if ((file->mode & O_CREAT) && (p_index >= 0)) {
        // 创建一个空闲的diritem项
        diritem_t item;
        diritem_init(&item, 0, path);
        int err = write_dir_entry(fat, &item, p_index);
        if (err < 0) {
            log_printf("create file failed.");
            return -1;
        }

        read_from_diritem(fat, file, &item, p_index);
        return 0;
    }

    return -1;
}

/**
 * @brief 读了文件
 */
int fatfs_read (char * buf, int size, file_t * file) {
    fat_t * fat = (fat_t *)file->fs->data;

    // 调整读取量，不要超过文件总量
    uint32_t nbytes = size;
    if (file->pos + nbytes > file->size) {
        nbytes = file->size - file->pos;
    }

    uint32_t total_read = 0;
    while (nbytes > 0) {
        uint32_t curr_read = nbytes;
		uint32_t cluster_offset = file->pos % fat->cluster_byte_size;
        uint32_t start_sector = fat->data_start + (file->cblk - 2)* fat->sec_per_cluster;  // 从2开始

        // 如果是整簇, 只读一簇
        if ((cluster_offset == 0) && (nbytes == fat->cluster_byte_size)) {
            int err = dev_read(fat->fs->dev_id, start_sector, buf, fat->sec_per_cluster);
            if (err < 0) {
                return total_read;
            }

            curr_read = fat->cluster_byte_size;
        } else {
            // 如果跨簇，只读第一个簇内的一部分
            if (cluster_offset + curr_read > fat->cluster_byte_size) {
                curr_read = fat->cluster_byte_size - cluster_offset;
            }

            // 读取整个簇，然后从中拷贝
            fat->curr_sector = -1;
            int err = dev_read(fat->fs->dev_id, start_sector, fat->fat_buffer, fat->sec_per_cluster);
            if (err < 0) {
                return total_read;
            }
            kernel_memcpy(buf, fat->fat_buffer + cluster_offset, curr_read);
        }

        buf += curr_read;
        nbytes -= curr_read;
        total_read += curr_read;

        // 前移文件指针
		int err = move_file_pos(file, fat, curr_read, 0);
		if (err < 0) {
            return total_read;
        }
	}

    return total_read;
}

/**
 * @brief 写文件数据
 */
int fatfs_write (char * buf, int size, file_t * file) {
    fat_t * fat = (fat_t *)file->fs->data;

    // 如果文件大小不够，则先扩展文件大小
    if (file->pos + size > file->size) {
        int inc_size = file->pos + size - file->size;
        int err = expand_file(file, inc_size);
        if (err < 0) {
            return 0;
        }
    }

    uint32_t nbytes = size;
    uint32_t total_write = 0;
	while (nbytes) {
        // 每次写的数据量取决于当前簇中剩余的空间，以及size的量综合
        uint32_t curr_write = nbytes;
		uint32_t cluster_offset = file->pos % fat->cluster_byte_size;
        uint32_t start_sector = fat->data_start + (file->cblk - 2)* fat->sec_per_cluster;  // 从2开始

        // 如果是整簇, 写整簇
        if ((cluster_offset == 0) && (nbytes == fat->cluster_byte_size)) {
            int err = dev_write(fat->fs->dev_id, start_sector, buf, fat->sec_per_cluster);
            if (err < 0) {
                return total_write;
            }

            curr_write = fat->cluster_byte_size;
        } else {
            // 如果跨簇，只写第一个簇内的一部分
            if (cluster_offset + curr_write > fat->cluster_byte_size) {
                curr_write = fat->cluster_byte_size - cluster_offset;
            }

            fat->curr_sector = -1;
            int err = dev_read(fat->fs->dev_id, start_sector, fat->fat_buffer, fat->sec_per_cluster);
            if (err < 0) {
                return total_write;
            }
            kernel_memcpy(fat->fat_buffer + cluster_offset, buf, curr_write);        
            
            // 写整个簇，然后从中拷贝
            err = dev_write(fat->fs->dev_id, start_sector, fat->fat_buffer, fat->sec_per_cluster);
            if (err < 0) {
                return total_write;
            }
        }

        buf += curr_write;
        nbytes -= curr_write;
        total_write += curr_write;
        file->size += curr_write;

        // 前移文件指针
		int err = move_file_pos(file, fat, curr_write, 1);
		if (err < 0) {
            return total_write;
        }
    }

    return total_write;
}

/**
 * @brief 关闭文件
 */
void fatfs_close (file_t * file) {
    if (file->mode == O_RDONLY) {
        return;
    }

    fat_t * fat = (fat_t *)file->fs->data;

    diritem_t * item = read_dir_entry(fat, file->p_index);
    if (item == (diritem_t *)0) {
        return;
    }

    item->DIR_FileSize = file->size;
    item->DIR_FstClusHI = (uint16_t )(file->sblk >> 16);
    item->DIR_FstClusL0 = (uint16_t )(file->sblk & 0xFFFF);
    write_dir_entry(fat, item, file->p_index);
}

/**
 * @brief 文件读写位置的调整
 */
int fatfs_seek (file_t * file, uint32_t offset, int dir) {
     // 只支持基于文件开头的定位
    if (dir != 0) {
        return -1;
    }

    fat_t * fat = (fat_t *)file->fs->data;
    cluster_t curr_cluster = file->sblk;
    uint32_t curr_pos = 0;
    uint32_t offset_to_move = offset;

    while (offset_to_move > 0) {
        uint32_t c_off = curr_pos % fat->cluster_byte_size;
        uint32_t curr_move = offset_to_move;

        // 不超过一簇，直接调整位置，无需跑到下一簇
        if (c_off + curr_move < fat->cluster_byte_size) {
            curr_pos += curr_move;
            break;
        }

        // 超过一簇，只在当前簇内移动
        curr_move = fat->cluster_byte_size - c_off;
        curr_pos += curr_move;
        offset_to_move -= curr_move;

        // 取下一簇
        curr_cluster = cluster_get_next(fat, curr_cluster);
        if (!cluster_is_valid(curr_cluster)) {
            return -1;
        }
    }

    // 最后记录一下位置
    file->pos = curr_pos;
    file->cblk = curr_cluster;
    return 0;
}

int fatfs_stat (file_t * file, struct stat *st) {
    return -1;
}

/**
 * @brief 打开目录。只是简单地读取位置重设为0
 */
int fatfs_opendir (struct _fs_t * fs,const char * name, DIR * dir) {
    dir->index = 0;
    return 0;
}

/**
 * @brief 读取一个目录项
 */
int fatfs_readdir (struct _fs_t * fs,DIR* dir, struct dirent * dirent) {
    fat_t * fat = (fat_t *)fs->data;

    // 做一些简单的判断，检查
    while (dir->index < fat->root_ent_cnt) {
        diritem_t * item = read_dir_entry(fat, dir->index);
        if (item == (diritem_t *)0) {
            return -1;
        }

        // 结束项，不需要再扫描了，同时index也不能往前走
        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        // 只显示普通文件和目录，其它的不显示
        if (item->DIR_Name[0] != DIRITEM_NAME_FREE) {
            file_type_t type = diritem_get_type(item);
            if ((type == FILE_NORMAL) || (type == FILE_DIR)) {
                dirent->index = dir->index++;
                dirent->type = diritem_get_type(item);
                dirent->size = item->DIR_FileSize;
                diritem_get_name(item, dirent->name);
                return 0;
            }
        }

        dir->index++;
    }

    return -1;
}

/**
 * @brief 关闭文件扫描读取
 */
int fatfs_closedir (struct _fs_t * fs,DIR *dir) {
    return 0;
}

/**
 * @brief 删除文件
 */
int fatfs_unlink (struct _fs_t * fs, const char * path) {
    fat_t * fat = (fat_t *)fs->data;

    // 遍历根目录的数据区，找到已经存在的匹配项
    for (int i = 0; i < fat->root_ent_cnt; i++) {
        diritem_t * item = read_dir_entry(fat, i);
        if (item == (diritem_t *)0) {
            return -1;
        }

         // 结束项，不需要再扫描了，同时index也不能往前走
        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        // 只显示普通文件和目录，其它的不显示
        if (item->DIR_Name[0] == DIRITEM_NAME_FREE) {
            continue;
        }

        // 找到要打开的目录
        if (diritem_name_match(item, path)) {
            // 释放簇
            int cluster = (item->DIR_FstClusHI << 16) | item->DIR_FstClusL0;
            cluster_free_chain(fat, cluster);

            // 写diritem项
            diritem_t item;
            kernel_memset(&item, 0, sizeof(diritem_t));
            return write_dir_entry(fat, &item, i);
        }
    }

    return -1;
}

fs_op_t fatfs_op = {
    .mount = fatfs_mount,
    .unmount = fatfs_unmount,
    .open = fatfs_open,
    .read = fatfs_read,
    .write = fatfs_write,
    .seek = fatfs_seek,
    .stat = fatfs_stat,
    .close = fatfs_close,

    .opendir = fatfs_opendir,
    .readdir = fatfs_readdir,
    .closedir = fatfs_closedir,
    .unlink = fatfs_unlink,
};