#include "STM32F411CEU6.h"

#include "stdio.h"    // 需要使用printf
#include <stdarg.h>


// 打印日志的函数，接收日志级别、格式化字符串及变参
void log_print(int level, const char *fmt, ...) {
    if (level >= CURRENT_LOG_LEVEL) {
        va_list args;
        va_start(args, fmt);

        switch (level) {
            case LOG_LEVEL_DEBUG:
                printf("DEBUG: ");
                break;
            case LOG_LEVEL_INFO:
                printf("INFO: ");
                break;
            case LOG_LEVEL_WARNING:
                printf("WARNING: ");
                break;
            case LOG_LEVEL_ERROR:
                printf("ERROR: ");
                break;
            default:
                break;
        }

        // 打印传入的格式化字符串
        vprintf(fmt, args);
        va_end(args);
    }
}

/*****************************************************************************
*函  数：void NVIV_Config(void)
*功  能：配置工程中所有中断的优先级
*参  数：无
*返回值：无
*备  注：此优先级中断不要随便更改哦
*****************************************************************************/
void STM32F411CEU6_NVICConfig(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断分组2
	
	//程序时基 TIM1 定时器中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; //抢断优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //子优选级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	//USART1中断
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //抢断优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //子优选级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	
	//USART2中断
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; //抢断优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//子优选级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
