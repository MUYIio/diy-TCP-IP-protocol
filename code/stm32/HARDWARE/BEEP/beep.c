#include "beep.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//��������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/6/10
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//��ʼ��PF8Ϊ�����
//BEEP IO��ʼ��
void BEEP_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); //ʹ��GPIOFʱ��
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8;  //PF8����
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT; //���ģʽ
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz; //�ٶ�100M
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP; //�������
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;   //����
	GPIO_Init(GPIOF,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOF,GPIO_Pin_8); //PF8����
}
