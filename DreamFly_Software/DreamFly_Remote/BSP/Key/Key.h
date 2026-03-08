#ifndef __Key_H
#define __Key_H

#include "stm32f10x.h"

#define KEY_NONE    0x00 //无按键按下
#define KEY1        (0x01 << 0) // 第一个按键按下
#define KEY2        (0x01 << 1) // 第二个按键按下
#define KEY3        (0x01 << 2) // 第三个按键按下
#define KEY4        (0x01 << 3) // 第一个按键按下
#define KEY5        (0x01 << 4) // 第二个按键按下
#define KEY6        (0x01 << 5) // 第三个按键按下

#define KEY1_SINGLE  KEY1
#define KEY2_SINGLE  KEY2
#define KEY3_SINGLE  KEY3
#define KEY4_SINGLE  KEY4
#define KEY5_SINGLE  KEY5
#define KEY6_SINGLE  KEY6

#define KEY1_DOUBLE (KEY1 | 0x01 << 6)
#define KEY2_DOUBLE (KEY2 | 0x01 << 6)
#define KEY3_DOUBLE (KEY3 | 0x01 << 6)
#define KEY4_DOUBLE (KEY4 | 0x01 << 6)
#define KEY5_DOUBLE (KEY5 | 0x01 << 6)
#define KEY6_DOUBLE (KEY6 | 0x01 << 6)
        
#define KEY1_LONG (KEY1 | 0x01 << 7)
#define KEY2_LONG (KEY2 | 0x01 << 7)
#define KEY3_LONG (KEY3 | 0x01 << 7)
#define KEY4_LONG (KEY4 | 0x01 << 7)
#define KEY5_LONG (KEY5 | 0x01 << 7)
#define KEY6_LONG (KEY6 | 0x01 << 7)

void Key_Init(void);
uint8_t Key_Scan(void);

#endif
