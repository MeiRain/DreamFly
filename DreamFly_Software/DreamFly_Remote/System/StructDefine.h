#ifndef __STRUCTDEFINE_H
#define __STRUCTDEFINE_H	

typedef struct 
{
	uint16_t Thr;
	int16_t Yaw;
	int16_t Rol;
	int16_t Pit;
	int16_t Alt;
	int16_t Bat;
}Flight_Data;

extern Flight_Data	Flight;

#endif
