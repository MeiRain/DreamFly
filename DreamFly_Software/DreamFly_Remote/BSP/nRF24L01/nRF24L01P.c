#include "SPI.h"

#include "nRF24L01P.h"
#include "nRF24L01P_Register.h"

/**********************************************代码移植修改区**********************************************/
					                                                                
#define nRF24L01P_CE_RCC    		RCC_APB2Periph_GPIOA  									//端口时钟
#define nRF24L01P_CE_PORT   		GPIOA                 									//端口
#define nRF24L01P_CE_PIN        	GPIO_Pin_3            									//引脚
					                                                                
#define nRF24L01P_IRQ_RCC   		RCC_APB2Periph_GPIOB 									//端口时钟
#define nRF24L01P_IRQ_PORT  		GPIOB                 									//端口
#define nRF24L01P_IRQ_PIN       	GPIO_Pin_3            									//引脚
                                                                                    
#define nRF24L01P_CE_LOW    		nRF24L01P_CE_PORT->BRR  |= nRF24L01P_CE_PIN               // CE 引脚低电平
#define nRF24L01P_CE_HIGH   		nRF24L01P_CE_PORT->BSRR |= nRF24L01P_CE_PIN               // CE 引脚高电平
                                                                         
/**********************************************************************************************************/

uint8_t TX_ADDRESS[TX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0xFF};	
uint8_t RX_ADDRESS[RX_ADR_WIDTH]= {0x34,0x43,0x10,0x10,0xFF};	

/******************************************** 全局变量定义区 *********************************************/

//__IO uint8_t status;							//接收从STATUS寄存器中返回的值
//uint8_t FLYDataRx_OK = 0;                      	//飞机数据接收完成
//uint8_t FLY_Connect_OK = 0;                    	//遥控与飞机已连接
//uint8_t Reconnection_flag = 0;                 	//遥控与飞机已断开需要重连
//extern __IO uint8_t KeyNumber;					//用来保存哪个按键刚被按下了，原始定义在：Key.c文件中
//extern uint16_t TX_CNT ;                    	//遥控数据发送计数，原始定义在：SendData.c文件中
//extern uint16_t TX_ERROR ;                   	//遥控数据发送失败计数，原始定义在：SendData.c文件中
//extern uint8_t ADC_CALIBRATOR_READY;        		//遥控通道ADC校准标志，原始定义在：ADC_DMA.h中
/**********************************************************************************************************/


/*************************************************************************
*函  数：uint8_t nRF24L01P_Check(void)
*功  能：检查 nRF24L01P 与 MCU 的 SPI 总线是否通信正常
*参  数：无
*返回值：0 未连接；1 已连接 
*备  注：此函数通过写入发送地址并读取地址，禁止在初始化后调用！！！
**************************************************************************/
uint8_t nRF24L01P_Check(void)
{
	uint8_t i = 0;
	uint8_t Buffer[5] = {0xA8, 0xA8, 0xA8, 0xA8, 0xA8};	 
	nRF24L01P_WriteBuffer(W_REGISTER + TX_ADDR, Buffer, 5); 				//写入5个字节的地址
	nRF24L01P_ReadBuffer(TX_ADDR, Buffer, 5); 								//读出写入的地址  
	for (i = 0; i < 5; i ++)
	{
		if (Buffer[i] != 0xA8)
			break;	 		
	}		
	if (i != 5)
		return 0;
	else
		return 1;				
}

/*************************************************************************
*函  数：void nRF24L01P_Init(void)
*功  能：nRF24L01P 引脚 GPIO 初始化
*参  数：无
*返回值：无
*备  注：中断产生时，IRQ 引脚会被拉低，所以这里要配置成上拉输入
**************************************************************************/
void nRF24L01P_Init(uint8_t Mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(nRF24L01P_CE_RCC | nRF24L01P_IRQ_RCC, ENABLE);
	
	//初始化 CE 引脚为推挽输出
	GPIO_InitStructure.GPIO_Pin = nRF24L01P_CE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(nRF24L01P_CE_PORT, &GPIO_InitStructure);

	//配置 IRQ 引脚为上拉输入
	GPIO_InitStructure.GPIO_Pin = nRF24L01P_IRQ_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 				
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(nRF24L01P_IRQ_PORT, &GPIO_InitStructure);
	
	nRF24L01P_SetMode(Mode);
}

void nRF24L01P_SetMode(uint8_t Mode)
{
	nRF24L01P_CE_LOW;

	nRF24L01P_WriteBuffer(W_REGISTER + TX_ADDR, (uint8_t*)TX_ADDRESS, TX_ADR_WIDTH); 		//写 TX节点 的地址 TX_ADDR（发送与接收地址需要一致）
	nRF24L01P_WriteBuffer(W_REGISTER + RX_ADDR_P0, (uint8_t*)RX_ADDRESS, RX_ADR_WIDTH);	//写 RX节点 的地址（主要是为了使能 Auto Ack） RX_ADDR_P0
	nRF24L01P_WriteByte(W_REGISTER + EN_AA, 0x01); 										//使能 AUTO ACK EN_AA
	nRF24L01P_WriteByte(W_REGISTER + EN_RXADDR, 0x01);										//使能 PIPE 0 EN_RXADDR
	nRF24L01P_WriteByte(W_REGISTER + SETUP_RETR, 0x1A);									//配置自动重发次数 SETUP_RETR
	nRF24L01P_WriteByte(W_REGISTER + RF_CH, 0x48);	                                    	//选择通信频率 RF_CH
	nRF24L01P_WriteByte(W_REGISTER + RF_SETUP, 0x0F); 										//配置发射参数（低噪放大器增益、发射功率、无线速率） RF_SETUP								
                                                                              
	if (Mode == 1)																		//普通 RX模式
	{
		nRF24L01P_WriteByte(W_REGISTER + RX_PW_P0, RX_PAYLOAD_WIDTH);						//选择通道 0 有效数据宽度 RX_PW_P0	
		nRF24L01P_WriteByte(W_REGISTER + CONFIG, 0x0F);   									//IRQ 收发完成中断开启，16位 CRC，主接收
	}
	
	else if (Mode == 2)																	//普通 TX模式
	{
		nRF24L01P_WriteByte(W_REGISTER + RX_PW_P0, RX_PAYLOAD_WIDTH);						//选择通道 0 有效数据宽度 RX_PW_P0	 
		nRF24L01P_WriteByte(W_REGISTER + CONFIG, 0x0E);   		 							// IRQ收发完成中断开启，16位CRC，主发送
	}
	
	else if (Mode == 3)																	//发送带有负载数据的 ACK信号 的 RX模式
	{		
		nRF24L01P_WriteByte(W_REGISTER + DYNPD, 0x01);										//使能接收 管道0 动态负载长度
		nRF24L01P_WriteByte(W_REGISTER + FEATURE, 0x06);                                   //使能动态负载长度，使能 ACK负载（带负载数据的 ACK包）
		nRF24L01P_WriteByte(W_REGISTER + CONFIG, 0x0F);   		 							// IRQ收发完成中断开启，16位CRC，主接收		
	}
	
	else if (Mode == 4)																	//发送带有负载数据的 ACK信号 的 TX模式
	{		
		nRF24L01P_WriteByte(W_REGISTER + DYNPD, 0x01);										//使能接收管道0 动态负载长度
		nRF24L01P_WriteByte(W_REGISTER + FEATURE, 0x06);									//使能动态负载长度，使能 ACK负载（带负载数据的 ACK包）
		nRF24L01P_WriteByte(W_REGISTER + CONFIG, 0x0E);   		 							// IRQ收发完成中断开启，16位CRC，主发送
	}
	
	nRF24L01P_CE_HIGH;
	
//	//读回配置信息，防止配置出错
//	printf("1.EN_AA     : 0x%02x\r\n", nRF24L01P_ReadByte(R_REGISTER + EN_AA));
//	printf("2.EN_RXADDR : 0x%02x\r\n", nRF24L01P_ReadByte(R_REGISTER + EN_RXADDR));
//	printf("3.RF_CH     : 0x%02x\r\n", nRF24L01P_ReadByte(R_REGISTER + RF_CH));	
//	printf("4.RX_PW_P0  : 0x%02x\r\n", nRF24L01P_ReadByte(R_REGISTER + RX_PW_P0));
//	printf("5.RF_SETUP  : 0x%02x\r\n", nRF24L01P_ReadByte(R_REGISTER + RF_SETUP));
//	printf("7.CONFIG    : 0x%02x\r\n", nRF24L01P_ReadByte(R_REGISTER + CONFIG));
//	printf("\r\n");
}

void nRF24L01P_NormalPacket(uint8_t* txbuffer, uint8_t length)
{
	nRF24L01P_CE_LOW;																		//CE 拉低，使能nRF24L01P配置
	nRF24L01P_WriteBuffer(W_TX_PAYLOAD, txbuffer, length);	   								//写数据到 TX FIFO ，32 个字节
	nRF24L01P_CE_HIGH;																		//CE 置高，使能发送	   																
}

void nRF24L01P_AckPayloadPacket(uint8_t* txbuffer, uint8_t length)
{
	nRF24L01P_CE_LOW;		 																//StandBy I模式	
	nRF24L01P_WriteBuffer(W_ACK_PLAYLOAD, txbuffer, length); 								//装载数据
	nRF24L01P_CE_HIGH;		 																//置高CE
}

void nRF24L01P_IRQHandler(uint8_t *rxbuffer)
{
	uint8_t state = nRF24L01P_ReadByte(STATUS);  										//读取状态寄存器的值
	nRF24L01P_WriteByte(W_REGISTER + STATUS, state);               						//清除中断标志

	if (state & BIT_RX_DR)																//判断是否收到数据
	{
		uint8_t rx_len = nRF24L01P_ReadByte(R_RX_PL_WID);       						//读出接收字节数
		
		if (rx_len < 33)																//判断是否超出接收字节限制
		{
			nRF24L01P_ReadBuffer(R_RX_PAYLOAD, rxbuffer, rx_len); 			//未超出，接收数据
			nRF24L01P_WriteByte(FLUSH_RX, 0xFF); 										//清除RX FIFO
//			printf("== RX TEST AREA ==\r\n");
//			for(uint8_t i = 0; i < rx_len; i ++)
//			{
//				printf(" RXDATA[%d] = 0x%02x\r\n", i, rxbuffer[i]);
//			}
//			printf("==================\r\n\r\n");
		}
		else 
		{
			nRF24L01P_WriteByte(FLUSH_RX, 0xFF);											//超出，清除 RX缓冲区，丢弃数据					
		}
	}

	if (state & BIT_TX_DS)
//		printf("Send Success!\r\n\r\n");


	if (state & BIT_MAX_RT)																//判断是否达到最大重发次数
	{
		if (state & BIT_TX_FULL)														//TX FIFO 满，清除 TX缓冲区
			nRF24L01P_WriteByte(FLUSH_TX, 0xFF);
		
//		printf("Send Fail!\r\n\r\n");
	}
}

/*************************************************************************
*函  数：uint8_t nRF24L01P_ReadByte(uint8_t regaddress)
*功  能：从寄存器读一字节数据
*参  数：regaddress：寄存器地址
*返回值：status
*备  注：nRF24L01P 代码移植只需把SPI驱动修改成自己的即可
**************************************************************************/
uint8_t nRF24L01P_ReadByte(uint8_t regaddress)
{
	return SPI1_ReadByte(regaddress);
}

/*************************************************************************
*函  数：uint8_t nRF24L01P_WriteByte(uint8_t regaddress, uint8_t byte)
*功  能：写一字节数据到寄存器
*参  数：regaddress：寄存器地址
*        byte	   ：要写入的数据
*返回值：status
*备  注：nRF24L01P 代码移植只需把SPI驱动修改成自己的即可
**************************************************************************/
uint8_t nRF24L01P_WriteByte(uint8_t regaddress, uint8_t byte)
{
	return SPI1_WriteByte(regaddress, byte);	
}

/*****************************************************************************************************
*函  数：uint8_t nRF24L01P_ReadBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len)
*功  能：从寄存器读一组数据
*参  数：regaddress	  ：寄存器地址
*        pbuffer	  ：要读取数据的地址
*        len: 要读取的数据长度
*返回值：status
*备  注：nRF24L01P 代码移植只需把SPI驱动修改成自己的即可
******************************************************************************************************/
uint8_t nRF24L01P_ReadBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len)
{
	return SPI1_ReadBuffer(regaddress, pbuffer, len);
}

/*****************************************************************************************************
*函  数：uint8_t nRF24L01P_WriteBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len)
*功  能：写一组数据到寄存器
*参  数：regaddress    ：寄存器地址
*        pbuffer       ：要写入数据的地址
*        len: 要写入的数据长度
*返回值：status
*备  注：nRF24L01P 代码移植只需把SPI驱动修改成自己的即可
******************************************************************************************************/
uint8_t nRF24L01P_WriteBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len)
{
	return SPI1_WriteBuffer(regaddress, pbuffer, len);
}
