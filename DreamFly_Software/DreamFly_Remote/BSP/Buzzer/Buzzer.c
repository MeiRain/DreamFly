#include "Buzzer.h"

/************************代码移植修改区************************************/

#define BUZZER_CLK    	RCC_APB2Periph_GPIOB  		//端口时钟
#define BUZZER_PORT		GPIOB                 		//端口
#define BUZZER_PIN		GPIO_Pin_5            		//引脚

/**************************************************************************/

void Buzzer_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;	
	
	RCC_APB2PeriphClockCmd(BUZZER_CLK, ENABLE);
	
	//设置蜂鸣器控制 IO 口为推挽输出模式		
	GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(BUZZER_PORT, &GPIO_InitStructure);	
	
	GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);	
}								

void Buzzer_On(void)
{
	GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);
}

void Buzzer_Off(void)
{
	GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
}
