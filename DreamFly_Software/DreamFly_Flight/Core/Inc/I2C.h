#ifndef __I2C_H
#define __I2C_H

#include "stm32f4xx.h"

void I2C1_Init(void);
uint8_t I2C1_ReadByte(uint8_t DEVAddress, uint8_t reg_address, uint8_t *pByte);
uint8_t I2C1_WriteByte(uint8_t DEVAddress, uint8_t reg_address, uint8_t Byte);
uint8_t I2C1_ReadBuffer(uint8_t DEVAddress, uint8_t reg_address, uint8_t *pBuffer, uint16_t NumByteToRead);
uint8_t I2C1_WriteBuffer(uint8_t DEVAddress, uint8_t reg_address, uint8_t *pBuffer, uint16_t NumByteToWrite);

#endif
