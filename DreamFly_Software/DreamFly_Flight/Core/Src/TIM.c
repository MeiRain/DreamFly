#include "TIM.h"

timer_t Timer;

/*****************************************************************************
 *函  数：void TIM2_Init(void)
 *功  能：TIM2初始化为1ms计数一次
 *参  数：无
 *返回值：无
 *备  注：更新中断时间 Tout = (ARR-1)*(PSC-1)/CK_INT
 *****************************************************************************/
void TIM2_TimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; // 时基单元结构体

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	TIM_TimeBaseStructure.TIM_Prescaler = 100 - 1;				// 预分频器，即 PSC 的值
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数器模式，选择向上计数
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;				// 计数周期，即 ARR 的值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			// 重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	TIM_ClearFlag(TIM2, TIM_FLAG_Update); // 清除定时器更新标志位，TIM_TimeBaseInit 函数末尾，手动产生了更新事件

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // 开启 TIM2 的更新中断

	TIM_Cmd(TIM2, DISABLE);
}

/*****************************************************************************
 *函  数：void TIM3_Init(void)
 *功  能：TIM3初始化为1ms计数一次
 *参  数：无
 *返回值：无
 *备  注：更新中断时间 Tout = (ARR-1)*(PSC-1)/CK_INT
 *****************************************************************************/
void TIM3_OCInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; // 时基单元结构体
	TIM_OCInitTypeDef TIM_OCInitStructure;		   // 输出比较结构体

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	TIM_TimeBaseStructure.TIM_Prescaler = 100 - 1;				// 预分频器，即 PSC 的值
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数器模式，选择向上计数
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;				// 计数周期，即 ARR 的值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			// 重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_OCStructInit(&TIM_OCInitStructure); // 结构体初始化，给结构体所有成员都赋一个默认值

	// 配置 TIM3 通道3 为 PWM1 模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;			  // 输出比较模式，选择 PWM 模式 1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;							  // 初始 CCR 的值，用于配置占空比
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	  // 输出极性，选择相同，不取反
	TIM_OC3Init(TIM3, &TIM_OCInitStructure);

	// 配置 TIM3 通道4 为 PWM1 模式
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC4Init(TIM3, &TIM_OCInitStructure);

	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable); // 打开 TIM3 通道3 的影子寄存器
	TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable); // 打开 TIM3 通道4 的影子寄存器

	TIM_ARRPreloadConfig(TIM3, ENABLE); // 使能通道重装载

	TIM_Cmd(TIM3, ENABLE);
}

/*****************************************************************************
 *函  数：void TIM4_Init(void)
 *功  能：TIM4初始化为1ms计数一次
 *参  数：无
 *返回值：无
 *备  注：更新中断时间 Tout = (ARR-1)*(PSC-1)/CK_INT
 *****************************************************************************/
void TIM4_OCInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; // 时基单元结构体
	TIM_OCInitTypeDef TIM_OCInitStructure;		   // 输出比较结构体

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

	TIM_TimeBaseStructure.TIM_Prescaler = 100 - 1;				// 预分频器，即 PSC 的值
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数器模式，选择向上计数
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;				// 计数周期，即 ARR 的值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			// 重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_OCStructInit(&TIM_OCInitStructure); // 结构体初始化，给结构体所有成员都赋一个默认值

	// 配置 TIM4 通道1 为 PWM1 模式
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;			  // 输出比较模式，选择 PWM 模式 1
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; // 输出使能
	TIM_OCInitStructure.TIM_Pulse = 0;							  // 初始 CCR 的值，用于配置占空比
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;	  // 输出极性，选择相同，不取反
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);

	// 配置 TIM4 通道2 为 PWM1 模式
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OC2Init(TIM4, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable); // 打开 TIM4 通道1 的预装载寄存器
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable); // 打开 TIM4 通道2 的预装载寄存器

	TIM_ARRPreloadConfig(TIM4, ENABLE); // 使能通道重装载

	TIM_Cmd(TIM4, ENABLE);
}

/*****************************************************************************
 *函  数：void TIM2_Init(void)
 *功  能：TIM2初始化为1ms计数一次
 *参  数：无
 *返回值：无
 *备  注：更新中断时间 Tout = (ARR-1)*(PSC-1)/CK_INT
 *****************************************************************************/
void TIM5_TimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; // 时基单元结构体

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

	TIM_TimeBaseStructure.TIM_Prescaler = 100 - 1;				// 预分频器，即 PSC 的值
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 计数器模式，选择向上计数
	TIM_TimeBaseStructure.TIM_Period = 1000 - 1;				// 计数周期，即 ARR 的值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;		// 时钟分频，选择不分频，此参数用于配置滤波器时钟，不影响时基单元功能
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;			// 重复计数器，高级定时器才会用到
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	TIM_ClearFlag(TIM5, TIM_FLAG_Update); // 清除定时器更新标志位，TIM_TimeBaseInit 函数末尾，手动产生了更新事件

	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE); // 开启 TIM5 的更新中断

	TIM_Cmd(TIM5, DISABLE);
}
