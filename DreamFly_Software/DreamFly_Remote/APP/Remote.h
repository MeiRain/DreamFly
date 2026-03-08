#ifndef __REMOTE_H
#define __REMOTE_H

#include "stm32f10x.h"

typedef struct
{
	uint16_t TX_Address;
	uint16_t RX_Address;
	__IO uint16_t ROL_Bias;
	__IO uint16_t PIT_Bias;
	__IO uint16_t YAW_Bias;
	__IO uint16_t THR_Bias;
} flash_t;

typedef struct
{
	int16_t ROL;
	int16_t PIT;	
	int16_t YAW;
	int16_t THR;
} joystick_t;

typedef struct
{
	uint8_t JoyStickOffSet;
	uint8_t BuzzerBeep
} remoteflag_t;

typedef struct
{
	int16_t ROL;
	int16_t PIT;	
	int16_t YAW;
	int16_t THR;
	uint8_t KEY;
} remotedata_t;

typedef struct
{
	uint8_t MOD;
	int16_t ROL;
	int16_t PIT;
	int16_t YAW;
	int16_t THR;
	int16_t ALT;
	int16_t BAT;
} flight_t;

void Remote_HardwareCheck(void);
void Remote_ParameterInit(void);
void Remote_AllInit(void);
void Remote_ReadFlashData(void);
void Remote_WriteFlashData(void);
void Remote_JoyStickScan(void);
void Remote_JoyStickOffSet(void);
void Remote_KeyScanAndHandle(void);
void Remote_BuzzerControl(void);
void Remote_ReceiveFlightData(void);
void Remote_SendDataToFlight(void);

#endif
