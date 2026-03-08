#ifndef __IIC_H
#define __IIC_H

#include "stm32f4xx.h"

// IIC所有操作函数
void IIC_Init(void);       // 初始化IIC的IO口
void IIC_Start(void);      // 发送IIC开始信号
void IIC_Stop(void);       // 发送IIC停止信号
void IIC_Ack(void);        // IIC发送ACK信号
void IIC_NAck(void);       // IIC不发送ACK信号
uint8_t IIC_WaitAck(void); // IIC等待ACK信号
void IIC_SendByteToBus(uint8_t data);      // IIC发送一个字节
uint8_t IIC_ReceiveByteToBus(uint8_t ack); // IIC读取一个字节
uint8_t IIC_ReadByte(uint8_t DEVAddress, uint8_t reg_address, uint8_t *pByte);
uint8_t IIC_WriteByte(uint8_t DEVAddress, uint8_t reg_address, uint8_t Byte);
uint8_t IIC_ReadBuffer(uint8_t DEVAddress, uint8_t reg_address, uint8_t *pBuffer, uint8_t NumByteToRead);
uint8_t IIC_WriteBuffer(uint8_t DEVAddress, uint8_t reg_address, uint8_t *pBuffer, uint8_t NumByteToWrite);

void VL53L1X_SendByte(uint8_t txd);
uint8_t VL53L1X_ReceiveByte(void);
uint8_t VL53L1X_WriteByte(uint8_t SlaveAddress, uint16_t REG_Address,uint8_t REG_data);
uint8_t VL53L1X_ReadByte(uint8_t SlaveAddress, uint16_t REG_Address,uint8_t *REG_data);
uint8_t VL53L1X_ReadBuffer(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *buf, uint16_t len);
uint8_t VL53L1X_WriteBuffer(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *buf, uint16_t len);

#endif
