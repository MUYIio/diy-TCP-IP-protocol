#include "sys.h"
#include "usart.h"
#include "os_api.h"
#include <stdio.h>
#include <stdarg.h>
#include <rt_sys.h>

static os_queue_t * rx_queue;

void usart_write (int ch) {
	if (ch == '\n') {
		while ((USART1->SR & 0X40) == 0);
		USART1->DR = (u8)'\r';
	}

	while ((USART1->SR & 0X40) == 0);
	USART1->DR = (u8)ch;
}

int usart_read (void) {
	int ch;
	os_queue_wait(rx_queue, 0, 0, &ch);
	return ch;
}

#if EN_USART1_RX // ���ʹ���˽���
// ����1�жϷ������
// ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 USART_RX_BUF[USART_REC_LEN]; // ���ջ���,���USART_REC_LEN���ֽ�.
// ����״̬
// bit15��	������ɱ�־
// bit14��	���յ�0x0d
// bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA = 0; // ����״̬���

// ��ʼ��IO ����1
// bound:������
void uart_init(u32 bound)
{
	// GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);  // ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // ʹ��USART1ʱ��

	// ����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);  // GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1); // GPIOA10����ΪUSART1

	// USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; // GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			// ���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// �ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			// ���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			// ����
	GPIO_Init(GPIOA, &GPIO_InitStructure);					// ��ʼ��PA9��PA10

	// USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;										// ����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// �ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								// ����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// �շ�ģʽ
	USART_Init(USART1, &USART_InitStructure);										// ��ʼ������1

	USART_Cmd(USART1, ENABLE); // ʹ�ܴ���1

	// USART_ClearFlag(USART1, USART_FLAG_TC);

#if EN_USART1_RX
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // ��������ж�

	// Usart1 NVIC ����
	rx_queue = os_queue_create(1, 64);
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		  // ����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // ��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // �����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ�����

#endif
}

void USART1_IRQHandler(void) // ����1�жϷ������
{
#if SYSTEM_SUPPORT_OS // ���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();
#endif
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) // �����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		uint8_t Res = USART_ReceiveData(USART1); //(USART1->DR);	//��ȡ���յ�������
		os_queue_release(rx_queue, -1, 0, &Res);
	}
#if SYSTEM_SUPPORT_OS // ���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();
#endif
}
#endif
