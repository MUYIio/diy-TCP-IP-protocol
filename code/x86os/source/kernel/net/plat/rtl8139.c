// 参考资料：https://www.wfbsoftware.de/realtek-rtl8139-network-interface-card/
// https://wiki.osdev.org/RTL8139
// https://github.com/krisbellemans/rtl8139/blob/master/rtl8139.c
// http://www.jbox.dk/sanos/source/sys/dev/rtl8139.c.html
#include "netif.h"
#include "dbg.h"
#include "sys.h"
#include "ether.h"
#include "net_cfg.h"
#include "rtl8139.h"
#include "comm/cpu_instr.h"
#include "cpu/irq.h"

#define PCI_CONFIG_ADDR                 0xCF8
#define PCI_CONFIG_DATA                 0xCFC

#define PCI_CONFIG_CMD_STAT             0x04

#if defined(NET_DRIVER_RTL8139)

static inline uint8_t pci_read_byte(int busno, int devno, int funcno, int addr) {
  outl(PCI_CONFIG_ADDR, ((uint32_t) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  return inb(PCI_CONFIG_DATA);
}

static inline uint16_t pci_read_word(int busno, int devno, int funcno, int addr) {
  outl(PCI_CONFIG_ADDR, ((uint32_t) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  return inw(PCI_CONFIG_DATA);
}

static inline uint32_t pci_read_dword(int busno, int devno, int funcno, int addr) {
  outl(PCI_CONFIG_ADDR, ((uint32_t) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  return inl(PCI_CONFIG_DATA);
}

static inline void pci_write_byte(int busno, int devno, int funcno, int addr, uint8_t value) {
  outl(PCI_CONFIG_ADDR, ((uint32_t) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  outb(PCI_CONFIG_DATA, value);
}

static inline void pci_write_word(int busno, int devno, int funcno, int addr, uint16_t value) {
  outl(PCI_CONFIG_ADDR, ((uint32_t) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  outw(PCI_CONFIG_DATA, value);
}

static inline void pci_write_dword(int busno, int devno, int funcno, int addr, uint32_t value) {
  outl(PCI_CONFIG_ADDR, ((uint32_t) 0x80000000 | (busno << 16) | (devno << 11) | (funcno << 8) | addr));
  outl(PCI_CONFIG_DATA, value);
}

uint8_t pci_read_config_byte(int busno, int devno, int funcno, int addr) {
  return pci_read_byte(busno, devno, funcno, addr); 
}

uint16_t pci_read_config_word(int busno, int devno, int funcno, int addr) {
  return pci_read_word(busno, devno, funcno, addr); 
}

uint32_t pci_read_config_dword(int busno, int devno, int funcno, int addr) {
  return pci_read_dword(busno, devno, funcno, addr); 
}

void pci_write_config_byte(int busno, int devno, int funcno, int addr,uint8_t value) {
  pci_write_byte(busno, devno, funcno, addr, value); 
}

void pci_write_config_word(int busno, int devno, int funcno, int addr, uint16_t value) {
  pci_write_word(busno, devno, funcno, addr, value); 
}

void pci_write_config_dword(int busno, int devno, int funcno, int addr, uint32_t value) {
  pci_write_dword(busno, devno, funcno, addr, value); 
}

void pci_enable_busmastering(int busno, int devno, int funcno) {
  uint32_t value;

  value = pci_read_config_dword(busno, devno, funcno, PCI_CONFIG_CMD_STAT);
  value |= 0x00000004;
  pci_write_config_dword(busno, devno, funcno, PCI_CONFIG_CMD_STAT, value);
}

#define NETIF_NR           2
static netif_t * netif_tbl[NETIF_NR];          // 支持的netif表

/**
 * @brief 硬件复位RTL8139
 */
int rtl8139_reset (rtl8139_priv_t * priv) {
    // 发送复位命令
    int i;
    outb(priv->ioaddr + CR, CR_RST);
    for (i = 100000; i > 0; i--) {
        if (!((inb(priv->ioaddr + CR) & CR_RST))) {
            log_printf("rtl8139 reset complete");
            break;
        }
    }

    // 复位失败
    if (i == 0) {
        log_printf("rtl8139 reset failed.");
        return -1;
    }

    return 0;
}

// Enable the transmitter, set up the transmission settings
// Tell hardware where to DMA
void rtl8139_setup_tx (rtl8139_priv_t * priv ) {    
    uint8_t cr = inb(priv->ioaddr + CR);

    // 打开发送允许位
    outb(priv->ioaddr + CR, cr | CR_TE);

    // 配置发送相关的模型，960ns用于100mbps，添加CRC，DMA 1024B
    outl(priv->ioaddr + TCR,  TCR_IFG_96 | TCR_MXDMA_1024);

    // 配置发送缓存,共4个缓冲区，并设备各个缓冲区首地址TSAD0-TSAD3
    for (int i = 0; i < R8139DN_TX_DESC_NB ; i++) {
        uint32_t addr = (uint32_t)(priv->tx.buffer + i * R8139DN_TX_DESC_SIZE);
        priv->tx.data[i] = (uint8_t *)addr;
        outl(priv->ioaddr + TSAD0 + i * TSAD_GAP, addr);
    }

    // 指向第0个缓存包
    priv->tx.curr = 0;
    priv->tx.free_count = R8139DN_TX_DESC_NB;
}

// Enable the receiver, set up the reception settings
// Tell hardware where to DMA
void rtl8139_setup_rx (rtl8139_priv_t * priv ) {
    // 设置接收缓存
    priv->rx.data = priv->rx.buffer;

    // Multiple Interrupt Select Register?
    outw(priv->ioaddr + MULINT, 0);

    // 接收缓存起始地址
    outl(priv->ioaddr + RBSTART, (uint32_t)priv->rx.data);

    // 接收配置：广播+单播匹配、DMA bust 1024？接收缓存长16KB+16字节
    outl(priv->ioaddr + RCR, RCR_MXDMA_1024 | RCR_APM | RCR_AB | RCR_RBLEN_16K | RCR_WRAP);

    // 允许接收数据
    uint8_t cr = inb(priv->ioaddr + CR);
    outb(priv->ioaddr + CR, cr | CR_RE );
}

// func - 0-7
// slot - 0-31
// bus - 0-255
//
// described here: https://en.wikipedia.org/wiki/PCI_configuration_space under
// the section "software implementation"
// parameter pcireg: 0 will read the first 32bit dword of the pci control space
// which is DeviceID and Vendor ID
// pcireg = 1 will read the second 32bit dword which is status and command
// and so on...
uint32_t r_pci_32(uint8_t bus, uint8_t device, uint8_t func, uint8_t pcireg) {
    // compute the index
    //
    // pcireg is left shifted twice to multiply it by 4 because each register
    // is 4 byte long (32 bit registers)
    uint32_t index = PCI_ENABLE_BIT | (bus << 16) | (device << 11) | (func << 8) | (pcireg << 2);

    // write the index value onto the index port
    outl(PCI_CONFIG_ADDRESS, index);
    //outl(index, PCI_CONFIG_ADDRESS);

    // read a value from the data port
    return inl(PCI_CONFIG_DATA);
}

/**
 * @brief 遍历RTL8139
 * 参考资料：https://wiki.osdev.org/PCI
 */
static int rtl8139_probe (rtl8139_priv_t * priv, int index) {
    int total_found = 0;

    // 遍历256个bus
    for (int bus = 0; bus != 0xff; bus++) {
        // 遍历每条bus上最多32个设备
        for (int device = 0; device < 32; device++) {
            // 遍历每个设备上的8个功能号
            for (int func = 0; func < 8; func++) {

                // read the first dword (index 0) from the PCI configuration space
                // dword 0 contains the vendor and device values from the PCI configuration space
                uint32_t data = r_pci_32(bus, device, func, 0);
                if (data == 0xffffffff) {
                    continue;
                }

                // 取厂商和设备号，不是RTL8139则跳过
                uint16_t device_value = (data >> 16);
                uint16_t vendor = data & 0xFFFF;
                if ((vendor != 0x10ec) || (device_value != 0x8139)) {
                    continue;
                }

                // 取指定的序号
                if (total_found++ != index) {
                    continue;
                }

                // 取中断相关的配置, 测试读的是值为IRQ11(0xB)
                data = pci_read_config_dword(bus, device, func, 0x3C);

                // 获取IO地址
                priv->bus = bus;
                priv->device = device;
                priv->func = func;
                priv->irq = (data & 0xF) + IRQ0_BASE;
                priv->ioaddr = r_pci_32(bus, device, func, 4) & ~3;
                log_printf("RTL8139 found! bus: %d, device: %d, func: %d, ioaddr: %x", 
                                    bus, device, func, priv->ioaddr);
                return 1;
            }
        }
    }
    return 0;
}

/**
 * 网络设备打开
 */
static net_err_t rtl8139_open(struct _netif_t* netif, void* ops_data) {    
    static int idx = 0;
    rtl8139_priv_t * priv = (rtl8139_priv_t *)ops_data;

    if (idx >= NETIF_NR) {
        dbg_error(DBG_NETIF, "too many netif open");
        return NET_ERR_PARAM;
    }

    // 遍历查找网络接口
    int find = rtl8139_probe(priv, idx);
    if (find == 0) {
        log_printf("open netdev%d failed: rtl8139", idx);
        return NET_ERR_NOT_EXIST;
    }

    // 启动PCI总线，以便DMA能够传输网卡数据
    pci_enable_busmastering(priv->bus, priv->device, priv->func);

    // 给网卡通电
    outb(priv->ioaddr + CONFIG1, 0);

    // 开始8139, 使LWAKE + LWPTN变高
    outb(priv->ioaddr + CONFIG1, 0x0);
    log_printf("enable rtl8139");

    // 复位网卡
    if (rtl8139_reset(priv) < 0) {
        goto open_error;
    }

    // 配置复位
    rtl8139_setup_tx(priv);
    rtl8139_setup_rx(priv);

    // 读取mac地址。这里读取的值应当和qemu中网卡中配置的相同
    netif->hwaddr.addr[0] = inb(priv->ioaddr + 0);
    netif->hwaddr.addr[1] = inb(priv->ioaddr + 1);
    netif->hwaddr.addr[2] = inb(priv->ioaddr + 2);
    netif->hwaddr.addr[3] = inb(priv->ioaddr + 3);
    netif->hwaddr.addr[4] = inb(priv->ioaddr + 4);
    netif->hwaddr.addr[5] = inb(priv->ioaddr + 5);
    netif->hwaddr.len = 6;

    netif->type = NETIF_TYPE_ETHER;
    netif->mtu = ETH_MTU; 

    // 系统还没初始化完毕, 启用所有与接收和发送相关的中断
    outw(priv->ioaddr + IMR, 0xFFFF);       
    irq_install(priv->irq, exception_handler_rtl8139);
    irq_enable(priv->irq);

    netif_tbl[idx++]= netif;
    return NET_ERR_OK;
open_error:
    log_printf("rtl8139 open error");
    return NET_ERR_IO;
}

/**
 * 关闭pcap网络接口
 * @param netif 待关闭的接口
 */
static void rtl8139_close(struct _netif_t* netif) {
    // 不支持了，让网卡一直在运行吧
}
 
/**
 * 向接口发送命令
 */
static net_err_t rtl8139_xmit (struct _netif_t* netif) {
    rtl8139_priv_t * priv = (rtl8139_priv_t *)netif->ops_data;

    // 如果发送缓存已经满，即写入比较快，退出，让中断自行处理发送过程
    if (priv->tx.free_count <= 0) {
        dbg_info(DBG_NETIF, "rtl8139 tx buffer full");
        return NET_ERR_OK;
    }

    // 取数据包
    pktbuf_t* buf = netif_get_out(netif, 0);
    dbg_info(DBG_NETIF, "rtl8139 send packeet: %d", buf->total_size);

    // 禁止响应中断，以便进行一定程序的保护
    irq_state_t state = irq_enter_protection();

    // 写入发送缓存，不必调整大小，上面已经调整好
    int len = buf->total_size;
    pktbuf_read(buf, priv->tx.data[priv->tx.curr], len);
    pktbuf_free(buf);

    // 启动整个发送过程, 只需要写TSD，TSAD已经在发送时完成
    // 104 bytes of early TX threshold，104字节才开始发送??，以及发送的包的字节长度，清除OWN位
    uint32_t tx_flags = (1 << TSD_ERTXTH_SHIFT) | len;
    outl(priv->ioaddr + TSD0 + priv->tx.curr * TSD_GAP, tx_flags);

    // 取下一个描述符f地址
    priv->tx.curr += 1;
    if (priv->tx.curr >= R8139DN_TX_DESC_NB) {
        priv->tx.curr = 0;
    }

    // 减小空闲计数
    priv->tx.free_count--;

    irq_leave_protection(state);
    return NET_ERR_OK;
}


/**
 * @brief RTL8139中断处理
 * 如果在中断处理中区分当前所用的是哪个网卡？
 */
void do_handler_rtl8139 (void) {
    dbg_info(DBG_NETIF, "rtl8139 interrupt in");
    int curr_irq = -1;

    // 遍历找到对应的netif进行处理
    for (int i = 0; i < NETIF_NR; i++) {
        netif_t * netif = netif_tbl[i];
        rtl8139_priv_t * priv = (rtl8139_priv_t *)netif->ops_data;

        // 先读出所有中断，然后清除中断寄存器的状态
        uint16_t isr = inw(priv->ioaddr + ISR);
        if (isr == 0) {
            continue;
        }
        
        curr_irq = priv->irq;
        outw(priv->ioaddr + ISR, 0xFFFF);
        
        // 先处理发送完成和错误
        if ((isr & INT_TOK) || (isr & INT_TER)) {
            dbg_info(DBG_NETIF, "tx interrupt");

            // 尽可能取出多的数据包，然后将缓存填满
            while (priv->tx.free_count < 4) {
                // 取dirty位置的发送状态, 如果发送未完成，即无法写了，那么就退出
                uint32_t tsd = inl(priv->ioaddr + TSD0 + (priv->tx.dirty * TSD_GAP));
                if (!(tsd & (TSD_TOK | TSD_TABT | TSD_TUN))) {
                    dbg_info(DBG_NETIF, "send error, tsd: %d", tsd);
                    break;
                }

                // 增加计数
                if (++priv->tx.dirty >= R8139DN_TX_DESC_NB) {
                    priv->tx.dirty = 0;
                }

                // 尝试取包, 没有数据包则继续扫下一个
                pktbuf_t * buf = netif_get_out(netif, -1);
                if (!buf) {
                    priv->tx.free_count++;
                    continue;;
                }

                // 写入发送缓存，不必调整大小，上面已经调整好
                int len = buf->total_size;
                pktbuf_read(buf, priv->tx.data[priv->tx.curr], len);
                pktbuf_free(buf);

                // 启动整个发送过程, 只需要写TSD，TSAD已经在发送时完成
                // 104 bytes of early TX threshold，104字节才开始发送??，以及发送的包的字节长度，清除OWN位
                uint32_t tx_flags = (1 << TSD_ERTXTH_SHIFT) | len;
                outl(priv->ioaddr + TSD0 + priv->tx.curr * TSD_GAP, tx_flags);

                // 取下一个描述符f地址
                priv->tx.curr += 1;
                if (priv->tx.curr >= R8139DN_TX_DESC_NB) {
                    priv->tx.curr = 0;
                }

                dbg_info(DBG_NETIF, "after send, curr: %d, dirty: %d, free_count: %d", 
                        priv->tx.curr, priv->tx.dirty, priv->tx.free_count);
            }
        }

        // 出现接收错误
        if (isr & INT_RER) {
            dbg_info(DBG_NETIF, "rtl8139 recv error");
        } 
        
        // 对于接收的测试，可以ping本地广播来测试。否则windows上，ping有时会不发包，而是要过一段时间，难得等
        if (isr & INT_ROK) {
            dbg_info(DBG_NETIF, "rtl8139 recv ok");

            // 循环读取，直到缓存为空
            while ((inb(priv->ioaddr + CR) & CR_BUFE) == 0) {
                rtl8139_rpkthdr_t *hdr = (rtl8139_rpkthdr_t *)(priv->rx.data + priv->rx.curr);

                // 进行一些简单的错误检查，有问题的包就不要写入接收队列了
                if (hdr->FAE || hdr->CRC || hdr->LONG || hdr->RUNT || hdr->ISE) {
                    dbg_error(DBG_NETIF, "%s: rcv pkt error: %x", netif->name);
                    // 重新对接收进行设置，重启。还是忽略这个包，还是忽略吧
                    rtl8139_setup_rx(priv);
                } else {
        
                    // 分配包缓存空间
                    int pkt_size = hdr->size - CRC_LEN;
                    pktbuf_t * buf = pktbuf_alloc(pkt_size);
                    if (!buf) {
                        // 包不够则丢弃
                        dbg_info(DBG_NETIF, "no buffer");
                        break;
                    }

                    // 这里要考虑跨包拷贝的情况
                    if (priv->rx.curr + hdr->size > R8139DN_RX_DMA_SIZE) {
                        // 分割了两块空间
                        int count = R8139DN_RX_DMA_SIZE - priv->rx.curr - sizeof(rtl8139_rpkthdr_t);
                        pktbuf_write(buf, priv->rx.data + priv->rx.curr + sizeof(rtl8139_rpkthdr_t), count);        // 第一部分到尾部
                        pktbuf_write(buf, priv->rx.data, pkt_size - count); 
                        dbg_info(DBG_NETIF, "copy twice, rx.curr: %d", priv->rx.curr);
                    } else {
                        // 跳过开头的4个字节的接收状态头
                        pktbuf_write(buf, priv->rx.data + priv->rx.curr + sizeof(rtl8139_rpkthdr_t), pkt_size);
                    }

                    // 将收到的包发送队列
                    netif_put_in(netif, buf, -1);

                    // 移至下一个包
                    priv->rx.curr = (priv->rx.curr + hdr->size + sizeof(rtl8139_rpkthdr_t) + 3) & ~3;       // 4字节对齐
                    dbg_info(DBG_NETIF, "recv end, rx.curr: %d", priv->rx.curr);

                    // 调整curr在整个有效的缓存空间内
                    if (priv->rx.curr > R8139DN_RX_BUFLEN) {
                        dbg_info(DBG_NETIF, "priv->rx.curr >= R8139DN_RX_DMA_SIZE");
                        priv->rx.curr -= R8139DN_RX_BUFLEN;
                    }
                
                    // 更新接收地址，通知网卡可以继续接收数据
                    outw(priv->ioaddr + CAPR, priv->rx.curr - 16);
                }
            }
        }
    }
    // 别忘了这个，不然中断不会响应
    if (curr_irq > 0) {
        pic_send_eoi(curr_irq);
    }
}

/**
 * pcap驱动结构
 */
const netif_ops_t netdev8139_ops = {
    .open = rtl8139_open,
    .close = rtl8139_close,
    .xmit = rtl8139_xmit,
};
#endif 