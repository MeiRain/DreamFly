#include <math.h>

#include "Delay.h"
#include "IIC.h"

#define STATUS_OK       0x00
#define STATUS_FAIL     0x01

/************************代码移植修改区************************************/

#define IIC_SCL_RCC    	RCC_AHB1Periph_GPIOB  //端口时钟
#define IIC_SCL_PORT   	GPIOB                 //端口
#define IIC_SCL_PIN    	GPIO_Pin_4            //引脚

#define IIC_SDA_RCC    	RCC_AHB1Periph_GPIOB  //端口时钟
#define IIC_SDA_PORT  	GPIOB                 //端口
#define IIC_SDA_PIN     GPIO_Pin_5            //引脚

/**************************************************************************/

//求偏移量
#define IIC_SCL_PIN_OFFSET (uint8_t)(log(IIC_SCL_PIN)/log(2))
#define IIC_SDA_PIN_OFFSET (uint8_t)(log(IIC_SDA_PIN)/log(2))

/***************************************************************************
GPIOx_MODER 端口模式寄存器
这里第一步是将目标位设置为00，第二步才是设置（即设置新的值，让其为什么模式）
***************************************************************************/
//00输入模式
#define SDA_IN()  {IIC_SDA_PORT->MODER&=~(3<<(IIC_SDA_PIN_OFFSET*2));IIC_SDA_PORT->MODER|=0<<IIC_SDA_PIN_OFFSET*2;}
//01通用输出
#define SDA_OUT() {IIC_SDA_PORT->MODER&=~(3<<(IIC_SDA_PIN_OFFSET*2));IIC_SDA_PORT->MODER|=1<<IIC_SDA_PIN_OFFSET*2;} 

/****************************************************************************
GPIOx_BSRR 端口设置/清除寄存器
BSRRH表示BSRR寄存器高16位。某位为’1’，则对应的I/O端口管脚置’0’（低电平）；
						   某位为’0’，则对应的I/O端口管脚保持不变。
BSRRL表示BSRR寄存器低16位。某位为’1’，则对应的I/O端口管脚置’1’（高电平）；
						   某位为’0’，则对应的I/O端口管脚保持不变。
GPIOx_IDR 端口输入数据寄存器
****************************************************************************/
//IO操作函数	 
#define IIC_SCL_H    	 IIC_SCL_PORT->BSRRL |= IIC_SCL_PIN 			//SCL
#define IIC_SCL_L    	 IIC_SCL_PORT->BSRRH |= IIC_SCL_PIN 			//SCL
#define IIC_SDA_H    	 IIC_SDA_PORT->BSRRL |= IIC_SDA_PIN 			//SDA	 
#define IIC_SDA_L    	 IIC_SDA_PORT->BSRRH |= IIC_SDA_PIN 			//SDA	 
#define IIC_SDA_READ	(IIC_SDA_PORT->IDR & IIC_SDA_PIN) ?  1 : 0		//输入SDA 

/******************************************************************************
*函  数：void IIC_Init(void)
*功　能：IIC初始化
*参  数：无
*返回值：无
*备  注：无
*******************************************************************************/	
void IIC_Init(void)
{			
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(IIC_SCL_RCC | IIC_SDA_RCC, ENABLE);

	GPIO_InitStructure.GPIO_Pin = IIC_SCL_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 			//选择模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD; 			//开漏输出类型  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
	GPIO_Init(IIC_SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = IIC_SDA_PIN;			
	GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
	
	IIC_SCL_H;
	IIC_SDA_H;
}

/******************************************************************************
*函  数：void IIC_Start(void)
*功　能：产生IIC起始信号
*参  数：无
*返回值：无
*备  注：时序片段 1
*******************************************************************************/	
void IIC_Start(void)
{
	SDA_OUT(); 						//SDA线输出 
	IIC_SDA_H;						//释放SDA，确保SDA为高电平
	IIC_SCL_H;      				//释放SCL，确保SCL为高电平
	Delay_us(4);        			
 	IIC_SDA_L; 						//在SCL高电平期间，拉低SDA，产生起始信号
	Delay_us(4);					
	IIC_SCL_L; 						//起始后把SCL也拉低，即为了占用总线，也为了方便总线时序的拼接
}				

/******************************************************************************
*函  数：void IIC_Stop(void)
*功　能：产生IIC停止信号
*参  数：无
*返回值：无
*备  注：时序片段 2
*******************************************************************************/	  
void IIC_Stop(void)
{
	SDA_OUT(); 						//SDA线输出
	IIC_SCL_L;						//拉低SCL，确保SDA为低电平
	IIC_SDA_L;						//拉低SDA，确保SDA为低电平
	Delay_us(4);  					
	IIC_SCL_H; 						//释放SCL，使SCL呈现高电平
	IIC_SDA_H; 						//在SCL高电平期间，释放SDA，产生终止信号
	Delay_us(4);							   	
}

/******************************************************************************
*函  数: uint8_t IIC_WaitAck(void)
*功　能: 等待应答信号到来
		（有效应答：从机第9个 SCL=0 时 SDA 被从机拉低，并且 SCL = 1时 SDA依然为低）
*参  数：无
*返回值：0 接收应答成功   1 接收应答失败
*备  注：时序片段 3
*******************************************************************************/
uint8_t IIC_WaitAck(void)
{
	uint8_t ucErrTime=0;
	SDA_IN(); 						//SDA设置为输入
	IIC_SDA_H;						//接收前，主机先确保拉高SDA，避免干扰从机的数据发送							
	Delay_us(1);	   										
	IIC_SCL_H;						//拉高SCL，给从机准备应答
	Delay_us(1);	 											
	while(IIC_SDA_READ)					//主机机在SCL高电平期间读取SDA				
	{											            
		ucErrTime++;											
		if(ucErrTime>250)											
		{											
			IIC_Stop();				//超时未响应则结束通信								
			return 1;											
		}											
	}											
	IIC_SCL_L; 	  					//拉低SCL，开始下一个时序模块							 
	return 0;  						//返回定义应答位变量	
} 										

/******************************************************************************
*函  数: void IIC_Ack(void)
*功　能: 产生ACK应答 
		（主机接收完一个字节数据后，主机产生的ACK通知从机一个字节数据已正确接收）
*参  数：无
*返回值：无
*备  注：时序片段 4
*******************************************************************************/

void IIC_Ack(void)
{
	IIC_SCL_L;						//主机拉低SCL，准备应答
	SDA_OUT();	    					
	IIC_SDA_L;						//主机拉低SDA，应答从机
	Delay_us(1);					
	IIC_SCL_H;						//释放SCL，从机在SCL高电平期间，读取应答位
	Delay_us(2);					
	IIC_SCL_L;						//拉低SCL，开始下一个时序模块
}

/******************************************************************************
*函  数: void IIC_NoAck(void)
*功　能: 产生NACK应答 （主机接收完最后一个字节数据后，主机产生的NACK通知从机发送结束，释放SDA,以便主机产生停止信号）
*参  数：无
*返回值：无
*备  注：时序片段 5
*******************************************************************************/
void IIC_NoAck(void)
{
	IIC_SCL_L;						//主机拉低SCL，准备应答
	SDA_OUT();      				    
	IIC_SDA_H;      				//主机拉高SDA，不应答从机
	Delay_us(1);    				    
	IIC_SCL_H;      				//释放SCL，从机在SCL高电平期间，读取应答位
	Delay_us(1);    				    
	IIC_SCL_L;      				//拉低SCL，开始下一个时序模块
}					 				     

/******************************************************************************
*函  数：void IIC_SendByte(uint8_t txd)
*功  能：IIC发送一个字节
*参  数：data 要写的数据
*返回值：无
*备  注：时序片段 6
*******************************************************************************/		  
void IIC_SendByte(uint8_t data)
{                        
    uint8_t t;   
	SDA_OUT(); 	    
    IIC_SCL_L; 									//拉低时钟开始数据传输
    for(t=0; t < 8;t ++)
    {              
		if((data & 0x80)>>7)
			IIC_SDA_H;
		else
			IIC_SDA_L;
		data <<= 1;
		Delay_us(1);			
		IIC_SCL_H;								//拉高SCL，从机在SCL高电平期间读取SDA
		Delay_us(1);
		IIC_SCL_L;								//拉低SCL，主机开始发送下一位数据
		Delay_us(1);
    }	 
} 	 
   
/******************************************************************************
*函  数：uint8_t IIC_ReceiveByte(uint8_t ack)
*功  能：IIC读取一个字节
*参  数：ack=1 时，主机数据还没接收完 ack=0 时主机数据已全部接收完成
*返回值：无
*备  注：时序片段 7
*******************************************************************************/	
uint8_t IIC_ReceiveByte(uint8_t ack)
{
	uint8_t i, Byte = 0x00;
	SDA_IN(); 									//SDA设置为输入模式 等待接收从机返回数据
	for(i=0;i<8;i++ )
	{
		IIC_SCL_L; 								//拉低SCL，从机在SCL低电平期间写入SDA
		Delay_us(1);
		IIC_SCL_H;								//释放SCL，主机机在SCL高电平期间读取SDA
		if (IIC_SDA_READ){Byte |= (0x80 >> i);}		//读取SDA数据，并存储到Byte变量
												//当SDA为1时，置变量指定位为1，当SDA为0时，不做处理，指定位为默认的初值0
		Delay_us(1); 
	}					 
	if(ack)
		IIC_Ack(); 								//发送ACK 
    else
        IIC_NoAck(); 							//发送nACK  
    return Byte;
}

/******************************************************************************
*函  数：uint8_t IIC_ReadByte(uint8_t DEVAddress, uint8_t reg_address, uint8_t* pData)
*功　能：读取指定设备，指定寄存器的一个字节
*参  数：DEVAddress	目标设备地址
		 reg_address	    寄存器地址
         *buf      	读取数据要存储的地址    
*返回值：0成功 1失败 
*备  注：无
*******************************************************************************/ 
uint8_t IIC_ReadByte(uint8_t DEVAddress, uint8_t reg_address, uint8_t* pByte)
{
	IIC_Start();	
	IIC_SendByte(DEVAddress << 1 | 0);			//发送从机地址，左移1位并或0，表示写入
	if(IIC_WaitAck()) 							//如果从机未应答则数据发送失败，退出通信
	{	
		IIC_Stop();	
		return 1;	
	}	
	IIC_SendByte(reg_address); 							//发送寄存器地址
	IIC_WaitAck();	  							//等待应答
		
	IIC_Start();	
	IIC_SendByte(DEVAddress << 1 | 1); 			//发送从机地址，左移1位并或1，表示读取			   
	IIC_WaitAck();								//等待应答
	*pByte = IIC_ReceiveByte(0);	   				//接收指定寄存器的数据，接收后不应答
	IIC_Stop(); 								//产生一个停止条件
	return 0;	
}

/******************************************************************************
*函  数：uint8_t IIC_WriteReg_Byte_Byte(uint8_t DEVAddress, uint8_t reg_address, uint8_t Data)
*功　能：写入指定设备，指定寄存器的一个字节
*参  数：DEVAddress	目标设备地址
		 reg_address	    寄存器地址
         buf       	要写入的数据
*返回值：0成功 1失败
*备  注：无
*******************************************************************************/ 
uint8_t IIC_WriteByte(uint8_t DEVAddress, uint8_t reg_address, uint8_t Byte)
{
	IIC_Start();
	IIC_SendByte(DEVAddress << 1 | 0); 	//发送从机地址
	if(IIC_WaitAck())
	{
		IIC_Stop();
		return 1; 								//从机地址写入失败
	}
	IIC_SendByte(reg_address); 						//发送寄存器地址
	IIC_WaitAck();	  
	IIC_SendByte(Byte); 
	if(IIC_WaitAck())
	{
		IIC_Stop(); 
		return 1; 								//数据写入失败
	}
	IIC_Stop(); 							//产生一个停止条件
	return 0;
}

/******************************************************************************
*函  数：uint8_t IIC_ReadBuffer(uint8_t DEVAddress, uint8_t reg_address, uint8_t length, uint8_t* pBuffer)
*功　能：读取指定设备，指定寄存器的 length 个字节
*参  数：DEVAddress   目标设备地址
		 reg_address	   	寄存器地址
         length		要读的字节数
		 *pBuffer   	读出的数据将要存放的指针
*返回值：0成功 1失败
*备  注：无
*******************************************************************************/ 
uint8_t IIC_ReadBuffer(uint8_t DEVAddress, uint8_t reg_address, uint8_t* pBuffer, uint8_t NumByteToRead)
{
	uint8_t count = 0;
	uint8_t temp;
	IIC_Start();
	IIC_SendByte(DEVAddress << 1 | 0); 	//发送从机地址
	if(IIC_WaitAck())
	{
		IIC_Stop(); 
		return 1; 								//从机地址写入失败
	}
	IIC_SendByte(reg_address); 						//发送寄存器地址
	IIC_WaitAck();	  
	IIC_Start();
	IIC_SendByte(DEVAddress << 1 | 1); 	//进入接收模式	
	IIC_WaitAck();
	for(count = 0; count < NumByteToRead; count ++)
	{
		if(count != (NumByteToRead - 1))
			temp = IIC_ReceiveByte(1); 		//带ACK的读数据
		else  
			temp = IIC_ReceiveByte(0); 		//最后一个字节NACK

		pBuffer[count] = temp;
	}
    IIC_Stop(); 							//产生一个停止条件
	return 0;
}

/******************************************************************************
*函  数：uint8_t IIC_WriteBuffer(uint8_t DEVAddress, uint8_t reg_address, uint8_t length, uint8_t* pBuffer)
*功　能：将 length 个字节写入指定设备，指定寄存器
*参  数：DEVAddress	目标设备地址
		 reg_address	   	寄存器地址
         length		要写的字节数
		 *pBuffer   	要写入的数据将要存放的指针
*返回值：0成功 1失败  
*备  注：无
*******************************************************************************/ 
uint8_t IIC_WriteBuffer(uint8_t DEVAddress, uint8_t reg_address, uint8_t* pBuffer, uint8_t NumByteToWrite)
{
  
 	uint8_t count = 0;
	IIC_Start();
	IIC_SendByte(DEVAddress); 		//发送从机地址
	if(IIC_WaitAck())
	{
		IIC_Stop();
		return 1; 									//从机地址写入失败
	}
	IIC_SendByte(reg_address); 							//发送寄存器地址
	IIC_WaitAck();	  
	for(count = 0; count < NumByteToWrite; count ++)
	{
		IIC_SendByte(pBuffer[count]); 
		if(IIC_WaitAck()) 						//每一个字节都要等从机应答
		{
			IIC_Stop();
			return 1; 								//数据写入失败
		}
	}
	IIC_Stop(); //产生一个停止条件

	return 0;
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void VL53L1X_SendByte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL_L;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
		if((txd&0x80)>>7)
			IIC_SDA_H;
		else
			IIC_SDA_L;
		txd<<=1; 	  
		Delay_us(2);  
		IIC_SCL_H;
		Delay_us(2); 
		IIC_SCL_L;	
		Delay_us(2);
    }	 
} 

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 VL53L1X_ReceiveByte(void)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
	IIC_SDA_H;
	Delay_us(4);
	for(i=0;i<8;i++ )
	{
		receive<<=1;
		IIC_SCL_L; 
		Delay_us(4);
	  IIC_SCL_H;
		Delay_us(4);
		if(IIC_SDA_READ)
			receive |= 0x01;   
	  Delay_us(4); //1
	}	
  IIC_SCL_L;	
	return receive;
}

//IIC写一个字节数据
u8 VL53L1X_WriteByte(u8 SlaveAddress, u16 REG_Address,u8 REG_data)
{
	IIC_Start();
	VL53L1X_SendByte(SlaveAddress);
	if(IIC_WaitAck())
	{
		IIC_Stop();//释放总线
		return 1;//没应答则退出

	}
	VL53L1X_SendByte(REG_Address >> 8);
	IIC_WaitAck();	
	VL53L1X_SendByte(REG_Address & 0x00ff);
	IIC_WaitAck();
	Delay_ms(1);
	VL53L1X_SendByte(REG_data);
	IIC_WaitAck();	
	IIC_Stop();

	return 0;
}

//IIC读一个字节数据
u8 VL53L1X_ReadByte(u8 SlaveAddress, u16 REG_Address,u8 *REG_data)
{
	IIC_Start();
	VL53L1X_SendByte(SlaveAddress);//发写命令
	if(IIC_WaitAck())
	{
		 IIC_Stop();//释放总线
		 return 1;//没应答则退出
	}		
	VL53L1X_SendByte(REG_Address >> 8);
	IIC_WaitAck();
	VL53L1X_SendByte((REG_Address & 0x00ff));
	IIC_WaitAck();
	Delay_ms(1);
	IIC_Start(); 
	VL53L1X_SendByte(SlaveAddress|0x01);//发读命令
	IIC_WaitAck();
	*REG_data = VL53L1X_ReceiveByte();
	IIC_Stop();

	return 0;
}

//I2C read n bytes
uint8_t VL53L1X_ReadBuffer(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *buf, uint16_t len)
{
	IIC_Start();
	VL53L1X_SendByte(SlaveAddress);//发写命令
	if(IIC_WaitAck()) 
	{
		IIC_Stop();//释放总线
		return 1;//没应答则退出
	}
	VL53L1X_SendByte(REG_Address >> 8);
	IIC_WaitAck();
	VL53L1X_SendByte((REG_Address & 0x00ff));
	IIC_WaitAck();
	Delay_ms(1);
	IIC_Start(); 
	VL53L1X_SendByte(SlaveAddress|0x01);//发读命令
	IIC_WaitAck();
	while(len)
	{
		*buf = VL53L1X_ReceiveByte();
		if(1 == len)
		{
			IIC_NoAck();
		}
		else
		{
			IIC_Ack();
		}
		buf++;
		len--;
	}
	IIC_Stop();

	return STATUS_OK;
}

//I2C write n bytes
uint8_t VL53L1X_WriteBuffer(uint8_t SlaveAddress, uint16_t REG_Address, uint8_t *buf, uint16_t len)
{
	IIC_Start();
	VL53L1X_SendByte(SlaveAddress);//发写命令
	if(IIC_WaitAck()) 
	{
		IIC_Stop();//释放总线
		return 1;//没应答则退出
	}
	VL53L1X_SendByte(REG_Address >> 8);
	IIC_WaitAck();
	VL53L1X_SendByte(REG_Address & 0x00ff);
	IIC_WaitAck();
	while(len--)
	{
		VL53L1X_SendByte(*buf++);
		IIC_WaitAck();
	}
	IIC_Stop();

	return STATUS_OK;
}
