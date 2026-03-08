/************************************************************************
USART传输数据或者打印数据
移植时，只需修改下面代码修改区中的代码即可

主函数初始化代码为：
	USART_Config();
	
测试代码为：
	USART_Config();
	printf("vvvv");
************************************************************************/
#include <stdio.h>

#include "USART.h" 

/*******************************************************************/

#define USART_PRINTF USART1					//printf重定向串口

#define USART1_RX_CLK RCC_APB2Periph_GPIOA// 端口时钟
#define USART1_RX_PORT GPIOA              // 端口
#define USART1_RX_PIN GPIO_Pin_10         // 引脚

#define USART1_TX_CLK RCC_APB2Periph_GPIOA// 端口时钟
#define USART1_TX_PORT GPIOA              // 端口
#define USART1_TX_PIN GPIO_Pin_9          // 引脚

/*******************************************************************/



/******************************函数区************************************/
void USART1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(USART1_RX_CLK|USART1_TX_CLK, ENABLE);	//开启USART1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//开启GPIOA的时钟
	
	GPIO_InitStructure.GPIO_Pin = USART1_RX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(USART1_RX_PORT, &GPIO_InitStructure);					//将PA10引脚初始化为上拉输入	

	GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(USART1_TX_PORT, &GPIO_InitStructure);					//将PA9引脚初始化为复用推挽输出
	
	/*USART初始化*/
	USART_InitStructure.USART_BaudRate = 115200;				//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//硬件流控制，不需要
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//模式，发送模式和接收模式均选择
	USART_InitStructure.USART_Parity = USART_Parity_No;		//奇偶校验，不需要
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//停止位，选择1位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长，选择8位
	USART_Init(USART1, &USART_InitStructure);				//将结构体变量交给USART_Init，配置USART1
	
	/*USART使能*/
	USART_Cmd(USART1, ENABLE);								//使能USART1，串口开始运行
}	

//内部调用函数，注意要勾选 OPTIONS 中的 USE Micro LIB 选项
int fputc(int ch, FILE* f)
{
	USART_SendData(USART_PRINTF, ch);
	while(USART_GetFlagStatus(USART_PRINTF, USART_FLAG_TXE) == RESET);
	return ch;
}
