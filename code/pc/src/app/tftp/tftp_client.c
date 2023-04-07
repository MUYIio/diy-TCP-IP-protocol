/**
 * @file tftp_client.c
 * @author lishutong(527676163@qq.com)
 * @brief tftp客户端的实现
 * @version 0.1
 * @date 2022-11-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <string.h>
#include "tftp_client.h"

static tftp_t tftp;                 // 控制结构

/**
 * @brief 准备TFTP连接
 */
static int tftp_open (const char * ip, uint16_t port, int blksize) {
    // 打开套接字
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        printf("error: create server socket failed\n");
        return -1;
    }

    // 设置各项缺省值
    tftp.socket = sockfd;
    tftp.error = TFTP_ERR_OK;
    tftp.block_size = blksize;
    tftp.tmo_retry = TFTP_MAX_RETRY;
    tftp.tmo_sec = TFTP_TMO_SEC;
    tftp.tx_size = 0;

    // 设置远程地址，注意不要使用connect，因为服务器可能会用另一端口与自己通信
    struct sockaddr_in * sockaddr = (struct sockaddr_in * )&tftp.remote;
    memset(sockaddr, 0, sizeof(struct sockaddr_in));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = inet_addr(ip);
    sockaddr->sin_port = htons(port);   

    // 设置超时
    struct timeval tmo;
    tmo.tv_sec = tftp.tmo_sec;
    tmo.tv_usec = 0;
    setsockopt(tftp.socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tmo, sizeof(tmo));
    return 0;
}

/**
 * @brief 关闭tftp连接
 */
static void tftp_close (void) {
    // 关闭套接字
    close(tftp.socket);
}

/**
 * @brief 从远端服务器获取文件，并写入指定的路径
 * 如果文件已经存在，将被覆盖
 */
int do_tftp_get(int blksize, const char * ip, uint16_t port, const char* filename) {
    // 打开套接字
    if (tftp_open(ip, port, blksize) < 0) {
        printf("tftp connect failed.");
        return -1;
    }

    printf("tftp: try to get file %s\n", filename);

    // 创建本地文件，注意以二进制写入，否则以文本模式会进行换行符号的改写，即遇到0x0A时，实际写入0x0A, 0x0D
    // 进而使得写入的数据比实际收到的数据要多一点
    FILE * file = fopen(filename, "wb");
    if (file == (FILE*)0) {
        printf( "tftp: create local file failed: %s\n", filename);
        tftp_close();
        return -1;
    }
                
    // 创建RRQ请求，默认使用的带选项的ack
    int err = tftp_send_request(&tftp, 1, filename, 0);
    if (err < 0) {
        printf( "tftp: send tftp rrq failed\n");
        goto get_error;
    }

    size_t recv_size;
    err = tftp_wait_packet(&tftp, TFTP_PKT_OACK, 0, &recv_size);
    if (err < 0) {
        // 其它错误，应当直接退出
        printf( "tftp: wait error, file: %s\n", filename);
        goto get_error;
    } 

    // 建立连接，后续只与该链接通信
    connect(tftp.socket, &tftp.remote, sizeof(tftp.remote));

    // 发送响应
    err = tftp_send_ack(&tftp, 0);
    if (err < 0) {
        printf( "tftp: reply to oack failed, file: %s\n", filename);
        goto get_error;
    } 
    printf("tftp: file size %d byte\n", tftp.file_size);

    // 循环读取文件内容
    uint16_t next_block = 1;
    uint32_t total_size = 0;            // 收发的总字节量
    uint32_t total_block = 0;           // 收发的总块数量
    while (1) {
        // 等待数据包及其中的块序号
        err = tftp_wait_packet(&tftp, TFTP_PKT_DATA, next_block, &recv_size);
        if (err < 0) {
            // 其它错误，应当直接退出
            printf( "tftp: wait error, block: %d, file: %s\n", next_block, filename);
            goto get_error;
        } 

        size_t block_size = recv_size - 4;         // 去掉block + opcode大小

        // 将数据写入文件，然后继续申请下一包，减4-opcode和block_num
        size_t size = 0;
        if (block_size) {
            size = fwrite(tftp.rx_packet.data.data, 1, block_size, file);
            if (size < block_size) {
                err = -1;
                printf("tftp: write file failed, %s\n", filename);
                goto get_error;
            }
        }

        // 写成功后，以当前的块序号发送响应，块序号增加
        err = tftp_send_ack(&tftp, next_block);
        if (err < 0) {
            printf( "tftp: send ack failed. ack block = %d,\n", next_block);
            goto get_error;
        }
        next_block++;

        // 显示进度信息
        total_size += (uint32_t)size;
        if (++total_block % 0x40 == 0) {
            printf(".");
            fflush(stdout);
        }

        //printf("%d. %d\n", block_size, tftp.block_size);

        // 不足完成的块，即表示最后一块，传输结束
        if (block_size < tftp.block_size) {
            err = 0;
            break;
        }
    };
    printf("\ntftp: total send: %u byte, %u blocks\n", total_size, total_block);
    if (total_size != tftp.file_size) {
        err = -1;
        printf("tftp: size incorrect, %d != %d\n", total_size, tftp.file_size);
    }
get_error:
    printf("tftp: get file %s\n", err < 0 ?  "Failed" : "OK");
    if (file) {
        fclose(file);
    }
    tftp_close();
    return err;
}

/**
 * @brief 上传文件至远端服务器
 */
int do_tftp_put(int blksize, const char * ip, uint16_t port, const char* filename) {
    // 打开套接字
    if (tftp_open(ip, port, blksize) < 0) {
        printf("tftp connect failed.");
        return -1;
    }

    printf("tftp: try to put file %s\n", filename);

    // 以只读二进制的方式打开文件，注意要加b，表示二进制读取
    FILE * file = fopen(filename, "rb");
    if (file == (FILE*)0) {
        printf( "tftp: open file failed, %s\n", filename);
        tftp_close();
        return -1;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 创建WRQ请求，然后等待响应
    int err = tftp_send_request(&tftp, 0, filename, filesize);
    if (err < 0) {
        printf( "tftp: send tftp wrq failed\n");
        goto put_error;
    }

    // 等待OACK响应
    size_t recv_size;
    err = tftp_wait_packet(&tftp, TFTP_PKT_OACK, 0, &recv_size);
    if (err < 0) {
        // 其它错误，应当直接退出
        printf( "tftp: wait error, file: %s\n", filename);
        goto put_error;
    } 

    // 建立连接，后续只与该链接通信
    connect(tftp.socket, &tftp.remote, sizeof(tftp.remote));

    // 循环读取文件内容发送
    uint32_t total_size = 0;            // 收发的总字节量
    uint32_t total_block = 0;           // 收发的总块数量
    uint16_t pre_block = 0;              // 写数据时，发害的起始序号
    while (1) {
        // 默认读最大的块
        size_t block_size = (size_t )fread(&tftp.tx_packet.data.data, 1, tftp.block_size, file);

        // 读取错误
        if (!feof(file) && (block_size != tftp.block_size)) {
            err = -1;
            printf( "tftp: read file failed, %s\n", filename);
            goto put_error;
        }

        // 读取成功后，以当前的块序号发送响应
        // 当前可能已经读取到0字节
        err = tftp_send_data(&tftp, ++pre_block, block_size);
        if (err < 0) {
            printf( "tftp: send data failed. block: %d\n", pre_block);
            break;
        }

        // 持续等待ACK响应
        size_t recv_size;
        err = tftp_wait_packet(&tftp, TFTP_PKT_ACK, pre_block, &recv_size);
        if (err < 0) {
            // 其它错误，应当直接退出
            printf( "tftp: wait timeout, block: %d, file: %s, error: %d\n", pre_block, filename, err);
            goto put_error;
        }

        // 显示一些进度信息
        total_size += (uint32_t)block_size;
        if (++total_block % 0x40 == 0) {
            printf(".");
            fflush(stdout);
        }

        // 文件已经读取完毕，可以退出了
        if (block_size < tftp.block_size) {
            err = 0;
            break;
        }
    };

    printf("\ntftp: total send: %u byte, %u blocks", total_size, total_block);
put_error:
    printf("\ntftp: put file %s\n", (err == 0) ? "OK" : "Failed");
    if (file) {
        fclose(file);
    }
    tftp_close();
    return err;
}

/**
 * @brief 显示命令列表
 */
void show_cmd_list (void) {
    printf("usage: cmd arg0, arg1...\n");
    printf("    get filename   download file from server\n");
    printf("    put filename   upload file to server\n");
    printf("    blk size       set block size\n");
    printf("    quit           quit\n");
}

/**
 * @brief 启动tftp客户端
 */
int tftp_start (const char * ip, uint16_t port) {
    int blksize = TFTP_DEF_BLKSIZE;
	char cmd_buf[TFTP_CMD_BUF_SIZE];

    // 未指定端口号，则使用默认端口
    if (port == 0) {
        port = TFTP_DEF_PORT;
    }

    // 首先显示命令
    printf("welcome to use tftp client\n");
    show_cmd_list();
    while (1) {
        printf("tftp >>");
        fflush(stdout);

        // 获取命令行输入
		memset(cmd_buf, 0, sizeof(cmd_buf));
		char * buf = fgets(cmd_buf, sizeof(cmd_buf), stdin);
		if(buf == NULL){
			continue;
		}
        // 读取的字符串中结尾可能有换行符，去掉之
        char * cr = strchr(buf, '\n');
        if (cr) {
            *cr = '\0';
        }
        cr = strchr(buf, '\r');
        if (cr) {
            *cr = '\0';
        }

        // 分割命令行，取得各项参数
        const char * split = " \t\n";
		char * cmd = strtok (buf, split);
		if(cmd) {
            // 下面的命令处理较简单，所以写在一起，可能不是那么好看
            if (strcmp(cmd, "get") == 0) {
                // 下载文件
                char * filename = strtok(NULL, split);
                if (filename) {
                    do_tftp_get(blksize, ip, port, filename);
                } else {
                    printf("error: no file\n");
                }
            } else if (strcmp(buf, "put") == 0) {
                // 上传文件
                char * filename = strtok(NULL, split);
                if (filename) {
                    do_tftp_put(blksize, ip, port, filename);
                } else {
                    printf("error: no file\n");
                }
            } else if (strcmp(buf, "blk") == 0) {
                // 设置传输使用的块大小
                char * blk = strtok(NULL, split);
                if (blk) {
                    int blk_size = atoi(blk);
                    if (blk_size <= 0) {
                        printf("blk size %d error, set to default: %d\n", blk_size, TFTP_DEF_BLKSIZE);
                        blk_size = TFTP_DEF_BLKSIZE;
                    } else if (blk_size > TFTP_BLKSIZE_MAX) {
                        printf("blk size %d too big, set to %d\n", blk_size, TFTP_BLKSIZE_MAX);
                        blk_size = TFTP_BLKSIZE_MAX;
                    } else {
                        blksize = blk_size < TFTP_DEF_BLKSIZE ? TFTP_DEF_BLKSIZE : blk_size;
                    }
                } else {
                    printf("error: no size\n");
                }

            } else if (strcmp(buf, "quit") == 0) {
                printf("quit from tftp");
                break;
            } else {
                printf("unknown cmd\n");
                show_cmd_list();
            }
		}
    }
    return 0;
}

/**
 * @brief 获取文件
 */
int tftp_get(const char * ip, uint16_t port, int block_size, const char* filename) {
    printf("try to get file %s from %s\n", filename, ip);
    if (block_size > TFTP_BLKSIZE_MAX) {
        block_size = TFTP_BLKSIZE_MAX;
    }
    return do_tftp_get(block_size, ip, port, filename);
}

/**
 * @brief 上传文件
 */
int tftp_put(const char* ip, uint16_t port, int block_size, const char* filename) {
    printf("try to put file %s to %s\n", filename, ip);
    if (block_size > TFTP_BLKSIZE_MAX) {
        block_size = TFTP_BLKSIZE_MAX;
    }
    return do_tftp_put(block_size, ip, port, filename);
}
