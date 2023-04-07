#include "lan8720.h"
#include "stm32f4x7_eth.h"
#include "usart.h"
#include "delay.h"
#include "os_api.h"

__align(4) ETH_DMADESCTypeDef *DMARxDscrTab; // 锟斤拷太锟斤拷DMA锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟捷结构锟斤拷指锟斤拷
__align(4) ETH_DMADESCTypeDef *DMATxDscrTab; // 锟斤拷太锟斤拷DMA锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟捷结构锟斤拷指锟斤拷
__align(4) uint8_t *Rx_Buff;				 // 锟斤拷太锟斤拷锟阶诧拷锟斤拷锟斤拷锟斤拷锟斤拷buffers指锟斤拷
__align(4) uint8_t *Tx_Buff;				 // 锟斤拷太锟斤拷锟阶诧拷锟斤拷锟斤拷锟斤拷锟斤拷buffers指锟斤拷

static void ETHERNET_NVICConfiguration(void);
// LAN8720锟斤拷始锟斤拷
// 锟斤拷锟斤拷值:0,锟缴癸拷;
//     锟斤拷锟斤拷,失锟斤拷
u8 LAN8720_Init(void)
{
	u8 rval = 0;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOG, ENABLE); // 使锟斤拷GPIO时锟斤拷 RMII锟接匡拷
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);												// 使锟斤拷SYSCFG时锟斤拷

	SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII); // MAC锟斤拷PHY之锟斤拷使锟斤拷RMII锟接匡拷

	/*锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 RMII锟接匡拷
	  ETH_MDIO -------------------------> PA2
	  ETH_MDC --------------------------> PC1
	  ETH_RMII_REF_CLK------------------> PA1
	  ETH_RMII_CRS_DV ------------------> PA7
	  ETH_RMII_RXD0 --------------------> PC4
	  ETH_RMII_RXD1 --------------------> PC5
	  ETH_RMII_TX_EN -------------------> PG11
	  ETH_RMII_TXD0 --------------------> PG13
	  ETH_RMII_TXD1 --------------------> PG14
	  ETH_RESET-------------------------> PD3*/

	// 锟斤拷锟斤拷PA1 PA2 PA7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH); // 锟斤拷锟脚革拷锟矫碉拷锟斤拷锟斤拷涌锟斤拷锟�
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

	// 锟斤拷锟斤拷PC1,PC4 and PC5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH); // 锟斤拷锟脚革拷锟矫碉拷锟斤拷锟斤拷涌锟斤拷锟�
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);

	// 锟斤拷锟斤拷PG11, PG14 and PG13
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource13, GPIO_AF_ETH);
	GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);

	// 锟斤拷锟斤拷PD3为锟斤拷锟斤拷锟斤拷锟�
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // 锟斤拷锟斤拷锟斤拷锟�
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	LAN8720_RST = 0; // 硬锟斤拷锟斤拷位LAN8720
	os_task_sleep(50);
	LAN8720_RST = 1; // 锟斤拷位锟斤拷锟斤拷
	ETHERNET_NVICConfiguration();
	rval = ETH_MACDMA_Config();
	return !rval; // ETH锟侥癸拷锟斤拷为:0,失锟斤拷;1,锟缴癸拷;锟斤拷锟斤拷要取锟斤拷一锟斤拷
}

// 锟斤拷太锟斤拷锟叫断凤拷锟斤拷锟斤拷锟斤拷
void ETHERNET_NVICConfiguration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;				 // 锟斤拷太锟斤拷锟叫讹拷
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0X00; // 锟叫断寄达拷锟斤拷锟斤拷2锟斤拷锟斤拷锟斤拷燃锟�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0X00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// 锟矫碉拷8720锟斤拷锟劫讹拷模式
// 锟斤拷锟斤拷值:
// 001:10M锟斤拷双锟斤拷
// 101:10M全双锟斤拷
// 010:100M锟斤拷双锟斤拷
// 110:100M全双锟斤拷
// 锟斤拷锟斤拷:锟斤拷锟斤拷.
u8 LAN8720_Get_Speed(void)
{
	u8 speed;
	speed = ((ETH_ReadPHYRegister(0x00, 31) & 0x1C) >> 2); // 锟斤拷LAN8720锟斤拷31锟脚寄达拷锟斤拷锟叫讹拷取锟斤拷锟斤拷锟劫度猴拷双锟斤拷模式
	return speed;
}
/////////////////////////////////////////////////////////////////////////////////////////////////
// 锟斤拷锟铰诧拷锟斤拷为STM32F407锟斤拷锟斤拷锟斤拷锟斤拷/锟接口猴拷锟斤拷.

// 锟斤拷始锟斤拷ETH MAC锟姐及DMA锟斤拷锟斤拷
// 锟斤拷锟斤拷值:ETH_ERROR,锟斤拷锟斤拷失锟斤拷(0)
//		ETH_SUCCESS,锟斤拷锟酵成癸拷(1)
u8 ETH_MACDMA_Config(void)
{
	ETH_InitTypeDef ETH_InitStructure;

	// 使锟斤拷锟斤拷太锟斤拷时锟斤拷
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_ETH_MAC | RCC_AHB1Periph_ETH_MAC_Tx | RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);

	ETH_DeInit();		 // AHB锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷太锟斤拷
	ETH_SoftwareReset(); // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷
	while (ETH_GetSoftwareResetStatus() == SET)
		;								// 锟饺达拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟�
	ETH_StructInit(&ETH_InitStructure); // 锟斤拷始锟斤拷锟斤拷锟斤拷为默锟斤拷值

	/// 锟斤拷锟斤拷MAC锟斤拷锟斤拷锟斤拷锟斤拷
	ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;					  // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷应锟斤拷锟斤拷
	ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;						  // 锟截闭凤拷锟斤拷
	ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;			  // 锟截憋拷锟截达拷锟斤拷锟斤拷
	ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;		  // 锟截憋拷锟皆讹拷去锟斤拷PDA/CRC锟斤拷锟斤拷
	ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;							  // 锟截闭斤拷锟斤拷锟斤拷锟叫碉拷帧
	ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable; // 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟叫广播帧
	ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;				  // 锟截闭伙拷锟侥Ｊ斤拷牡锟街凤拷锟斤拷锟�
	ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;	  // 锟斤拷锟斤拷锟介播锟斤拷址使锟斤拷锟斤拷锟斤拷锟斤拷址锟斤拷锟斤拷
	ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;		  // 锟皆碉拷锟斤拷锟斤拷址使锟斤拷锟斤拷锟斤拷锟斤拷址锟斤拷锟斤拷
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable; // 锟斤拷锟斤拷ipv4锟斤拷TCP/UDP/ICMP锟斤拷帧校锟斤拷锟叫讹拷锟�
#endif
	// 锟斤拷锟斤拷锟斤拷使锟斤拷帧校锟斤拷锟叫讹拷毓锟斤拷艿锟绞憋拷锟揭伙拷锟揭癸拷艽娲⒆拷锟侥Ｊ�,锟芥储转锟斤拷模式锟斤拷要锟斤拷证锟斤拷锟斤拷帧锟芥储锟斤拷FIFO锟斤拷,
	// 锟斤拷锟斤拷MAC锟杰诧拷锟斤拷/识锟斤拷锟街⌒ｏ拷锟街�,锟斤拷锟斤拷校锟斤拷锟斤拷确锟斤拷时锟斤拷DMA锟酵匡拷锟皆达拷锟斤拷帧,锟斤拷锟斤拷投锟斤拷锟斤拷锟斤拷锟街�
	ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; // 锟斤拷锟斤拷锟斤拷锟斤拷TCP/IP锟斤拷锟斤拷帧
	ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;					// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟捷的存储转锟斤拷模式
	ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;				// 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟捷的存储转锟斤拷模式
	ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;					 // 锟斤拷止转锟斤拷锟斤拷锟斤拷帧
	ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable; // 锟斤拷转锟斤拷锟斤拷小锟侥猴拷帧
	ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;					 // 锟津开达拷锟斤拷锟节讹拷帧锟斤拷锟斤拷
	ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;					 // 锟斤拷锟斤拷DMA锟斤拷锟斤拷牡锟街凤拷锟斤拷牍︼拷锟�
	ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;									 // 锟斤拷锟斤拷锟教讹拷突锟斤拷锟斤拷锟斤拷
	ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;						 // DMA锟斤拷锟酵碉拷锟斤拷锟酵伙拷锟斤拷锟斤拷锟轿�32锟斤拷锟斤拷锟斤拷
	ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;						 // DMA锟斤拷锟秸碉拷锟斤拷锟酵伙拷锟斤拷锟斤拷锟轿�32锟斤拷锟斤拷锟斤拷
	ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;
	uint32_t err = ETH_Init(&ETH_InitStructure, LAN8720_PHY_ADDRESS); // 锟斤拷锟斤拷ETH
	if (err == ETH_SUCCESS) {
		// 启用收发、普通中断
		ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_T | ETH_DMA_IT_R, ENABLE);

	}
	return err;
}

// 锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷锟捷帮拷
// 锟斤拷锟斤拷值:锟斤拷锟斤拷锟斤拷锟捷帮拷帧锟结构锟斤拷
FrameTypeDef ETH_Rx_Packet(void)
{
	u32 framelength = 0;
	FrameTypeDef frame = {0, 0};
	// 锟斤拷榈鼻帮拷锟斤拷锟斤拷锟�,锟角凤拷锟斤拷锟斤拷ETHERNET DMA(锟斤拷锟矫碉拷时锟斤拷)/CPU(锟斤拷位锟斤拷时锟斤拷)
	if ((DMARxDescToGet->Status & ETH_DMARxDesc_OWN) != (u32)RESET)
	{
		frame.length = ETH_ERROR;
		if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)
		{
			ETH->DMASR = ETH_DMASR_RBUS; // 锟斤拷锟紼TH DMA锟斤拷RBUS位
			ETH->DMARPDR = 0;			 // 锟街革拷DMA锟斤拷锟斤拷
		}
		return frame; // 锟斤拷锟斤拷,OWN位锟斤拷锟斤拷锟斤拷锟斤拷
	}
	if (((DMARxDescToGet->Status & ETH_DMARxDesc_ES) == (u32)RESET) &&
		((DMARxDescToGet->Status & ETH_DMARxDesc_LS) != (u32)RESET) &&
		((DMARxDescToGet->Status & ETH_DMARxDesc_FS) != (u32)RESET))
	{
		framelength = ((DMARxDescToGet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4; // 锟矫碉拷锟斤拷锟秸帮拷帧锟斤拷锟斤拷(锟斤拷锟斤拷锟斤拷4锟街斤拷CRC)
		frame.buffer = DMARxDescToGet->Buffer1Addr;														   // 锟矫碉拷锟斤拷锟斤拷锟斤拷锟斤拷锟节碉拷位锟斤拷
	}
	else
		framelength = ETH_ERROR; // 锟斤拷锟斤拷
	frame.length = framelength;
	frame.descriptor = DMARxDescToGet;
	// 锟斤拷锟斤拷ETH DMA全锟斤拷Rx锟斤拷锟斤拷锟斤拷为锟斤拷一锟斤拷Rx锟斤拷锟斤拷锟斤拷
	// 为锟斤拷一锟斤拷buffer锟斤拷取锟斤拷锟斤拷锟斤拷一锟斤拷DMA Rx锟斤拷锟斤拷锟斤拷
	DMARxDescToGet = (ETH_DMADESCTypeDef *)(DMARxDescToGet->Buffer2NextDescAddr);
	return frame;
}
// 锟斤拷锟斤拷一锟斤拷锟斤拷锟斤拷锟斤拷锟捷帮拷
// FrameLength:锟斤拷锟捷帮拷锟斤拷锟斤拷
// 锟斤拷锟斤拷值:ETH_ERROR,锟斤拷锟斤拷失锟斤拷(0)
//		ETH_SUCCESS,锟斤拷锟酵成癸拷(1)
u8 ETH_Tx_Packet(u16 FrameLength)
{
	// 锟斤拷榈鼻帮拷锟斤拷锟斤拷锟�,锟角凤拷锟斤拷锟斤拷ETHERNET DMA(锟斤拷锟矫碉拷时锟斤拷)/CPU(锟斤拷位锟斤拷时锟斤拷)
	if ((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
		return ETH_ERROR;													// 锟斤拷锟斤拷,OWN位锟斤拷锟斤拷锟斤拷锟斤拷
	DMATxDescToSet->ControlBufferSize = (FrameLength & ETH_DMATxDesc_TBS1); // 锟斤拷锟斤拷帧锟斤拷锟斤拷,bits[12:0]
	DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;			// 锟斤拷锟斤拷锟斤拷锟揭伙拷锟斤拷偷锟揭伙拷锟轿伙拷锟斤拷锟轿�(1锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷一帧)
	DMATxDescToSet->Status |= ETH_DMATxDesc_OWN | ETH_DMATxDesc_IC;							// 锟斤拷锟斤拷Tx锟斤拷锟斤拷锟斤拷锟斤拷OWN位,buffer锟截癸拷ETH DMA
	if ((ETH->DMASR & ETH_DMASR_TBUS) != (u32)RESET)						// 锟斤拷Tx Buffer锟斤拷锟斤拷锟斤拷位(TBUS)锟斤拷锟斤拷锟矫碉拷时锟斤拷,锟斤拷锟斤拷锟斤拷.锟街革拷锟斤拷锟斤拷
	{
		ETH->DMASR = ETH_DMASR_TBUS; // 锟斤拷锟斤拷ETH DMA TBUS位
		ETH->DMATPDR = 0;			 // 锟街革拷DMA锟斤拷锟斤拷
	}
	// 锟斤拷锟斤拷ETH DMA全锟斤拷Tx锟斤拷锟斤拷锟斤拷为锟斤拷一锟斤拷Tx锟斤拷锟斤拷锟斤拷
	// 为锟斤拷一锟斤拷buffer锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷一锟斤拷DMA Tx锟斤拷锟斤拷锟斤拷
	DMATxDescToSet = (ETH_DMADESCTypeDef *)(DMATxDescToSet->Buffer2NextDescAddr);
	return ETH_SUCCESS;
}
// 锟矫碉拷锟斤拷前锟斤拷锟斤拷锟斤拷锟斤拷Tx buffer锟斤拷址
// 锟斤拷锟斤拷值:Tx buffer锟斤拷址
u32 ETH_GetCurrentTxBuffer(void)
{
	return DMATxDescToSet->Buffer1Addr; // 锟斤拷锟斤拷Tx buffer锟斤拷址
}

#if 1
// 为ETH锟阶诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟节达拷
// 锟斤拷锟斤拷值:0,锟斤拷锟斤拷
//     锟斤拷锟斤拷,失锟斤拷
u8 ETH_Mem_Malloc(void)
{
	// DMARxDscrTab=mymalloc(SRAMIN,ETH_RXBUFNB*sizeof(ETH_DMADESCTypeDef));//锟斤拷锟斤拷锟节达拷
	// DMATxDscrTab=mymalloc(SRAMIN,ETH_TXBUFNB*sizeof(ETH_DMADESCTypeDef));//锟斤拷锟斤拷锟节达拷
	// Rx_Buff=mymalloc(SRAMIN,ETH_RX_BUF_SIZE*ETH_RXBUFNB);	//锟斤拷锟斤拷锟节达拷
	// Tx_Buff=mymalloc(SRAMIN,ETH_TX_BUF_SIZE*ETH_TXBUFNB);	//锟斤拷锟斤拷锟节达拷

	static ETH_DMADESCTypeDef rx_desc_tab[ETH_RXBUFNB];
	static ETH_DMADESCTypeDef tx_desc_tab[ETH_TXBUFNB];
	static uint8_t rx_buffer[ETH_RX_BUF_SIZE * ETH_RXBUFNB];
	static uint8_t tx_buffer[ETH_TX_BUF_SIZE * ETH_TXBUFNB];
	DMARxDscrTab = rx_desc_tab;
	DMATxDscrTab = tx_desc_tab;
	Rx_Buff = rx_buffer;
	Tx_Buff = tx_buffer;
	if (!DMARxDscrTab || !DMATxDscrTab || !Rx_Buff || !Tx_Buff)
	{
		ETH_Mem_Free();
		return 1; // 锟斤拷锟斤拷失锟斤拷
	}
	return 0; // 锟斤拷锟斤拷晒锟�
}

// 锟酵凤拷ETH 锟阶诧拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷诖锟�
void ETH_Mem_Free(void)
{
	// myfree(SRAMIN,DMARxDscrTab);//锟酵凤拷锟节达拷
	// myfree(SRAMIN,DMATxDscrTab);//锟酵凤拷锟节达拷
	// myfree(SRAMIN,Rx_Buff);		//锟酵凤拷锟节达拷
	// myfree(SRAMIN,Tx_Buff);		//锟酵凤拷锟节达拷
}
#endif
