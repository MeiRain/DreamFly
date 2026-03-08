#ifndef __USART_H
#define __USART_H

#include "stm32f4xx.h"

void USART1_Init(uint32_t Baudrate);
void USART1_SendByte(uint8_t Byte);
void USART1_SendArray(uint8_t* pArray, uint8_t ArrayLength);

void USART2_Init(uint32_t Baudrate);
void USART2_SendByte(uint8_t Byte);
void USART2_SendArray(uint8_t* pArray, uint8_t ArrayLength);

#endif
