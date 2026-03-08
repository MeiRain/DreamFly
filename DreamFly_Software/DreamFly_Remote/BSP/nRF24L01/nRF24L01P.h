#ifndef __nRF24L01P_H
#define __nRF24L01P_H

#include "stm32f10x.h"

#define MODE_RX1				1				//普通接收
#define MODE_TX1				2				//普通发送
#define MODE_RX2				3				//接收模式2，用于双向传输
#define MODE_TX2				4				//发送模式2，用于双向传输
#define TX_ADR_WIDTH    		5   		//接收端的接收通道的地址的宽度
#define RX_ADR_WIDTH    		5   		//本机接收通道 0 的地址宽度
#define TX_PAYLOAD_WIDTH  		32  			//要发送的有效数据长度
#define RX_PAYLOAD_WIDTH  		32  			//要接收的有效数据长度

extern uint8_t TX_ADDRESS[TX_ADR_WIDTH];
extern uint8_t RX_ADDRESS[RX_ADR_WIDTH];

uint8_t nRF24L01P_Check(void);
void nRF24L01P_Init(uint8_t Mode);
void nRF24L01P_SetMode(uint8_t Mode);
void nRF24L01P_NormalPacket(uint8_t* txbuffer, uint8_t length);
void nRF24L01P_AckPayloadPacket(uint8_t* txbuffer, uint8_t length);
void nRF24L01P_IRQHandler(uint8_t *rxbuffer);
uint8_t nRF24L01P_ReadByte(uint8_t regaddress);
uint8_t nRF24L01P_WriteByte(uint8_t regaddress, uint8_t byte);
uint8_t nRF24L01P_ReadBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len);
uint8_t nRF24L01P_WriteBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len);	

#endif
