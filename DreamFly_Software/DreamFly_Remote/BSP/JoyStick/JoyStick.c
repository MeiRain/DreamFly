//#include <stdio.h>

//#include "ADC.h"

//#include "JoyStick.h"

//uint8_t g_JoyStick_Calibration_Flag;  	//遥控通道ADC校准标志

//joystick_t JoyStick_Define;
//joystick_t JoyStick_Bias;

//void JoyStick_GetADCValue(joystick_t *joystick_adc)
//{
//	joystick_adc->ROL = ADC_GetValue(ADC_ROL);
//	joystick_adc->PIT = ADC_GetValue(ADC_PIT);
//	joystick_adc->YAW = ADC_GetValue(ADC_YAW);
//	joystick_adc->THR = ADC_GetValue(ADC_THR);
//}

///*****************************************************************************
//*函  数：void ADC_Calibration_Test(void)
//*功  能：进行摇杆校准条件检测，左摇杆拉至最低，右摇杆拉至左下角
//*参  数：无
//*返回值：无
//*备  注：无
//*****************************************************************************/
//void JoyStick_Check_CalibrationCondition(joystick_t *joystick_adc)
//{
//	if (joystick_adc->ROL < 5 && joystick_adc->PIT < 5 &&
//		joystick_adc->YAW > 4090 && joystick_adc->THR < 5)
//	{
//		g_JoyStick_Calibration_Flag = 1;
//		printf("RC CALIBLIBRATION READY!\r\n");
//	}
//}

///*****************************************************************************
//*函  数：__IO uint16_t ADC_ValueLimit(__IO uint16_t value)
//*功  能：进行摇杆限幅和中点校准
//*参  数：value：转换后的 ADC 值，即 RC_Value
//*返回值：无
//*备  注：无
//*****************************************************************************/
//int16_t JoyStick_ValueLimit(int16_t value)
//{
////	if(value > 1480 && value < 1520)
////		value = 1500;							//中点校准，在范围内即视为中点

//	value = (value <= 1000) ? 1000 : value;		//摇杆限幅，防止越界
//	value = (value >= 2000) ? 2000 : value;		//摇杆限幅，防止越界
//	
//	return value;
//}

///*****************************************************************************
//*函  数：void ADC_DealData(void)
//*功  能：对 ADC 数据进行处理和校准
//*参  数：无
//*返回值：无
//*备  注：无
//*****************************************************************************/
//void JoyStick_Scan(void)
//{
//	static uint8_t cnt = 0;
//	joystick_t JoyStick_ADC;

//	JoyStick_GetADCValue(&JoyStick_ADC);
//	JoyStick_Check_CalibrationCondition(&JoyStick_ADC);
//	
//	JoyStick_Define.ROL = (1000 + JoyStick_ADC.ROL * 1000 / 4095) - JoyStick_Bias.ROL;
//	JoyStick_Define.ROL = JoyStick_ValueLimit(JoyStick_Define.ROL);
//	JoyStick_Define.PIT = (1000 + JoyStick_ADC.PIT * 1000 / 4095) - JoyStick_Bias.PIT;
//	JoyStick_Define.PIT = JoyStick_ValueLimit(JoyStick_Define.PIT);
//	JoyStick_Define.YAW = (1000 + JoyStick_ADC.YAW * 1000 / 4095) - JoyStick_Bias.YAW;
//	JoyStick_Define.YAW = JoyStick_ValueLimit(JoyStick_Define.YAW);
//	JoyStick_Define.THR = (1000 + JoyStick_ADC.THR * 1000 / 4095) - JoyStick_Bias.THR;
//	JoyStick_Define.THR = JoyStick_ValueLimit(JoyStick_Define.THR);
//	
//	if (g_JoyStick_Calibration_Flag == 1)
//	{
//		JoyStick_Calibration();
//	}
//	
////	JoyStick_Value[JOY_ROL] = ADC_ValueLimit(RC_Value[2]);
////	JoyStick_Value[JOY_PIT] = ADC_ValueLimit(RC_Value[3]);
////	JoyStick_Value[JOY_YAW] = ADC_ValueLimit(RC_Value[0]);
////	printf("ADC_Value: %d   %d   %d   %d\r\n",
////	ADC_Value[0], ADC_Value[1], ADC_Value[2], ADC_Value[3]);
//	
////	JoyStick_Check_CalibrationCondition(JoyStick_ADCValue);
//	cnt++;
//	
//	
//	if(cnt == 20)
//	{
//		printf("RC_Value: %d   %d   %d   %d\r\n",
//		JoyStick_Define.ROL, JoyStick_Define.PIT, JoyStick_Define.YAW, JoyStick_Define.THR);
//		printf("======================================\r\n");
//		cnt = 0;
//	}
//	
////	printf("ADC_Calibrator1:%d\r\n",ADC_OffsetValue[0]);
////	printf("ADC_Calibrator2:%d\r\n",ADC_OffsetValue[1]);
////	printf("ADC_Calibrator3:%d\r\n",ADC_OffsetValue[2]);
////	printf("ADC_Calibrator4:%d\r\n",ADC_OffsetValue[3]);
//	

//}

///*****************************************************************************
//*函  数：void ADC_Calibration(void)
//*功  能：进行摇杆校准
//*参  数：无
//*返回值：无
//*备  注：求出的校准值为 60 次和的平均值
//*****************************************************************************/
//void JoyStick_Calibration(void)
//{
//	static uint8_t cnt = 0;
//	static __IO int32_t temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0;
//	if (JoyStick_Define.ROL > 1485 && JoyStick_Define.ROL < 1515 &&			//左摇杆中轴检测，右摇杆中点检测	 
//		JoyStick_Define.PIT > 1485 && JoyStick_Define.PIT < 1515 &&
//		JoyStick_Define.YAW > 1485 && JoyStick_Define.YAW < 1515 &&
//		JoyStick_Define.THR < 1010)
//	{
//		if (cnt == 0)
//		{
//			temp1 = 0;
//			temp2 = 0;
//			temp3 = 0;
//			temp4 = 0;
////			JoyStick_Bias.ROL = 0;
////			JoyStick_Bias.PIT = 0;
////			JoyStick_Bias.YAW = 0;
////			JoyStick_Bias.THR = 0;
//		}

//		cnt ++;
//		temp1 += JoyStick_Define.ROL - 1500;		//将 yaw 的 AD 值转为 1000 — 2000，并求和
//		temp2 += JoyStick_Define.PIT - 1500;		//将 Thr 的 AD 值转为 0 — 1000，并求和
//		temp3 += JoyStick_Define.YAW - 1500;		//将 roll 的 AD 值转为 1000 — 2000，并求和
//		temp4 += JoyStick_Define.THR - 1000;		//将 pitch 的 AD 值转为 1000 — 2000，并求和

//		
//		if(cnt >= 60)
//		{
//			JoyStick_Bias.ROL = temp1 / cnt;				//求出 60 次和的平均数
//			JoyStick_Bias.PIT = temp2 / cnt;				//油门校准值不用	
//			JoyStick_Bias.YAW = temp3 / cnt;					
//			JoyStick_Bias.THR = temp4 / cnt;					
//			g_JoyStick_Calibration_Flag = 0;
//			cnt = 0;
////			PID_WriteFlash();
//		}
//	}
//}
