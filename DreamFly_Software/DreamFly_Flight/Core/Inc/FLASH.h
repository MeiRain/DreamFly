#ifndef __FLASH_H
#define __FLASH_H

#include "stm32f4xx.h"

/* FLASH 扇区的起始地址 */
#define FLASH_SECTOR0_ADDR     ((uint32_t)0x08000000) 	//扇区0起始地址，16Kb
#define FLASH_SECTOR1_ADDR     ((uint32_t)0x08004000) 	//扇区1起始地址，16Kb 
#define FLASH_SECTOR2_ADDR     ((uint32_t)0x08008000) 	//扇区2起始地址，16Kb 
#define FLASH_SECTOR3_ADDR     ((uint32_t)0x0800C000) 	//扇区3起始地址，16Kb 
#define FLASH_SECTOR4_ADDR     ((uint32_t)0x08010000) 	//扇区4起始地址，64Kb 
#define FLASH_SECTOR5_ADDR     ((uint32_t)0x08020000) 	//扇区5起始地址，128Kb  
#define FLASH_SECTOR6_ADDR     ((uint32_t)0x08040000) 	//扇区6起始地址，128Kb 
#define FLASH_SECTOR7_ADDR     ((uint32_t)0x08060000) 	//扇区7起始地址，128Kb 
#define FLASH_SECTOR8_ADDR	   ((uint32_t)0x08080000)	//扇区8起始地址，128Kb 
#define FLASH_SECTOR9_ADDR     ((uint32_t)0x080A0000)	//扇区9起始地址，128Kb 
#define FLASH_SECTOR10_ADDR    ((uint32_t)0x080C0000)	//扇区10起始地址，128Kb 
#define FLASH_SECTOR11_ADDR    ((uint32_t)0x080E0000)	//扇区11起始地址，128Kb 
                      
#define FLASH_SECTOR12_ADDR     ((uint32_t)0x08100000)	//扇区12起始地址，16Kb
#define FLASH_SECTOR13_ADDR     ((uint32_t)0x08104000)	//扇区13起始地址，16Kb 
#define FLASH_SECTOR14_ADDR     ((uint32_t)0x08108000)	//扇区14起始地址，16Kb 
#define FLASH_SECTOR15_ADDR     ((uint32_t)0x0810C000)	//扇区15起始地址，16Kb 
#define FLASH_SECTOR16_ADDR     ((uint32_t)0x08110000)	//扇区16起始地址，64Kb 
#define FLASH_SECTOR17_ADDR     ((uint32_t)0x08120000)	//扇区17起始地址，128Kb 
#define FLASH_SECTOR18_ADDR     ((uint32_t)0x08140000)	//扇区18起始地址，128Kb 
#define FLASH_SECTOR19_ADDR     ((uint32_t)0x08160000)	//扇区19起始地址，128Kb 
#define FLASH_SECTOR20_ADDR     ((uint32_t)0x08180000)	//扇区20起始地址，128Kb
#define FLASH_SECTOR21_ADDR     ((uint32_t)0x081A0000)  //扇区21起始地址，128Kb
#define FLASH_SECTOR22_ADDR     ((uint32_t)0x081C0000)  //扇区22起始地址，128Kb
#define FLASH_SECTOR23_ADDR     ((uint32_t)0x081E0000)  //扇区23起始地址，128Kb

/* 设置 Flash 保存地址，地址必须为偶数，且所在扇区要大于程序代码所占用到的扇区，
   否则在写操作的时候，可能会导致擦除整个扇区，从而引起部分程序丢失导致死机。 */
#define FLASH_USER_START_ADDR  FLASH_SECTOR7_ADDR
#define FLASH_USER_END_ADDR  FLASH_SECTOR8_ADDR

void Flash_WriteBuffer(uint32_t write_addr, uint32_t *pbuffer, uint32_t size);
						   
#endif
