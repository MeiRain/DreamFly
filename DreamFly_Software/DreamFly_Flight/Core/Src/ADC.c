#include "ADC.h"

void ADC1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    // PA0电源测试脚模拟输入模式选择
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_144Cycles); // ADC1的ADC_Channel_0进行规则转换配置

    // ADC通用配置(ADC时钟频率最好不要超过36MHz)
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                     // 独立模式，即单独使用ADC1
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;                  // 4 分频 fplck2/4 = 25MHz
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;      // DMA失能
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 两个采样之间间隔5个时钟
    ADC_CommonInit(&ADC_CommonInitStructure);

    // ADC1参数初始化
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                      // 12位采样精度
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                               // 扫描模式，失能，只转换规则组的序列1这一个位置
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                         // 连续转换，失能，每转换一次规则组序列后停止
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 不开启触发，使用软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                      // 数据右对齐
    ADC_InitStructure.ADC_NbrOfConversion = 1;                                  // 转换通道数目
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE); // 使能ADC1
}

/******************************************************************************************
 *函  数：uint16_t Get_ADC(uint8_t ch)
 *功  能：获取电池采样点电压的ADC值
 *参  数：ch ADC采样通道
 *返回值：返回通道AD值
 *备  注：电池电压采样点的ADC值，电池电压采样电路见原理图
 *******************************************************************************************/
uint16_t ADC_GetValue(void)
{
    ADC_SoftwareStartConv(ADC1);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        ;
    return ADC_GetConversionValue(ADC1);
}
