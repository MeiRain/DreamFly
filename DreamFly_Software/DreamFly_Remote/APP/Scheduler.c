#include "TIM.h"

#include "Scheduler.h"
#include "Remote.h"

void Scheduler_Init(void)
{
	TIM_Cmd(TIM2, ENABLE);			//使能TIM2，定时器开始运行
}

static void Scheduler_Loop500Hz(void)	//2ms执行一次
{	
	Remote_JoyStickScan();
//	ANO_NRF_Check_Event();//检查是否收到飞机数据
//	if(NRF_Evnet)//如果收到数据就把它发到上位机
//	{
//		Usb_Hid_Adddata(NRF24L01_2_RXDATA , RX_LEN);
//		Usb_Hid_Send();   //数据发到上位机
//		NRF_Evnet = 0;
//	}
//	ANO_Stick_Scan();  //读取遥杆等数据
}

static void Scheduler_Loop200Hz(void)	//4ms执行一次
{
	Remote_ReceiveFlightData();
	Remote_SendDataToFlight();  //发送控制数据给飞机
}

static void Scheduler_Loop100Hz(void)	//10ms执行一次
{
	Remote_KeyScanAndHandle();
//	Gesture_Check();		//显示屏摇杆上下选择操作检测
//	SysTick_count++; 		 //按键扫描延时用
//	
//	if(!Show.Connect_Succeed)//如果与飞机连接失败就发送遥控数据到电脑
//	{
//		ANO_DT_Send_RCData_To_Pc();
//	}
}

static void Scheduler_Loop50Hz(void)	//20ms执行一次
{
	Remote_BuzzerControl();
}

static void Scheduler_Loop20Hz(void)	//50ms执行一次
{
//	LED_Toggle();
//	Show.oled_delay = 1;//屏幕显示延时
//	NRF_Check_Ch();//自动对频检测
//	
//	
//	if(send_flag)//发送设置数据到飞机	
//	ANO_DT_Send_Flag_To_Fly(set_temp,0);
}

static void Scheduler_Loop2Hz(void) // 1000ms执行一次
{
//	printf("YAW: %u\r\n", ADC_GetValue(YAW));
//	printf("THR: %u\r\n", ADC_GetValue(THR));
//	printf("ROL: %u\r\n", ADC_GetValue(ROL));
//	printf("PIT: %u\r\n", ADC_GetValue(PIT));
//	printf("BAT: %u\r\n\r\n", ADC_GetValue(BAT));
	LED_Toggle();
//	//更新电压值
//	Show.Battery_Rc = Rc.AUX5;
//	if(Show.Battery_Rc<345 && Show.Battery_Rc>320)  //判断电压提示电池电量低
//		Show.low_power = 1;
//	else                    
//		Show.low_power = 0;
//	
//	//计算收到飞机数据的帧率
//	NRF_SSI = NRF_SSI_CNT;
//	Rc.AUX6 = NRF_SSI;
//	NRF_SSI_CNT = 0;
//	
//	/*如果帧率为0标记为失联状态*/
//	if(NRF_SSI==0)
//	{
//		ANO_LED_blue_OFF;
//		Show.Rc_num = 0;
//		Show.Connect_Succeed = 0;
//		Show.hardware_type = 0;
//		Show.test_flag = 0;
//		Show.windows = 0;
//	}
//	else
//	{
//		ANO_LED_blue_ON;
//		Show.Connect_Succeed = 1;
//	}
}

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
	if (Timer.Period_2Hz == 1) // 2Hz Task
	{
		Scheduler_Loop2Hz();
		Timer.Period_2Hz = 0;
	}
}
/******************* (C) COPYRIGHT 2014 ANO TECH *****END OF FILE************/
