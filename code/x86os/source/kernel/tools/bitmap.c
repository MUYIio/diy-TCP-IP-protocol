/**
 * 位图数据结构
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "tools/bitmap.h"
#include "tools/klib.h"

/**
 * @brief 获取所需要的字节数量
 */
int bitmap_byte_count (int bit_count) {
    return (bit_count + 8 - 1) / 8;         // 向上取整
}

/**
 * @brief 位图初始化
 */
void bitmap_init (bitmap_t * bitmap, uint8_t * bits, int count, int init_bit) {
    bitmap->bit_count = count;
    bitmap->bits = bits;

    int bytes = bitmap_byte_count(bitmap->bit_count);
    kernel_memset(bitmap->bits, init_bit ? 0xFF: 0, bytes);
}

/**
 * @brief 连续设置N个位
 */
void bitmap_set_bit (bitmap_t * bitmap, int index, int count, int bit) {
    for (int i = 0; (i < count) && (index < bitmap->bit_count); i++, index++) {
        // 可以考虑进行一定程序的优化!!
        if (bit) {
            bitmap->bits[index / 8] |= 1 << (index % 8);
        } else {
            bitmap->bits[index / 8] &= ~(1 << (index % 8));
        }
    }
} 

/**
 * @brief 获取指定位的状态
 */
int bitmap_get_bit (bitmap_t * bitmap, int index) {
    return bitmap->bits[index / 8] & (1 << (index % 8));
}

/**
 * @brief 检查指定位是否置1
 */
int bitmap_is_set (bitmap_t * bitmap, int index) {
    return bitmap_get_bit(bitmap, index) ? 1 : 0;
}

/**
 * @brief 连续分配若干指定比特位，返回起始索引
 */
int bitmap_alloc_nbits (bitmap_t * bitmap, int bit, int count) {
    int search_idx = 0;
    int ok_idx = -1;

    while (search_idx < bitmap->bit_count) {
        // 定位到第一个相同的索引处
        if (bitmap_get_bit(bitmap, search_idx) != bit) {
            // 不同，继续寻找起始的bit
            search_idx++;
            continue;
        }

        // 记录起始索引
        ok_idx = search_idx;

        // 继续计算下一部分
        int i;
        for (i = 1; (i < count) && (search_idx < bitmap->bit_count); i++) {
            if (bitmap_get_bit(bitmap, search_idx++) != bit) {
                // 不足count个，退出，重新进行最外层的比较
                ok_idx = -1;
                break;
            }
        }

        // 找到，设置各位，然后退出
        if (i >= count) {
            bitmap_set_bit(bitmap, ok_idx, count, ~bit);
            return ok_idx;
        }
    }

    return -1;
}

