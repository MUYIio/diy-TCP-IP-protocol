// //
// // rtl8139.c
// //
// // RealTek RTL8129/RTL8139 PCI NIC network driver
// //
// // Written 1997-2002 by Donald Becker.
// // Ported to sanos 2002 by Michael Ringgaard.
// //
// // This software may be used and distributed according to the terms of
// // the GNU General Public License (GPL), incorporated herein by reference.
// // Drivers based on or derived from this code fall under the GPL and must
// // retain the authorship, copyright and license notice.  This file is not
// // a complete program and may only be used when the entire operating
// // system is licensed under the GPL.
// // 
// // This driver is for boards based on the RTL8129 and RTL8139 PCI ethernet
// // chips.
// // 
// // The author may be reached as becker@scyld.com, or C/O
// // Scyld Computing Corporation
// // 410 Severn Ave., Suite 210
// // Annapolis MD 21403
// // 
// // 以下是我根据自己的实际需求进行的改动
#ifndef RTL8139_H
#define RTL8139_H

#define PCI_ENABLE_BIT          0x80000000
#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC
/**
 * @brief RTL8139寄存器
 */
enum rtl8139_reg {
    IDR0            = 0x00,       // ID，共6个，从EEPROM加载，存储了MAC地址
    IDR1            = 0x01,       // ID，共6个，从EEPROM加载，存储了MAC地址
    IDR2            = 0x02,       // ID，共6个，从EEPROM加载，存储了MAC地址
    IDR3            = 0x03,       // ID，共6个，从EEPROM加载，存储了MAC地址
    IDR4            = 0x04,       // ID，共6个，从EEPROM加载，存储了MAC地址
    IDR5            = 0x05,       // ID，共6个，从EEPROM加载，存储了MAC地
    MAR0            = 0x08,       // 多播寄存器，共8个
    MAR1            = 0x09,       // 多播寄存器，共8个
    MAR2            = 0x0A,       // 多播寄存器，共8个
    MAR3            = 0x0B,       // 多播寄存器，共8个
    MAR4            = 0x0C,       // 多播寄存器，共8个
    MAR5            = 0x0D,       // 多播寄存器，共8个
    MAR6            = 0x0E,       // 多播寄存器，共8个
    MAR7            = 0x0F,       // 多播寄存器，共8
    TSD0            = 0x10,       // 发送状态，共4个32位 (Four 32bit registers)
    TSD1            = 0x14,       // 发送状态，共4个32位 (Four 32bit registers)
    TSD2            = 0x18,       // 发送状态，共4个32位 (Four 32bit registers)
    TSD3            = 0x1C,       // 发送状态，共4个32位 (Four 32bit registers
    TSAD0           = 0x20,       // Tx descriptors (also four 32bit)，起始地址，从0-3
    TSAD1           = 0x24,       // Tx descriptors (also four 32bit)，起始地址，从0-3
    TSAD2           = 0x28,       // Tx descriptors (also four 32bit)，起始地址，从0-3
    TSAD3           = 0x2C,       // Tx descriptors (also four 32bit)，起始地址，从0-
    RBSTART         = 0x30,      // 接收缓存起始
    ERBCR           = 0x34,      // Early Receive (Rx) Byte Count Register
    ERSR            = 0x36,      // Early Rx Status Register
    CR              = 0x37,      // Command Register
    CAPR            = 0x38,      // Current Address of Packet Read
    CBR             = 0x3A,      // Current Buffer Address: The initial value is 0000h.
    IMR             = 0x3C,      // Interrupt Mask Register
    ISR             = 0x3E,      // Interrupt Status Register
    TCR             = 0x40,      // Transmit (Tx) Configuration Register
    RCR             = 0x44,      // Receive (Rx) Configuration Register
    TCTR            = 0x48,      // A general-purpose counter
    MPC             = 0x4C,      // 24 bits valid, write clears
    EE_CR           = 0x50,      //
    CONFIG0         = 0x51, 
    CONFIG1         = 0x52,
    TIMERINT        = 0x54, 
    MSR             = 0x58, 
    CONFIG3         = 0x59, // PCI Grant signal select, PARM_EN, WOL, CLKRUN_EN, Fast Back to Back
    CONFIG4         = 0x5a, // RXFAutoClr, AnaOff, Long Wakeup Frames, LW/PMEB, LW pattern, Preboot Wakeup
    MULINT          = 0x5c, // Multiple Interrupt Select
    RERID           = 0x5e, // PCI Revision ID
    TSAD            = 0x60, // Transmit Status of All Descriptors

    // PHY registers
    BMCR            = 0x62, // Basic Mode Control Register                (MII 0)
    BMSR            = 0x64, // Basic Mode Status Register                 (MII 1)
    ANAR            = 0x66, // Auto-Negociation Advertisement Register    (MII 4)
    ANLPAR          = 0x68, // Auto-Negociation Link Partner Register     (MII 5)
    ANER            = 0x6a, // Auto-Negociation Expansion Register        (MII 6)
    DIS             = 0x6c, // Disconnect Counter
    FCSC            = 0x6e, // False Carrier Sense Counter
    NWAYTR          = 0x70, // N-Way Test Register
    RXERCNT         = 0x72, // RX ERror CouNTer
    CSCR            = 0x74, // Carrier Sense Configuration Register
    PHY1_PARM       = 0x78, // PHY Parameter 1                            (XXX MII 2?)
    TW_PARM         = 0x7c, // Twister Parameter
    PHY2_PARM       = 0x80, // PHY Parameter 2                            (XXX MII 3?)

    CRC0            = 0x84, // Power Management CRC register0 for Wakeup frame0
    CRC1            = 0x85, // Power Management CRC register1 for Wakeup frame1
    CRC2            = 0x86, // Power Management CRC register2 for Wakeup frame2
    CRC3            = 0x87, // Power Management CRC register3 for Wakeup frame3
    CRC4            = 0x88, // Power Management CRC register4 for Wakeup frame4
    CRC5            = 0x89, // Power Management CRC register5 for Wakeup frame5
    CRC6            = 0x8a, // Power Management CRC register6 for Wakeup frame6
    CRC7            = 0x8b, // Power Management CRC register7 for Wakeup frame7

    WAKEUP0         = 0x8c, // Power Management Wakeup frame0
    WAKEUP1         = 0x94, // Power Management Wakeup frame1
    WAKEUP2         = 0x9c, // Power Management Wakeup frame2
    WAKEUP3         = 0xa4, // Power Management Wakeup frame3
    WAKEUP4         = 0xac, // Power Management Wakeup frame4
    WAKEUP5         = 0xb4, // Power Management Wakeup frame5
    WAKEUP6         = 0xbc, // Power Management Wakeup frame6
    WAKEUP7         = 0xc4, // Power Management Wakeup frame7

    LSBCRC0         = 0xcc, // LSB of the mask byte of Wakeup frame0 in offset 12 to 75
    LSBCRC1         = 0xcd, // LSB of the mask byte of Wakeup frame1 in offset 12 to 75
    LSBCRC2         = 0xce, // LSB of the mask byte of Wakeup frame2 in offset 12 to 75
    LSBCRC3         = 0xcf, // LSB of the mask byte of Wakeup frame3 in offset 12 to 75
    LSBCRC4         = 0xd0, // LSB of the mask byte of Wakeup frame4 in offset 12 to 75
    LSBCRC5         = 0xd1, // LSB of the mask byte of Wakeup frame5 in offset 12 to 75
    LSBCRC6         = 0xd2, // LSB of the mask byte of Wakeup frame6 in offset 12 to 75
    LSBCRC7         = 0xd3, // LSB of the mask byte of Wakeup frame7 in offset 12 to 75
};

enum TSDx {
    TSD_GAP  = ( TSD1 - TSD0 ),
    TSD_CRS    = ( 1 << 31 ),       // Carrier Sense Lost during TX
    TSD_TABT   = ( 1 << 30 ),       // TX Abort, because TX Retry Count has been reached
    TSD_OWC    = ( 1 << 29 ),       // Out of Window Collision during TX
    TSD_CDH    = ( 1 << 28 ),       // NIC failed to send CD Heart Beat
    TSD_NCC    = ( 0xff << 24 ),    // Number of Collision Count
    TSD_ERTXTH_SHIFT = 16,          // Early TX Threshold (TX FIFO Threshold)
    TSD_ERTXTH = ( 0x3f << TSD_ERTXTH_SHIFT ),
    TSD_TOK    = ( 1 << 15 ),       // TX OK (TX happened successfully)
    TSD_TUN    = ( 1 << 14 ),       // TX Underrun (ISR can be TOK or TER)
    TSD_OWN    = ( 1 << 13 ),       // Own (DMA transfer completed)
    TSD_SIZE   = ( 0x1fff ),        // Size of Ethernet frame to send
};

// Transmit Start Address of Descriptor x Register
enum TSADx {
    TSAD_GAP  = ( TSAD1 - TSAD0 ),
};

// RX Status Register (this is not really a register)
// The status is in the RX packet header
enum RSR {
    RSR_MAR  = ( 1 << 15 ), // Multicast Address Received
    RSR_PAM  = ( 1 << 14 ), // Physical Address Matched
    RSR_BAR  = ( 1 << 13 ), // Broadcast Address Received
    // Reserved 12 -> 6
    RSR_ISE  = ( 1 << 5 ), // Invalid Symbol Error (100BASE-TX 5B/4B)
    RSR_RUNT = ( 1 << 4 ), // Runt packet received (size < 64 bytes)
    RSR_LONG = ( 1 << 3 ), // Long packet received (size > 4096 bytes)
    RSR_CRC  = ( 1 << 2 ), // CRC error
    RSR_FAE  = ( 1 << 1 ), // Frame Alignment Error
    RSR_ROK  = ( 1 << 0 ), // Receive OK
};

// Early RX Status Register
enum ERSR {
    // Reserved 7 -> 4
    ERSR_ERGOOD = ( 1 << 3 ), // Early RX Good Packet
    ERSR_ERBAD  = ( 1 << 2 ), // Early RX Bad Packet
    ERSR_EROVW  = ( 1 << 1 ), // Early RX OverWrite
    ERSR_EROK   = ( 1 << 0 ), // Set when ERBCR > RCR_ERTH. Auto cleared. Triggers ROK/RER
};

// Command Register
enum CR {
    // Reserved  7 -> 5
    CR_RST   = ( 1 << 4 ),
    CR_RE    = ( 1 << 3 ),
    CR_TE    = ( 1 << 2 ),
    // Reserved       1
    CR_BUFE  = ( 1 << 0 ),
};

// Interrupt (Mask or Status) Register
enum IMR_ISR {
    INT_SERR        = ( 1 << 15 ), // System Error (on the PCI Bus)
    INT_TIMEOUT     = ( 1 << 14 ), // Timeout Interrupt (see TCTR and TIMERINT)
    INT_LENCHG      = ( 1 << 13 ), // Cable Length Changed after RX has been enabled
    // Reserved        12 -> 7
    INT_FOVW        = ( 1 << 6 ), // RX FIFO Overflow
    INT_LNKCHG_PUN  = ( 1 << 5 ), // Link Change / RX Packet Underrun
    INT_RXOVW       = ( 1 << 4 ), // RX Buffer Overflow
    INT_TER         = ( 1 << 3 ), // TX Error (TX Underrun or TX Abort)
    INT_TOK         = ( 1 << 2 ), // TX OK
    INT_RER         = ( 1 << 1 ), // RX Error (CRC or frame alignment error)
    INT_ROK         = ( 1 << 0 ), // RX OK

    INT_TX          = INT_TOK | INT_TER,
    INT_RX          = INT_ROK | INT_RER | INT_RXOVW | INT_FOVW,
    INT_CLEAR       = 0xffff,
};

// TX Configuration Register
enum TCR {
    TCR_HWVERID_MASK    = 0x7cc00000, // Mask to fetch the chip version
    // Interframe Gap Time
    TCR_IFG_SHIFT       = 24,
    TCR_IFG_84      = ( 0 << TCR_IFG_SHIFT ), // 8.4 us / 840 ns
    TCR_IFG_88      = ( 1 << TCR_IFG_SHIFT ), // 8.8 us / 880 ns
    TCR_IFG_92      = ( 2 << TCR_IFG_SHIFT ), // 9.2 us / 920 ns
    TCR_IFG_96      = ( 3 << TCR_IFG_SHIFT ), // 9.6 us / 960 ns
    TCR_IFG_DEFAULT = TCR_IFG_96,
    // Loopback mode
    TCR_LBK_SHIFT       = 17,
    TCR_LBK_DISABLE = ( 0 << TCR_LBK_SHIFT ),
    TCR_LBK_ENABLE  = ( 3 << TCR_LBK_SHIFT ),   // Packets won't really be TXed
    // Append CRC FCS at the end of the frame?
    TCR_CRC             = ( 1 << 16 ),
    // Max DMA Burst (16 -> 2048 bytes)
    TCR_MXDMA_SHIFT     = 8,
    TCR_MXDMA_16    = ( 0 << TCR_MXDMA_SHIFT ),
    TCR_MXDMA_32    = ( 1 << TCR_MXDMA_SHIFT ),
    TCR_MXDMA_64    = ( 2 << TCR_MXDMA_SHIFT ),
    TCR_MXDMA_128   = ( 3 << TCR_MXDMA_SHIFT ),
    TCR_MXDMA_256   = ( 4 << TCR_MXDMA_SHIFT ),
    TCR_MXDMA_512   = ( 5 << TCR_MXDMA_SHIFT ),
    TCR_MXDMA_1024  = ( 6 << TCR_MXDMA_SHIFT ),
    TCR_MXDMA_2048  = ( 7 << TCR_MXDMA_SHIFT ),
};

// RX Configuration Register
enum RCR {
    // Early RX Threshold multiplier of the whole packet (0/16 -> 15/16)
    RCR_ERTH_SHIFT      = 24,
    RCR_ERTH        = ( 0xf << RCR_ERTH_SHIFT ),
    // Reserved        23 -> 18
    RCR_MUL_ER_INT      = ( 1 << 17 ),              // Multiple Early Interrupt
    RCR_RER8            = ( 1 << 16 ),              // Receive Error packets (8 <= size < 64)?
    // RX FIFO Threshold
    // Threshold = 2^(4 + RXFTH) bytes. 7: transfer when whole packet is in FIFO
    RCR_RXFTH_SHIFT     = 13,
        RCR_RXFTH       = ( 7 << RCR_RXFTH_SHIFT ),
    // RX Buffer Length
    // Size = 2^(3 + RBLEN) * 1024 + 16 bytes
    RCR_RBLEN_SHIFT     = 11,
    RCR_RBLEN_8K    = ( 0 << RCR_RBLEN_SHIFT ),
    RCR_RBLEN_16K   = ( 1 << RCR_RBLEN_SHIFT ),
    RCR_RBLEN_32K   = ( 2 << RCR_RBLEN_SHIFT ),
    RCR_RBLEN_64K   = ( 3 << RCR_RBLEN_SHIFT ),
    // Max RX DMA Burst Size
    // Burst size = 2^(4 + MXDMA) bytes. 7: unlimited burst size
    RCR_MXDMA_SHIFT     = 8,
    RCR_MXDMA_16    = ( 0 << RCR_MXDMA_SHIFT ),
    RCR_MXDMA_32    = ( 1 << RCR_MXDMA_SHIFT ),
    RCR_MXDMA_64    = ( 2 << RCR_MXDMA_SHIFT ),
    RCR_MXDMA_128   = ( 3 << RCR_MXDMA_SHIFT ),
    RCR_MXDMA_256   = ( 4 << RCR_MXDMA_SHIFT ),
    RCR_MXDMA_512   = ( 5 << RCR_MXDMA_SHIFT ),
    RCR_MXDMA_1024  = ( 6 << RCR_MXDMA_SHIFT ),
    RCR_MXDMA_NOLIM = ( 7 << RCR_MXDMA_SHIFT ),
    RCR_WRAP        = ( 1 << 7 ),
    // Reserved              6
    RCR_AER         = ( 1 << 5 ), // Accept ERror packets (CRC, align, collided)
    RCR_AR          = ( 1 << 4 ), // Accept Runt (packets < 64 bytes but >= 8 bytes)
    RCR_AB          = ( 1 << 3 ), // Accept Broadcast packets
    RCR_AM          = ( 1 << 2 ), // Accept Multicast packets
    RCR_APM         = ( 1 << 1 ), // Accept Physical Match packets
    RCR_AAP         = ( 1 << 0 ), // Accept All packets (promiscuous)
};

// EEPROM Control Register
enum EE {
    EE_CR_NORMAL    = 0x00, // Normal mode (network/host communication)
    EE_CR_AUTO_LOAD = 0x40, // 93C46 contents auto-loaded (take 2ms), then goes back to normal mode.
    EE_CR_PROGRAM   = 0x80, // EECS, EESK, EEDI, EEDO will reflect the real EEPROM pins
    EE_CR_EECS = 0x08, // Chip Select
    EE_CR_EESK = 0x04, // Serial Data Clock
    EE_CR_EEDI = 0x02, // Serial Data inbut
    EE_CR_EEDO = 0x01, // Serial Data Output
    // Following come from ATMEL AT93C46 EEPROM Datasheet
    // Start Bit | Opcode | Address [| Data]
    // The SB (Start Bit) is included in the following opcodes
    EE_CMD_READ  = 0x06, // Reads data stored in memory
    EE_CMD_EWEN  = 0x04, // Write enable (must precede all programming modes)
    EE_CMD_EWEN_ADDR  = 0x30,
    EE_CMD_ERASE = 0x07, // Erases memory location
    EE_CMD_WRITE = 0x05, // Writes memory location. After address: 16bits of data to write
    EE_CMD_ERAL  = 0x04, // Erases all memory locations
    EE_CMD_ERAL_ADDR  = 0x20,
    EE_CMD_WRAL  = 0x04, // Writes all memory locations. After address: 16bits of data to write
    EE_CMD_WRAL_ADDR  = 0x10,
    EE_CMD_EWDS  = 0x04, // Disables all programming instructions
    EE_CMD_EWDS_ADDR = 0x00,
    EE_ADDRLEN   = 0x06, // Size of address after the start bit and opcode
    EE_OPCODELEN = 0x03, // Size of opcode (with Start Bit included)
    EE_CMD_READ_LEN  = ( EE_OPCODELEN + EE_ADDRLEN ),
    // Address of data inside the EEPROM (in word)
    EE_DATA_MAC = 0x07,

    EE_CR_CFG_WRITE_ENABLE = 0xc0, // Unlock write access to IDR0~5, CONFIG0~4 and bit 13,12,8 of BCMR
};

// Configuration Register 1
enum CONFIG1
{
    // LEDS 0, 1 & 2 configuration (CFG1_LEDS_<0>_<1>_<2>)
    // On my PCI adaptor, only LED0 & 1 are connected. There is no LED2.
    CFG1_LEDS_SHIFT = 6,
    CFG1_LEDS_TXRX_LNK100_LNK10 = ( 0 << CFG1_LEDS_SHIFT ),
    CFG1_LEDS_TXRX_LNK_FDX      = ( 1 << CFG1_LEDS_SHIFT ),
    CFG1_LEDS_TX_LNK_RX         = ( 2 << CFG1_LEDS_SHIFT ),
    CFG1_LEDS_TX_LNK100_LNK10   = ( 3 << CFG1_LEDS_SHIFT ),
    CFG1_LEDS_MASK              = CFG1_LEDS_TX_LNK100_LNK10,
};

// Media Status Register
enum MSR {
    MSR_TXFCE      = ( 1 << 7 ), // TX Flow Control Enable
    MSR_RXFCE      = ( 1 << 6 ), // TX Flow Control Enable
    // Reserved             5
    MSR_AUX_STATUS = ( 1 << 4 ), // Auxiliary Power Present Status
    MSR_SPD_10     = ( 1 << 3 ), // 0: 100Mps, 1: 10Mps
    MSR_LINK_BAD   = ( 1 << 2 ), // 0: Link OK, 1: Link fail
    MSR_TXPF       = ( 1 << 1 ), // 0: Sends timer done packet, 1: Sends Pause Packet
    MSR_RXPF       = ( 1 << 0 ), // 0: Pause state clear, 1: Backoff state because Pause Packet received
};

// Maximum Ethernet frame size that can be handled by the device
#define R8139DN_MAX_ETH_SIZE 1792           // 1514还是???
#define R8139DN_MAX_MTU ( R8139DN_MAX_ETH_SIZE - ETH_HLEN - ETH_FCS_LEN )       // 发送的MTU

// Number and size of TX descriptors
#define R8139DN_TX_DESC_NB 4 // Warning: we use a property requiring this to be a power of 2
#define R8139DN_TX_DESC_SIZE R8139DN_MAX_ETH_SIZE       // 发送描述符，不包含其它结构
#define R8139DN_TX_DMA_SIZE (R8139DN_TX_DESC_SIZE * R8139DN_TX_DESC_NB)     // 整个缓存的长度

// RX DMA size
#define R8139DN_RX_PAD 16               
#define R8139DN_RX_BUFLEN (16384)         // 硬件配置，16KB+16B
#define R8139DN_RX_DMA_SIZE (R8139DN_RX_BUFLEN + R8139DN_RX_PAD + 2048)    // 整体长度，加上回绕的包长

#define R8139DN_RX_HEADER_SIZE 4

// 地址对齐，8字节
#define R8139DN_RX_ALIGN_ADD 3
#define R8139DN_RX_ALIGN_MASK ( ~R8139DN_RX_ALIGN_ADD )
#define R8139DN_RX_ALIGN(val) ( ( ( val ) + R8139DN_RX_ALIGN_ADD ) & R8139DN_RX_ALIGN_MASK )

#define CRC_LEN         4

typedef struct _rtl8139_rpkthdr_t {
    uint16_t ROK : 1;           // 接收到正确的包
    uint16_t FAE : 1;           // 对齐错误
    uint16_t CRC : 1;           // CRC错误
    uint16_t LONG : 1;          // 过长的包，超4KB
    uint16_t RUNT : 1;          // Runt包
    uint16_t ISE : 1;           // 不可用的符号错误？
    uint16_t : 6;   
    uint16_t BAR : 1;           // 广播包
    uint16_t PAM : 1;           // 单摠包
    uint16_t MAR : 1;           ///多播包 
    uint16_t size;              // 接收大小
}rtl8139_rpkthdr_t;

struct r8139dn_rx_header {
    uint16_t status;
    uint16_t size;
};

typedef struct _rtl8139_priv_t {
    int bus, device, func;          // PCI总线的位置
    int irq;
    uint32_t ioaddr;            // IO地址

    struct {
        uint8_t * data[R8139DN_TX_DESC_NB];
        uint8_t buffer[R8139DN_TX_DMA_SIZE];
        int curr, dirty;
        int free_count;
    }tx;

    struct {
        uint8_t * data;
        int curr;
        uint8_t buffer[R8139DN_RX_DMA_SIZE];
    }rx;
}rtl8139_priv_t;

extern const netif_ops_t netdev8139_ops;

void exception_handler_rtl8139 (void);

#endif // RTL8139_H
