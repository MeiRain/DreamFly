#include "LED.h"

/******************************* 代码移植修改区 *******************************/

#define LED_RCC 		RCC_AHB1Periph_GPIOC 		// 端口时钟
#define LED_PORT 		GPIOC               		// 端口
#define LED_PIN 		GPIO_Pin_13         		// 引脚

#define LED_LOW    		LED_PORT->BSRRH  |= LED_PIN				// CE 引脚低电平
#define LED_HIGH   		LED_PORT->BSRRL  |= LED_PIN				// CE 引脚高电平

/*****************************************************************************/

/******************************************************************************
 *函  数：void LED_Init(void)
 *功  能：初始化LED控制引脚
 *参  数：无
 *返回值: 无
 *备  注: 无
 *****************************************************************************/
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(LED_RCC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // 选择模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // 输出类型
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
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
    LED_LOW;
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
    LED_HIGH;
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
        LED_HIGH;
    }
    else
    {
        flag = 1;
        LED_LOW;
    }
}
