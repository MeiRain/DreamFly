#include "NVIC.h"

void NVIC_SystemInit(void)
{
	/***************************stm32优先级配置方式**********************************************/
	NVIC_InitTypeDef NVIC_InitStructure;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//	
//	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//	NVIC_Init(&NVIC_InitStructure);
	
//	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_Init(&NVIC_InitStructure);
//	
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_Init(&NVIC_InitStructure);
/******************************************************************************/

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //EXTIx 中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2; //抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道使能
    NVIC_Init(&NVIC_InitStructure); //根据指定的参数初始化NVIC寄存器
	
	
	//SysTick_IRQn中断由于在有些中断函数中也会使用到，所以它的优先级是最高的
//	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_PRIORITY_GROUP_5, 0, 0)); 			//SysTick_IRQn中断
//	NVIC_SetPriority(EXTI1_IRQn, NVIC_EncodePriority(NVIC_PRIORITY_GROUP_5, 1, 0));				//EXTI1_IRQn中断，NRF的IRQ引脚中断
//	NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_PRIORITY_GROUP_5, 1, 1));		//DMA1_Channel1_IRQn中断，DMA将ADC数据传输完成中断
//	NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_PRIORITY_GROUP_5, 2, 2));			//EXTI15_10_IRQn中断,按键触发中断（优先级必须小于EXTI1_IRQn否则会导致NRFl连接中断）
}
