#ifndef __ADC_H
#define __ADC_H

#include "stm32f4xx.h"

void ADC1_Init(void);
uint16_t ADC_GetValue(void);

#endif
