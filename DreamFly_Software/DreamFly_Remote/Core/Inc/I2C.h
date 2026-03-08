#ifndef __I2C_H
#define __I2C_H

#include "stm32f10x.h"

void I2C1_Init(void);
uint8_t I2C1_ReadByte(uint8_t devaddress, uint8_t regaddress, uint8_t *pbyte);
uint8_t I2C1_WriteByte(uint8_t devaddress, uint8_t regaddress, uint8_t byte);
uint8_t I2C1_ReadBuffer(uint8_t devaddress, uint8_t regaddress, uint8_t *pbuffer, uint16_t len);
uint8_t I2C1_WriteBuffer(uint8_t devaddress, uint8_t regaddress, uint8_t *pbuffer, uint16_t len);

#endif
