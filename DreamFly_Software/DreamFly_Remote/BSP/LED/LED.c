/********************************************************************************
点亮遥控器上的LED灯

注意：
1.当要使用LED_CyclieOn(...)函数循环点亮LED1和LED2灯时，请先初始化SysTick_init()

主函数初始化代码：
	LED_Config();
	
测试代码为：
	SysTick_init();
	LED_Config();
	LED_CyclieOn(1000);
	
	while(1){}
*******************************************************************************/
#include "LED.h"

/******************************* 代码移植修改区 *******************************/

#define LED_RCC 		RCC_APB2Periph_GPIOA 		// 端口时钟
#define LED_PORT 		GPIOA               		// 端口
#define LED_PIN 		GPIO_Pin_8         			// 引脚

/*****************************************************************************/

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = LED_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);
	
	GPIO_SetBits(LED_PORT, LED_PIN);
}

/******************************************************************************
 *函  数：void LED_On(void)
 *功  能：点亮 LED
 *参  数：无
 *返回值：无
 *备  注: 无
 *****************************************************************************/
void LED_On(void)
{
    GPIO_ResetBits(LED_PORT, LED_PIN);
}

/******************************************************************************
 *函  数：void LED_Off(void)
 *功  能：关闭 LED
 *参  数：无
 *返回值：无
 *备  注: 无
 *****************************************************************************/
void LED_Off(void)
{
    GPIO_SetBits(LED_PORT, LED_PIN);
}

/******************************************************************************
 *函  数：void LED_Toggle(void)
 *功  能：翻转 LED
 *参  数：无
 *返回值：无
 *备  注: 无
 *****************************************************************************/
void LED_Toggle(void)
{
    static uint8_t flag = 1;
    if (flag)
    {
        flag = 0;
        LED_Off();
    }
    else
    {
        flag = 1;
        LED_On();
    }
}
