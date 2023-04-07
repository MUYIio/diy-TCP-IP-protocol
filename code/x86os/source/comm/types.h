/**
 * 系统内部的基本数据类型
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TYPES_H
#define TYPES_H

// 基本整数类型，下面的写法和视频中的不同，加了一些宏处理
// 主要是因为将要使用newlib库，newlib有同样使用typedef定义uint8_t类型
// 为了避免冲突，加上_UINT8_T_DECLARED的配置
#ifndef _UINT8_T_DECLARED
#define _UINT8_T_DECLARED
typedef unsigned char uint8_t;
#endif

#ifndef _UINT16_T_DECLARED
#define _UINT16_T_DECLARED
typedef unsigned short uint16_t;
#endif

#ifndef _UINT32_T_DECLARED
#define _UINT32_T_DECLARED
typedef unsigned long uint32_t;
#endif

#endif

