#ifndef __TIM_H
#define __TIM_H

#include "stm32f10x.h"

typedef struct
{
    uint8_t Period_500Hz;
    uint8_t Period_200Hz;
    uint8_t Period_100Hz;
    uint8_t Period_50Hz;
    uint8_t Period_20Hz;
    uint16_t Period_2Hz;
} tim_timer_t;

extern tim_timer_t Timer;

void TIM2_TimerInit(void);

#endif
