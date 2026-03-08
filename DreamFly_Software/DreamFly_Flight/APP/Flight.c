#include <stdio.h>

#include "STM32F411CEU6.h"

#include "NVIC.h"
#include "Flash.h"
#include "USART.h"
#include "TIM.h"
#include "ADC.h"
#include "IIC.h"
#include "I2C.h"
#include "SPI.h"

#include "LED.h"
#include "WS2812B.h"
#include "Motor.h"
#include "ICM20948.h"
#include "AK09916.h"
#include "BMP280.h"
#include "Si24R1.h"
#include "VL53l1X.h"
#include "PMW3901MB.h"

#include "Filter.h"
#include "Attitude.h"
#include "Altitude.h"
#include "PID.h"
#include "ANOAssistant.h"

#include "Flight.h"

// 数据拆分宏定义，在发送大于1字节的数据类型时，把数据拆分成单独字节进行发送
#define BYTE0(dwTemp) (*((char *)(&dwTemp)))
#define BYTE1(dwTemp) (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp) (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp) (*((char *)(&dwTemp) + 3))

//
#define COMMUNICATION_HEADER 0xAB
#define FLIGHT_ADDR 0x66
#define REMOTE_ADDR 0x88

// 按键状态定义
typedef enum
{
	Connect,
	UNLOCK, // 等待油门打到最低阶段
	Lock,
	LowVoltage, // 等待油门打到最高阶段
} FlightState_e;

// 按键状态定义
typedef enum
{
	UNLOCK_WATTING1, // 等待油门打到最低阶段
	UNLOCK_WATTING2, // 等待油门打到最高阶段
	UNLOCK_WATTING3, // 等待油门打到最低阶段
	UNLOCK_SUCCESS,	 // 解锁成功阶段
	UNLOCK_ENTER,		 // 进入解锁状态阶段，已经解锁
	UNLOCK_EXTI,	 // 退出解锁阶段，进行上锁
} UnlockState_e;

/*****************************************************************************/
/**************************** 数组变量定义声明区 *****************************/

static int16_t ICM20948_Bias[6];

/***************************** 结构体定义声明区 ******************************/

static battery_value_t g_Battery; // 电池信息相关结构体
static flightflag_t g_FlightFlag; // 飞控相关标志位结构体
static remotedata_t g_RemoteData; // 接收到的遥控器信息相关结构体

/************************* 外部结构体调用定义声明区 **************************/
static motor_t g_MOTOR;
static icm_t g_ICM20948_Raw;
//static icm_t g_ICM20948_Bias;
static icm_t g_ICM20948_Filter;
static bmp_t g_BMP280_Raw;
static pmw3901mb_t g_PMW3901MB_Raw;
static opticalflow_t g_OpticalFlow;
static angle_t g_ATT_Angle; // 飞机姿态数据
static pid_t g_PID_AngleX;   //横滚角ROL：X轴
static pid_t g_PID_AngleY;   //俯仰角PIT：Y轴
static pid_t g_PID_AngleZ;   //偏航角YAW：Z轴
static pid_t g_PID_RateX;	//横滚角ROL：X轴
static pid_t g_PID_RateY;   //俯仰角PIT：Y轴
static pid_t g_PID_RateZ;   //偏航角YAW：Z轴
//static pid_t *g_PIDs[] = {&g_PID_AngleX, &g_PID_AngleY, &g_PID_AngleZ, 
//						  &g_PID_RateX, &g_PID_RateY, &g_PID_RateZ};//数组指针

/************************* 初始化 ****************************/
void Flight_HardwareCheck(void)
{
	while (!ICM20948_Check())
	{
		printf("\r\nICM20948 Connect Fail!\r\n");
		WS2812B_AllViolet();
	}
	
	while (!BMP280_Check())
	{
		printf("\r\nBMP280 Connect Fail!r\n");
		WS2812B_AllBlue();
	}
	
	while (!Si24R1_Check())
	{
		printf("\r\nSi24R1 Connect Fail!r\n");
		WS2812B_AllWhite();
	}
}

void Flight_ParameterInit(void)
{
	g_Battery.ADCV = 0;		  // 电池电压采集ADC值
	g_Battery.RealV = 3.31f;  // 实际测量的 MCU 的 ADC 管脚供电电压（注意此电压必须亲测）
	g_Battery.MeasureV = 0;	  // 程序测量的实际电池电压
	g_Battery.AlarmV1 = 3.2f; // 电池低电压报警瞬时值 (这个值需要根据机身不同重量实测，实测380mh是2.8v)
	g_Battery.AlarmV2 = 3.7f;
	
	g_MOTOR.PWM1 = 0;
	g_MOTOR.PWM2 = 0;
	g_MOTOR.PWM3 = 0;
	g_MOTOR.PWM4 = 0;

	g_FlightFlag.Ready = 0;
	g_FlightFlag.Unlock = 0;
	g_FlightFlag.Connect = 0;
	g_FlightFlag.LowVoltage = 0;
	
	//内环
	g_PID_RateX.Kp = -3.0f; // -3.0
	g_PID_RateY.Kp = 2.0f;  // 2.0
	g_PID_RateZ.Kp = -2.0f; // -2.0
		
	g_PID_RateX.Ki = 0.0f;
	g_PID_RateY.Ki = 0.0f;
	g_PID_RateZ.Ki = 0.0f;
	
	g_PID_RateX.Kd = -0.08f; //-0.08
	g_PID_RateY.Kd = 0.08f;  // 0.08
	g_PID_RateZ.Kd = 0.00f;

	//外环
	g_PID_AngleX.Kp = 7.0f;  //7.0
	g_PID_AngleY.Kp = 7.0f; 
	g_PID_AngleZ.Kp = 0.0f;
	
	g_PID_AngleX.Ki = 0.0f;
	g_PID_AngleY.Ki = 0.0f;
	g_PID_AngleZ.Ki = 0.0f;
		
	g_PID_AngleX.Kd = 0.0f;
	g_PID_AngleY.Kd = 0.0f;
	g_PID_AngleZ.Kd = 0.0f;
	
	//ROLL轴
	g_PID_RateX.Ilimit = 250;    //Roll轴角速度积分范围
	g_PID_RateX.Irang = 2000;    //Roll轴角速度积分限幅度（由于电机输出有限，所以积分输出也是有限的）
	g_PID_AngleX.Ilimit = 35;    //Roll轴角度积分范围
	g_PID_AngleX.Irang = 1000;    //Roll轴角度积分限幅度（由于电机输出有限，所以积分输出也是有限的）

	//PITCH轴
	g_PID_RateY.Ilimit = 250;    //Pitch轴角速度积分范围
	g_PID_RateY.Irang = 2000;    //Pitch轴角速度积分限幅度（由于电机输出有限，所以积分输出也是有限的）
	g_PID_AngleY.Ilimit = 35;    //Roll轴角度积分范围
	g_PID_AngleY.Irang = 1000;    //Roll轴角度积分限幅度（由于电机输出有限，所以积分输出也是有限的）
	
	//YAW轴
	g_PID_RateZ.Ilimit = 150;    //Yaw轴角速度积分范围
	g_PID_RateZ.Irang = 1200;    //Yaw轴角速度积分限幅度（由于电机输出有限，所以积分输出也是有限的）
	g_PID_AngleZ.Ilimit = 35;    //Yaw轴角度积分范围
	g_PID_AngleZ.Irang = 200;    //Yaw轴角度积分限幅度（由于电机输出有限，所以积分输出也是有限的）

	//高度环
//	PID_ALT_Rate.Ilimit = 0;
//	PID_ALT_Rate.Irang = 0;
//	PID_ALT.Ilimit_flag = 0;
//	PID_ALT.Ilimit = 100;
//	PID_ALT.Irang = 200;
}

void Flight_AllInit(void)
{
	NVIC_SystemInit();
	USART1_Init(115200);
	USART2_Init(460800);
	TIM2_TimerInit();
	TIM3_OCInit();
	TIM4_OCInit();
	ADC1_Init();
	IIC_Init();
	I2C1_Init();
	SPI1_Init();
	SPI2_Init();
	
	LED_Init();
	WS2812B_Init();
	Motor_Init();
//	Flight_HardwareCheck();
	ICM20948_Init(); // 故障灯紫色
//	AK09916_Init();	// 故障灯紫色
	BMP280_Init();	// 故障灯紫色
//	VL53L1X_Init();	// 故障灯紫色
	PMW3901MB_Init();	// 故障灯紫色
	Si24R1_Init(MODE_RX2); // 故障灯黄色
	Flight_ParameterInit();		//飞控参数初始化
	
//	Flight_ParameterSave();
//	Flight_OffSetICM20948Data();	//校准陀螺仪
	
	g_FlightFlag.Ready = 1;	//飞控初始化完毕，准备起飞
	
	LOG_DEBUG("This is a debug message: %d\n", 123);
    LOG_INFO("This is an info message: %s\n", "STM32");
    LOG_WARNING("This is a warning message\n");
    LOG_ERROR("This is an error message\n");
}

void Flight_ReadFlashData(void)
{
	
}

void Flight_WriteFlashData(void)
{
	
}

void Flight_LightControl(void)
{
	LED_Toggle();
}


void Flight_LEDControl(void)
{
	LED_Toggle();
}

void Flight_WS2812BControl(void)
{
	//灯控判断逻辑优先级依次从高到低
	static uint8_t TimeCount = 0;
	static uint8_t UnlockStatus = 0;
	
	if (!g_FlightFlag.Connect)
		WS2812B_AllWhite();
	
//	else if (g_FlightFlag.LowVoltage)
//		WS2812B_RedFlash();	
	
	else if (!g_FlightFlag.Unlock && g_FlightFlag.Connect)
	{
		UnlockStatus = 1;
		WS2812B_Run();
	}

	else if (g_FlightFlag.Unlock && g_FlightFlag.Connect)
	{
		TimeCount++;
		if(UnlockStatus)//消除1s的滞后延迟
		{
			UnlockStatus = 0;
			WS2812B_AllGreen();
		}
		
		if(TimeCount == 10)
		{
			TimeCount = 0;
			WS2812B_FlyIndicator();
		}
	}
	else
		WS2812B_AllOFF();
		
}

void Flight_CheckBatteryVoltage(void)
{
	static uint8_t cnt1 = 0, cnt2 = 0;

	Filter_MovingAver((float)ADC_GetValue(), &g_Battery.ADCV, 6); // 滑动滤波一下电压值，提高精度
	g_Battery.ADCV = ADC_GetValue();
	g_Battery.MeasureV = (g_Battery.ADCV * g_Battery.RealV / 4095.0f) * 2;
	//	printf("Test Voltage: %0.2f   ADC: %0.0f\r\n", g_Battery.MeasureV, g_Battery.ADCV);

	if (g_FlightFlag.Unlock) // 飞行时测量
	{
		if (g_Battery.MeasureV < g_Battery.AlarmV1)
		{
			if (cnt1++ > 10)
			{
				cnt1 = 0;
				g_FlightFlag.LowVoltage = 1;
			}
		}
		else
		{
			cnt1 = 0;
			g_FlightFlag.LowVoltage = 0;
		}
	}

	else
	{
		if (g_Battery.MeasureV < g_Battery.AlarmV2) // 落地时测量（380mh时是3.5V）
		{
			if (cnt2++ > 10)
			{
				cnt2 = 0;
				g_FlightFlag.LowVoltage = 1;
			}
		}
		else
		{
			cnt2 = 0;
			g_FlightFlag.LowVoltage = 0;
		}
	}
}

void Flight_GetICM20948Data(void)
{
	
}

void Flight_OffSetICM20948Data(void)
{
	uint8_t i = 30;
	const int8_t QUIET_GYRO_MIN = -5;
	const int8_t QUIET_GYRO_MAX = 5;
	int16_t LastGyro[3] = {0};
	int16_t ErrorGyro[3] = {0};
	int32_t Buffer[6] = {0};

	// 判断传感器是否处于静止状态，内层 do - while 循环，等待满足静止条件
	while (i--)
	{
		WS2812B_AllRed();
		do
		{
			Delay_ms(10);
			Flight_GetICM20948Data();
			ErrorGyro[0] = g_ICM20948_Raw.GyrX - LastGyro[0];
			ErrorGyro[1] = g_ICM20948_Raw.GyrY - LastGyro[1];
			ErrorGyro[2] = g_ICM20948_Raw.GyrZ - LastGyro[2];
			LastGyro[0] = g_ICM20948_Raw.GyrX;
			LastGyro[1] = g_ICM20948_Raw.GyrY;
			LastGyro[2] = g_ICM20948_Raw.GyrZ;
		} while (ErrorGyro[0] < QUIET_GYRO_MIN || ErrorGyro[0] > QUIET_GYRO_MAX || ErrorGyro[1] < QUIET_GYRO_MIN || ErrorGyro[1] > QUIET_GYRO_MAX || ErrorGyro[2] < QUIET_GYRO_MIN || ErrorGyro[2] > QUIET_GYRO_MAX);
	}

	for (uint16_t i = 0; i < 365; i++)
	{
		Flight_GetICM20948Data();
		if (i >= 100)
		{
			Buffer[0] += g_ICM20948_Raw.AccX;
			Buffer[1] += g_ICM20948_Raw.AccY;
			Buffer[2] += g_ICM20948_Raw.AccZ - 8192;
			Buffer[3] += g_ICM20948_Raw.GyrX;
			Buffer[4] += g_ICM20948_Raw.GyrY;
			Buffer[5] += g_ICM20948_Raw.GyrZ;
		}
	}

	for (uint8_t i = 0; i < 6; i++)
	{
		ICM20948_Bias[i] = Buffer[i] >> 8;
	}

	printf("== ICM Bias TEST ==\r\n");
	printf(" Bias ACCX: %d\r\n", ICM20948_Bias[0]);
	printf(" Bias ACCY: %d\r\n", ICM20948_Bias[1]);
	printf(" Bias ACCZ: %d\r\n", ICM20948_Bias[2]);
	printf(" Bias GYRX: %d\r\n", ICM20948_Bias[3]);
	printf(" Bias GYRY: %d\r\n", ICM20948_Bias[4]);
	printf(" Bias GYRZ: %d\r\n", ICM20948_Bias[5]);
	printf("===================\r\n\r\n");

	WS2812B_BlueFlash();
}

void Flight_GetBMP280Data(void)
{
	
}

void Flight_GetPMW3901MBData(void)
{
	
}

void Flight_GetIMUData(void)
{
	// 设置加速度滤波参数
	static _1_ekf_filter EKF_Filter[3] = {
		{0.02, 0, 0, 0, 0.001, 0.543},
		{0.02, 0, 0, 0, 0.001, 0.543},
		{0.02, 0, 0, 0, 0.001, 0.543}};

	// 设置陀螺仪滤波参数
	static _1_lpf_filter LPF_Filter[3] = {
		{0.85, 0, 0},
		{0.85, 0, 0},
		{0.85, 0, 0}};

	ICM20948_GetAccel(&g_ICM20948_Raw.AccX, &g_ICM20948_Raw.AccY, &g_ICM20948_Raw.AccZ);
	ICM20948_GetGyro(&g_ICM20948_Raw.GyrX, &g_ICM20948_Raw.GyrY, &g_ICM20948_Raw.GyrZ);

	// printf("== ICM Raw TEST ==\r\n");
	// printf(" Raw ACCX: %d\r\n", g_ICM20948_Raw.AccX);
	// printf(" Raw ACCY: %d\r\n", g_ICM20948_Raw.AccY);
	// printf(" Raw ACCZ: %d\r\n", g_ICM20948_Raw.AccZ);
	// printf(" Raw GYRX: %d\r\n", g_ICM20948_Raw.GyrX);
	// printf(" Raw GYRY: %d\r\n", g_ICM20948_Raw.GyrY);
	// printf(" Raw GYRZ: %d\r\n", g_ICM20948_Raw.GyrZ);
	// printf("==================\r\n\r\n");

	g_ICM20948_Raw.AccX = g_ICM20948_Raw.AccX - 18;		//ICM20948_Bias[0];
	g_ICM20948_Raw.AccY = g_ICM20948_Raw.AccY - 36;		//ICM20948_Bias[1];
	g_ICM20948_Raw.AccZ = g_ICM20948_Raw.AccZ - 160;	//ICM20948_Bias[2];
	g_ICM20948_Raw.GyrX = g_ICM20948_Raw.GyrX - 4;		//ICM20948_Bias[3];
	g_ICM20948_Raw.GyrY = g_ICM20948_Raw.GyrY - 7;		//ICM20948_Bias[4];
	g_ICM20948_Raw.GyrZ = g_ICM20948_Raw.GyrZ - (-6);	//ICM20948_Bias[5];

	// 对加速度进行一阶扩展卡尔曼滤波
	Filter_ExtenedKalman_1D(&EKF_Filter[0], g_ICM20948_Raw.AccX);
	g_ICM20948_Filter.AccX = (int16_t)EKF_Filter[0].Out;
	Filter_ExtenedKalman_1D(&EKF_Filter[1], g_ICM20948_Raw.AccY);
	g_ICM20948_Filter.AccY = (int16_t)EKF_Filter[1].Out;
	Filter_ExtenedKalman_1D(&EKF_Filter[2], g_ICM20948_Raw.AccZ);
	g_ICM20948_Filter.AccZ = (int16_t)EKF_Filter[2].Out;

	// 对角速度进行一阶低通滤波
	Filter_LowPass_1st(&LPF_Filter[0], g_ICM20948_Raw.GyrX);
	g_ICM20948_Filter.GyrX = (int16_t)LPF_Filter[0].Out;
	Filter_LowPass_1st(&LPF_Filter[1], g_ICM20948_Raw.GyrY);
	g_ICM20948_Filter.GyrY = (int16_t)LPF_Filter[1].Out;
	Filter_LowPass_1st(&LPF_Filter[2], g_ICM20948_Raw.GyrZ);
	g_ICM20948_Filter.GyrZ = (int16_t)LPF_Filter[2].Out;

// 	printf("= ICM Filter TEST =\r\n");
// 	printf(" Filter ACCX: %d\r\n", g_ICM20948_Filter.AccX);
// 	printf(" Filter ACCY: %d\r\n", g_ICM20948_Filter.AccY);
// 	printf(" Filter ACCZ: %d\r\n", g_ICM20948_Filter.AccZ);
// 	printf(" Filter GYRX: %d\r\n", g_ICM20948_Filter.GyrX);
// 	printf(" Filter GYRY: %d\r\n", g_ICM20948_Filter.GyrY);
// 	printf(" Filter GYRZ: %d\r\n", g_ICM20948_Filter.GyrZ);
// 	printf("===================\r\n\r\n");

//	BMP280_GetRawData(&g_BMP280_Raw);
}

void Flight_GetOpticalFlowData(void)
{
	PMW3901MB_GetRawData(&g_PMW3901MB_Raw.Delata_X, &g_PMW3901MB_Raw.Delata_Y);
	
	g_PMW3901MB_Raw.POS_X += g_PMW3901MB_Raw.Delata_X;
	g_PMW3901MB_Raw.POX_Y += g_PMW3901MB_Raw.Delata_Y;
}

void Flight_FusionOpticalFlowData(float dT)
{
	// 设置陀螺仪滤波参数
	static _1_lpf_filter LPF_Filter = {0.80, 0, 0};
	
	float cpi = ((5*0.01f) / 11.914f) *2.54f ;
	////////////////////////*积分位移处理*////////////////////////////
	//低通滤波
	g_OpticalFlow.Filter_POS_X += ((g_PMW3901MB_Raw.POS_X - g_OpticalFlow.Filter_POS_X) * 0.2f);
	g_OpticalFlow.Filter_POS_Y += ((g_PMW3901MB_Raw.POX_Y - g_OpticalFlow.Filter_POS_Y) * 0.2f);
	
	//传感器倾角参数  用姿态角去补偿积分位移（#define  angle_to_rad  0.0174f  //角度转弧度）
	g_OpticalFlow.Angle_X += (600.0f * tan(-g_ATT_Angle.PIT * DEGTORAD) - g_OpticalFlow.Angle_X) * 0.2f;
	g_OpticalFlow.Angle_Y += (600.0f * tan(-g_ATT_Angle.ROL * DEGTORAD) - g_OpticalFlow.Angle_Y) * 0.2f;

	//位移与角度互补融合
	g_OpticalFlow.Fusion_POS_X = g_OpticalFlow.Filter_POS_X - g_OpticalFlow.Angle_X;  
	g_OpticalFlow.Fusion_POS_Y = g_OpticalFlow.Filter_POS_Y - g_OpticalFlow.Angle_Y;
	
	////////////////////////*微分位移处理*////////////////////////////
	//对积分位移进行微分处理，得到速度。
	//求微分速度
	g_OpticalFlow.Raw_Speed_X = (g_OpticalFlow.Fusion_POS_X - g_OpticalFlow.out_x_i_o)/dT;
	g_OpticalFlow.out_x_i_o = g_OpticalFlow.Fusion_POS_X;
	g_OpticalFlow.Raw_Speed_Y = (g_OpticalFlow.Fusion_POS_Y - g_OpticalFlow.out_y_i_o)/dT;
	g_OpticalFlow.out_y_i_o = g_OpticalFlow.Fusion_POS_Y;
	
	//低通滤波
	g_OpticalFlow.Filter_Speed_X += (g_OpticalFlow.Raw_Speed_X - g_OpticalFlow.Filter_Speed_X ) * 0.1f;
	g_OpticalFlow.Filter_Speed_Y += (g_OpticalFlow.Raw_Speed_Y - g_OpticalFlow.Filter_Speed_Y ) * 0.1f;
	
	///////////////////*光流数据与高度数据融合*//////////////////////////
		
//	//式中HIGH为实际高度，单位：米
//	cpi = ((FlightData.High.bara_height*0.01f) / 11.914f) *2.54f ;
//	pixel_flow.fix_High = cpi;
	 
//	//积分位移值单位转换为：厘米
//	pixel_flow.loc_x = pixel_flow.out_x_i * cpi;
//	pixel_flow.loc_y = pixel_flow.out_y_i * cpi;
//	 
//	//微分速度值单位转换为：厘米/秒
//	pixel_flow.loc_xs = pixel_flow.fix_x * cpi; 
//	pixel_flow.loc_ys = pixel_flow.fix_y * cpi;
	
	static uint8_t cnt = 0;
	cnt++;
	if(cnt==5)
	{
	printf("===========\r\n");
	printf(" Raw POSX: %f\r\n", g_OpticalFlow.Fusion_POS_X* cpi);
	printf(" Raw POSY: %f\r\n", g_OpticalFlow.Fusion_POS_Y* cpi);
	printf(" Raw VX: %f\r\n", g_OpticalFlow.Filter_Speed_X* cpi);
	printf(" Raw VY: %f\r\n", g_OpticalFlow.Filter_Speed_Y* cpi);
	}

}

void Flight_AttitudeAlgorithm(void)
{
	Attitude_Update(&g_ICM20948_Filter.AccX, &g_ICM20948_Filter.AccY, &g_ICM20948_Filter.AccZ,
					&g_ICM20948_Filter.GyrX, &g_ICM20948_Filter.GyrY, &g_ICM20948_Filter.GyrZ, &g_ATT_Angle);
}

void Flight_ReceiveRemoteData(void)
{
	static uint8_t PacketLength;//接收数据包长度
	static uint16_t ErrorCount = 1; //接收计数器，0接收成功，>1接收失败
	uint8_t SumCheck = 0;//和校验
	uint8_t Si24R1_RXBuffer[RX_PAYLOAD_WIDTH]; // Si24R1 接收数组

	Si24R1_IRQHandler(Si24R1_RXBuffer, &ErrorCount); // 接收遥控器数据，接收成功 ErrorCount 被清零
	
	if (ErrorCount == 0)
	{
		g_FlightFlag.Connect = 1;
		
		// 判断帧头、源地址、目标地址
		if (!(*(Si24R1_RXBuffer) == COMMUNICATION_HEADER && *(Si24R1_RXBuffer + 1) == REMOTE_ADDR && *(Si24R1_RXBuffer + 2) == FLIGHT_ADDR))
			return;
	
		PacketLength = *(Si24R1_RXBuffer + 3) + 4;
	
		for (uint8_t i = 0; i < PacketLength; i++)
			SumCheck += *(Si24R1_RXBuffer + i);
	
		if (!(SumCheck == *(Si24R1_RXBuffer + PacketLength)))
			return; // 判断sum
	
		g_RemoteData.ROL = (int16_t)(*(Si24R1_RXBuffer + 4) << 8) | *(Si24R1_RXBuffer + 5);
		g_RemoteData.ROL = (g_RemoteData.ROL <= 1000) ? 1000 : g_RemoteData.ROL; // 摇杆限幅，防止越界
		g_RemoteData.ROL = (g_RemoteData.ROL >= 2000) ? 2000 : g_RemoteData.ROL; // 摇杆限幅，防止越界
	
		g_RemoteData.PIT = (int16_t)(*(Si24R1_RXBuffer + 6) << 8) | *(Si24R1_RXBuffer + 7);
		g_RemoteData.PIT = (g_RemoteData.PIT <= 1000) ? 1000 : g_RemoteData.PIT; // 摇杆限幅，防止越界
		g_RemoteData.PIT = (g_RemoteData.PIT >= 2000) ? 2000 : g_RemoteData.PIT; // 摇杆限幅，防止越界
	
		g_RemoteData.YAW = (int16_t)(*(Si24R1_RXBuffer + 8) << 8) | *(Si24R1_RXBuffer + 9);
		g_RemoteData.YAW = (g_RemoteData.YAW <= 1000) ? 1000 : g_RemoteData.YAW; // 摇杆限幅，防止越界
		g_RemoteData.YAW = (g_RemoteData.YAW >= 2000) ? 2000 : g_RemoteData.YAW; // 摇杆限幅，防止越界
	
		g_RemoteData.THR = (int16_t)(*(Si24R1_RXBuffer + 10) << 8) | *(Si24R1_RXBuffer + 11);
		g_RemoteData.THR = (g_RemoteData.THR <= 1000) ? 1000 : g_RemoteData.THR; // 摇杆限幅，防止越界
		g_RemoteData.THR = (g_RemoteData.THR >= 2000) ? 2000 : g_RemoteData.THR; // 摇杆限幅，防止越界
	
		g_RemoteData.KEY = *(Si24R1_RXBuffer + 12);
	}
	
	ErrorCount++;
	
	if (ErrorCount > 750)//超过3s无连接，判断遥控器失联
	{
		ErrorCount = 1;
		g_FlightFlag.Connect = 0;
		printf("\r\nNo Connect!\r\n");
	}
	
//	static uint8_t cnt = 0;
//	cnt++;
//	if (cnt == 50)
//	{
//		printf("RC_Value: %d   %d   %d   %d\r\n",
//			g_RemoteData.ROL, g_RemoteData.PIT, g_RemoteData.YAW, g_RemoteData.THR);
//		//		printf("RC_Value: %x\r\n", g_RemoteData.KEY);
//		printf("======================================\r\n");
//		cnt = 0;
//	}
}

void Flight_SendFlightDataToRemote(void)
{
	int16_t temp;
	uint8_t cnt = 0, Sum_Check = 0;
	uint8_t Si24R1_TXBuffer[TX_PAYLOAD_WIDTH]; // Si24R1 发送数组

	//	if(g_FlightFlag.Unclock ==1)
	//	{
	//		SENSER_FLAG_SET(FLY_ENABLE); //解锁模式置位
	//	}

	//	else
	//	{
	//		SENSER_FLAG_RESET(FLY_ENABLE); //上锁模式复位
	//	}

	// 4
	Si24R1_TXBuffer[cnt++] = COMMUNICATION_HEADER; // 帧头
	Si24R1_TXBuffer[cnt++] = FLIGHT_ADDR;		   // 源地址
	Si24R1_TXBuffer[cnt++] = REMOTE_ADDR;		   // 目标地址
	Si24R1_TXBuffer[cnt++] = 0x0D;				   // 数据包长度
	// 13
	Si24R1_TXBuffer[cnt++] = 0x00;		 // 飞机解锁标志位
	temp = (int)(g_ATT_Angle.ROL * 100); // 飞机实际横滚数据
	Si24R1_TXBuffer[cnt++] = BYTE1(temp);
	Si24R1_TXBuffer[cnt++] = BYTE0(temp);
	temp = (int)(g_ATT_Angle.PIT * 100); // 飞机实际俯仰数据
	Si24R1_TXBuffer[cnt++] = BYTE1(temp);
	Si24R1_TXBuffer[cnt++] = BYTE0(temp);
	temp = (int)(g_ATT_Angle.YAW * 100); // 飞机实际航向数据
	Si24R1_TXBuffer[cnt++] = BYTE1(temp);
	Si24R1_TXBuffer[cnt++] = BYTE0(temp);
	temp = (int)g_RemoteData.THR; // 飞机期望油门数据
	Si24R1_TXBuffer[cnt++] = BYTE1(temp);
	Si24R1_TXBuffer[cnt++] = BYTE0(temp);
	temp = (int)(0); // 飞机实际高度数据
	Si24R1_TXBuffer[cnt++] = BYTE1(temp);
	Si24R1_TXBuffer[cnt++] = BYTE0(temp);
	temp = (int)(g_Battery.MeasureV * 100); // 飞机电池电压数据
	Si24R1_TXBuffer[cnt++] = BYTE1(temp);
	Si24R1_TXBuffer[cnt++] = BYTE0(temp);

	for (uint8_t i = 0; i < cnt; i++)
		Sum_Check += Si24R1_TXBuffer[i];

	Si24R1_TXBuffer[cnt++] = Sum_Check;

	Si24R1_AckPayloadPacket(Si24R1_TXBuffer, cnt); // Si24R1发送函数
}

static void Flight_JudgmentUnLock(void)
{
	// 解锁指令：油门最低-->油门最高-->油门最低-->解锁
	static uint8_t Status = UNLOCK_WATTING1;
	static uint16_t TimeCount = 0;

	if (g_RemoteData.ROL < 1030 && g_RemoteData.PIT < 1030 && g_RemoteData.YAW > 1970 && g_RemoteData.THR < 1030) // 油门遥杆右下角锁定飞机
		Status = UNLOCK_EXTI;																					  // End if

	switch (Status)
	{
	case UNLOCK_WATTING1:			 // 等待解锁
		if (g_RemoteData.THR < 1030) // 解锁三步，油门最低->油门最高->油门最低 看到LED灯不闪了 即完成解锁
			Status = UNLOCK_WATTING2;
		break;

	case UNLOCK_WATTING2:
		if (g_RemoteData.THR > 1970) // 拉高油门
		{
			TimeCount++;
			if (TimeCount > 20) // 最高油门需保持300ms以上，防止遥控开机初始化未完成的错误数据
			{
				TimeCount = 0;
				Status = UNLOCK_WATTING3;
			}
		}
		break;

	case UNLOCK_WATTING3:
		if (g_RemoteData.THR < 1030) // 拉低油门解锁
		{
			Status = UNLOCK_SUCCESS; // 解锁标志位
			//				baro_start=1;            //气压清零标志
		}
		break;

	case UNLOCK_SUCCESS: // 解锁成功
		printf("\r\nUNLOCK SUCCESS!\r\n");		
		g_FlightFlag.Unlock = 1;
		Status = UNLOCK_ENTER;
		break;

	case UNLOCK_ENTER: // 进入解锁状态
		if (g_RemoteData.THR < 1030)
		{
			if (TimeCount++ > 1000) // 解锁后不动油门遥杆处于最低10S自动上锁
				Status = UNLOCK_EXTI;
		}
		else if (!g_FlightFlag.Unlock)
		{
			Status = UNLOCK_EXTI;
		}
		else
		{
			TimeCount = 0;
		}
		break;

	case UNLOCK_EXTI: // 解锁失败，进入锁定
		printf("\r\nLOCK SUCCESS!\r\n");	
		Status = UNLOCK_WATTING1;
		g_FlightFlag.Unlock = 0;
		TimeCount = 0;
		break;

	default:
		Status = UNLOCK_EXTI;
		break;
	}
}

void Flight_ExecuteRemoteCommand(void)
{
	static uint8_t TimeCount = 0;

	if (g_FlightFlag.Connect == 1)//连接正常
		Flight_JudgmentUnLock();

	else // 失联了3s
	{
		g_RemoteData.ROL = 1500;	 // 通道1   数据归中
		g_RemoteData.PIT = 1500;	 // 通道2	数据归中
		g_RemoteData.YAW = 1500;	 // 通道4	数据归中
		if (g_RemoteData.THR < 1030) // 判断油门，油门很低直接关闭油门
		{
			g_RemoteData.THR = 1000; // 关闭油门
			g_FlightFlag.Unlock = 0; // 退出控制
			Si24R1_Init(MODE_RX2); // 复位一下2.4G模块
		}
		else // 油门很高，慢慢降低油门直到进入g_RemoteData.THR < 1030条件的if函数
		{
			if (TimeCount++ > 100) // 控制油门减小的时间，假设函数10ms执行一次，if执行周期为1s一次
			{
				TimeCount = 0;
				g_RemoteData.THR -= 20; // 通道3 油门通道在原来的基础上自动慢慢减小  起到飞机慢慢下降
			}
		}
	}// End if
}

void Flight_MotorPIDCascadeControl(void)
{
	const float PIDCONTROL_TERM = 0.002;
	static uint8_t Integral_Flag = 0;
	static int16_t Throttle = 0;
	
	Throttle = g_RemoteData.THR - 1000;
	
	if (g_FlightFlag.Unlock && Throttle >= 180)
		Integral_Flag = 1;
	else
		Integral_Flag = 0;

	//赋值角度的测量值
	g_PID_AngleX.Measure = g_ATT_Angle.ROL;
	g_PID_AngleY.Measure = g_ATT_Angle.PIT;
	g_PID_AngleZ.Measure = g_ATT_Angle.YAW;
	//赋值角速度的测量值
	g_PID_RateX.Measure = g_ICM20948_Filter.GyrX * GYR_DEG_SCALEFACTOR;
	g_PID_RateY.Measure = g_ICM20948_Filter.GyrY * GYR_DEG_SCALEFACTOR;
	g_PID_RateZ.Measure = g_ICM20948_Filter.GyrZ * GYR_DEG_SCALEFACTOR;
	/*
		俯仰角 ---》 Y轴角速度
		横滚角 ---》 X轴角速度
		偏航角 ---》 Z轴角速度
	*/
	PID_CascadeControl(&g_PID_AngleX, &g_PID_RateX, Integral_Flag, PIDCONTROL_TERM);
	PID_CascadeControl(&g_PID_AngleY, &g_PID_RateY, Integral_Flag, PIDCONTROL_TERM);
	PID_CascadeControl(&g_PID_AngleZ, &g_PID_RateZ, Integral_Flag, PIDCONTROL_TERM);

	if(g_FlightFlag.Unlock && Throttle >= 180)//当油门大于150时和飞机解锁时动力分配才生效
	{
		g_MOTOR.PWM1 = Throttle + g_PID_RateX.Out + g_PID_RateY.Out + g_PID_RateZ.Out;
		g_MOTOR.PWM2 = Throttle + g_PID_RateX.Out - g_PID_RateY.Out - g_PID_RateZ.Out;
		g_MOTOR.PWM3 = Throttle - g_PID_RateX.Out - g_PID_RateY.Out + g_PID_RateZ.Out;
		g_MOTOR.PWM4 = Throttle - g_PID_RateX.Out + g_PID_RateY.Out - g_PID_RateZ.Out;
	}
	else
	{
		g_MOTOR.PWM1 = 0;
		g_MOTOR.PWM2 = 0;
		g_MOTOR.PWM3 = 0;
		g_MOTOR.PWM4 = 0;
	}
	Motor_Control(&g_MOTOR); //将此数值分配到定时器，输出对应占空比的PWM波
}



void Flight_ControlModeSelect(void)
{
		const float REMOTE_CONTROL_SCALEFACTOR = 0.04f;
	
//		if(ALL_flag.unlock == 1)  //判断解锁
//		{
//			if(Remote.AUX2 < 1700)   //如果大于1700 则进入定点模式显示
//			{
//				Command.FlightMode = HEIGHT;
//				ALL_flag.height_lock = 1;
//				Flow_mode_two();       // 遥控-光流控制姿
//				set_flag=0x21;         // OLED定高定点模式显示
//			}
//			else                     //姿态模式
//			{  
//				Command.FlightMode = NORMOL;	
//				ALL_flag.height_lock = 0;
//				set_flag=0x00;        //OLED姿态模式显示
				
				g_PID_AngleX.Target = (g_RemoteData.ROL - 1500) * REMOTE_CONTROL_SCALEFACTOR;  //摇杆控制
				g_PID_AngleY.Target = (g_RemoteData.PIT - 1500) * REMOTE_CONTROL_SCALEFACTOR;  //摇杆控制
//			}
//		}					
}

void Flight_ANOPollingData(void)
{
	static anosendflag_t ANOSendFlag = {0};
	ANOSendFlag.Frame0x01++;
	ANOSendFlag.Frame0x02++;
	ANOSendFlag.Frame0x03++;
	ANOSendFlag.Frame0x04++;
	ANOSendFlag.Frame0x05++;
	ANOSendFlag.Frame0x0D++;

//	if (ANOSendFlag.Frame0x03 >= 50)
//	{
//		ANOSendFlag.Frame0x03 = 0;
		ANO_SendFrame0x03(g_ATT_Angle.ROL, -g_ATT_Angle.PIT, -g_ATT_Angle.YAW, 1);
//	}
//	if (ANOSendFlag.Frame0x01 >= 5)
//	{
//		ANOSendFlag.Frame0x01 = 0;
		ANO_SendFrame0x01(g_ICM20948_Filter.AccX, g_ICM20948_Filter.AccY, g_ICM20948_Filter.AccZ, g_ICM20948_Filter.GyrX, g_ICM20948_Filter.GyrY, g_ICM20948_Filter.GyrZ, 1);
//	}
}


uint16_t Distance_data[2] = {0,0};
extern VL53L1_Dev_t VL53L1_dev[];	//2, device param, include I2C
extern uint8_t Ajusted[];//2, adjusted sign, 0-not, 1-had
extern VL53L1_RangingMeasurementData_t VL53L1_data[];//2, ranging result struct, distance, max distance,etc.
extern uint16_t Distance_data[];//2, the catched distance. VL53L1_data->RangeMilliMeter; 

void flight(void)
{
	uint8_t Status = 0;
//	uint8_t data = 0;
	//VL53L1_dev[0].I2cDevAddr=0x52;
	//VL53L1_dev[1].I2cDevAddr=0x54;
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//分组2
	
	Status = VL53L1X_Init(&VL53L1_dev[0]);
	if(0 != Status)
	{
		printf("VL53L1 Init error!\r\n");
	}
	else
	{
		printf("VL53L1 Init OK\r\n");
	}
	
//	if(VL53L1_set_mode(&VL53L1_dev[0],mode))
//	{
//		printf("mode set error\r\n");
//	}
//	else
//	{
//		printf("set mode OK\r\n");
//	}
	while(1)
	{
		/*VL53L1_RdByte(&VL53L1_dev[0],0x010f,&data);
		printf("data = 0x%x\r\n",data);
		VL53L1_RdByte(&VL53L1_dev[0],0x0110,&data);
		printf("data = 0x%x\r\n",data);*/
		Status = VL53L1_single_test(&VL53L1_dev[0],&VL53L1_data[0]);
		if(Status==VL53L1_Error_NONE)
		{
			//Distance_data[0]=VL53L1_data[0].RangeMilliMeter;
			printf("Distance = %4d\r\n",Distance);
		}
		else
		{
			printf("measure error\r\n");
		}
		Delay_ms(100);
	}
}
///******************************************************************************************
//*函  数：int16_t Yaw_Control(float TARGET_YAW)
//*功  能：航向角不回中控制
//*参  数：TARGET_YAW 目标航向角
//*返回值：针对目标航向角计算出的航向角
//*备  注：由于遥控器航向舵会自动回中，所以需要对目标航向角进行不回中处理;
//*        由于APP遥控航向舵和油门在一起容易误操作，所以在手机操作时不允许改变航向角;
//*******************************************************************************************/
// int16_t Yaw_Control(float TARGET_YAW)
//{
//	static int16_t YAW = 0; //根据目标航向角计算出的不回中角度
//	if(Airplane_Enable)
//	{
//		if(!WiFi_Controlflag) //遥控器控制航向角
//		{
//			if(TARGET_YAW > 2) //目标航向角为正时YAW增大
//				YAW += 2;
//			if(TARGET_YAW < -2) //目标航向角为负时YAW减小
//				YAW -= 2;
//		}
//		else //APP遥控时，不允许改变航向角
//		{
//			YAW = YAW;
//		}
//	}
//	return YAW;
//}

///******************************************************************************************
//*函  数：void Yaw_Carefree(FLOAT_ANGLE *Target_Angle, const FLOAT_ANGLE *Measure_Angle)
//*功  能：无头角度控制
//*参  数：*Target_Angle 指向目标姿态角的指针
//*        *Measure_Angle 测量姿态角的指针
//*返回值：针对目标航向角计算出的航向角
//*备  注：无头模式需调用此函数
//*******************************************************************************************/
// void Yaw_Carefree(FLOAT_ANGLE* Target_Angle, const FLOAT_ANGLE* Measure_Angle)
//{
//	float yawRad = fabs(Measure_Angle->yaw) * DegtoRad;
//	float cosy = cosf(yawRad);
//	float siny = sinf(yawRad);
//	float originalRoll = Target_Angle->rol;
//	float originalPitch = Target_Angle->pit;

//	Target_Angle->rol = originalRoll * cosy + originalPitch * siny;
//	Target_Angle->pit = originalPitch * cosy - originalRoll * siny;
//}

///******************************************************************************************
//*函  数：void Safety_Check(void)
//*功  能：飞机姿态安全监测
//*参  数：无
//*返回值：无
//*备  注：如果飞机角度和加速度异常就将飞机上锁并停止电机，防止电机狂转打坏桨叶
//*******************************************************************************************/
// void Safety_Check(void)
//{
//  if((fabs(g_ATT_Angle.pit) > 45.0f || fabs(g_ATT_Angle.rol) > 45.0f)
//	  && (fabs(Acc_filt.X) > 9.0f || fabs(Acc_filt.Y) > 9.0f))
//	{
//		 Airplane_Enable = 0;
//		 MOTOR_PWM_1 = 0;
//		 MOTOR_PWM_2 = 0;
//		 MOTOR_PWM_3 = 0;
//		 MOTOR_PWM_4 = 0;
//	}
//}

/******************************************************************************************
*函  数：void Control(FLOAT_ANGLE *att_in,FLOAT_XYZ *gyr_in, RC_TYPE *rc_in, uint8_t armed)
*功  能：姿态控制,角度环控制和角速度环控制
*参  数：att_in 测量值
*        gry_in:MPU6050读取的角速度值
*        rc_in:遥控器设定值
*        armed记录命令
*返回值：无
*备  注：小四轴机头与电机示意图
				 机头
			 M3    ↑    M2
			   \   |   /
				\  |  /
				 \ | /
		 Y<————-———+————————
				 / | \
				/  |  \
			   /   |   \
			 M4    |    M1
				   X
	 
		 1. M1、M3电机逆时针旋转，M2、M4电机顺时针旋转
		 2. X：是陀螺仪的 X 轴；Y：是陀螺仪的 Y 轴
		 3. 绕 X 轴旋转为 ROL角；绕 Y 轴旋转为 PIT角；绕 Z 轴旋转为 YAW角
*******************************************************************************************/
