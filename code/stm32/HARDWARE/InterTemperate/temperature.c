#include "delay.h"
#include "temperature.h"
#include "adc.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//ADC 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/6
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//初始化ADC
//这里我们仅以规则通道为例
//开启ADCI_CH16用于温度转换
void Adc_Temperate_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE); //使能ADC1时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //使能GPIOA时钟
	
	//PA5 初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;//模拟输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure);
		
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent; //独立模式
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;//ADCCLK=PCLK2/4=84/4=21Mhz,ADC时钟最好不要超过36Mhz
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled; //不使用DMA
	ADC_CommonInit(&ADC_CommonInitStructure); //ADC通用初始化
	
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;  //12位分辨率
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;  //非扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; //不使用连续转换模式
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; //规则通道外部触发边沿选择,此处未用到
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //数据右对齐
	ADC_InitStructure.ADC_NbrOfConversion = 1; //1个转换在规则序列中 也就是只转换规则序列1 
	ADC_Init(ADC1,&ADC_InitStructure); //ADC1初始化
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_16,1,ADC_SampleTime_480Cycles); //规则通道组设置 通道1  480个周期,提高采样时间可以提高精确度	
	ADC_TempSensorVrefintCmd(ENABLE);  //开启温度传感器
	ADC_Cmd(ADC1,ENABLE); //开启ADC1
}
//获得ADC值
//ch:通道值 0~16
//返回值:转换结果
static u16 Get_Adc(u8 ch)
{
	u32 temp;
	ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_480Cycles); //规则序列1,通道ch
	ADC_SoftwareStartConv(ADC1); //开启转换
	while(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC) == RESET);//等待转换完成
	temp = ADC_GetConversionValue(ADC1);
	return temp;//返回转换结果
}

//获取通道ch的转换值，取times次,然后平均 
//ch:通道编号
//times:获取次数
//返回值:通道ch的times次转换结果平均值
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
//得到温度值
//返回值:温度值(扩大了100倍,单位:℃.)
short Get_Temprate(void)
{
	u32 adcx;
	short result;
	double temperate;
	adcx = Get_Adc_Average(ADC_TEMP,10);//获取通道16的值
	temperate = (float)adcx*(3.3/4096); //得到电压值
	temperate = (temperate-0.76)/0.0025 + 25; //转换为温度值
	result = temperate*100; //扩大100倍
	return result;
}


