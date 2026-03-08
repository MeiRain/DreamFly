#include <stdio.h>

#include "USART.h"

/*******************************************************************
不同的串口挂载的总线不一样，时钟使能函数也不一样，串口 1 和 6 是
RCC_APB2PeriphClockCmd，串口 2 / 3 / 4 / 5 是 RCC_APB1PeriphClockCmd
********************************************************************/

/*******************************************************************/

#define USART_PRINTF USART1					//printf重定向串口

#define USART1_RX_CLK RCC_AHB1Periph_GPIOA// 端口时钟
#define USART1_RX_PORT GPIOA              // 端口
#define USART1_RX_PIN GPIO_Pin_10         // 引脚
#define USART1_RX_SOURCE GPIO_PinSource10 // 复用引脚

#define USART1_TX_CLK RCC_AHB1Periph_GPIOA// 端口时钟
#define USART1_TX_PORT GPIOA              // 端口
#define USART1_TX_PIN GPIO_Pin_9          // 引脚
#define USART1_TX_SOURCE GPIO_PinSource9  // 复用引脚

#define USART2_RX_CLK RCC_AHB1Periph_GPIOA// 端口时钟
#define USART2_RX_PORT GPIOA              // 端口
#define USART2_RX_PIN GPIO_Pin_3          // 引脚
#define USART2_RX_SOURCE GPIO_PinSource3  // 复用引脚

#define USART2_TX_CLK RCC_AHB1Periph_GPIOA// 端口时钟
#define USART2_TX_PORT GPIOA              // 端口
#define USART2_TX_PIN GPIO_Pin_2          // 引脚
#define USART2_TX_SOURCE GPIO_PinSource2  // 复用引脚

/*******************************************************************/

// static uint8_t RxBuffer[30],MVRxBuffer[10];
// static uint8_t RxCounter = 0,MVRxCounter = 0;

/*****************************************************************************
 *函  数：int fputc(int ch, FILE *f)
 *功  能：从写的一个printf()函数
 *参  数：ch 要发送的数据
 *返回值：无
 *备  注：无
 *****************************************************************************/
int fputc(int ch, FILE *f)
{
	USART_SendData(USART_PRINTF, ch);
	while (USART_GetFlagStatus(USART_PRINTF, USART_FLAG_TXE) == RESET)
		;
	return ch;
}

/*****************************************************************************
 *函  数：void USART1_Init(uint32_t Baudrate)
 *功  能：Usart1初始化为双工模式
 *参  数：Baudrate 波特率
 *返回值：无
 *备  注：对于连续的数据帧的接收 接收中断与空闲中断配合能解决丢包问题，
 *****************************************************************************/
void USART1_Init(uint32_t Baudrate)
{
	// 定义 GPIO 和 USART1 结构体变量
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开相应 GPIO 和 USART1 的时钟
	RCC_AHB1PeriphClockCmd(USART1_RX_CLK | USART1_TX_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// 复用为 USART1 的对应引脚
	GPIO_PinAFConfig(USART1_RX_PORT, USART1_RX_SOURCE, GPIO_AF_USART1);
	GPIO_PinAFConfig(USART1_TX_PORT, USART1_TX_SOURCE, GPIO_AF_USART1);

	// 配置 RX 引脚为上拉输入
	GPIO_InitStructure.GPIO_Pin = USART1_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(USART1_RX_PORT, &GPIO_InitStructure);

	// 配置 TX 引脚为复用推挽
	GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(USART1_TX_PORT, &GPIO_InitStructure);

	// USART1 结构体配置
	USART_InitStructure.USART_BaudRate = Baudrate;									// 波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长，选择 8 位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 停止位，选择1位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 奇偶校验，不需要
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 硬件流控制，关闭
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 模式，发送模式和接收模式均选择
	USART_Init(USART1, &USART_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // USART1 接收中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE); // USART1 空闲中断

	USART_Cmd(USART1, ENABLE); // USART1 使能
}

void USART1_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		;
}

void USART1_SendArray(uint8_t *pArray, uint8_t ArrayLength)
{
	uint16_t i;
	for (i = 0; i < ArrayLength; i++)
	{
		USART1_SendByte(*(pArray + i));
	}
}

/*****************************************************************************
 *函  数：void USART2_Init(uint32_t baudrate)
 *功  能：Usart2初始化为双工模式(ESP8266(wifi) 用此串口)
 *参  数：baudrate 波特率
 *返回值：无
 *备  注：对于连续的数据帧的接收 接收中断与空闲中断配合能解决丢包问题
 *****************************************************************************/
void USART2_Init(uint32_t Baudrate)
{
	// 定义 GPIO 和 USART2 结构体变量
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// 打开相应 GPIO 和 USART2 的时钟
	RCC_AHB1PeriphClockCmd(USART2_RX_CLK | USART2_TX_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	// 复用为 USART2 的对应引脚
	GPIO_PinAFConfig(USART2_RX_PORT, USART2_RX_SOURCE, GPIO_AF_USART2);
	GPIO_PinAFConfig(USART2_TX_PORT, USART2_TX_SOURCE, GPIO_AF_USART2);

	// 配置 RX 引脚为上拉输入
	GPIO_InitStructure.GPIO_Pin = USART2_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(USART2_RX_PORT, &GPIO_InitStructure);

	// 配置 TX 引脚为复用推挽
	GPIO_InitStructure.GPIO_Pin = USART2_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(USART2_TX_PORT, &GPIO_InitStructure);

	// USART2 结构体配置
	USART_InitStructure.USART_BaudRate = Baudrate;									// 波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长，选择 8 位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 停止位，选择1位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 奇偶校验，不需要
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 硬件流控制，关闭
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 模式，发送模式和接收模式均选择
	USART_Init(USART2, &USART_InitStructure);

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // USART2 接收中断
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); // USART2 空闲中断

	USART_Cmd(USART2, ENABLE); // USART2 使能
}

void USART2_SendByte(uint8_t Byte)
{
	USART_SendData(USART2, Byte);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET)
		;
}

void USART2_SendArray(uint8_t *pArray, uint8_t ArrayLength)
{
	uint16_t i;
	for (i = 0; i < ArrayLength; i++)
	{
		USART2_SendByte(*(pArray + i));
	}
}
