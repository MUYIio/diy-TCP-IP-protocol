#ifndef _ADC_H
#define _ADC_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//ADC ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	 

#define ADC_CH5  5  //ͨ��5
#define ADC_TEMP 16 //ͨ��16 �¶ȴ�������ADCͨ��

void Adc_Temperate_Init(void);				//�¶ȴ�������ʼ��
static u16 Get_Adc(u8 ch);
static u16 Get_Adc_Average(u8 ch,u8 times);
short Get_Temprate(void);  //��ȡ�¶�ֵ
#endif

