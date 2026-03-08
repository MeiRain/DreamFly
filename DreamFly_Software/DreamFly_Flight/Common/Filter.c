#include "Filter.h"

void Filter_ExtenedKalman_1D(_1_ekf_filter* efk, float input)
{
	efk->NowP = efk->LastP + efk->Q;
	efk->Kg = efk->NowP / (efk->NowP + efk->R);
	efk->Out = efk->Out + efk->Kg * (input - efk->Out);
	efk->LastP = (1 - efk->Kg) * efk->NowP;
}

void Filter_LowPass_1st(_1_lpf_filter* lpf, float input)
{  
	lpf->Out = lpf->Factor * input + (1.0f - lpf->Factor) * lpf->LastData;
	lpf->LastData = lpf->Out;
}

#define N 20      //滤波缓存数组大小
#define M_PI_F 3.1416f
/*******************************************************************************
*函  数 ：float FindPos(float* a, int low, int high)
*功  能 ：确定一个元素在顺序排列中位序
*参  数 ：a  	数组首地址
*         low	数组最小下标
*         high	数组最大下标
*返回值 ：返回元素的位序 low
*备  注 : 无
*******************************************************************************/
float FindPos(float* a, int low, int high)
{
    float val = a[low];                      	//选定一个要确定值val确定位置，将第一个元素的值赋给临时值变量
    while(low < high)							//循环结束条件是下标重合
    {
		while(low < high && a[high] >= val)		//如果 a[high] 的数大于 val，下标 high 往前移
			high--;                       		
		a[low] = a[high];             			//当右边的值小于 val 则赋值给 a[low]
		while(low < high && a[low] <= val)		//如果左边的数小于 val，下标 low 往后移
            low++;                        		
        a[high] = a[low];             			//当左边的值大于 val 则赋值给 a[high]
    }
    a[low] = val;								//此时low = high
    return low;									
}

/*******************************************************************************
*函  数 ：void QuiteSort(float* a, int low, int high)
*功  能 ：快速排序
*参  数 ：a  	数组首地址
*         low	数组最小下标
*         high	数组最大下标
*返回值 ：无
*备  注 : 无
*******************************************************************************/
void QuiteSort(float* a, int low, int high)
{
	int pos;
    if(low < high)						//微操作：low = high 时不排序（数组只有一个元素）
    {
        pos = FindPos(a, low, high); 	//微操作：使 a[pos] 左边比它小，a[pos] 的右边比它大
        QuiteSort(a, low, pos - 1); 	//超级操作：递归调用（在函数中调用自己），对子数组左边划分
		QuiteSort(a, pos + 1, high);	//超级操作：递归调用，对子数组右边划分
    }
}

 /*******************************************************************************
*函  数 ：void SortAver_Filter(float value, float* filter, uint8_t n)
*功  能 ：去最值平均值滤波一组数据
*参  数 ：value  	采样的数据
*         filter	滤波后的平均值
*         n			采样数
*返回值 ：返回滤波后的数据
*备  注 : 无
*******************************************************************************/
void SortAver_Filter(float value, float* filter, uint8_t n)
{
	static float buf[N] = {0.0};
	static uint8_t cnt = 0, flag = 1;
	float temp = 0;
	uint8_t i = 0;
	buf[cnt ++] = value;
	if(cnt < n && flag) 			//数组填不满不计算	
		return;  					
	else 
		flag = 0; 
	QuiteSort(buf, 0, n - 1);
	for(i = 1; i < n - 1; i ++)		//将最小值和最大值略去
	{
		temp += buf[i];				//求数组和
	}
	if(cnt >= n) 					//清零cnt
		cnt = 0;
	*filter = temp / (n - 2);		//求平均值
}

 /*******************************************************************************
*函  数 ：float  SortAver_Filter1(float value)
*功  能 ：去最值平均值滤波一组数据
*参  数 ：value  	采样的数据
*         filter	滤波后的平均值
*         n			采样数
*返回值 ：返回滤波后的数据
*备  注 : 无
*******************************************************************************/
void SortAver_Filter1(float value, float* filter, uint8_t n)
{
	static float buf[N];
	static uint8_t cnt = 0 , flag = 1;
	float temp = 0;
	uint8_t i;
	buf[cnt ++] = value;
	if(cnt < n && flag) 
		return;   //数组填不满不计算
	else
		flag = 0;
	QuiteSort(buf, 0, n - 1);
	for(i = 1; i < n - 1; i ++)
	{
		temp += buf[i];
	}
	if(cnt >= n) 
		cnt = 0;
	*filter = temp / (n - 2);
}

// /*******************************************************************************
// *函  数 ：void  SortAver_FilterXYZ(INT16_XYZ *acc,FLOAT_XYZ *Acc_filt,uint8_t n)
// *功  能 ：去最值平均值滤波三组数据
// *参  数 ：*acc 要滤波数据地址
// *         *Acc_filt 滤波后数据地址
// *返回值 ：返回滤波后的数据
// *备  注 : 无
// *******************************************************************************/
// void  SortAver_FilterXYZ(INT16_XYZ *acc,FLOAT_XYZ *Acc_filt,uint8_t n)
// {
// 	static float bufx[N], bufy[N], bufz[N];
// 	static uint8_t cnt = 0, flag = 1;
// 	float temp1 = 0, temp2 = 0, temp3 = 0;
// 	uint8_t i;
// 	bufx[cnt] = acc->X;		//取出X轴加速度数据
// 	bufy[cnt] = acc->Y;		//取出Y轴加速度数据
// 	bufz[cnt] = acc->Z;		//取出Z轴加速度数据
// 	cnt ++;      			//这个的位置必须在赋值语句后，否则bufx[0]不会被赋值
// 	if(cnt < n && flag) 	//数组填不满不计算
// 		return;   			
// 	else
// 		flag = 0;
	
// 	QuiteSort(bufx, 0, n - 1);
// 	QuiteSort(bufy, 0, n - 1);
// 	QuiteSort(bufz, 0, n - 1);
// 	for(i = 1; i < n - 1; i ++)
// 	{
// 		temp1 += bufx[i];
// 		temp2 += bufy[i];
// 		temp3 += bufz[i];
// 	}
// 	if(cnt >= n) 
// 		 cnt = 0;
// 	Acc_filt->X  = temp1 / (n-2);
// 	Acc_filt->Y  = temp2 / (n-2);
// 	Acc_filt->Z  = temp3 / (n-2);
// }

// /*******************************************************************************
// *函  数 ：void Aver_FilterXYZ6(INT16_XYZ *acc,INT16_XYZ *gry,FLOAT_XYZ *Acc_filt,
//                               FLOAT_XYZ *Gry_filt,uint8_t n)
// *功  能 ：滑动窗口滤波六组数据
// *参  数 ：*acc ,*gry 要滤波数据地址
// *         *Acc_filt,*Gry_filt 滤波后数据地址
// *返回值 ：返回滤波后的数据
// *备  注 : 无
// *******************************************************************************/
// void Aver_FilterXYZ6(INT16_XYZ *acc,INT16_XYZ *gry,FLOAT_XYZ *Acc_filt,FLOAT_XYZ *Gry_filt,uint8_t n)
// {
// 	static float bufax[N], bufay[N], bufaz[N], bufgx[N], bufgy[N], bufgz[N];
// 	static uint8_t cnt = 0, flag = 1;
// 	float temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0, temp5 = 0, temp6 = 0;
// 	uint8_t i;
// 	bufax[cnt] = acc->X;
// 	bufay[cnt] = acc->Y;
// 	bufaz[cnt] = acc->Z;
// 	bufgx[cnt] = gry->X;
// 	bufgy[cnt] = gry->Y;
// 	bufgz[cnt] = gry->Z;
// 	cnt ++;      
// 	if(cnt < n && flag) 
// 		return;  
// 	else
// 		flag = 0;
// 	for(i = 0; i < n; i ++)
// 	{
// 		temp1 += bufax[i];
// 		temp2 += bufay[i];
// 		temp3 += bufaz[i];
		
// 		temp4 += bufgx[i];
// 		temp5 += bufgy[i];
// 		temp6 += bufgz[i];
// 	}
// 	if(cnt>=n) 
// 		cnt = 0;
// 	Acc_filt->X  = temp1 / n;
// 	Acc_filt->Y  = temp2 / n;
// 	Acc_filt->Z  = temp3 / n;             
// 	Gry_filt->X  = temp4 / n;
// 	Gry_filt->Y  = temp5 / n;
// 	Gry_filt->Z  = temp6 / n;                  
// }

// /*******************************************************************************
// *函  数 ：void Aver_FilterXYZ(INT16_XYZ *acc,FLOAT_XYZ *Acc_filt,uint8_t n)
// *功  能 ：滑动窗口滤波三组数据
// *参  数 ：*acc  要滤波数据地址
// *         *Acc_filt 滤波后数据地址
// *返回值 ：返回滤波后的数据
// *备  注 : 无
// *******************************************************************************/
// void Aver_FilterXYZ(INT16_XYZ *acc, FLOAT_XYZ *Acc_filt, uint8_t n)
// {
// 	static int32_t bufax[N], bufay[N], bufaz[N];
// 	static uint8_t cnt = 0, flag = 1;
// 	int32_t temp1 = 0, temp2 = 0, temp3 = 0, i;
// 	bufax[cnt] = acc->X;
// 	bufay[cnt] = acc->Y;
// 	bufaz[cnt] = acc->Z;
// 	cnt ++;      //这个的位置必须在赋值语句后，否则bufax[0]不会被赋值
// 	if(cnt < n && flag) 
// 		return;   //数组填不满不计算
// 	else
// 		flag = 0;
// 	for(i = 0; i < n; i ++)
// 	{
// 		temp1 += bufax[i];
// 		temp2 += bufay[i];
// 		temp3 += bufaz[i];
// 	}
// 	 if(cnt >= n)  
// 		 cnt = 0;
// 	 Acc_filt->X  = temp1 / n;
// 	 Acc_filt->Y  = temp2 / n;
// 	 Acc_filt->Z  = temp3 / n;
// }                          

 /*******************************************************************************
 *函  数 ：void MovingAver_Filter(float data,float *filt_data,uint8_t n
 *功  能 ：滑动窗口滤波一组数据
 *参  数 ：data  要滤波数据
 *         *filt_data 滤波后数据地址
 *返回值 ：返回滤波后的数据
 *备  注 : 无
 *******************************************************************************/
void Filter_MovingAver(float data, float *filt_data, uint8_t n)
{
	static float buf[N];
	static uint8_t cnt = 0, flag = 1;
	float temp = 0;
	uint8_t i;
	buf[cnt ++] = data;
	if(cnt < n && flag) 
		return;   				//数组填不满不计算，返回
	else
		flag = 0;				//一旦数组填满，flag 为 0
	for(i = 0; i < n; i ++)
	{
		temp += buf[i];
	}
	if(cnt >= n) 
		cnt = 0;
	*filt_data = temp / n;
}
