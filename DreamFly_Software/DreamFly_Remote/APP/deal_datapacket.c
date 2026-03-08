/****************************************************************************************
数据的打包和解包
*****************************************************************************************/
#include "STM32F103C8T6.h"



//u8 SENSER_OFFSET_FLAG;

//uint8_t packetData[12];		//打包后待发送的数据包，这个长度必须和接收端NRF的接收通道定义的有效数据长度相同，否则接收不到数据
//u8 dataPID = 0;			//数据包识别PID	
//vu16 accelerator = 0;	//记录油门上一次的数值，用于下拉油门的时候，防止动力损失过大造成的失衡

//extern u8 NRF24L01_RX_DATA[RX_PAYLOAD_WIDTH];	
//extern vu8 KeyNumber;				//用来保存哪个按键刚被按下了，原始定义在：button.c文件中

////vu16 RC_Value[4];



///*****************************************************************************
//*函  数：__IO uint16_t ADC_ValueLimit(vu16 value, vu16 L, vu16 R, vu16 min, vu16 max)
//*功  能：摇杆限幅
//*参  数：无
//*返回值：无
//*备  注：按照下面的通讯协议进行打包数据，以字节为单位：
//         前导码 -- Key Number -- ADC1 低 8 位 -- ADC1 高 8 位-- ADC2低 8 位 -- ADC2高 8 位-- ADC3低 8 位
//--ADC3高 8 位 -- ADC4低 8 位 -- ADC4高 8 位 -- 数据包标识 PID -- 校验码 0xA5（其中 ADC1、ADC2、ADC3、ADC4是
//指 ADC1 的 1、2、3、4 通道）
//      前导码只有 0x01 和 0x08 才表示有效的数据包，0x01 表示此数据包是由 ADC 采样完成触发的，0x08 表示此数据包
//是由遥控器上的按键触发的，数据包标识 PID 用于接收端 NRF24L01 识别是否是同一数据包的作用（这在飞机上主要用于当
//遥控信号中断时，自动开始降落）
//**************************************************************************************/
//void Remote_SendPacket(uint8_t firstByte)
//{	
//	uint8_t i = 0;
//	
//	//数据包识别PID自增，并且超过200时自动归零
//	if(dataPID >= 200){
//		dataPID = 0;
//	}else{
//		dataPID ++;
//	}
//	
//	//直接采用指针操作内存中的数值将 16 位转成 8 位，速度快且不会发生精度截取，注意，STM32 是小端地址
//	//这种方法通过将 Register_Value 的地址强制转换为 uint8_t* 类型的指针，逐个访问其字节。然而，它依赖
//	//于内存对齐和硬件平台的特性，不推荐在跨平台或移植性要求高的情况下使用。
//	packetData[i ++] = firstByte;						    //前导码
//	packetData[i ++] = KeyNumber;                   		//按键码
//	packetData[i ++] = *(((uint8_t*)RC_Value) + 0);		//YAW   高 8 位
//	packetData[i ++] = *(((uint8_t*)RC_Value) + 1);		//YAW   低 8 位
//	packetData[i ++] = *(((uint8_t*)RC_Value) + 2);		//THR   高 8 位
//	packetData[i ++] = *(((uint8_t*)RC_Value) + 3);		//THR   低 8 位
//	packetData[i ++] = *(((uint8_t*)RC_Value) + 4);       	//ROLL  高 8 位
//	packetData[i ++] = *(((uint8_t*)RC_Value) + 5);		//ROLL  低 8 位
//	packetData[i ++] = *(((uint8_t*)RC_Value) + 6);		//PITCH 高 8 位
//	packetData[i ++] = *(((uint8_t*)RC_Value) + 7);		//PITCH 低 8 位
//	packetData[i ++] = dataPID;								//这个非常重要，这是防止飞机逃脱遥控的保证
//	packetData[i ++] = 0xA5;								//校验码：1010 0101
//}

////接收飞机发送过来的数值
//void Remote_ReceivePacket(void)
//{
//	if(NRF24L01_RX_DATA[0] == 0xFF )
//	{
//		SENSER_OFFSET_FLAG = NRF24L01_RX_DATA[1];
//		Flight.Thr = (         (NRF24L01_RX_DATA[2] <<8) | NRF24L01_RX_DATA[3]);
//		Flight.Yaw = ((int16_t)(NRF24L01_RX_DATA[4] <<8) | NRF24L01_RX_DATA[5])  / 100;
//		Flight.Pit = ((int16_t)(NRF24L01_RX_DATA[6] <<8) | NRF24L01_RX_DATA[7])  / 100;
//		Flight.Rol = ((int16_t)(NRF24L01_RX_DATA[8] <<8) | NRF24L01_RX_DATA[9])  / 100;
//		Flight.Alt = ((int16_t)(NRF24L01_RX_DATA[10]<<8) | NRF24L01_RX_DATA[11]) / 100;
//		Flight.Bat = ((int16_t)(NRF24L01_RX_DATA[12]<<8) | NRF24L01_RX_DATA[13]);
////		printf("SENSER_OFFSET_FLAG:0x%x\r\n",SENSER_OFFSET_FLAG);
////		printf("Thr:%d Yaw:%d Rol:%d Pit:%d\r\n",Thr,Yaw,Rol,Pit);
//	}
//}
