#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx.h"

/************************** ´úÂëÒÆÖ²ÐÞ¸ÄÇø ***************************/

#define SPI1_CS_CLK                 RCC_AHB1Periph_GPIOA
#define SPI1_CS_PORT                GPIOA
#define SPI1_CS_PIN                 GPIO_Pin_1

#define SPI1_SCK_CLK              	RCC_AHB1Periph_GPIOA
#define SPI1_SCK_PORT             	GPIOA
#define SPI1_SCK_PIN                GPIO_Pin_5
#define SPI1_SCK_PINSOURCE			GPIO_PinSource5
	
#define SPI1_MISO_CLK          		RCC_AHB1Periph_GPIOA
#define SPI1_MISO_PORT         		GPIOA
#define SPI1_MISO_PIN               GPIO_Pin_6
#define SPI1_MISO_PINSOURCE         GPIO_PinSource6
	
#define SPI1_MOSI_CLK          		RCC_AHB1Periph_GPIOA
#define SPI1_MOSI_PORT         		GPIOA
#define SPI1_MOSI_PIN               GPIO_Pin_7
#define SPI1_MOSI_PINSOURCE         GPIO_PinSource7

#define SPI1_CS_LOW      		   SPI1_CS_PORT->BSRRH = SPI1_CS_PIN;
#define SPI1_CS_HIGH     		   SPI1_CS_PORT->BSRRL = SPI1_CS_PIN;

#define SPI2_CS_CLK                 RCC_AHB1Periph_GPIOB
#define SPI2_CS_PORT                GPIOB
#define SPI2_CS_PIN                 GPIO_Pin_12
	
#define SPI2_SCK_CLK              	RCC_AHB1Periph_GPIOB
#define SPI2_SCK_PORT             	GPIOB
#define SPI2_SCK_PIN                GPIO_Pin_13
#define SPI2_SCK_PINSOURCE			GPIO_PinSource13
	
#define SPI2_MISO_CLK          		RCC_AHB1Periph_GPIOB
#define SPI2_MISO_PORT         		GPIOB
#define SPI2_MISO_PIN               GPIO_Pin_14
#define SPI2_MISO_PINSOURCE         GPIO_PinSource14
	
#define SPI2_MOSI_CLK          		RCC_AHB1Periph_GPIOB
#define SPI2_MOSI_PORT         		GPIOB
#define SPI2_MOSI_PIN               GPIO_Pin_15
#define SPI2_MOSI_PINSOURCE         GPIO_PinSource15

#define SPI2_CS_LOW      		   SPI2_CS_PORT->BSRRH = SPI2_CS_PIN;
#define SPI2_CS_HIGH     		   SPI2_CS_PORT->BSRRL = SPI2_CS_PIN;

/*********************************************************************/

void SPI1_Init(void);
uint8_t SPI1_SwapByte(uint8_t swap_byte);
uint8_t SPI1_ReadByte(uint8_t reg_address);
uint8_t SPI1_WriteByte(uint8_t reg_address, uint8_t write_byte);
uint8_t SPI1_ReadBuffer(uint8_t reg_address, uint8_t *read_pbuffer, uint8_t read_size);
uint8_t SPI1_WriteBuffer(uint8_t reg_address, uint8_t *write_pbuffer, uint8_t write_size);
void SPI2_Init(void);
uint8_t SPI2_SwapByte(uint8_t swap_byte);
uint8_t SPI2_ReadByte(uint8_t reg_address);
uint8_t SPI2_WriteByte(uint8_t reg_address, uint8_t write_byte);
uint8_t SPI2_ReadBuffer(uint8_t reg_address, uint8_t *read_pbuffer, uint8_t read_size);
uint8_t SPI2_WriteBuffer(uint8_t reg_address, uint8_t *write_pbuffer, uint8_t write_size);

#endif
