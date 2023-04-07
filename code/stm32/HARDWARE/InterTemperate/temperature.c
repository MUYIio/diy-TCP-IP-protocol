#include "delay.h"
#include "temperature.h"
#include "adc.h"
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

//��ʼ��ADC
//�������ǽ��Թ���ͨ��Ϊ��
//����ADCI_CH16�����¶�ת��
void Adc_Temperate_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE); //ʹ��ADC1ʱ��
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //ʹ��GPIOAʱ��
	
	//PA5 ��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//ģ������ģʽ
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure);
		
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; //����ģʽ
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//ADCCLK=PCLK2/4=84/4=21Mhz,ADCʱ����ò�Ҫ����36Mhz
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //��ʹ��DMA
	ADC_CommonInit(&ADC_CommonInitStructure); //ADCͨ�ó�ʼ��
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;  //12λ�ֱ���
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;  //��ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //��ʹ������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //����ͨ���ⲿ��������ѡ��,�˴�δ�õ�
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //�����Ҷ���
	ADC_InitStructure.ADC_NbrOfConversion = 1; //1��ת���ڹ��������� Ҳ����ֻת����������1 
	ADC_Init(ADC1,&ADC_InitStructure); //ADC1��ʼ��
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_16,1,ADC_SampleTime_480Cycles); //����ͨ�������� ͨ��1  480������,��߲���ʱ�������߾�ȷ��	
	ADC_TempSensorVrefintCmd(ENABLE);  //�����¶ȴ�����
	ADC_Cmd(ADC1,ENABLE); //����ADC1
}
//���ADCֵ
//ch:ͨ��ֵ 0~16
//����ֵ:ת�����
static u16 Get_Adc(u8 ch)
{
	u32 temp;
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_480Cycles); //��������1,ͨ��ch
	ADC_SoftwareStartConv(ADC1); //����ת��
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);//�ȴ�ת�����
	temp = ADC_GetConversionValue(ADC1);
	return temp;//����ת�����
}

//��ȡͨ��ch��ת��ֵ��ȡtimes��,Ȼ��ƽ�� 
//ch:ͨ�����
//times:��ȡ����
//����ֵ:ͨ��ch��times��ת�����ƽ��ֵ
static u16 Get_Adc_Average(u8 ch,u8 times)
{
	u32 temp_val = 0;
	u8 t;
	for(t=0;t<times;t++)
	{
		temp_val += Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
}
//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��,��λ:��.)
short Get_Temprate(void)
{
	u32 adcx;
	short result;
	double temperate;
	adcx = Get_Adc_Average(ADC_TEMP,10);//��ȡͨ��16��ֵ
	temperate = (float)adcx*(3.3/4096); //�õ���ѹֵ
	temperate = (temperate-0.76)/0.0025 + 25; //ת��Ϊ�¶�ֵ
	result = temperate*100; //����100��
	return result;
}


