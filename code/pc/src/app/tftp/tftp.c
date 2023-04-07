/**
 * @file tftp.c
 * @author lishutong(527676163@qq.com)
 * @brief tftp协议的实现
 * @version 0.1
 * @date 2022-11-02
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <string.h>
#include "tftp.h"

/**
 * @brief 获取TFTP错误信息串
 */
const char* tftp_error_msg(tftp_err_t err) {
    // TFTP错误信息串
    static const char* msg[] = {
       "Unknown error",
       "File not found",
       "Access violation",
       "Disk full or allocation exceeded",
       "Illegal TFTP operation",
       "Unknown transfer ID",
       "File already exists",
       "user not exist",
       "option error",
       "unknow opcode",
    };
    
    if (err >= NET_TFTP_ERR_END) {
        err = 0;
    }
    return msg[err];
}

/**
 * @brief 发送tftp包
 */
int tftp_send_packet(tftp_t * tftp, tftp_pkt_t *packet, int size) {
    ssize_t send_size = sendto(tftp->socket, (const char*)packet, size, 0, &tftp->remote, sizeof(tftp->remote));
    if (send_size < 0) {
        printf("tftp: send error\n");
        return -1;
    }

    tftp->tx_size = size;
    return 0;
}

/**
 * @brief 写选项字节
 */
static char * write_option (tftp_t * tftp, char * buf, const char * option, int value) {
    char * buf_end =  ((char *)&tftp->tx_packet) + sizeof(tftp_pkt_t);

    // 写之前检查是否有足够空间
    size_t opt_len = strlen(option) + 1;
    if (buf + opt_len > buf_end) {
        printf("tftp: send buffer too small\n");
        return NULL;
    }
    strcpy(buf, option);
    buf += opt_len;

    if (value >= 0) {
        // 接下来写数字
        if (buf + 16 >= buf_end) {
            printf("tftp: send buffer too small\n");
            return NULL;
        }

        sprintf(buf, "%d", value);
        buf += strlen(buf) + 1;
    }
    return buf;
}

/**
 * @brief 发送RRQ或者WRQ请求
 */
int tftp_send_request(tftp_t * tftp, int is_read, const char* filename, uint32_t file_size) {
    tftp_pkt_t * pkt = &tftp->tx_packet;

    // opcode
    pkt->opcode = htons(is_read ? TFTP_PKT_RRQ : TFTP_PKT_WRQ);

    // filename
    char * buf = (char *)pkt->req.args;
    buf = write_option(tftp, buf, filename, -1);
    if (buf == NULL) {
        printf("tftp: filename too long: %s\n", filename);
        return -1;
    }

    // 写模式
    buf = write_option(tftp, buf, "octet", -1);
    if (buf == NULL) {
        goto send_error;
    }  

    // 写blksize及块大小
    buf = write_option(tftp, buf, "blksize", tftp->block_size);
    if (buf == NULL) {
        goto send_error;
    }  

    // tsize
    buf = write_option(tftp, buf, "tsize", file_size);
    if (buf == NULL) {
        goto send_error;
    }  

    // 计算总长
    int size = (int)(buf - (char *)pkt->req.args) + 2;     // 加上opcode

    // 发包请求结果
    int err = tftp_send_packet(tftp, pkt, size);
    if (err < 0) {
        printf( "tftp: send req failed\n");
        return -1;
    }

    return 0;
send_error:
    return -1;
}

/**
 * @brief 发送ACK包
 */
int tftp_send_ack(tftp_t * tftp, uint16_t block_num) {
    tftp_pkt_t* pkt = &tftp->tx_packet;

    // opcde
    pkt->opcode = htons(TFTP_PKT_ACK);

    // block
    pkt->ack.block = htons(block_num);

    // 发包请求结果，长度为opcode(2B) + block(2B)
    int err = tftp_send_packet(tftp, pkt, 4);
    if (err < 0) {
        printf( "tftp: send ack failed. block_num = %d\n", block_num);
        return -1;
    }

    return 0;
}

/**
 * @brief 发送OACK包
 */
int tftp_send_oack(tftp_t * tftp) {
    tftp_pkt_t* pkt = &tftp->tx_packet;

    // opcde
    pkt->opcode = htons(TFTP_PKT_OACK);
    char * buf = pkt->oack.option;

    // 写块大小
    buf = write_option(tftp, buf, "blksize", tftp->block_size);

    // 写传输数量
    buf = write_option(tftp, buf, "tsize", tftp->file_size);

    // 发包请求结果，长度为opcode(2B) + block(2B)
    int err = tftp_send_packet(tftp, pkt, (int)(buf - (char *)pkt));
    if (err < 0) {
        printf( "tftp: send oack failed.\n");
        return -1;
    }

    return 0;
}

/**
 * @brief 发送data包
 * 数据的填充，由调用者来负责。
 */
int tftp_send_data(tftp_t * tftp, uint16_t block_num, size_t size) {
    tftp_pkt_t* pkt = &tftp->tx_packet;

    // opcde
    pkt->opcode = htons(TFTP_PKT_DATA);

    // block
    pkt->data.block = htons(block_num);

    // 发包请求结果, opcode(2B) + block(2B) + data_size
    int err = tftp_send_packet(tftp, pkt, 4 + (int)size);
    if (err < 0) {
        printf( "tftp: send ack failed. block_num = %d\n", block_num);
        return -1;
    }

    return 0;
}

/**
 * @brief 发送错误包
 */
int tftp_send_error(tftp_t * tftp, uint16_t code) {
    tftp_pkt_t* pkt = &tftp->tx_packet;

    // opcde
    pkt->opcode = htons(TFTP_PKT_ERROR);

    // block
    pkt->err.code = htons(code);

    // message
    const char* msg = tftp_error_msg(code);
    strcpy(pkt->err.msg, msg);

    // 发包请求结果, 4 = opcode(2B) + block(2B) + msg
    int err = tftp_send_packet(tftp, pkt, 4 + (int)strlen(msg) + 1);
    if (err < 0) {
        printf( "tftp: send error failed. code = %d\n", code);
        return -1;
    }

    return 0;
}

/**
 * @brief 重发上次的数据包
 */
int tftp_resend(tftp_t * tftp) {
    tftp_pkt_t* pkt = &tftp->tx_packet;

    // 发包请求结果
    int err = tftp_send_packet(tftp, pkt, tftp->tx_size);
    if (err < 0) {
        printf( "tftp: resend error failed. \n");
        return -1;
    }

    return 0;
}

/**
 * @brief 解析收到的oack包
 */
int tftp_parse_oack (tftp_t * tftp) {
    char * buf = (char *)&tftp->rx_packet.oack.option;
    char * end = (char *)&tftp->rx_packet + sizeof(tftp_pkt_t);

    while ((buf < end) && *buf) {
        if (strcmp(buf, "blksize") == 0) {
            buf += strlen(buf) + 1;

            int blksize = atoi(buf);
            if (blksize == 0) {
                // 转换失败，发送错误
                printf("tftp: unknown blksize\n");
                tftp_send_error(tftp, TFTP_ERR_OPTION);
                return -1;
            } else if (blksize < tftp->block_size) {
                // 服务器建立用更小的值
                tftp->block_size = blksize;
                printf("tftp: use new blksize %d\n", blksize);
                return 0;
            } else if (blksize > tftp->block_size) {
                // 块大小更大
                printf("tftp: server use blksize %d\n", blksize);
                tftp_send_error(tftp, TFTP_ERR_OPTION);
                return -1;
            }

            buf += strlen(buf) + 1;
        } else if (strcmp(buf, "tsize") == 0) {
            buf += strlen(buf) + 1;
            tftp->file_size = atoi(buf);

            buf += strlen(buf) + 1;
        } else {
            buf += strlen(buf) + 1;
        }
    }

    return 0;
}

/**
 * @brief 等待期望类型的TFTP数据包的到达，非期望类型的包，直接丢弃并忽略
 */
int tftp_wait_packet(tftp_t * tftp, tftp_op_t op, uint16_t block, size_t * pkt_size) {
    tftp_pkt_t * pkt = &tftp->rx_packet;
    
    // 重设重传超时，之前已经发过一次了
    tftp->tmo_retry = TFTP_MAX_RETRY;

    while (1) {
        // 从套接字读取数据包
        socklen_t len = sizeof(tftp->remote);
        ssize_t size = recvfrom(tftp->socket, (uint8_t*)pkt, sizeof(tftp_pkt_t), 0, &tftp->remote, &len);
        if (size < TFTP_PACKET_MIN_SIZE) {
            // 非超时错误，退出
            if (size < 0) {
                printf("tftp: recv error");
                return -1;
            }

            // 超过超时重试次数，退出.否则，重发
            if (--tftp->tmo_retry == 0) {
                printf("tftp: wait timemout\n");
                return -1;
            } else if ((op != TFTP_PKT_RRQ) && (op != TFTP_PKT_WRQ) && (op != TFTP_PKT_REQ)) {
                // 非请求包，重发上次的包
                tftp_resend(tftp);
            }
            continue;
        } 
        
        *pkt_size = (size_t)size;
        uint16_t opcode = ntohs(pkt->opcode);

        // 错误包，打印，报错退出
        if (opcode == TFTP_PKT_ERROR) {
            pkt->err.msg[tftp->block_size - 1] = '\0';
            printf("tftp: recv error=%d, reason: %s\n", ntohs(pkt->err.code), pkt->err.msg);
            return -1;
        }

        // 检查操作码，类型不匹配则重新等
        if ((opcode != (op & 0xFF)) && (opcode != (op >> 8))) {
            tftp_resend(tftp);
            continue;
        }

        // 解析oack中的数据
        if (opcode == TFTP_PKT_OACK) {
            tftp_parse_oack(tftp);
            break;
        }

        // 检查block，如果不相同则继续
        if ((opcode == TFTP_PKT_ACK) || (opcode == TFTP_PKT_DATA)) {
            if (ntohs(pkt->data.block) != block) {
                tftp_resend(tftp);
                continue;
            }
        }

        break;
    };

    return 0;
}

