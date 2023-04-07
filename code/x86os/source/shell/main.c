/**
 * 简单的命令行解释器
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include <string.h>
#include "lib_syscall.h"
#include "main.h"
#include <getopt.h>
#include <stdlib.h>
#include <sys/file.h>
#include "fs/file.h"
#include "httpd.h"

static cli_t cli;
static const char * promot = "sh >>";       // 命令行提示符

/**
 * 显示命令行提示符
 */
static void show_promot(void) {
    printf("%s", cli.promot);
    fflush(stdout);
}

/**
 * help命令
 */
static int do_help(int argc, char **argv) {
    const cli_cmd_t * start = cli.cmd_start;

    // 循环打印名称及用法
    while (start < cli.cmd_end) {
        printf("%s %s\n",  start->name, start->useage);
        start++;
    }
    return 0;
}

/**
 * 清屏命令
 */
static int do_clear (int argc, char ** argv) {
    printf("%s", ESC_CLEAR_SCREEN);
    printf("%s", ESC_MOVE_CURSOR(0, 0));
    return 0;
}

/**
 * 回显命令
 */
static int do_echo (int argc, char ** argv) {
    // 只有一个参数，需要先手动输入，再输出
    if (argc == 1) {
        char msg_buf[128];

        fgets(msg_buf, sizeof(msg_buf), stdin);
        msg_buf[sizeof(msg_buf) - 1] = '\0';
        puts(msg_buf);
        return 0;
    }

    // https://www.cnblogs.com/yinghao-liu/p/7123622.html
    // optind是下一个要处理的元素在argv中的索引
    // 当没有选项时，变为argv第一个不是选项元素的索引。
    int count = 1;    // 缺省只打印一次
    int ch;
    while ((ch = getopt(argc, argv, "n:h")) != -1) {
        switch (ch) {
            case 'h':
                puts("echo echo any message");
                puts("Usage: echo [-n count] msg");
                optind = 1;        // getopt需要多次调用，需要重置
                return 0;
            case 'n':
                count = atoi(optarg);
                break;
            case '?':
                if (optarg) {
                    fprintf(stderr, "Unknown option: -%s\n", optarg);
                }
                optind = 1;        // getopt需要多次调用，需要重置
                return -1;
        }
    }

    // 索引已经超过了最后一个参数的位置，意味着没有传入要发送的信息
    if (optind > argc - 1) {
        fprintf(stderr, "Message is empty \n");
        optind = 1;        // getopt需要多次调用，需要重置
        return -1;
    }

    // 循环打印消息
    char * msg = argv[optind];
    for (int i = 0; i < count; i++) {
        puts(msg);
    }
    optind = 1;        // getopt需要多次调用，需要重置
    return 0;
}

/**
 * 程序退出命令
 */
static int do_exit (int argc, char ** argv) {
    exit(0);
    return 0;
}

/**
 * 启动HTTP服务器
 */
static int do_httpd (int argc, char ** argv) {
    httpd_start("", 80);
    return 0;
}


/**
 * @brief 列出文本文件内容
 */
static int do_less (int argc, char ** argv) {
    int line_mode = 0;

    int ch;
    while ((ch = getopt(argc, argv, "lh")) != -1) {
        switch (ch) {
            case 'h':
                puts("show file content");
                puts("less [-l] file");
                puts("-l show file line by line.");
                break;
            case 'l':
                line_mode = 1;
                break;
            case '?':
                if (optarg) {
                    fprintf(stderr, "Unknown option: -%s\n", optarg);
                }
                optind = 1;        // getopt需要多次调用，需要重置
                return -1;
        }
    }

    // 索引已经超过了最后一个参数的位置，意味着没有传入要发送的信息
    if (optind > argc - 1) {
        fprintf(stderr, "no file\n");
        optind = 1;        // getopt需要多次调用，需要重置
        return -1;
    }

    FILE * file = fopen(argv[optind], "r");
    if (file == NULL) {
        fprintf(stderr, "open file failed. %s", argv[optind]);
        optind = 1;        // getopt需要多次调用，需要重置
        return -1;
    }

    char * buf = (char *)malloc(255);

    if (line_mode == 0) {
        while (fgets(buf, 255, file) != NULL)  {
            fputs(buf, stdout);
        }
    } else {
        // 不使用缓存，这样能直接立即读取到输入而不用等回车
        setvbuf(stdin, NULL, _IONBF, 0);
        ioctl(0, TTY_CMD_ECHO, 0, 0);
        while (1) {
            char * b = fgets(buf, 255, file);
            if (b == NULL ) {
                break;
            }
            fputs(buf, stdout);

            int ch;
            while ((ch = fgetc(stdin)) != 'n') {
                if (ch == 'q') {
                    goto less_quit;
                }
            }
        }
    less_quit:
    // 恢复为行缓存
        setvbuf(stdin, NULL,_IOLBF, BUFSIZ);
        ioctl(0, TTY_CMD_ECHO, 1, 0);
    }
    free(buf);
    fclose(file);
    optind = 1;        // getopt需要多次调用，需要重置
    return 0;
}

/**
 * @brief 列出目录内容
 */
static int do_ls (int argc, char ** argv) {
    // 打开目录
	DIR * p_dir = opendir("temp");
	if (p_dir == NULL) {
		printf("open dir failed\n");
		return -1;
	}

    // 然后进行遍历
	struct dirent * entry;
	while((entry = readdir(p_dir)) != NULL) {
        strlwr(entry->name);
		printf("%c %s %d\n",
                entry->type == FILE_DIR ? 'd' : 'f',
                entry->name,
                entry->size);
	}
	closedir(p_dir);

    return 0;
}

/**
 * @brief 复制文件命令
 */
static int do_cp (int argc, char ** argv) {
    if (argc < 3) {
        puts("no [from] or no [to]");
        return -1;
    }

    FILE * from, * to;
    from = fopen(argv[1], "rb");
    to = fopen(argv[2], "wb");
    if (!from || !to) {
        puts("open file failed.");
        goto ls_failed;
    }

    char * buf = (char *)malloc(255);
    int size = 0;
    while ((size = fread(buf, 1, 255, from)) > 0) {
        fwrite(buf, 1, size, to);
    }
    free(buf);

ls_failed:
    if (from) {
        fclose(from);
    }
    if (to) {
        fclose(to);
    }
    return 0;
}

/**
 * @brief 删除文件命令
 */
static int do_remove (int argc, char ** argv) {
    if (argc < 2) {
        fprintf(stderr, "no file");
        return -1;
    }

    int err = unlink(argv[1]);
    if (err < 0) {
        fprintf(stderr, "rm file failed: %s", argv[1]);
        return err;
    }

    return 0;
}

// 命令列表
static const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
		.useage = "help -- list support command",
		.do_func = do_help,
    },
    {
        .name = "clear",
		.useage = "clear -- clear the screen",
		.do_func = do_clear,
    },
	{
		.name = "echo",
		.useage = "echo [-n count] msg  -- echo something",
		.do_func = do_echo,
	},
    {
        .name = "ls",
        .useage = "ls [dir] -- list director",
        .do_func = do_ls,
    },
    {
        .name = "less",
        .useage = "list text file content",
        .do_func = do_less,
    },
    {
        .name = "cp",
        .useage = "cp from to -- copy file",
        .do_func = do_cp,
    },
    {
       .name = "rm",
        .useage = "rm file -- remove file",
        .do_func = do_remove,
    },
    {
        .name = "quit",
        .useage = "quit from shell",
        .do_func = do_exit,
    },
    {
        .name = "httpd",
        .useage = "start http server",
        .do_func = do_httpd,
    },
};

/**
 * 命令行初始化
 */
static void cli_init(const char * promot, const cli_cmd_t * cmd_list, int cnt) {
    cli.promot = promot;
    
    memset(cli.curr_inbut, 0, CLI_inbUT_SIZE);
    
    cli.cmd_start = cmd_list;
    cli.cmd_end = cmd_list + cnt;
}

/**
 * 在内部命令中搜索
 */
static const cli_cmd_t * find_builtin (const char * name) {
    for (const cli_cmd_t * cmd = cli.cmd_start; cmd < cli.cmd_end; cmd++) {
        if (strcmp(cmd->name, name) != 0) {
            continue;
        }

        return cmd;
    }

    return (const cli_cmd_t *)0;
}

/**
 * 运行内部命令
 */
static void run_builtin (const cli_cmd_t * cmd, int argc, char ** argv) {
    int ret = cmd->do_func(argc, argv);
    if (ret < 0) {
        fprintf(stderr,ESC_COLOR_ERROR"error: %d\n"ESC_COLOR_DEFAULT, ret);
    }
}

/**
 * 遍历搜索目录，看看文件是否存在，存在返回文件所在路径
 */
static const char * find_exec_path (const char * file_name) {
    static char path[255];

    int fd = open(file_name, 0);
    if (fd < 0) {
        sprintf(path, "%s.elf", file_name);
        fd = open(path, 0);
        if (fd < 0) {
            return (const char * )0;
        }
        close(fd);
        return path;
    } else {
        close(fd);
        return file_name;
    }
}

/**
 * 试图运行当前文件
 */
static void run_exec_file (const char * path, int argc, char ** argv) {
    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "fork failed: %s", path);
    } else if (pid == 0) {
        // 子进程
        int err = execve(path, argv, (char * const *)0);
        if (err < 0) {
            fprintf(stderr, "exec failed: %s", path);
        }
        exit(-1);
    } else {
		// 等待子进程执行完毕
        int status;
        int pid = wait(&status);
        fprintf(stderr, "cmd %s result: %d, pid = %d\n", path, status, pid);
    }
}

int main (int argc, char **argv) {
	open(argv[0], O_RDWR);
    dup(0);     // 标准输出
    dup(0);     // 标准错误输出

   	cli_init(promot, cmd_list, sizeof(cmd_list) / sizeof(cli_cmd_t));
	for (;;) {
        // 显示提示符，开始工作
        show_promot();

        // 获取输入的字符串，然后进行处理.
        // 注意，读取到的字符串结尾中会包含换行符和0
        char * str = fgets(cli.curr_inbut, CLI_inbUT_SIZE, stdin);
        if (str == (char *)0) {
            // 读不到错误，或f发生错误，则退出
            break;
        }

        // 读取的字符串中结尾可能有换行符，去掉之
        char * cr = strchr(cli.curr_inbut, '\n');
        if (cr) {
            *cr = '\0';
        }
        cr = strchr(cli.curr_inbut, '\r');
        if (cr) {
            *cr = '\0';
        }

        int argc = 0;
        char * argv[CLI_MAX_ARG_COUNT];
        memset(argv, 0, sizeof(argv));

        // 提取出命令，找命令表
        const char * space = " ";  // 字符分割器
        char *token = strtok(cli.curr_inbut, space);
        while (token) {
            // 记录参数
            argv[argc++] = token;

            // 先获取下一位置
            token = strtok(NULL, space);
        }

        // 没有任何输入，则x继续循环
        if (argc == 0) {
            continue;
        }

        // 试图作为内部命令加载执行
        const cli_cmd_t * cmd = find_builtin(argv[0]);
        if (cmd) {
            run_builtin(cmd, argc, argv);
            continue;
        }

        // 试图作为外部命令执行。只检查文件是否存在，不考虑是否可执行
        const char * path = find_exec_path(argv[0]);
        if (path) {
            run_exec_file(path, argc, argv);
            continue;
        }

        // 找不到命令，提示错误
        fprintf(stderr, ESC_COLOR_ERROR"Unknown command: %s\n"ESC_COLOR_DEFAULT, cli.curr_inbut);
    }

    return 0;
}   