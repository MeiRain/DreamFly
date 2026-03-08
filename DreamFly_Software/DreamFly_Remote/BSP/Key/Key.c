#include <stdio.h>

#include "Key.h"

/************************代码移植修改区************************************/

#define KEY_NUMBERS 6 

#define KEY1_CLK 	RCC_APB2Periph_GPIOB // 端口时钟
#define KEY1_PORT 	GPIOB				 // 端口
#define KEY1_PIN 	GPIO_Pin_10			 // 引脚

#define KEY2_CLK 	RCC_APB2Periph_GPIOB // 端口时钟
#define KEY2_PORT 	GPIOB				 // 端口
#define KEY2_PIN 	GPIO_Pin_11			 // 引脚

#define KEY3_CLK 	RCC_APB2Periph_GPIOB // 端口时钟
#define KEY3_PORT 	GPIOB				 // 端口
#define KEY3_PIN 	GPIO_Pin_12			 // 引脚

#define KEY4_CLK 	RCC_APB2Periph_GPIOB // 端口时钟
#define KEY4_PORT 	GPIOB				 // 端口
#define KEY4_PIN 	GPIO_Pin_13			 // 引脚

#define KEY5_CLK 	RCC_APB2Periph_GPIOB // 端口时钟
#define KEY5_PORT 	GPIOB				 // 端口
#define KEY5_PIN 	GPIO_Pin_14			 // 引脚

#define KEY6_CLK 	RCC_APB2Periph_GPIOB // 端口时钟
#define KEY6_PORT 	GPIOB				 // 端口
#define KEY6_PIN 	GPIO_Pin_15			 // 引脚

/**************************************************************************/

#define SHAKE_TIME     4
#define DOUBLE_CLICK_TIME  200  // 双击最大间隔时间（ms）  
#define LONG_PRESS_TIME  150  	// 长按最小持续时间（ms）

// 按键状态定义  
typedef enum 
{  
    KEY_RELEASE,  				//松开阶段
	KEY_SHAKE,					//抖动阶段
    KEY_PRESSE,  				//按下阶段
    KEY_SINGLE_CLICK,  			//单击阶段
    KEY_DOUBLE_CLICK,  			//双击阶段
    KEY_LONGP_RESSE,  			//长按阶段
	KEY_WAIT_RELEASE,			//等待按钮释放阶段
} KeyState_e;

void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(KEY1_CLK | KEY2_CLK | KEY3_CLK 
						 | KEY4_CLK | KEY5_CLK | KEY6_CLK, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN 
								| KEY4_PIN | KEY5_PIN | KEY6_PIN;		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(KEY1_PORT, &GPIO_InitStructure);
	GPIO_Init(KEY2_PORT, &GPIO_InitStructure);
	GPIO_Init(KEY3_PORT, &GPIO_InitStructure);
	GPIO_Init(KEY4_PORT, &GPIO_InitStructure);
	GPIO_Init(KEY5_PORT, &GPIO_InitStructure);
	GPIO_Init(KEY6_PORT, &GPIO_InitStructure);					
}
 
uint8_t Key_GetNumber(void)
{
    if(GPIO_ReadInputDataBit(KEY1_PORT, KEY1_PIN) == 0)
        return KEY1;
    if(GPIO_ReadInputDataBit(KEY2_PORT, KEY2_PIN) == 0)
        return KEY2;
    if(GPIO_ReadInputDataBit(KEY3_PORT, KEY3_PIN) == 0)
        return KEY3;
    if(GPIO_ReadInputDataBit(KEY4_PORT, KEY4_PIN) == 0)
        return KEY4;
    if(GPIO_ReadInputDataBit(KEY5_PORT, KEY5_PIN) == 0)
        return KEY5;
    if(GPIO_ReadInputDataBit(KEY6_PORT, KEY6_PIN) == 0)
        return KEY6;
    return KEY_NONE;
}
 
uint8_t Key_Scan(void)
{
    static uint8_t KeyState = 0;         // 按键状态变量
    static uint8_t KeyLastNumber = 0;//上一次扫描的键值    
	static uint16_t KeyTime_cnt = 0;           // 按键计时变量
	uint8_t KeyCurrentNumber;// 按键存放临时变量
	uint8_t KeyReturnNumber;// 函数最终返回的按键值
    uint8_t Key_LaetPress;//当前按键键值

    KeyReturnNumber = KEY_NONE;                         // 清除返回按键值

    Key_LaetPress = KeyCurrentNumber = Key_GetNumber();        // 读取当前键值

    if (Key_LaetPress) //保存按键的的键值只在非 0 的状态才改变其按键值
    {
        KeyLastNumber = KeyCurrentNumber;
    }

    switch (KeyState)
    {
		case KEY_RELEASE:                    // 空闲阶段，判断有无按键按下
			// 如果有按键按下并且等于上次按键值
			if ((KeyCurrentNumber != 0) & (KeyCurrentNumber == Key_LaetPress))  
			{
				KeyTime_cnt = 0;             // 清零时间间隔计数
				KeyState = KEY_SHAKE;        // 然后进入抖动阶段
			}
			break;

		case KEY_SHAKE:                       // 抖动阶段，进行消抖
			if (KeyCurrentNumber == Key_LaetPress)
			{
				KeyTime_cnt++;                     // 一次10ms
				if(KeyTime_cnt >= SHAKE_TIME)   // 消抖时间为：SHAKE_TIME*10ms
				{
					KeyState = KEY_PRESSE;    // 按键时间超过消抖时间，判定为按键有效
				}
			}
			else
			{
				KeyState = KEY_RELEASE;       // 如果按键时间没有超过，判定为误触
			}
			break;

		case KEY_PRESSE:                       // 按下阶段，判定种类：是单击，还是长按
			if(KeyCurrentNumber == 0)          // 如果按键在设定的长按时间内释放，则判定为单击
			{
				KeyReturnNumber = KeyLastNumber; // 返回 有效按键值：单击
				KeyState = KEY_RELEASE;       // 返回 按键状态A，继续等待按键
			}
			else
			{
				KeyTime_cnt++;
				if(KeyTime_cnt >= LONG_PRESS_TIME)   // 如果按键时间超过 设定的长按时间（LONG_KEY_TIME*10ms=200*10ms=2000ms）, 则判定为 长按
				{
					KeyReturnNumber = (KeyLastNumber | 0x80); // 返回有效键值值：长按
					KeyState = KEY_WAIT_RELEASE;    // 等待释放状态
				}
			}
			break;

		case KEY_WAIT_RELEASE:                         // 等待释放阶段，判断按键是否释放
			if (KeyCurrentNumber == 0)
			{
				KeyState = KEY_RELEASE;          // 按键释放后，进入 按键状态A ，进行下一次按键的判定
			}
			break;

		default: // 特殊情况：KeyState是其他值得情况，清零KeyState。这种情况一般出现在 没有初始化KeyState，第一次执行这个函数的时候
			KeyState = KEY_RELEASE;
			break;
    }
    return KeyReturnNumber;                          // 返回 按键值
}  
