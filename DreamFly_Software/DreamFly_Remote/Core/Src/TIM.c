/**********************************************************************************
利用定时器TIM4的通道4的输出比较功能，给ADC1提供采样触发信号，这里触发周期为：100ms
触发周期计算公式为：psc/72*period/1000000（单位为秒）

主函数初始化代码为：
	TIM_OCTigrConfig();
**********************************************************************************/
#include "TIM.h"
#include "Remote.h"
tim_timer_t Timer;

void TIM2_TimerInit(void)
{	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				//定义结构体变量
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			//开启TIM2的时钟
	
	TIM_InternalClockConfig(TIM2);		//选择TIM2为内部时钟，若不调用此函数，TIM默认也为内部时钟
	
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	//计数器模式，选择向上计数
	TIM_TimeBaseInitStructure.TIM_Period = 100 - 1;				//计数周期，即ARR的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 720 - 1;				//预分频器，即PSC的值
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;			//重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);				//将结构体变量交给TIM_TimeBaseInit，配置TIM2的时基单元	
	
	/*中断输出配置*/
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);						//清除定时器更新标志位
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);					//开启TIM2的更新中断
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				//配置NVIC为分组2
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;				//选择配置NVIC的TIM2线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//指定NVIC线路的抢占优先级为2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);								//将结构体变量交给NVIC_Init，配置NVIC外设
	
	TIM_Cmd(TIM2, DISABLE);			//失能TIM2
}


void TIM2_IRQHandler(void)
{
	// 分频系数
	static uint8_t cnt_2 = 0, cnt_5 = 0, cnt_10 = 0, cnt_20 = 0, cnt_50 = 0;
	static uint16_t cnt_500 = 0;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		cnt_2++;
		cnt_5++;
		cnt_10++;
		cnt_20++;
		cnt_50++;
		cnt_500++;

		if (cnt_2 >= 2) // 500Hz
		{
			cnt_2 = 0;
			Timer.Period_500Hz = 1;
		}
		if (cnt_5 >= 5) // 200Hz
		{
			cnt_5 = 0;
			Timer.Period_200Hz = 1;
		}
		if (cnt_10 >= 10) // 100Hz
		{
			cnt_10 = 0;
			Timer.Period_100Hz = 1;
		}
		if (cnt_20 >= 20) // 50Hz
		{
			cnt_20 = 0;
			Timer.Period_50Hz = 1;
		}
		if (cnt_50 >= 50) // 20Hz
		{
			cnt_50 = 0;
			Timer.Period_20Hz = 1;
		}
		if (cnt_500 >= 500) // 2Hz
		{
			cnt_500 = 0;
			Timer.Period_2Hz = 1;
		}
	}

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}
