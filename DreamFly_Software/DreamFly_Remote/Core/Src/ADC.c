#include "ADC.h"

#define ADC_SAMPLE_NUM	10

static uint16_t ADC_Value[5 * ADC_SAMPLE_NUM];		//ADC采集值存放缓冲区

/*****************************************************************************
*函  数：void AD_Init(void)
*功  能：ADC 和 DMA 初始化
*参  数：无
*返回值：无
*备  注：摇杆右上角为采样最大值 4095，左下角为采样最小值 0
		 ADC_XX[0] 为遥控器电源电压数值；
         ADC_XX[1] 为右遥控器 X 轴数值，代表飞机横滚；
		 ADC_XX[2] 为右遥控器 Y 轴数值，代表飞机俯仰；
	     ADC_XX[3] 为左遥控器 X 轴数值，代表飞机航向；
		 ADC_XX[4] 为左遥控器 Y 轴数值，代表飞机油门
*****************************************************************************/
void ADC1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);//开启 ADC 测量引脚的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);						
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);							
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);											//设置ADC分频因子72M/6=12，ADC最大时间不能超过14M
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;								//ADC输入管脚需要为模拟输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//配置ADC的规则通道的采样顺序和采样时间
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);//1个通道转换一次耗时21us 4个通道
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_71Cycles5); //采样个数ADC_SAMPLE_NUM
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_71Cycles5); //总共耗时4*21*ADC_SAMPLE_NUM（64）=5.4ms<10ms
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 4, ADC_SampleTime_71Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 5, ADC_SampleTime_71Cycles5);
	
	//配置ADC1，由 TIM4 的 CCR 触发采样开始
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;							//独立模式，单独使用 ADC1
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;						//数据对齐模式，右对齐
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;			//触发模式，软件触发
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;							//连续转换，开启
	ADC_InitStructure.ADC_ScanConvMode = ENABLE; 								//扫描模式，开启
	ADC_InitStructure.ADC_NbrOfChannel = 5;										//通道数，5 个通道
	ADC_Init(ADC1, &ADC_InitStructure);
	
	//配置DMA1通道1，将ADC采样转换得到的数据传输到内存数组中
	DMA_InitStructure.DMA_PeripheralBaseAddr = ((uint32_t)&ADC1->DR);			//外设基地址，&ADC1->DR
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;	//以半字为单位进行数据的传输
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;			//外设地址固定，不自增	
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_Value;					//数据保存到内存中数组的首地址
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;			//以 16 位为单位进行数据的传输，半字传输
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;						//内存地址自增（地址是每次增加 1，以半字作为单位传输的）
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;							//传输方向为：外设->内存
	DMA_InitStructure.DMA_BufferSize = 5 * ADC_SAMPLE_NUM;						//每次传输的数据的个数
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;								//循环传输，必须为循环传输方式，否则会导致 DMA 只能传输一次
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;								//失能内存到内存的传输方式
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;						//DMA 通道 1 的优先级设置为中级
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);								//当同一个 DMA 的不同通道同时有传输数据的要求时，优先级高的先进行传输
	
	ADC_Cmd(ADC1, ENABLE);														//开启 ADC
	ADC_DMACmd(ADC1, ENABLE);													//开启 ADC — DMA 数据传输通道
	DMA_Cmd(DMA1_Channel1, ENABLE);												//开启 DMA1 的通道 1
									
	ADC_ResetCalibration(ADC1);													//重置 ADC 采样校准器，防止出现较大的误差
	while(ADC_GetCalibrationStatus(ADC1));										//等待校准成功
	ADC_StartCalibration(ADC1);													//开启 ADC 采样状态
	while(ADC_GetCalibrationStatus(ADC1));										//等到开启成功
									
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);										//使能指定的ADC1的软件转换启动功能
}

/*****************************************************************************
*函  数：uint16_t ADC_GetValue(uint8_t ch)
*功  能：ADC 和 DMA 初始化
*参  数：ch：通道编号，具体定义见 "ADC.h"
*返回值：通道 X 的采样 ADC_SAMPLE_NUM 次的平均值
*备  注：无
*****************************************************************************/
uint16_t ADC_GetValue(uint8_t ch)
{	
	uint32_t filter_value = 0;
	uint32_t sum = 0;
	for(uint8_t i = 0; i < ADC_SAMPLE_NUM; i ++)
	{
		sum += ADC_Value[5 * i + ch];
	}
	filter_value = sum / ADC_SAMPLE_NUM;
	return filter_value;
}
