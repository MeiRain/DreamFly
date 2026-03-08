#ifndef __FLIGHT_H
#define __FLIGHT_H

#include "stm32f4xx.h"

typedef struct 
{
	float ADCV;
	float RealV;
	float MeasureV;
	float AlarmV1;
	float AlarmV2;
} battery_value_t;

////保存的参数数据结构 
//typedef struct
//{
//	//陀螺仪校准数据 
//	uint16_t  ACC_OFFSET_X;
//	uint16_t  ACC_OFFSET_Y;
//	uint16_t  ACC_OFFSET_Z;
//	uint16_t  GYRO_OFFSET_X;
//	uint16_t  GYRO_OFFSET_Y;
//	uint16_t  GYRO_OFFSET_Z;
//	//角度环 
//	uint16_t  ROL_Angle_P;
//	uint16_t  ROL_Angle_I;
//	uint16_t  ROL_Angle_D;
//	uint16_t  PIT_Angle_P;
//	uint16_t  PIT_Angle_I;
//	uint16_t  PIT_Angle_D;
//	uint16_t  YAW_Angle_P;
//	uint16_t  YAW_Angle_I;
//	uint16_t  YAW_Angle_D;
//	//角速度环 
//	uint16_t  ROL_Rate_P;
//	uint16_t  ROL_Rate_I;
//	uint16_t  ROL_Rate_D;
//	uint16_t  PIT_Rate_P;
//	uint16_t  PIT_Rate_I;
//	uint16_t  PIT_Rate_D;
//	uint16_t  YAW_Rate_P;
//	uint16_t  YAW_Rate_I;
//	uint16_t  YAW_Rate_D;
//	//高度环
//	uint16_t  ALT_Rate_P;
//	uint16_t  ALT_Rate_I;
//	uint16_t  ALT_Rate_D;
//	uint16_t  ALT_P;
//	uint16_t  ALT_I;
//	uint16_t  ALT_D;
//	uint16_t  NRFaddr;
//}parametersave_t;

typedef struct
{
	float Filter_POS_X;		//x轴积分滤波值
	float Filter_POS_Y;		//y轴积分滤波值
	float Fusion_POS_X;			//x轴积分输出值
	float Fusion_POS_Y;			//y轴积分输出值
	float Angle_X;
	float Angle_Y;
	float gyr_x;			
	float gyr_y;		

	float out_x_i_o;			
	float out_y_i_o;			
	
	float Raw_Speed_X;			//x轴速度原始值
	float Raw_Speed_Y;			//y轴速度原始值
	float Filter_Speed_X;		//x轴速度融合值
	float Filter_Speed_Y;		//y轴速度融合值
	
	float loc_x;
	float loc_y;
	float loc_xs;
	float loc_ys;
	
	float fix_High;
} opticalflow_t;

typedef struct
{
	uint8_t Ready;
	uint8_t Unlock;//上锁1，不上锁0
	uint8_t Connect;//已连接1，未连接0
	uint8_t LowVoltage;
} flightflag_t;

typedef struct
{
	int16_t ROL;
	int16_t PIT;	
	int16_t YAW;
	int16_t THR;
	uint8_t KEY;
} remotedata_t;

void Flight_HardwareCheck(void);
void Flight_ParameterInit(void);
void Flight_AllInit(void);
void Flight_ReadFlashData(void);
void Flight_WriteFlashData(void);
void Flight_LEDControl(void);
void Flight_WS2812BControl(void);
void Flight_CheckBatteryVoltage(void);
void Flight_OffSetICM20948Data(void);
void Flight_GetIMUData();
void Flight_GetOpticalFlowData(void);
void Flight_FusionOpticalFlowData(float dT);
void Flight_AttitudeAlgorithm(void);
void Flight_ReceiveRemoteData(void);
void Flight_SendFlightDataToRemote(void);
void Flight_ExecuteRemoteCommand(void);
void Flight_MotorPIDCascadeControl(void);
void Flight_ControlModeSelect(void);
void Flight_ANOPollingData(void);
void Flight_ParameterSave(void);
void flight(void);

#endif
