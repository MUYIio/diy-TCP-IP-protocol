/**
 * 磁盘驱动
 * 磁盘依次从sda,sdb,sdc开始编号，分区则从0开始递增
 * 其中0对应的分区信息为整个磁盘的信息
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "dev/disk.h"
#include "dev/dev.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "core/memory.h"
#include "core/task.h"

static disk_t disk_buf[DISK_CNT];  // 通道结构
static mutex_t mutex;     // 通道信号量
static sem_t op_sem;      // 通道操作的信号量
static int task_on_op;

/**
 * 发送ata命令，支持多达16位的扇区，对我们目前的程序来书够用了。
 */
static void ata_send_cmd (disk_t * disk, uint32_t start_sector, uint32_t sector_count, int cmd) {
    outb(DISK_DRIVE(disk), DISK_DRIVE_BASE | disk->drive);		// 使用LBA寻址，并设置驱动器

	// 必须先写高字节
	outb(DISK_SECTOR_COUNT(disk), (uint8_t) (sector_count >> 8));	// 扇区数高8位
	outb(DISK_LBA_LO(disk), (uint8_t) (start_sector >> 24));		// LBA参数的24~31位
	outb(DISK_LBA_MID(disk), 0);									// 高于32位不支持
	outb(DISK_LBA_HI(disk), 0);										// 高于32位不支持
	outb(DISK_SECTOR_COUNT(disk), (uint8_t) (sector_count));		// 扇区数量低8位
	outb(DISK_LBA_LO(disk), (uint8_t) (start_sector >> 0));			// LBA参数的0-7
	outb(DISK_LBA_MID(disk), (uint8_t) (start_sector >> 8));		// LBA参数的8-15位
	outb(DISK_LBA_HI(disk), (uint8_t) (start_sector >> 16));		// LBA参数的16-23位

	// 选择对应的主-从磁盘
	outb(DISK_CMD(disk), (uint8_t)cmd);
}

/**
 * 读取ATA数据端口
 */
static inline void ata_read_data (disk_t * disk, void * buf, int size) {
    uint16_t * c = (uint16_t *)buf;
    for (int i = 0; i < size / 2; i++) {
        *c++ = inw(DISK_DATA(disk));
    }
}

/**
 * 读取ATA数据端口
 */
static inline void ata_write_data (disk_t * disk, void * buf, int size) {
    uint16_t * c = (uint16_t *)buf;
    for (int i = 0; i < size / 2; i++) {
        outw(DISK_DATA(disk), *c++);
    }
}

/**
 * @brief 等待磁盘有数据到达
 */
static inline int ata_wait_data (disk_t * disk) {
    uint8_t status;
	do {
        // 等待数据或者有错误
        status = inb(DISK_STATUS(disk));
        if ((status & (DISK_STATUS_BUSY | DISK_STATUS_DRQ | DISK_STATUS_ERR))
                        != DISK_STATUS_BUSY) {
            break;
        }
    }while (1);

    // 检查是否有错误
    return (status & DISK_STATUS_ERR) ? -1 : 0;
}

/**
 * @brief 打印磁盘信息
 */
static void print_disk_info (disk_t * disk) {
    log_printf("%s:", disk->name);
    log_printf("  port_base: %x", disk->port_base);
    log_printf("  total_size: %d m", disk->sector_count * disk->sector_size / 1024 /1024);
    log_printf("  drive: %s", disk->drive == DISK_DISK_MASTER ? "Master" : "Slave");

    // 显示分区信息
    log_printf("  Part info:");
    for (int i = 0; i < DISK_PRIMARY_PART_CNT; i++) {
        partinfo_t * part_info = disk->partinfo + i;
        if (part_info->type != FS_INVALID) {
            log_printf("    %s: type: %x, start sector: %d, count %d",
                    part_info->name, part_info->type,
                    part_info->start_sector, part_info->total_sector);
        }
    }
}

/**
 * 获取指定序号的分区信息
 * 注意，该操作依赖物理分区分配，如果设备的分区结构有变化，则序号也会改变，得到的结果不同
 */
static int detect_part_info(disk_t * disk) {
    mbr_t mbr;

    // 读取mbr区
    ata_send_cmd(disk, 0, 1, DISK_CMD_READ);
    int err = ata_wait_data(disk);
    if (err < 0) {
        log_printf("read mbr failed");
        return err;
    }
    ata_read_data(disk, &mbr, sizeof(mbr));

	// 遍历4个主分区描述，不考虑支持扩展分区
	part_item_t * item = mbr.part_item;
    partinfo_t * part_info = disk->partinfo + 1;
	for (int i = 0; i < MBR_PRIMARY_PART_NR; i++, item++, part_info++) {
		part_info->type = item->system_id;

        // 没有分区，清空part_info
		if (part_info->type == FS_INVALID) {
			part_info->total_sector = 0;
            part_info->start_sector = 0;
            part_info->disk = (disk_t *)0;
        } else {
            // 在主分区中找到，复制信息
            kernel_sprintf(part_info->name, "%s%d", disk->name, i + 1);
            part_info->start_sector = item->relative_sectors;
            part_info->total_sector = item->total_sectors;
            part_info->disk = disk;
        }
	}
}

/**
 * @brief 检测磁盘相关的信息
 */
static int identify_disk (disk_t * disk) {
    ata_send_cmd(disk, 0, 0, DISK_CMD_IDENTIFY);

    // 检测状态，如果为0，则控制器不存在
    int err = inb(DISK_STATUS(disk));
    if (err == 0) {
        log_printf("%s doesn't exist\n", disk->name);
        return -1;
    }

    // 等待数据就绪, 此时中断还未开启，因此暂时可以使用查询模式
    err = ata_wait_data(disk);
    if (err < 0) {
        log_printf("disk[%s]: read failed!\n", disk->name);
        return err;
    }

    // 读取返回的数据，特别是uint16_t 100 through 103
    // 测试用的盘： 总共102400 = 0x19000， 实测会多一个扇区，为vhd磁盘格式增加的一个扇区
    uint16_t buf[256];
    ata_read_data(disk, buf, sizeof(buf));
    disk->sector_count = *(uint32_t *)(buf + 100);
    disk->sector_size = SECTOR_SIZE;            // 固定为512字节大小

    // 分区0保存了整个磁盘的信息
    partinfo_t * part = disk->partinfo + 0;
    part->disk = disk;
    kernel_sprintf(part->name, "%s%d", disk->name, 0);
    part->start_sector = 0;
    part->total_sector = disk->sector_count;
    part->type = FS_INVALID;

    // 接下来识别硬盘上的分区信息
    detect_part_info(disk);
    return 0;
}

/**
 * @brief 磁盘初始化及检测
 * 以下只是将相关磁盘相关的信息给读取到内存中
 */
void disk_init (void) {
    log_printf("Checking disk...");

    // 清空所有disk，以免数据错乱。不过引导程序应该有清0的，这里为安全再清一遍
    kernel_memset(disk_buf, 0, sizeof(disk_buf));

    // 信号量和锁
    mutex_init(&mutex);
    sem_init(&op_sem, 0);       // 没有操作完成

    // 检测各个硬盘, 读取硬件是否存在，有其相关信息
    for (int i = 0; i < DISK_PER_CHANNEL; i++) {
        disk_t * disk = disk_buf + i;

        // 先初始化各字段
        kernel_sprintf(disk->name, "sd%c", i + 'a');
        disk->drive = (i == 0) ? DISK_DISK_MASTER : DISK_DISK_SLAVE;
        disk->port_base = IOBASE_PRIMARY;
        disk->mutex = &mutex;
        disk->op_sem = &op_sem;

        // 识别磁盘，有错不处理，直接跳过
        int err = identify_disk(disk);
        if (err == 0) {
            print_disk_info(disk);
        }
    }
}


/**
 * @brief 打开磁盘设备
 */
int disk_open (device_t * dev) {
    int disk_idx = (dev->minor >> 4) - 0xa;
    int part_idx = dev->minor & 0xF;

    if ((disk_idx >= DISK_CNT) || (part_idx >= DISK_PRIMARY_PART_CNT)) {
        log_printf("device minor error: %d", dev->minor);
        return -1;
    }

    disk_t * disk = disk_buf + disk_idx;
    if (disk->sector_size == 0) {
        log_printf("disk not exist. device:sd%x", dev->minor);
        return -1;
    }

    partinfo_t * part_info = disk->partinfo + part_idx;
    if (part_info->total_sector == 0) {
        log_printf("part not exist. device:sd%x", dev->minor);
        return -1;
    }

    // 磁盘存在，建立关联
    dev->data = part_info;
    irq_install(IRQ14_HARDDISK_PRIMARY, exception_handler_ide_primary);
    irq_enable(IRQ14_HARDDISK_PRIMARY);
    return 0;
}

/**
 * @brief 读磁盘
 */
int disk_read (device_t * dev, int start_sector, char * buf, int count) {
    // 取分区信息
    partinfo_t * part_info = (partinfo_t *)dev->data;
    if (!part_info) {
        log_printf("Get part info failed! device = %d", dev->minor);
        return -1;
    }

    disk_t * disk = part_info->disk;
    if (disk == (disk_t *)0) {
        log_printf("No disk for device %d", dev->minor);
        return -1;
    }

    mutex_lock(disk->mutex);
    task_on_op = 1;

    int cnt;
    ata_send_cmd(disk, part_info->start_sector + start_sector, count, DISK_CMD_READ);
    for (cnt = 0; cnt < count; cnt++, buf += disk->sector_size) {
        // 利用信号量等待中断通知，然后再读取数据
        if (task_current()) {
            sem_wait(disk->op_sem);
        }

        // 这里虽然有调用等待，但是由于已经是操作完毕，所以并不会等
        int err = ata_wait_data(disk);
        if (err < 0) {
            log_printf("disk(%s) read error: start sect %d, count %d", disk->name, start_sector, count);
            break;
        }

        // 此处再读取数据
        ata_read_data(disk, buf, disk->sector_size);
    }

    mutex_unlock(disk->mutex);
    return cnt;
}

/**
 * @brief 写扇区
 */
int disk_write (device_t * dev, int start_sector, char * buf, int count) {
    // 取分区信息
    partinfo_t * part_info = (partinfo_t *)dev->data;
    if (!part_info) {
        log_printf("Get part info failed! device = %d", dev->minor);
        return -1;
    }

    disk_t * disk = part_info->disk;
    if (disk == (disk_t *)0) {
        log_printf("No disk for device %d", dev->minor);
        return -1;
    }

    mutex_lock(disk->mutex);
    task_on_op = 1;

    int cnt;
    ata_send_cmd(disk, part_info->start_sector + start_sector, count, DISK_CMD_WRITE);
    for (cnt = 0; cnt < count; cnt++, buf += disk->sector_size) {
        // 先写数据
        ata_write_data(disk, buf, disk->sector_size);

        // 利用信号量等待中断通知，等待写完成
        if (task_current()) {
            sem_wait(disk->op_sem);
        }

        // 这里虽然有调用等待，但是由于已经是操作完毕，所以并不会等
        int err = ata_wait_data(disk);
        if (err < 0) {
            log_printf("disk(%s) write error: start sect %d, count %d", disk->name, start_sector, count);
            break;
        }
    }

    mutex_unlock(disk->mutex);
    return cnt;
}

/**
 * @brief 向磁盘发命令
 *
 */
int disk_control (device_t * dev, int cmd, int arg0, int arg1) {
    return 0;
}

/**
 * @brief 关闭磁盘
 *
 */
void disk_close (device_t * dev) {
}

/**
 * @brief 磁盘主通道中断处理
 */
void do_handler_ide_primary (exception_frame_t *frame)  {
    pic_send_eoi(IRQ14_HARDDISK_PRIMARY);
    if (task_on_op && task_current()) {
        sem_notify(&op_sem);
    }
}

// 磁盘设备描述表
dev_desc_t dev_disk_desc = {
	.name = "disk",
	.major = DEV_DISK,
	.open = disk_open,
	.read = disk_read,
	.write = disk_write,
	.control = disk_control,
	.close = disk_close,
};
