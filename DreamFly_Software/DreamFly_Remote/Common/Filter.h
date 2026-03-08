#ifndef __FILTER_H
#define __FILTER_H

#include "stm32f10x.h"

// 扩展卡尔曼
typedef struct
{
	float LastP; // 上一时刻状态方差（或协方差）
	float NowP;	 // 当前时刻状态方差（或协方差）
	float Out;	 // 滤波器输出值，即估计的状态
	float Kg;	 // 卡尔曼增益，用于调节预测值和测量值之间的权重
	float Q;	 // 过程噪声的方差，反映系统模型的不确定性
	float R;	 // 测量噪声的方差，反映测量过程的不确定性
} _1_ekf_filter;

// 一阶低通滤波
typedef struct
{
	float Factor;	// 滤波因数
	float Out;		// 滤波器输出值
	float LastData; // 上一时刻输出值
} _1_lpf_filter;

extern _1_ekf_filter EKF_Filter[3];
extern _1_lpf_filter LPF_Filter[3];

void Filter_ExtenedKalman_1D(_1_ekf_filter* efk, float input);
void Filter_LowPass_1st(_1_lpf_filter* lpf, float input);
void Filter_MovingAver(float data, float *filt_data, uint8_t n);

//float Low_Filter(float value);
//void SortAver_Filter(float value,float *Filter,uint8_t N);
//void  SortAver_Filter1(float value,float *Filter,uint8_t n);
//void  SortAver_FilterXYZ(INT16_XYZ *acc,FLOAT_XYZ *Acc_filt,uint8_t N);
//void Aver_FilterXYZ6(INT16_XYZ *acc,INT16_XYZ *gry,FLOAT_XYZ *Acc_filt,FLOAT_XYZ *Gry_filt,uint8_t N);
//void Aver_FilterXYZ(INT16_XYZ *acc,FLOAT_XYZ *Acc_filt,uint8_t N);
//void MovingAver_Filter(float data,float *filt_data,uint8_t n);
//void Aver_Filter1(float data,float *filt_data,uint8_t n);
//void presssureFilter(float* in, float* out);

void LPF2pSetCutoffFreq_1(float sample_freq, float cutoff_freq);
float LPF2pApply_1(float sample);

#endif
