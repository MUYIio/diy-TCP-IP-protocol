/**
 * 命令行实现：仅支持几个内建命令，同时支持加载外部命令执行
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef CMD_H
#define CMD_H

#define CLI_inbUT_SIZE              1024            // 输入缓存区
#define	CLI_MAX_ARG_COUNT		    10			    // 最大接收的参数数量

#define ESC_CMD2(Pn, cmd)		    "\x1b["#Pn#cmd
#define	ESC_COLOR_ERROR			    ESC_CMD2(31, m)	// 红色错误
#define	ESC_COLOR_DEFAULT		    ESC_CMD2(39, m)	// 默认颜色
#define ESC_CLEAR_SCREEN		    ESC_CMD2(2, J)	// 擦除整屏幕
#define	ESC_MOVE_CURSOR(row, col)  "\x1b["#row";"#col"H"

/**
 * 命令列表
 */
typedef struct _cli_cmd_t {
    const char * name;          // 命令名称
    const char * useage;        // 使用方法
    int(*do_func)(int argc, char **argv);       // 回调函数
}cli_cmd_t;

/**
 * 命令行管理器
 */
typedef struct _cli_t {    
    char curr_inbut[CLI_inbUT_SIZE]; // 当前输入缓存
    const cli_cmd_t * cmd_start;     // 命令起始
    const cli_cmd_t * cmd_end;       // 命令起始
    const char * promot;        	 // 提示符
}cli_t;

#endif
