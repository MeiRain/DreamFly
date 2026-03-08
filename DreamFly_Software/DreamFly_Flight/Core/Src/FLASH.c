#include "USART.h"
#include <stdio.h>
#include "Flash.h"

static uint32_t Flash_GetSector(uint32_t address)
{
    uint32_t sector = 0;

	if ((address >= FLASH_SECTOR0_ADDR) && (address < FLASH_SECTOR1_ADDR)) 
        sector = FLASH_Sector_0;
    else if ((address >= FLASH_SECTOR1_ADDR) && (address < FLASH_SECTOR2_ADDR))
        sector = FLASH_Sector_1;
    else if ((address >= FLASH_SECTOR2_ADDR) && (address < FLASH_SECTOR3_ADDR))
        sector = FLASH_Sector_2;
    else if ((address >= FLASH_SECTOR3_ADDR) && (address < FLASH_SECTOR4_ADDR))
        sector = FLASH_Sector_3;
    else if ((address >= FLASH_SECTOR4_ADDR) && (address < FLASH_SECTOR5_ADDR))
        sector = FLASH_Sector_4;
    else if ((address >= FLASH_SECTOR5_ADDR) && (address < FLASH_SECTOR6_ADDR))
        sector = FLASH_Sector_5;		
    else if ((address >= FLASH_SECTOR6_ADDR) && (address < FLASH_SECTOR7_ADDR))
        sector = FLASH_Sector_6;	
	else if ((address >= FLASH_SECTOR7_ADDR) && (address < FLASH_SECTOR8_ADDR))
        sector = FLASH_Sector_7;
	else if ((address >= FLASH_SECTOR8_ADDR) && (address < FLASH_SECTOR9_ADDR))
        sector = FLASH_Sector_8;
	else if ((address >= FLASH_SECTOR9_ADDR) && (address < FLASH_SECTOR10_ADDR))
        sector = FLASH_Sector_9;
	else if ((address >= FLASH_SECTOR10_ADDR) && (address < FLASH_SECTOR11_ADDR))
        sector = FLASH_Sector_10;
	else if ((address >= FLASH_SECTOR11_ADDR) && (address < FLASH_SECTOR12_ADDR))
        sector = FLASH_Sector_11;
	else if ((address >= FLASH_SECTOR12_ADDR) && (address < FLASH_SECTOR13_ADDR))
        sector = FLASH_Sector_12;
	else if ((address >= FLASH_SECTOR13_ADDR) && (address < FLASH_SECTOR14_ADDR))
        sector = FLASH_Sector_13;
	else if ((address >= FLASH_SECTOR14_ADDR) && (address < FLASH_SECTOR15_ADDR))
        sector = FLASH_Sector_14;
	else if ((address >= FLASH_SECTOR15_ADDR) && (address < FLASH_SECTOR16_ADDR))
        sector = FLASH_Sector_15;
	else if ((address >= FLASH_SECTOR16_ADDR) && (address < FLASH_SECTOR17_ADDR))
        sector = FLASH_Sector_16;
	else if ((address >= FLASH_SECTOR17_ADDR) && (address < FLASH_SECTOR18_ADDR))
        sector = FLASH_Sector_17;
	else if ((address >= FLASH_SECTOR18_ADDR) && (address < FLASH_SECTOR19_ADDR))
        sector = FLASH_Sector_18;
	else if ((address >= FLASH_SECTOR19_ADDR) && (address < FLASH_SECTOR20_ADDR))
        sector = FLASH_Sector_19;
	else if ((address >= FLASH_SECTOR20_ADDR) && (address < FLASH_SECTOR21_ADDR))
        sector = FLASH_Sector_20;
	else if ((address >= FLASH_SECTOR21_ADDR) && (address < FLASH_SECTOR22_ADDR))
        sector = FLASH_Sector_21;
	else if ((address >= FLASH_SECTOR22_ADDR) && (address < FLASH_SECTOR23_ADDR))
        sector = FLASH_Sector_22;
    else  /*((address >= FLASH_SECTOR23_ADDR) && (address < FLASH_END_ADDR))*/
        sector = FLASH_Sector_23;

    return sector;
}

u32 STMFLASH_ReadWord(u32 faddr)
{
	return *(vu32*)faddr; 
}  

void Flash_WriteBuffer(uint32_t write_addr, uint32_t *pbuffer, uint32_t size)
{
	FLASH_Status status = FLASH_COMPLETE;
	uint32_t WriteStartAddr=0;
	uint32_t WriteEndAddr=0;	
	if(write_addr < FLASH_SECTOR0_ADDR || write_addr%4)
		return;	//非法地址
	
	FLASH_Unlock();									 //解锁 
	FLASH_DataCacheCmd(DISABLE);     //FLASH擦除期间,必须禁止数据缓存
 		
	WriteStartAddr=write_addr;				         //写入的起始地址
	WriteEndAddr=write_addr+size*4;	 //写入的结束地址
	if(WriteStartAddr<0X1FFF0000)			       //只有主存储区,才需要执行擦除操作!!
	{
		while(WriteStartAddr<WriteEndAddr)		       //扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
		{
			if(STMFLASH_ReadWord(WriteStartAddr)!=0XFFFFFFFF)//有非0XFFFFFFFF的地方,要擦除这个扇区
			{   
				status=FLASH_EraseSector(Flash_GetSector(WriteStartAddr),VoltageRange_3);//VCC=2.7~3.6V之间!!
				if(status!=FLASH_COMPLETE)break;	    //发生错误了
			}else WriteStartAddr+=4;
		} 
	}
	if(status==FLASH_COMPLETE)
	{
		while(write_addr<WriteEndAddr) //写数据
		{
			if(FLASH_ProgramWord(write_addr,*pbuffer)!=FLASH_COMPLETE) //写入数据
			{ 
				break; //写入异常
			}
			write_addr+=4;
			pbuffer++;
		} 
	}
  FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
	FLASH_Lock(); //上锁
} 

///******************************************************************************
//*函  数：void STMFLASH_Read(uint32_t ReadAddr, uint32_t *pbuffer, uint32_t NumToRead)   	 
//*功　能：从指定地址开始读出指定长度的数据
//*参  数：ReadAddr	起始地址
//*        pbuffer	数据指针
//*        NumToRead	字(4位)数
//*返回值：无
//*备  注：无
//*******************************************************************************/
//void Flash_ReadBuffer(uint32_t ReadAddr, uint32_t *pbuffer, uint32_t NumToRead)   	
//{
//	__IO uint32_t uwData32 = 0;
//    __IO uint32_t uwMemoryProgramStatus = 0;
//	
//	uint32_t i;
//	for(i = 0; i < NumToRead; i ++)
//	{
//		pbuffer[i] = InternalFLASH_ReadWord(ReadAddr);//读取4个字节.
//		ReadAddr += 4;//偏移4个字节.	
//	}
//}

///******************************************************************************
//*函  数：uint32_t STMFLASH_ReadWord(uint32_t faddr)
//*功　能：读取指定地址的字(32位数据) 
//*参  数：faddr:读地址 
//*返回值：对应数据
//*备  注：无
//*******************************************************************************/
//uint32_t Flash_ReadWord(uint32_t addr)
//{
//	return *((__IO uint32_t*)addr); 
//}
