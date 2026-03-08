#include <stdio.h>

#include "I2C.h"

/************************代码移植修改区************************************/

#define I2C1_SCL_CLK RCC_APB2Periph_GPIOB // 端口时钟
#define I2C1_SCL_PORT GPIOB				  // 端口
#define I2C1_SCL_PIN GPIO_Pin_6			  // 引脚

#define I2C1_SDA_CLK RCC_APB2Periph_GPIOB // 端口时钟
#define I2C1_SDA_PORT GPIOB				  // 端口
#define I2C1_SDA_PIN GPIO_Pin_7			  // 引脚

/**************************************************************************/

void I2C1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;
	
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(I2C1_SCL_CLK | I2C1_SDA_CLK, ENABLE);		//开启GPIOB的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);		//开启I2C2的时钟
	
	/*GPIO初始化*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = I2C1_SCL_PIN;
	GPIO_Init(I2C1_SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = I2C1_SDA_PIN;
	GPIO_Init(I2C1_SDA_PORT, &GPIO_InitStructure);
	
	/*I2C初始化*/
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;				//模式，选择为I2C模式
	I2C_InitStructure.I2C_ClockSpeed = 400000;				//时钟速度，选择为100KHz
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;		//时钟占空比，选择Tlow/Thigh = 2
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;				//应答，选择使能
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;	//应答地址，选择7位，从机模式下才有效
	I2C_InitStructure.I2C_OwnAddress1 = 0x66;				//自身地址，从机模式下才有效
	I2C_Init(I2C1, &I2C_InitStructure);						//将结构体变量交给I2C_Init，配置I2C2
	
	/*I2C使能*/
	I2C_Cmd(I2C1, ENABLE);									//使能I2C2，开始运行
}

static uint8_t I2C1_WaitEvent(uint32_t I2C_EVENT)
{
	uint32_t I2C_Timeout = 4096;
	while (I2C_CheckEvent(I2C1, I2C_EVENT) != SUCCESS) // 循环等待指定事件
	{
		if (I2C_Timeout-- == 0) // 自减到 0 后，等待超时
		{
			printf("I2C1 Timeout! ErrorCode = 0x%x\r\n", I2C_EVENT);
			return 0;
		}
	}
	return 1;
}

static uint8_t I2C1_WaitFree(void)
{
	uint32_t I2C_Timeout = 40960;
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY)) // 循环等待指定事件
	{
		if (I2C_Timeout-- == 0) // 自减到 0 后，等待超时
		{
			printf("I2C1 Busy!\r\n");
			return 0;
		}
	}
	return 1;
}

uint8_t I2C1_ReadByte(uint8_t devaddress, uint8_t regaddress, uint8_t *pbyte)
{
	I2C1_WaitFree(); // 判断 I2C1总线 是否忙碌

	I2C_GenerateSTART(I2C1, ENABLE);				   // 硬件 I2C 生成起始条件
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) // 等待 EV5
		return 0;

	I2C_Send7bitAddress(I2C1, devaddress, I2C_Direction_Transmitter); // 硬件 I2C 发送从机地址，方向为发送
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))  // 等待 EV6
		return 0;

	I2C_SendData(I2C1, regaddress);							// 硬件 I2C 发送寄存器地址
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // 等待 EV8_2
		return 0;

	I2C_GenerateSTART(I2C1, ENABLE);				   // 硬件 I2C 生成重复起始条件
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) // 等待 EV5
		return 0;

	I2C_Send7bitAddress(I2C1, devaddress, I2C_Direction_Receiver); // 硬件 I2C 发送从机地址，方向为接收
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))  // 等待 EV6
		return 0;

	I2C_AcknowledgeConfig(I2C1, DISABLE);				 // 在接收最后一个字节之前提前将应答失能
	I2C_GenerateSTOP(I2C1, ENABLE);						 // 在接收最后一个字节之前提前申请停止条件
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) // 等待 EV7
		return 0;

	*pbyte = I2C_ReceiveData(I2C1);		 // 接收数据寄存器
	I2C_AcknowledgeConfig(I2C1, ENABLE); // 将应答恢复为使能，为了不影响后续可能产生的读取多字节操作

	return 1;
}

uint8_t I2C1_WriteByte(uint8_t devaddress, uint8_t regaddress, uint8_t byte)
{
	I2C1_WaitFree(); // 判断 I2C1总线 是否忙碌

	I2C_GenerateSTART(I2C1, ENABLE);				   // 硬件 I2C 生成起始条件
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) // 等待 EV5
		return 0;

	I2C_Send7bitAddress(I2C1, devaddress, I2C_Direction_Transmitter); // 硬件 I2C 发送从机地址，方向为发送
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))  // 等待 EV6
		return 0;

	I2C_SendData(I2C1, regaddress);							 // 硬件 I2C 发送寄存器地址
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) // 等待 EV8
		return 0;

	I2C_SendData(I2C1, byte);								// 硬件 I2C 发送数据
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // 等待 EV8_2
		return 0;

	I2C_GenerateSTOP(I2C1, ENABLE); // 硬件 I2C 生成终止条件
	return 1;
}

uint8_t I2C1_ReadBuffer(uint8_t devaddress, uint8_t regaddress, uint8_t *pbuffer, uint16_t len)
{
	I2C1_WaitFree(); // 判断 I2C1总线 是否忙碌

	I2C_GenerateSTART(I2C1, ENABLE);				   // 硬件 I2C 生成起始条件
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) // 等待 EV5
		return 0;

	I2C_Send7bitAddress(I2C1, devaddress, I2C_Direction_Transmitter); // 硬件 I2C 发送从机地址，方向为发送
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))  // 等待 EV6
		return 0;

	I2C_Cmd(I2C1, ENABLE); // 通过重新设置PE位清除EV6事件

	I2C_SendData(I2C1, regaddress);							// 硬件 I2C 发送寄存器地址
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // 等待 EV8_2
		return 0;

	I2C_GenerateSTART(I2C1, ENABLE);				   // 硬件 I2C 生成重复起始条件
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) // 等待 EV5
		return 0;

	I2C_Send7bitAddress(I2C1, devaddress, I2C_Direction_Receiver); // 硬件 I2C 发送从机地址，方向为接收
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))  // 等待 EV6
		return 0;

	while (len)
	{
		if (len == 1)
		{
			I2C_AcknowledgeConfig(I2C1, DISABLE); // 在接收最后一个字节之前提前将应答失能
			I2C_GenerateSTOP(I2C1, ENABLE);		  // 在接收最后一个字节之前提前申请停止条件
		}

		if (!I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) // 等待 EV7
			return 0;

		*pbuffer = I2C_ReceiveData(I2C1);
		pbuffer++;
		len--;
	}

	I2C_AcknowledgeConfig(I2C1, ENABLE); // 将应答恢复为使能，为了不影响后续可能产生的读取多字节操作

	return 1;
}

uint8_t I2C1_WriteBuffer(uint8_t devaddress, uint8_t regaddress, uint8_t *pbuffer, uint16_t len)
{
	I2C1_WaitFree(); // 判断 I2C1总线 是否忙碌

	I2C_GenerateSTART(I2C1, ENABLE);				   // 硬件 I2C 生成起始条件
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) // 等待 EV5
		return 0;

	I2C_Send7bitAddress(I2C1, devaddress, I2C_Direction_Transmitter); // 硬件I2C发送从机地址，方向为发送
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))  // 等待 EV6
		return 0;

	I2C_SendData(I2C1, regaddress);							 // 硬件I2C发送寄存器地址
	if (!I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTING)) // 等待 EV8
		return 0;

	while (len)
	{
		I2C_SendData(I2C1, *pbuffer);							// 硬件I2C发送数据
		if (!I2C1_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) // 等待 EV8_2
			return 0;

		len--;
		pbuffer++;
	}

	I2C_GenerateSTOP(I2C1, ENABLE); // 硬件I2C生成终止条件

	return 1;
}
