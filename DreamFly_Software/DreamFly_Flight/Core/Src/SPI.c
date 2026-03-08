#include "SPI.h"   



/*********************************************************************
*函  数：void SPI1_Init(void)
*功  能：SPI1初始化
*参  数：无
*返回值：无
*备  注：采用 SPI0模式
**********************************************************************/
void SPI1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(SPI1_CS_CLK | SPI1_SCK_CLK | SPI1_MISO_CLK | SPI1_MOSI_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	//复用为 SPI1 的对应引脚 
	GPIO_PinAFConfig(SPI1_SCK_PORT, SPI1_SCK_PINSOURCE, GPIO_AF_SPI1);
	GPIO_PinAFConfig(SPI1_MISO_PORT, SPI1_MISO_PINSOURCE, GPIO_AF_SPI1);    
	GPIO_PinAFConfig(SPI1_MOSI_PORT, SPI1_MOSI_PINSOURCE, GPIO_AF_SPI1);
	
	//将 CS 引脚初始化为推挽输出
	GPIO_InitStructure.GPIO_Pin = SPI1_CS_PIN;		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(SPI1_CS_PORT, &GPIO_InitStructure);
	SPI1_CS_HIGH;
	
	//将 SCK、MISO、MOSI 引脚初始化为复用推挽输出
	GPIO_InitStructure.GPIO_Pin = SPI1_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(SPI1_SCK_PORT, &GPIO_InitStructure);
  
	GPIO_InitStructure.GPIO_Pin = SPI1_MISO_PIN;
	GPIO_Init(SPI1_MISO_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SPI1_MOSI_PIN;
	GPIO_Init(SPI1_MOSI_PORT, &GPIO_InitStructure);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 		//方向，选择双线全双工	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; 							//模式，选择为 SPI 主模式 
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 						//数据宽度，选择为 8 位 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 								//SPI 总线空闲时，SCK 线为低电平 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 							//在 SCK 的第一个时钟沿捕获采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 								//NSS，选择由软件控制	
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; 		//波特率分频，SPI1 的波特率 50M / 8 = 6.25M
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 						//先行位，选择高位先行
	SPI_InitStructure.SPI_CRCPolynomial = 7; 								//CRC 多项式，以完成 CRC 校验	
	SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
}

/*********************************************************************
*函  数：uint8_t SPI1_SwapByte(uint8_t Data)
*功  能：SPI1 交换一个字节
*参  数：无
*返回值：无
*备  注：无
**********************************************************************/
uint8_t SPI1_SwapByte(uint8_t swap_byte)
{
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) != SET);			//等待发送缓冲区为空，TXE事件	
	SPI_I2S_SendData(SPI1, swap_byte);                              		//写入数据到发送数据寄存器	
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) != SET);			//等待接收缓冲区非空，RXNE事件	
	return SPI_I2S_ReceiveData(SPI1);                          				//读取接收到的数据并返回
}

/*********************************************************************
*函  数：uint8_t SPI1_ReadByte(uint8_t reg_address)
*功  能：从寄存器读一字节数据
*参  数：reg_address：寄存器地址
*返回值：SwapByte
*备  注：无
**********************************************************************/
uint8_t SPI1_ReadByte(uint8_t reg_address)
{
	uint8_t SwapByte;
	SPI1_CS_LOW;
	SPI1_SwapByte(reg_address);
	SwapByte = SPI1_SwapByte(0xFF);
	SPI1_CS_HIGH;
	return SwapByte;
}

/*********************************************************************
*函  数：uint8_t SPI1_WriteByte(uint8_t reg_address, uint8_t byte)
*功  能：写一字节数据到寄存器
*参  数：reg_address：寄存器地址
*        byte	   ：要写入的数据
*返回值：SwapByte
*备  注：无
**********************************************************************/
uint8_t SPI1_WriteByte(uint8_t reg_address, uint8_t write_byte)
{
	uint8_t SwapByte;
	SPI1_CS_LOW;
	SwapByte = SPI1_SwapByte(reg_address | 0x80);
	SPI1_SwapByte(write_byte);
	SPI1_CS_HIGH;
	return SwapByte;
}

/***************************************************************************************************
*函  数：uint8_t SPI1_ReadBuffer(uint8_t reg_address, uint8_t* pbuffer, uint8_t read_size)
*功  能：从寄存器读一组数据
*参  数：reg_address	  ：寄存器地址
*        pbuffer	  ：要读取数据的地址
*        read_size: 要读取的数据长度
*返回值：SwapByte
*备  注：无
****************************************************************************************************/
uint8_t SPI1_ReadBuffer(uint8_t reg_address, uint8_t* read_pbuffer, uint8_t read_size)
{
	uint8_t SwapByte;
	SPI1_CS_LOW;
	SwapByte = SPI1_SwapByte(reg_address);	
	for (uint8_t i = 0; i < read_size; i ++)
	{
		*read_pbuffer = SPI1_SwapByte(0xFF);
		read_pbuffer ++;
	}
	SPI1_CS_HIGH;
	return SwapByte;
}

/***************************************************************************************************
*函  数：uint8_t SPI1_WriteBuffer(uint8_t reg_address, uint8_t* pbuffer, uint8_t write_size)
*功  能：写一组数据到寄存器
*参  数：reg_address    ：寄存器地址
*        pbuffer       ：要写入数据的地址
*        write_size: 要写入的数据长度
*返回值：SwapByte
*备  注：无
****************************************************************************************************/
uint8_t SPI1_WriteBuffer(uint8_t reg_address, uint8_t* write_pbuffer, uint8_t write_size)
{
	uint8_t SwapByte;
	SPI1_CS_LOW;
	SwapByte = SPI1_SwapByte(reg_address);
	for (uint8_t i = 0; i < write_size; i ++)
	{
		SPI1_SwapByte(*write_pbuffer);
		write_pbuffer ++;
	}
	SPI1_CS_HIGH;
	return SwapByte;
}

/*********************************************************************
*函  数：void SPI2_Init(void)
*功  能：SPI2初始化
*参  数：无
*返回值：无
*备  注：采用 SPI0模式
**********************************************************************/
void SPI2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	
	RCC_AHB1PeriphClockCmd(SPI2_CS_CLK | SPI2_SCK_CLK | SPI2_MISO_CLK | SPI2_MOSI_CLK, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	//复用为 SPI2 的对应引脚 
	GPIO_PinAFConfig(SPI2_SCK_PORT, SPI2_SCK_PINSOURCE, GPIO_AF_SPI2);
	GPIO_PinAFConfig(SPI2_MISO_PORT, SPI2_MISO_PINSOURCE, GPIO_AF_SPI2);    
	GPIO_PinAFConfig(SPI2_MOSI_PORT, SPI2_MOSI_PINSOURCE, GPIO_AF_SPI2);
	
	//将 CS 引脚初始化为推挽输出
	GPIO_InitStructure.GPIO_Pin = SPI2_CS_PIN;		
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;     		
	GPIO_Init(SPI2_CS_PORT, &GPIO_InitStructure);
	SPI2_CS_HIGH;
	
	//将 SCK、MISO、MOSI 引脚初始化为复用推挽输出
	GPIO_InitStructure.GPIO_Pin = SPI2_SCK_PIN; 	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;			
	GPIO_Init(SPI2_SCK_PORT, &GPIO_InitStructure);
  
	GPIO_InitStructure.GPIO_Pin = SPI2_MISO_PIN; 			
	GPIO_Init(SPI2_MISO_PORT, &GPIO_InitStructure);  

	GPIO_InitStructure.GPIO_Pin = SPI2_MOSI_PIN; 			
	GPIO_Init(SPI2_MOSI_PORT, &GPIO_InitStructure);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; 		//方向，选择双线全双工	
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; 							//模式，选择为 SPI 主模式 
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 						//数据宽度，选择为 8 位 
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 								//SPI 总线空闲时，SCK 线为低电平 
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 							//在 SCK 的第一个时钟沿捕获采样数据
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; 								//NSS，选择由软件控制	
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; 		//波特率分频，SPI1 的波特率 50M / 8 = 6.25M
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 						//先行位，选择高位先行
	SPI_InitStructure.SPI_CRCPolynomial = 7; 								//CRC 多项式，以完成 CRC 校验	
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2, ENABLE); 													
}

/*********************************************************************
*函  数：uint8_t SPI2_SwapByte(uint8_t Data)
*功  能：SPI2 交换一个字节
*参  数：无
*返回值：无
*备  注：无
**********************************************************************/
uint8_t SPI2_SwapByte(uint8_t swap_byte)
{
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) != SET);			//等待发送缓冲区为空，TXE事件	
	SPI_I2S_SendData(SPI2, swap_byte);                              		//写入数据到发送数据寄存器	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) != SET);			//等待接收缓冲区非空，RXNE事件	
	return SPI_I2S_ReceiveData(SPI2);                          				//读取接收到的数据并返回
}

/*********************************************************************
*函  数：uint8_t SPI2_ReadByte(uint8_t reg_address)
*功  能：从寄存器读一字节数据
*参  数：reg_address：寄存器地址
*返回值：SwapByte
*备  注：无
**********************************************************************/
uint8_t SPI2_ReadByte(uint8_t reg_address)
{
	uint8_t SwapByte;
	SPI2_CS_LOW;			
	SPI2_SwapByte(reg_address);
	SwapByte = SPI2_SwapByte(0xFF);
	SPI2_CS_HIGH;
	return SwapByte;
}

/*********************************************************************
*函  数：uint8_t SPI2_WriteByte(uint8_t reg_address, uint8_t byte)
*功  能：写一字节数据到寄存器
*参  数：reg_address：寄存器地址
*        byte	   ：要写入的数据
*返回值：SwapByte
*备  注：无
**********************************************************************/
uint8_t SPI2_WriteByte(uint8_t reg_address, uint8_t write_byte)
{
	uint8_t SwapByte;
	SPI2_CS_LOW;
	SwapByte = SPI2_SwapByte(reg_address);
	SPI2_SwapByte(write_byte);
	SPI2_CS_HIGH;
	return SwapByte;
}

/***************************************************************************************************
*函  数：uint8_t SPI2_ReadBuffer(uint8_t reg_address, uint8_t* read_pbuffer, uint8_t read_size)
*功  能：从寄存器读一组数据
*参  数：reg_address	  ：寄存器地址
*        read_pbuffer	  ：要读取数据的地址
*        read_size: 要读取的数据长度
*返回值：SwapByte
*备  注：无
****************************************************************************************************/
uint8_t SPI2_ReadBuffer(uint8_t reg_address, uint8_t* read_pbuffer, uint8_t read_size)
{
	uint8_t SwapByte;
	SPI2_CS_LOW;
	SwapByte = SPI2_SwapByte(reg_address);	
	for (uint8_t i = 0; i < read_size; i ++)
	{
		*read_pbuffer = SPI2_SwapByte(0xFF);
		read_pbuffer ++;
	}	
	SPI2_CS_HIGH;
	return SwapByte;
}

/***************************************************************************************************
*函  数：uint8_t SPI2_WriteBuffer(uint8_t reg_address, uint8_t* pbuffer, uint8_t write_size)
*功  能：写一组数据到寄存器
*参  数：reg_address    ：寄存器地址
*        pbuffer       ：要写入数据的地址
*        write_size: 要写入的数据长度
*返回值：SwapByte
*备  注：无
****************************************************************************************************/
uint8_t SPI2_WriteBuffer(uint8_t reg_address, uint8_t* write_pbuffer, uint8_t write_size)
{
	uint8_t SwapByte;
	SPI2_CS_LOW;
	SwapByte = SPI2_SwapByte(reg_address);
	for (uint8_t i = 0; i < write_size; i ++)
	{
		SPI2_SwapByte(*write_pbuffer);
		write_pbuffer ++;
	}
	SPI2_CS_HIGH;
	return SwapByte;
}
