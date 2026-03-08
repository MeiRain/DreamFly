#include <stdio.h>
#include <string.h>

#include "NVIC.h"
#include "GPIO.h"
#include "USART.h"
#include "TIM.h"
#include "ADC.h"
#include "I2C.h"
#include "SPI.h"
#include "Flash.h"

#include "JoyStick.h"
#include "LED.h"
#include "WS2812B.h"
#include "Buzzer.h"
#include "Key.h"
#include "OLED.h"
#include "nRF24L01P.h"

#include "Remote.h"

// 数据拆分宏定义，在发送大于1字节的数据类型时，把数据拆分成单独字节进行发送
#define BYTE0(dwTemp) (*((char *)(&dwTemp)))
#define BYTE1(dwTemp) (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp) (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp) (*((char *)(&dwTemp) + 3))
	
#define COMMUNICATION_HEADER 0xAB
#define FLIGHT_ADDR 0x66
#define REMOTE_ADDR 0x88

static flash_t g_Flash;
static joystick_t g_JoyStickBias;//保存遥控器偏差值
static joystick_t g_JoyStickTempBias;//比较是否回调到原偏差值
static remoteflag_t g_RemoteFlag;
static remotedata_t g_RemoteData;
static flight_t g_Flight;

void Remote_HardwareCheck(void)
{
	uint8_t CheckFlag = 0;
	
	CheckFlag = nRF24L01P_Check();
	if (!CheckFlag)
	{
		while(1)
		{
			printf("nRF24L01P Connect Fail!\r\n\r\n");	
			WS2812B_Yellow();
		}		
	}
}

void Remote_ParameterInit(void)
{
	g_RemoteFlag.JoyStickOffSet = 0;
}

void Remote_AllInit(void)
{	
	//系统初始化
	NVIC_SystemInit();
	GPIO_DebugInit();
	USART1_Init();
	TIM2_TimerInit();
	ADC1_Init();
	I2C1_Init();
	SPI1_Init();
	
//	Remote_HardwareCheck();
	
	//外设初始化
	LED_Init();
	WS2812B_Init();
	Buzzer_Init();
	Key_Init();
	OLED_Init();
	nRF24L01P_Init(MODE_TX2);
	
	Remote_ParameterInit();
}

void Remote_ReadFlashData(void)
{
	uint8_t size;
	size = sizeof(g_Flash)/sizeof(uint16_t);
//	STMFLASH_Read(FLASH_SAVE_ADDR,(uint16_t*)(&g_Flash),size);
	TX_ADDRESS[4] = g_Flash.TX_Address;
	RX_ADDRESS[4] = g_Flash.RX_Address;
	g_JoyStickBias.ROL = g_Flash.ROL_Bias;
	g_JoyStickBias.PIT = g_Flash.PIT_Bias; 
	g_JoyStickBias.YAW = g_Flash.YAW_Bias; 
	g_JoyStickBias.THR = g_Flash.THR_Bias;
}

void Remote_WriteFlashData(void)
{
	uint8_t size;
	g_Flash.TX_Address = TX_ADDRESS[4];
	g_Flash.RX_Address = RX_ADDRESS[4];
	g_Flash.ROL_Bias = g_JoyStickBias.ROL;
	g_Flash.PIT_Bias = g_JoyStickBias.PIT;
	g_Flash.YAW_Bias = g_JoyStickBias.YAW;
	g_Flash.THR_Bias = g_JoyStickBias.THR;
	size = sizeof(g_Flash)/sizeof(uint16_t);//保存的数据长度
//	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)(&g_Flash),size);
}

void Remote_JoyStickScan(void)
{
	//读取摇杆ADC值并转换为协议值、去偏差和限幅
	g_RemoteData.ROL = (1000 + ADC_GetValue(ADC_ROL) * 1000 / 4095) - g_JoyStickBias.ROL;
	g_RemoteData.ROL = (g_RemoteData.ROL <= 1000) ? 1000 : g_RemoteData.ROL;		//摇杆限幅，防止越界
	g_RemoteData.ROL = (g_RemoteData.ROL >= 2000) ? 2000 : g_RemoteData.ROL;		//摇杆限幅，防止越界
	
	g_RemoteData.PIT = (1000 + ADC_GetValue(ADC_PIT) * 1000 / 4095) - g_JoyStickBias.PIT;
	g_RemoteData.PIT = (g_RemoteData.PIT <= 1000) ? 1000 : g_RemoteData.PIT;		//摇杆限幅，防止越界
	g_RemoteData.PIT = (g_RemoteData.PIT >= 2000) ? 2000 : g_RemoteData.PIT;		//摇杆限幅，防止越界
	
	g_RemoteData.YAW = (1000 + ADC_GetValue(ADC_YAW) * 1000 / 4095) - g_JoyStickBias.YAW;
	g_RemoteData.YAW = (g_RemoteData.YAW <= 1000) ? 1000 : g_RemoteData.YAW;		//摇杆限幅，防止越界
	g_RemoteData.YAW = (g_RemoteData.YAW >= 2000) ? 2000 : g_RemoteData.YAW;		//摇杆限幅，防止越界
	
	g_RemoteData.THR = (1000 + ADC_GetValue(ADC_THR) * 1000 / 4095) - g_JoyStickBias.THR;
	g_RemoteData.THR = (g_RemoteData.THR <= 1000) ? 1000 : g_RemoteData.THR;		//摇杆限幅，防止越界
	g_RemoteData.THR = (g_RemoteData.THR >= 2000) ? 2000 : g_RemoteData.THR;		//摇杆限幅，防止越界
	
	if (g_RemoteFlag.JoyStickOffSet == 1)
	{
		Remote_JoyStickOffSet();
	}	

	static uint8_t cnt = 0;
	cnt++;
	if(cnt == 20)
	{
		printf("RC_Value: %d   %d   %d   %d\r\n",
		g_RemoteData.ROL, g_RemoteData.PIT, g_RemoteData.YAW, g_RemoteData.THR);
		printf("======================================\r\n");
		printf("ADC_Calibrator1:%d\r\n", g_JoyStickBias.ROL);
		printf("ADC_Calibrator2:%d\r\n", g_JoyStickBias.PIT);
		printf("ADC_Calibrator3:%d\r\n", g_JoyStickBias.YAW);
		printf("ADC_Calibrator4:%d\r\n", g_JoyStickBias.THR);
		cnt = 0;
	}
}

static void Remote_JoyStickOffSet(void)
{
	static uint8_t cnt = 0;
	static int32_t sum[4];
	
	if (g_RemoteData.THR < 1015)
	{
		if (cnt == 0)
		{
			memset(sum, 0, sizeof(sum));
			g_JoyStickBias.ROL = 0;
			g_JoyStickBias.PIT = 0;
			g_JoyStickBias.YAW = 0;
			g_JoyStickBias.THR = 0;
		}

		cnt ++;
		sum[0] += g_RemoteData.ROL;
		sum[1] += g_RemoteData.PIT;
		sum[2] += g_RemoteData.YAW;
		sum[3] += g_RemoteData.THR;
		
		if(cnt >= 60)
		{
			g_JoyStickTempBias.ROL = g_JoyStickBias.ROL = sum[0] / cnt - 1500;				//求出 60 次和的平均数
			g_JoyStickTempBias.PIT = g_JoyStickBias.PIT = sum[1] / cnt - 1500;					
			g_JoyStickTempBias.YAW = g_JoyStickBias.YAW = sum[2] / cnt - 1500;					
			g_JoyStickTempBias.THR = g_JoyStickBias.THR = sum[3] / cnt - 1000;					
			g_RemoteFlag.JoyStickOffSet = 0;
			cnt = 0;
//			PID_WriteFlash();
		}
	}
}

void Remote_KeyScanAndHandle(void)
{
	static uint8_t KeyNumber = 0x00;
	
	KeyNumber = Key_Scan();
	switch (KeyNumber)
	{
		case KEY1_SINGLE:
            printf("  This is 11 SINGLE key ! \r\n" );
			g_RemoteData.KEY ^= KEY1_SINGLE;
            break;
		
        case KEY1_LONG:
            printf("  This is 11 LONG key ! \r\n" );
			g_RemoteFlag.JoyStickOffSet = 1;
			break;
		
        case KEY2_SINGLE:
            printf("  This is 22 SINGLE key ! \r\n" );
			g_RemoteData.KEY ^= KEY2_SINGLE;
            break;
		
        case KEY2_LONG:
            printf("  This is 22 LONG key ! \r\n" );
			g_RemoteData.KEY ^= 0x80;
            break;
		
        case KEY3_SINGLE:
//            printf("  This is 33 SINGLE key ! \r\n" );
		
			g_JoyStickBias.ROL += 10;
					if (g_JoyStickTempBias.ROL == g_JoyStickBias.ROL)
			g_RemoteFlag.BuzzerBeep = 1;
			g_RemoteData.KEY = KEY3_SINGLE;
            break;
		
		case KEY4_SINGLE:
//            printf("  This is 44 SINGLE key ! \r\n" );
		
			g_JoyStickBias.PIT -= 10;
					if (g_JoyStickTempBias.PIT == g_JoyStickBias.PIT)
			g_RemoteFlag.BuzzerBeep = 1;
			g_RemoteData.KEY = KEY4_SINGLE;
            break;
		
        case KEY5_SINGLE:
//            printf("  This is 55 SINGLE key ! \r\n" );
		
			g_JoyStickBias.ROL -= 10;
					if (g_JoyStickTempBias.ROL == g_JoyStickBias.ROL)
			g_RemoteFlag.BuzzerBeep = 1;
			g_RemoteData.KEY = KEY5_SINGLE;
            break;
		
        case KEY6_SINGLE:
//            printf("  This is 66 SINGLE key ! \r\n" );
		
			g_JoyStickBias.PIT += 10;
				if (g_JoyStickTempBias.PIT == g_JoyStickBias.PIT)
			g_RemoteFlag.BuzzerBeep = 1;
			g_RemoteData.KEY = KEY6_SINGLE;
            break;
		
        default:
            break;
	}
}

void Remote_BuzzerControl(void)
{
	static uint8_t BuzzerCount = 0;
	if(g_RemoteFlag.BuzzerBeep == 1)	//微调回原始值就响2声蜂鸣器
	{
		BuzzerCount++;								     //声音延时用
		if(BuzzerCount==1||BuzzerCount==9||BuzzerCount==17)   //延时判断																							
			Buzzer_On();

		else if((BuzzerCount>9 && BuzzerCount<17) || (BuzzerCount>30 && BuzzerCount<40) || (BuzzerCount>50 && BuzzerCount<61))  //延时判断
		{
			Buzzer_Off();  						  
			if(BuzzerCount==60)
			{
				g_RemoteFlag.BuzzerBeep = 0;
				BuzzerCount=0;
			}
		}		 
	}
}

void Remote_ReceiveFlightData(void)
{
	uint8_t Sum_Check = 0;
	uint8_t PacketLength;
	uint8_t nRF24L01P_RXBuffer[RX_PAYLOAD_WIDTH];	//Si24R1 接收数组	

	nRF24L01P_IRQHandler(nRF24L01P_RXBuffer);//接收遥控器数据

	//判断帧头、源地址、目标地址
	if(!(*(nRF24L01P_RXBuffer) == COMMUNICATION_HEADER
		&& *(nRF24L01P_RXBuffer + 1) == FLIGHT_ADDR
		&& *(nRF24L01P_RXBuffer + 2) == REMOTE_ADDR))
		return;
	
	PacketLength = *(nRF24L01P_RXBuffer + 3) + 4;
	
//	printf("%d",PacketLength);
	
    for(uint8_t i = 0; i < PacketLength; i ++)
        Sum_Check += *(nRF24L01P_RXBuffer + i);
	
	
    if(!(Sum_Check==*(nRF24L01P_RXBuffer  + PacketLength)))
		return;     //判断sum
	
//	g_Flight.MOD = (int16_t)(*(nRF24L01P_RXBuffer+4);
	g_Flight.ROL = (int16_t)(*(nRF24L01P_RXBuffer+5)<<8)|*(nRF24L01P_RXBuffer+6);
	g_Flight.PIT = (int16_t)(*(nRF24L01P_RXBuffer+7)<<8)|*(nRF24L01P_RXBuffer+8);
	g_Flight.YAW = (int16_t)(*(nRF24L01P_RXBuffer+9)<<8)|*(nRF24L01P_RXBuffer+10);
	g_Flight.THR = (int16_t)(*(nRF24L01P_RXBuffer+11)<<8)|*(nRF24L01P_RXBuffer+12);
	g_Flight.ALT = (int16_t)(*(nRF24L01P_RXBuffer+13)<<8)|*(nRF24L01P_RXBuffer+14);
	g_Flight.BAT = (int16_t)(*(nRF24L01P_RXBuffer+15)<<8)|*(nRF24L01P_RXBuffer+16);
	
//	static uint8_t cnt = 0;
//	cnt++;
//	if(cnt == 50)
//	{
//		printf("RC_Value: %d   %d   %d   %d\r\n",
//		g_Flight.ROL, g_Flight.PIT, g_Flight.YAW, g_Flight.THR);
//		printf("======================================\r\n");
//		cnt = 0;
//	}
}

void Remote_SendDataToFlight(void)
{
	uint8_t cnt = 0;
	uint8_t Sum_Check = 0;
	uint8_t nRF24L01P_TXBuffer[TX_PAYLOAD_WIDTH];		//nRF24L01P 发送数组
	
	nRF24L01P_TXBuffer[cnt++] = COMMUNICATION_HEADER;	//帧头
	nRF24L01P_TXBuffer[cnt++] = REMOTE_ADDR;			//源地址
	nRF24L01P_TXBuffer[cnt++] = FLIGHT_ADDR;			//目标地址
	nRF24L01P_TXBuffer[cnt++] = 0x09;					//数据包长度

	nRF24L01P_TXBuffer[cnt++] = BYTE1(g_RemoteData.ROL);      //ROL
	nRF24L01P_TXBuffer[cnt++] = BYTE0(g_RemoteData.ROL);      
	nRF24L01P_TXBuffer[cnt++] = BYTE1(g_RemoteData.PIT);		//THR
	nRF24L01P_TXBuffer[cnt++] = BYTE0(g_RemoteData.PIT);      
	nRF24L01P_TXBuffer[cnt++] = BYTE1(g_RemoteData.YAW);      //ROLL
	nRF24L01P_TXBuffer[cnt++] = BYTE0(g_RemoteData.YAW);      
	nRF24L01P_TXBuffer[cnt++] = BYTE1(g_RemoteData.THR);      //PITCH
	nRF24L01P_TXBuffer[cnt++] = BYTE0(g_RemoteData.THR);      	
	nRF24L01P_TXBuffer[cnt++] = g_RemoteData.KEY;      	//按键

	for(uint8_t i = 0; i < cnt; i++)
		Sum_Check += nRF24L01P_TXBuffer[i];

	nRF24L01P_TXBuffer[cnt++] = Sum_Check;

	nRF24L01P_NormalPacket(nRF24L01P_TXBuffer, cnt);
}
