#include "TIM.h"

#include "Scheduler.h"
#include "Flight.h"

void Scheduler_Init(void)
{
	TIM_Cmd(TIM2, ENABLE);
}

static void Scheduler_Loop500Hz(void)	// 2ms执行一次
{
	Flight_GetIMUData();
	Flight_GetOpticalFlowData();
	Flight_MotorPIDCascadeControl();
}

static void Scheduler_Loop200Hz(void)	// 4ms执行一次
{
	Flight_ReceiveRemoteData();
	Flight_SendFlightDataToRemote();
	Flight_ControlModeSelect();
}

static void Scheduler_Loop100Hz(void)	// 10ms执行一次
{
	Flight_AttitudeAlgorithm();
	Flight_FusionOpticalFlowData(0.01);
	Flight_ExecuteRemoteCommand();
}

static void Scheduler_Loop50Hz(void)	// 20ms执行一次
{
	Flight_ANOPollingData();
}

static void Scheduler_Loop20Hz(void)	// 50ms执行一次
{
	Flight_CheckBatteryVoltage();
}

static void Scheduler_Loop10Hz(void)	// 100ms执行一次
{
	Flight_LEDControl();
	Flight_WS2812BControl();
}

static void Scheduler_Loop2Hz(void) 	// 500ms执行一次
{
	
}

///****************************************************************************************************
//*函  数: void USART1_IRQHandler(void)
//*功  能: USART1中断函数，上位机与OpenMV共用
//*参  数: 无
//*返回值: 无
//*备  注: 当插线调试参时，用USART1，对于连续的数据帧的接收，接收中断与空闲中断配合能解决对报问题
//****************************************************************************************************/
// void USART1_IRQHandler(void)
//{
//	uint8_t ClearFlag, res; //防止编译时报错
//
// 	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) 			//接收中断（RXNE）
// 	{
// 		res = USART1->DR; 											//写DR清除中断标志
////		ANO_DT_Data_Receive_Prepare(res); 							//上位机数据接收与解析
//// 		MVRxBuffer[MVRxCounter++] = res;
// 	}
//
// 	else if (USART_GetITStatus(USART1, USART_IT_IDLE) == SET) 		//空闲中断（IDIE）
// 	{
// 		ClearFlag = USART1->SR; 									//读SR寄存器
// 		ClearFlag = USART1->DR; 									//读DR寄存器（先读 SR，再读 DR，为了清除 IDIE 中断）
//// 		MVRxCounter = 0;
// 	}
//
// 	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
//}

///****************************************************************************************************
//*函  数: void USART2_IRQHandler(void)
//*功  能: USART2中断函数，上位机和WiFi遥控数据共用
//*参  数: 无
//*返回值: 无
//*备  注: 上位机与WiFi遥控最好不要同时用，当遥控数据一帧数据接收完成才触发空闲中断;
//*      : 当用WiFi无线调参的时候用USART2接收上位机数据;
//*      : 对于连续的数据帧的接收 接收中断与空闲中断配合能解决对报问题;
//****************************************************************************************************/
// void USART2_IRQHandler(void)
//{
//	uint8_t clear = clear; //防止编译时报错
//	uint8_t res;
//
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET) //接收中断
//	{
//		res = USART2->DR; //写DR清除中断标志
//		ANO_DT_Data_Receive_Prepare(res); //上位机数据接收与解析
//		RxBuffer[RxCounter++] = res;
//	}else if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET) //空闲中断
//	{
//		clear = USART2->SR; //读SR寄存器
//		clear = USART2->DR; //读DR寄存器（先读SR,再度DR,就是为了清除IDIE中断）
//		WiFi_Data_ReceiveAnalysis(RxBuffer,RxCounter-1); //WiFi遥控器数据的解析
//		RxCounter = 0;
//	}
//	USART_ClearITPendingBit(USART2,USART_IT_RXNE);
//}

/*****************************************************************************
 *函  数: void TIM2_IRQHandler(void)
 *功  能: TIM2定时器中断，1ms进一次中断也就是1000Hz
 *参  数: 无
 *返回值: 无
 *备  注: 此函数是整个程序的运行时基，不同的中断时间对应不同频率，
 *        对于一些计算对调用时间要求比较严格时可用此方法；
 *        扫描频率 = 1000Hz/分频系数；
 *****************************************************************************/
void TIM2_IRQHandler(void)
{
	// 分频系数
	static uint8_t cnt_2 = 0, cnt_5 = 0, cnt_10 = 0;
	static uint8_t cnt_20 = 0, cnt_50 = 0, cnt_100 = 0;
	static uint16_t cnt_500 = 0;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		cnt_2++;
		cnt_5++;
		cnt_10++;
		cnt_20++;
		cnt_50++;
		cnt_100++;
		cnt_500++;

		if (cnt_2 >= 2) // 500Hz
		{
			cnt_2 = 0;
			Timer.Period_500Hz = 1;
		}
		if (cnt_5 >= 5) // 200Hz
		{
			cnt_5 = 0;
			Timer.Period_200Hz = 1;
		}
		if (cnt_10 >= 10) // 100Hz
		{
			cnt_10 = 0;
			Timer.Period_100Hz = 1;
		}
		if (cnt_20 >= 20) // 50Hz
		{
			cnt_20 = 0;
			Timer.Period_50Hz = 1;
		}
		if (cnt_50 >= 50) // 20Hz
		{
			cnt_50 = 0;
			Timer.Period_20Hz = 1;
		}
		if (cnt_100 >= 100) // 10Hz
		{
			cnt_100 = 0;
			Timer.Period_10Hz = 1;
		}
		if (cnt_500 >= 500) // 2Hz
		{
			cnt_500 = 0;
			Timer.Period_2Hz = 1;
		}
	}

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

/*****************************************************************************
 *函  数: void TIM2_IRQHandler(void)
 *功  能: TIM2定时器中断，1ms进一次中断也就是1000Hz
 *参  数: 无
 *返回值: 无
 *备  注: 此函数是整个程序的运行时基，不同的中断时间对应不同频率，
 *        对于一些计算对调用时间要求比较严格时可用此方法；
 *        扫描频率 = 1000Hz/分频系数；
 *****************************************************************************/
void TIM5_IRQHandler(void)
{

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{

	}

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
}

/*****************************************************************************
 *函  数: void TIM2_IRQHandler(void)
 *功  能: TIM2定时器中断，1ms进一次中断也就是1000Hz
 *参  数: 无
 *返回值: 无
 *备  注: 此函数是整个程序的运行时基，不同的中断时间对应不同频率，
 *        对于一些计算对调用时间要求比较严格时可用此方法；
 *        扫描频率 = 1000Hz/分频系数；
 *****************************************************************************/
void Scheduler_Loop(void)
{
	if (Timer.Period_500Hz == 1) // 500Hz Task
	{
		Scheduler_Loop500Hz();
		Timer.Period_500Hz = 0;
	}
	if (Timer.Period_200Hz == 1) // 200Hz Task
	{
		Scheduler_Loop200Hz();
		Timer.Period_200Hz = 0;
	}
	if (Timer.Period_100Hz == 1) // 100Hz Task
	{
		Scheduler_Loop100Hz();
		Timer.Period_100Hz = 0;
	}
	if (Timer.Period_50Hz == 1) // 50Hz Task
	{
		Scheduler_Loop50Hz();
		Timer.Period_50Hz = 0;
	}
	if (Timer.Period_20Hz == 1) // 20Hz Task
	{
		Scheduler_Loop20Hz();
		Timer.Period_20Hz = 0;
	}
	if (Timer.Period_10Hz == 1) // 20Hz Task
	{
		Scheduler_Loop10Hz();
		Timer.Period_10Hz = 0;
	}
	if (Timer.Period_2Hz == 1) // 2Hz Task
	{
		Scheduler_Loop2Hz();
		Timer.Period_2Hz = 0;
	}
}

/***************** (C) COPYRIGHT 2024 梅雨 *** END OF FILE *******************/
