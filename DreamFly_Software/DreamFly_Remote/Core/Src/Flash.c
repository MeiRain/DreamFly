#include "Flash.h"

uint16_t STMFLASH_BUF[FLASH_PAGE_SIZE / 2];		//和 FLASH 地址对应的 SRAM 数组，最多是2K字节

/*****************************************************************************
*函  数：uint16_t STM32FLASH_ReadHalfWord(uint32_t address)
*功  能：读取 FLASH 一个16位的半字
*参  数：address 要读取数据的半字地址
*返回值：指定地址下的数据
*备  注：无
*****************************************************************************/
uint16_t Flash_ReadHalfWord(uint32_t read_addr)
{
	return *((__IO uint16_t*)(read_addr)); 
}

/*****************************************************************************
*函  数：void STM32FLASH_Read(uint32_t ReadAddr, uint16_t* pbuffer, uint16_t NumToRead)
*功  能：从指定地址开始读出指定长度的数据
*参  数：ReadAddr：  起始地址
*        pbuffer：   数据指针
*        size：半字（16位）数
*返回值：无
*备  注：无
*****************************************************************************/
void Flash_ReadHalfBuffer(uint32_t read_addr, uint16_t* pbuffer, uint16_t size)   	
{
	uint16_t i;
	for(i = 0; i < size; i ++)
	{
		pbuffer[i] = Flash_ReadHalfWord(read_addr);		//读取2个字节.
		read_addr += 2;//偏移2个字节.	
	}
} 

/*****************************************************************************
*函  数：void STM32FLASH_Save_NoUnlock(uint32_t write_addr, uint16_t* pbuffer, uint16_t size)
*功  能：不解锁的写入，不可单独使用
*参  数：write_addr： 起始地址
*        pbuffer：   数据指针
*        size：半字（16位）数
*返回值：指定地址下的数据
*备  注：无
*****************************************************************************/
void Flash_Save_NoUnlock(uint32_t write_addr, uint16_t* pbuffer, uint16_t size)   
{ 			 		 
	uint16_t i;
	for(i = 0; i < size; i ++)
	{
		FLASH_ProgramHalfWord(write_addr, pbuffer[i]);
	    write_addr += 2;									//地址增加2
	}  
}

/*****************************************************************************
*函  数：void STM32FLASH_Write(uint32_t write_addr, uint16_t* pbuffer, uint16_t size)	
*功  能：从指定地址开始写入指定长度的数据
*参  数：write_addr： 起始地址
*        pbuffer：   数据指针
*        size：半字（16位）数
*返回值：无
*备  注：1. 根据写的起始地址，确定要写的起始位置Page号以及结束位置在Page中偏移量
		 2. 根据写的起始地址和字节数判断是否跨Page写数据
		 3. 根据以上信息确定要操作的Page以及地址范围
		 4. 对于每一个Page，先遍历地址保存的数据是不是0xFF，如果都是就不用擦除，如果
有不是的，先读出里面数据，保存在缓存中再擦除Flash，再将要写入的数据先写入缓存，再写入到Flash
*****************************************************************************/
void Flash_WriteHalfBuffer(uint32_t write_addr, uint16_t* pbuffer, uint16_t size)	
{
	uint16_t i;
	uint32_t OffSetADDR;   		//相对于 FLASH 基地址的偏移地址，32 位地址
	uint32_t SectorADDR;	   	//扇区地址，范围为 0 ~ 127，32 位地址
	uint16_t SECOffSetADDR;	   	//扇区内偏移地址（16 位字计算）
	uint16_t SECRemainADDR; 	//扇区内剩余地址（16 位字计算）	 
	
	//写的地址在 FLASH 基地址外，则为非法地址
	if(write_addr < FLASH_START_ADDR || (write_addr >= (FLASH_START_ADDR + 1024 * FLASH_PAGE_NUMBER)))
		return;									
	
	FLASH_Unlock();											//解锁 FLASH
	
	OffSetADDR = write_addr - FLASH_START_ADDR;				//偏移地址，为写入地址减基地址
	SectorADDR = OffSetADDR / FLASH_PAGE_SIZE;				//扇区地址，为偏移地址除每一扇区大小（1 KB）
	SECOffSetADDR = (OffSetADDR % FLASH_PAGE_SIZE) / 2;		//扇区内的偏移地址（2 个字节为基本单位）
	SECRemainADDR = FLASH_PAGE_SIZE / 2 - SECOffSetADDR;	//扇区剩余空间（地址）大小（2 个字节为基本单位）
	
	if(size <= SECRemainADDR)							//写入字节数不大于扇区剩余空间范围
		SECRemainADDR = size;							
	
	while(1) 
	{	
		Flash_ReadHalfBuffer(SectorADDR * FLASH_PAGE_SIZE + FLASH_START_ADDR, STMFLASH_BUF, FLASH_PAGE_SIZE / 2);		//读出整个扇区的内容
		for(i = 0; i < SECRemainADDR; i ++)					//读出数据并校验数据
		{
			if(STMFLASH_BUF[SECOffSetADDR + i] != 0xFFFF)
				break;										//跳出循环则表示不是第一次用，需要擦除，此时 i < SECRemainADDR 
		}
		
		if(i < SECRemainADDR)													//i 小于剩余空间表示需要擦除
		{
			FLASH_ErasePage(SectorADDR * FLASH_PAGE_SIZE + FLASH_START_ADDR);	//擦除这个扇区
			for(i = 0;i < SECRemainADDR; i ++)									
			{
				STMFLASH_BUF[i + SECOffSetADDR] = pbuffer[i];	  				//将即将写入数组转移到地址对应的 SRAM 的 ”FLASH“ 数组
			}
			Flash_Save_NoUnlock(SectorADDR * FLASH_PAGE_SIZE + FLASH_START_ADDR, STMFLASH_BUF, FLASH_PAGE_SIZE / 2);	//写入整个扇区  
		}
		else 
			Flash_Save_NoUnlock(write_addr, pbuffer, SECRemainADDR);		//已经擦除了的，直接写入扇区剩余区间			   
		if(size == SECRemainADDR)
			break;										//写入结束了
		else											//写入未结束
		{
			SectorADDR ++;								//扇区地址增 1
			SECOffSetADDR = 0;							//偏移位置为 0	 
		   	pbuffer += SECRemainADDR;  					//指针偏移
			write_addr += SECRemainADDR;					//写地址偏移	   
		   	size -= SECRemainADDR;				//写入字节（16位）数减去已写入字节数
			if(size > (FLASH_PAGE_SIZE/2))
				SECRemainADDR = FLASH_PAGE_SIZE / 2;	//下一个扇区还是写不完
			else 
				SECRemainADDR = size;				//下一个扇区可以写完了
		} 
	}
	FLASH_Lock();										//上锁
}

///*****************************************************************************
//*函  数：Test_Write(uint32_t write_addr, uint16_t WriteData)	
//*功  能：从指定地址开始写入一个半字
//*参  数：write_addr：起始地址
//*        WriteData：写入的数据
//*返回值：无
//*备  注：无
//*****************************************************************************/
//void Test_Write(uint32_t write_addr, uint16_t WriteData)   	
//{
//	STM32FLASH_Write(write_addr, &WriteData, 1); 
//}

