#include "STM32F103C8T6.h"

#define INT160(dwTemp)       (*(int16_t*)(&dwTemp))
#define INT161(dwTemp)       (*((int16_t *)(&dwTemp) + 1))
#define INT162(dwTemp)       (*((int16_t *)(&dwTemp) + 2))
#define INT163(dwTemp)       (*((int16_t *)(&dwTemp) + 3))

extern uint8_t  TX_ADDRESS[5];
extern uint8_t  RX_ADDRESS[5];

extern uint8_t ADC_CALIBRATOR_READY; 

SAVE_TYPE flash;

void ParamsToTable(void)
{
	flash.TXaddr = TX_ADDRESS[4];
	flash.RXaddr = RX_ADDRESS[4];
	flash.YAW = ADC_OffsetValue[0];
	flash.THR = ADC_OffsetValue[1];
	flash.ROL = ADC_OffsetValue[2];
	flash.PIT = ADC_OffsetValue[3];
}

void TableToParams(void)
{
	TX_ADDRESS[4] = flash.TXaddr;
	RX_ADDRESS[4] = flash.RXaddr;
	ADC_OffsetValue[0] = flash.YAW;
	ADC_OffsetValue[1] = flash.THR;
	ADC_OffsetValue[2] = flash.ROL;
	ADC_OffsetValue[3] = flash.PIT;
}
void ParamsClearAll(void)
{
	flash.TXaddr = 0;
	flash.RXaddr = 0;
	flash.YAW = 0xFF;
	flash.THR = 0xFF;
	flash.ROL = 0xFF;
	flash.PIT = 0xFF;
}


void PID_ClearFlash(void)
{
	uint8_t size;
	ParamsClearAll();//数据清除
	size = sizeof(flash)/sizeof(uint16_t);//保存的数据长度
	STM32FLASH_Write(FLASH_SAVE_ADDR,(u16*)(&flash),size);
}
void PID_WriteFlash(void)
{
	uint8_t size;
	ParamsToTable();//浮点数转换成整数
	size = sizeof(flash)/sizeof(uint16_t);//保存的数据长度
	STM32FLASH_Write(FLASH_SAVE_ADDR,(u16*)(&flash),size);
}
void PID_ReadFlash(void)
{
	uint8_t size;
	size = sizeof(flash)/sizeof(uint16_t);
	STM32FLASH_Read(FLASH_SAVE_ADDR,(u16*)(&flash),size);
	TableToParams();//将整数转换成浮点数
}

void Flash_DataRead(void)
{
	PID_ReadFlash();
	if(ADC_OffsetValue[0]==0xFFFF && ADC_OffsetValue[1]==0xFFFF && ADC_OffsetValue[2]==0xFFFF && ADC_OffsetValue[3]==0xFFFF)
	{
		ADC_CALIBRATOR_READY = 1;
	}else
	{
	  ADC_CALIBRATOR_READY = 0;
	}
}
