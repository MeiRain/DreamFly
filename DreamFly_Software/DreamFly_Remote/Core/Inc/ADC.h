#ifndef __ADC_H
#define __ADC_H

#include "stm32f10x.h"

#define  ADC_BAT 	0	//电池通道 BAT 0
#define  ADC_ROL 	1	//横滚通道 ROL 1
#define  ADC_PIT 	2	//俯仰通道 PIT 2
#define  ADC_YAW 	3	//航向通道 YAW 3
#define  ADC_THR 	4	//油门通道 THR 4

void ADC1_Init(void);
uint16_t ADC_GetValue(uint8_t ch);

#endif
