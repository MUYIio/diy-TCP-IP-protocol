#ifndef __STM32F4x7_ETH_CONF_H
#define __STM32F4x7_ETH_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"

#define USE_ENHANCED_DMA_DESCRIPTORS

//#define USE_Delay

#ifdef USE_Delay
  #include "main.h"                /* Header file where the Delay function prototype is exported */  
  #define _eth_delay_    Delay     /* User can provide more timing precise _eth_delay_ function */
#else
  #define _eth_delay_    ETH_Delay /* Default _eth_delay_ function with less precise timing */
#endif

#define CUSTOM_DRIVER_BUFFERS_CONFIG
#ifdef  CUSTOM_DRIVER_BUFFERS_CONFIG
/* Redefinition of the Ethernet driver buffers size and count */   
 #define ETH_RX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for receive */
 #define ETH_TX_BUF_SIZE    ETH_MAX_PACKET_SIZE /* buffer size for transmit */
 #define ETH_RXBUFNB        4                  /* 20 Rx buffers of size ETH_RX_BUF_SIZE */
 #define ETH_TXBUFNB        4                   /* 5  Tx buffers of size ETH_TX_BUF_SIZE */
#endif


/* PHY���ÿ� **************************************************/
/* PHY��λ��ʱ*/ 
#define PHY_RESET_DELAY    ((uint32_t)0x000FFFFF) 

/* PHY������ʱ*/ 
#define PHY_CONFIG_DELAY   ((uint32_t)0x00FFFFFF)

//PHY��״̬�Ĵ���,�û���Ҫ�����Լ���PHYоƬ���ı�PHY_SR��ֵ
//#define PHY_SR    ((uint16_t)16) //DP83848��PHY״̬�Ĵ���
#define PHY_SR		((uint16_t)31) //LAN8720��PHY״̬�Ĵ�����ַ

//�ٶȺ�˫�����͵ĵ�ֵ,�û���Ҫ�����Լ���PHYоƬ������
//#define PHY_SPEED_STATUS            ((uint16_t)0x0002) /*DP83848 PHY�ٶ�ֵ*/
//#define PHY_DUPLEX_STATUS           ((uint16_t)0x0004) /*DP83848 PHY����״ֵ̬*/
#define PHY_SPEED_STATUS            ((uint16_t)0x0004) /*LAN8720 PHY�ٶ�ֵ*/
#define PHY_DUPLEX_STATUS           ((uint16_t)0x00010) /*LAN8720 PHY����״ֵ̬*/  

#ifdef __cplusplus
}
#endif

#endif 


