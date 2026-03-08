#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f10x.h"

#define FLASH_PAGE_SIZE  1024 			//扇区总字节
#define FLASH_PAGE_NUMBER 64 	 		//所选 STM32 的 FLASH 容量大小（单位为 KB）
#define FLASH_START_ADDR 0x08000000		//STM32 FLASH 的起始地址
#define FLASH_SAVE_ADDR  0x0800FE00		//设置保存至 FLASH 的目标地址

uint16_t Flash_ReadHalfWord(uint32_t address);
void Flash_ReadHalfBuffer(uint32_t read_addr, uint16_t* pbuffer, uint16_t size);

#endif
