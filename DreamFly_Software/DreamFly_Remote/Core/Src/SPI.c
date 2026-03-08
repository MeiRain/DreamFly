#include "SPI.h"

/************************** 代码移植修改区 ***************************/

#define SPI1_CS_CLK                 RCC_APB2Periph_GPIOA
#define SPI1_CS_PORT                GPIOA
#define SPI1_CS_PIN                 GPIO_Pin_4
	
#define SPI1_SCK_CLK              	RCC_APB2Periph_GPIOA
#define SPI1_SCK_PORT             	GPIOA
#define SPI1_SCK_PIN                GPIO_Pin_5
#define SPI1_SCK_PINSOURCE			GPIO_PinSource5
	
#define SPI1_MISO_CLK          		RCC_APB2Periph_GPIOA
#define SPI1_MISO_PORT         		GPIOA
#define SPI1_MISO_PIN               GPIO_Pin_6
#define SPI1_MISO_PINSOURCE         GPIO_PinSource6
	
#define SPI1_MOSI_CLK          		RCC_APB2Periph_GPIOA
#define SPI1_MOSI_PORT         		GPIOA
#define SPI1_MOSI_PIN               GPIO_Pin_7
#define SPI1_MOSI_PINSOURCE         GPIO_PinSource7

#define SPI1_CS_LOW()      		   {SPI1_CS_PORT->BRR  |= SPI1_CS_PIN;}
#define SPI1_CS_HIGH()     		   {SPI1_CS_PORT->BSRR |= SPI1_CS_PIN;}

/*********************************************************************/

/*********************************************************************
*函  数：void SPI1_Init(void)
*功  能：SPI1 初始化
*参  数：无
*返回值：无
*备  注：采用 SPI0模式
**********************************************************************/
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_APB2PeriphClockCmd(SPI1_CS_CLK | SPI1_SCK_CLK | SPI1_MISO_CLK | SPI1_MOSI_CLK, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);	
	
//	配置 CSN 引脚为推挽输出
	GPIO_InitStructure.GPIO_Pin = SPI1_CS_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(SPI1_CS_PORT, &GPIO_InitStructure);
	
//	配置 SCK、MOSI、MISO 引脚为复用推挽输出
	GPIO_InitStructure.GPIO_Pin = SPI1_SCK_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(SPI1_SCK_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
	GPIO_Init(SPI1_MISO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_PIN;
	GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStructure);				
					
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;		//方向，选择双线全双工	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;							//模式，选择为 SPI 主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;						//数据宽度，选择为 8 位
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;								//SPI 总线空闲时，SCK 线为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;							//在 SCK 的第一个时钟沿捕获采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;								//NSS，选择由软件控制	
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;		//波特率分频，选择 16 分频
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;						//先行位，选择高位先行
	SPI_InitStructure.SPI_CRCPolynomial = 7;								//CRC 多项式，以完成 CRC 校验
	SPI_Init(SPI1, &SPI_InitStructure);										
	
	SPI_Cmd(SPI1, ENABLE);													//使能SPI1，开始运行
}

/*********************************************************************
*函  数：uint8_t SPI1_SwapByte(uint8_t ByteToSwap)
*功  能：SPI1 交换一个字节
*参  数：无
*返回值：无
*备  注：无
**********************************************************************/
uint8_t SPI1_SwapByte(uint8_t ByteToSwap)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET);			//等待发送数据寄存器空
		
	SPI_I2S_SendData(SPI1, ByteToSwap);										//写入数据到发送数据寄存器，开始产生时序
	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) != SET);			//等待接收数据寄存器非空
	
	return SPI_I2S_ReceiveData(SPI1);										//读取接收到的数据并返回
}

/*********************************************************************
*函  数：uint8_t SPI1_ReadByte(uint8_t regaddress)
*功  能：从寄存器读一字节数据
*参  数：regaddress：寄存器地址
*返回值：status
*备  注：无
**********************************************************************/
uint8_t SPI1_ReadByte(uint8_t regaddress)
{
	uint8_t status;
	SPI1_CS_LOW();			
	SPI1_SwapByte(regaddress);
	status = SPI1_SwapByte(0xFF);
	SPI1_CS_HIGH();
	
	return status;
}

/*********************************************************************
*函  数：uint8_t SPI1_WriteByte(uint8_t regaddress, uint8_t byte)
*功  能：写一字节数据到寄存器
*参  数：regaddress：寄存器地址
*        byte	   ：要写入的数据
*返回值：status
*备  注：无
**********************************************************************/
uint8_t SPI1_WriteByte(uint8_t regaddress, uint8_t byte)
{
	uint8_t status;
	SPI1_CS_LOW();
	status = SPI1_SwapByte(regaddress);
	SPI1_SwapByte(byte);
	SPI1_CS_HIGH();
	return status;
}

/***************************************************************************************************
*函  数：uint8_t SPI1_ReadBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len)
*功  能：从寄存器读一组数据
*参  数：regaddress	  ：寄存器地址
*        pbuffer	  ：要读取数据的地址
*        len: 要读取的数据长度
*返回值：status
*备  注：无
****************************************************************************************************/
uint8_t SPI1_ReadBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len)
{
	uint8_t status;
	SPI1_CS_LOW();
	status = SPI1_SwapByte(regaddress);
	
	for (uint8_t i = 0; i < len; i ++)
	{
		*pbuffer = SPI1_SwapByte(0xFF);
		pbuffer ++;
	}
	
	SPI1_CS_HIGH();
	
	return status;
}

/***************************************************************************************************
*函  数：uint8_t SPI1_WriteBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len)
*功  能：写一组数据到寄存器
*参  数：regaddress    ：寄存器地址
*        pbuffer       ：要写入数据的地址
*        len: 要写入的数据长度
*返回值：status
*备  注：无
****************************************************************************************************/
uint8_t SPI1_WriteBuffer(uint8_t regaddress, uint8_t* pbuffer, uint8_t len)
{
	uint8_t status;
	SPI1_CS_LOW();
	status = SPI1_SwapByte(regaddress);
	
	for (uint8_t i = 0; i < len; i ++)
	{
		SPI1_SwapByte(*pbuffer);
		pbuffer ++;
	}
	
	SPI1_CS_HIGH();
	
	return status;
}
