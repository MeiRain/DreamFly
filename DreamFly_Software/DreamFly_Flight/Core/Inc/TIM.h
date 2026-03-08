#ifndef __TIM_H
#define __TIM_H

#include "stm32f4xx.h"

typedef struct
{
    uint8_t Period_500Hz;
    uint8_t Period_200Hz;
    uint8_t Period_100Hz;
    uint8_t Period_50Hz;
    uint8_t Period_20Hz;
	uint8_t Period_10Hz;
    uint8_t Period_2Hz;
} timer_t;

extern timer_t Timer;

void TIM2_TimerInit(void);
void TIM3_OCInit(void);
void TIM4_OCInit(void);

#endif
