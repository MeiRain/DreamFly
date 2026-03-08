#ifndef __SPI_H
#define __SPI_H

#include "stm32f10x.h"

void SPI1_Init(void);
uint8_t SPI1_SwapByte(uint8_t ByteToSwap);
uint8_t SPI1_ReadByte(uint8_t regaddress);
uint8_t SPI1_WriteByte(uint8_t regaddress, uint8_t byte);
uint8_t SPI1_ReadBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len);
uint8_t SPI1_WriteBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len);

#endif

