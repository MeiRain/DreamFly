/*****************************************************************************
将打包好的数据包发送出去
触发数据发送的有：按键中断，用于传输ADC采样的DMA传输完成中断

注意：
1.由于用到了几个中断函数，所以要在nvic.c文件中配置好他们的中断优先级顺序，
2.这个源文件，只需将其头文件包含进main.c文件中即可，无需在主函数中加入初始化函数
****************************************************************************/
#include "STM32F103C8T6.h"

u16 TX_CNT = 0;                     //遥控数据发送计数
u16 TX_ERROR = 0;                   //遥控数据发送失败计数
float TX_ERROR_PERCENT = 0;         //遥控数据丢包率
extern  u8 FLY_Connect_OK;             //飞机数据接收完成
extern	u8 Reconnection_flag;          //遥控与飞机已连接

//extern u8 packetData[11];				    //打包后待发送的数据包，原始定义在：nRF.c文件中
extern vu8 KeyNumber;					    //标记哪个按钮被点击了，原始定义在：button.c文件中



//求遥控数据丢包率
void Get_TxErrorPercent(void)
{
		if(TX_CNT++ == 100) //满100清零
		{
			TX_CNT = 0; 
			TX_ERROR = 0;
		}
		TX_ERROR_PERCENT = (float)TX_ERROR/(float)TX_CNT;
		if(TX_ERROR_PERCENT == 1)
		{
			Reconnection_flag = 1;			                 //重连标志置位
			FLY_Connect_OK = 0 ;                         //飞机与遥控器断开连接
		}
	//	printf("TX_CNT:%d TX_ERROR:%d baifen:%0.2f\r\n",TX_CNT,TX_ERROR,TX_ERROR_PERCENT);
}


