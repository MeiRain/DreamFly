#include <stdlib.h>
#include "Delay.h"
#include "WS2812B.h"

/************************代码移植修改区************************************/

#define WS2812B_CLK RCC_AHB1Periph_GPIOB // 端口时钟
#define WS2812B_PORT GPIOB				 // 端口
#define WS2812B_PIN GPIO_Pin_10			 // 引脚

/**************************************************************************/

#define WS2812B_AMOUNT 4
#define WS2812B_SET1 WS2812B_PORT->BSRRL |= WS2812B_PIN
#define WS2812B_SET0 WS2812B_PORT->BSRRH |= WS2812B_PIN

// 跑马灯WS2812B三元色配出七彩跑马灯
static uint32_t Run_buf[][16] =
{
	{0xFFA500,0,0,0,0xFFA500,0,0,0,0xFFA500,0,0,0,0xFFA500,0,0,0,},//橙色
	{0x00FF00,0,0,0,0x00FF00,0,0,0,0x00FF00,0,0,0,0x00FF00,0,0,0,},//绿色
	{0xFF00FF,0,0,0,0xFF00FF,0,0,0,0xFF00FF,0,0,0,0xFF00FF,0,0,0,},//紫色
	{0x00FFFF,0,0,0,0x00FFFF,0,0,0,0x00FFFF,0,0,0,0x00FFFF,0,0,0,},//青色
	{0x0000FF,0,0,0,0x0000FF,0,0,0,0x0000FF,0,0,0,0x0000FF,0,0,0,},//蓝色
	{0xFFFF00,0,0,0,0xFFFF00,0,0,0,0xFFFF00,0,0,0,0xFFFF00,0,0,0,},//黄色
	{0xFFFFFF,0,0,0,0xFFFFFF,0,0,0,0xFFFFFF,0,0,0,0xFFFFFF,0,0,0,},//白色
};

uint8_t Run_flag = 1; // 跑马灯标志

/*********************************************************************
 *函  数：void LED_Init(void)
 *功  能：初始化LED控制引脚
 *参  数：无
 *返回值: 无
 *备  注: 无
 *********************************************************************/
void WS2812B_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(WS2812B_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = WS2812B_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // 选择模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // 输出类型
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(WS2812B_PORT, &GPIO_InitStructure);
	
	GPIO_ResetBits(WS2812B_PORT, WS2812B_PIN);
}

/***************************************************************************
 *函  数：void WS2812B_Reset(void)
 *功  能：WS2812B灯复位
 *参  数：无
 *返回值：无
 *备  注: 无
 ***************************************************************************/
void WS2812B_Reset(void)
{
	Delay_us(80);
}

/***************************************************************************
 *函  数：void WS2812B_Write0(void)
 *功  能：写0
 *参  数：无
 *返回值：无
 *备  注: 不同的系统时钟频率下需要微调（默认HCLK=100MHz）
 ***************************************************************************/
static void WS2812B_Write0(void)
{
	uint8_t cnt1 = 2, cnt2 = 7;
	WS2812B_SET1;
	while (cnt1--) // 高电平300ns
		__nop();

	WS2812B_SET0;
	while (cnt2--) // 低电平900ns
		__nop();
}

/***************************************************************************
 *函  数：void WS2812B_Write1(void)
 *功  能：写1
 *参  数：无
 *返回值：无
 *备  注: 不同的系统时钟频率下需要微调（默认HCLK=100MHz）
 ***************************************************************************/
static void WS2812B_Write1(void)
{
	uint8_t cnt1 = 7, cnt2 = 2;
	WS2812B_SET1;
	while (cnt1--) // 高电平900ns
		__nop();

	WS2812B_SET0;
	while (cnt2--) // 低电平300ns
		__nop();
}

/***************************************************************************
 *函  数：void WS2812B_WriteByte(uint8_t data)
 *功  能：写一个字节数据
 *参  数：data
 *返回值: 无
 *备  注: 无
 ***************************************************************************/
static void WS2812B_WriteByte(uint8_t byte)
{
	uint8_t i = 0;
	for (i = 0; i < 8; i++)
	{
		if ((byte << i) & 0x80) // 从高位取到低位
			WS2812B_Write1();
		else
			WS2812B_Write0();
	}
}

/***************************************************************************
 *函  数：void WS2812B_SetColour(uint8_t green,uint8_t red,uint8_t blue)
 *功  能：设置一个灯的色彩
 *参  数：green red blue，红绿蓝光所占比例大小,范围0~255
 *返回值: 无
 *备  注: 发送顺序为GRB
 ***************************************************************************/
void WS2812B_SetColour(uint8_t green, uint8_t red, uint8_t blue)
{
	WS2812B_WriteByte(green);
	WS2812B_WriteByte(red);
	WS2812B_WriteByte(blue);
}

/**************************************************************************
 *函  数：void RGB_Rand(void)
 *功  能：随机变换颜色
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_Rand(void)
{
	uint8_t i, red = 0, green = 0, blue = 0;
	for (i = 0; i < WS2812B_AMOUNT; i++)
	{
		green = rand() % 18 + 2; // 产生一个0~20的随机数
		red = rand() % 18 + 2;
		blue = rand() % 18 + 2;
		WS2812B_SetColour(green, red, blue); // 合成颜色
	}
	WS2812B_Reset(); // 复位显示
}

/**************************************************************************
 *函  数：void WS2812B_Runing(void)
 *功  能：跑马灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_Run(void)
{
	static uint8_t m = 0, n = 0, times = 0;
	static uint8_t red = 0, green = 0, blue = 0;

	// 循环点亮同一颜色16个灯，清零times并转换下一颜色
	// 一次循环4个周期，一个周期4个灯，总共分16次点灯
	if (times++ >= (WS2812B_AMOUNT * 4))
	{
		times = 0;
		m++; // 显示完4个周期后，移动行，换颜色
	}
	WS2812B_Reset();
	//0123,4012,3401,2340,一次循环发送 4*24bit 给4个灯
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++) // 整个4个循环发送3个24bit，点亮一个新灯
	{
		if (n > WS2812B_AMOUNT)
			n = 0;							   // 清零4次循环后的n，即n永远小于等于5
		red = ((Run_buf[m][n] >> 16) & 0xFF);  // 右移16位，取高8位
		green = ((Run_buf[m][n] >> 8) & 0xFF); // 右移8位，取中8位
		blue = ((Run_buf[m][n] >> 0) & 0xFF);  // 右移0位，取低8位
		WS2812B_SetColour(green, red, blue);   // 合成颜色
		n++;								   // 显示完1个周期后，移动列
	}
	if (m == 7) // 清零7次循环后的m
		m = 0;
}

void WS2812B_FlyIndicator(void)
{
	static uint8_t FlashFlag = 0;

		if(FlashFlag)
		{
			FlashFlag = 0;
			WS2812B_AllGreen();
		}
		else
		{			
			FlashFlag = 1;
			WS2812B_SetColour(0, 0xFF, 0);
			WS2812B_SetColour(0, 0xFF, 0);
			WS2812B_SetColour(0, 0, 0);
			WS2812B_SetColour(0, 0, 0);
		}

}

/**************************************************************************
*函  数：void BATT_Alarm_LED(void)
*功  能：低电量红灯快闪
*参  数：无
*返回值：无
*备  注: 无
**************************************************************************/
void WS2812B_RedFlash(void)
{
	static uint8_t FlashFlag = 0;
	
	if(FlashFlag)
	{
		FlashFlag = 0;
		WS2812B_AllRed();
	}
	
	else
	{
		FlashFlag = 1;
		WS2812B_AllOFF();
	}
}

/**************************************************************************
 *函  数：void GYRO_Ready(void)
 *功  能：蓝灯闪烁
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_BlueFlash(void)
{
	WS2812B_AllOFF();
	WS2812B_AllBlue();
	Delay_ms(100);
	WS2812B_AllOFF();
	Delay_ms(100);
	WS2812B_AllBlue();
	Delay_ms(100);
	WS2812B_AllOFF();
	Delay_ms(100);
	WS2812B_AllBlue();
	Delay_ms(100);
	WS2812B_AllOFF();
}

///**************************************************************************
// *函  数：void ACC_Ready(void)
// *功  能：加速度校准完成绿灯闪烁
// *参  数：无
// *返回值：无
// *备  注: 无
// **************************************************************************/
// void WS2812B_ACC_Ready(void)
//{
//	WS2812B_OFF();
//	WS2812B_Green();
//	Delay_ms(100);
//	WS2812B_OFF();
//	Delay_ms(100);
//	WS2812B_Green();
//	Delay_ms(100);
//	WS2812B_OFF();
//	Delay_ms(100);
//	WS2812B_Green();
//	Delay_ms(100);
//	WS2812B_OFF();
//}

///**************************************************************************
// *函  数：void BAR_Ready(void)
// *功  能：气压计校准完成紫灯闪烁
// *参  数：无
// *返回值：无
// *备  注: 无
// **************************************************************************/
// void WS2812B_BAR_Ready(void)
//{
//	WS2812B_OFF();
//	WS2812B_Violet();
//	Delay_ms(100);
//	WS2812B_OFF();
//	Delay_ms(100);
//	WS2812B_Violet();
//	Delay_ms(100);
//	WS2812B_OFF();
//	Delay_ms(100);
//	WS2812B_Violet();
//	Delay_ms(100);
//	WS2812B_OFF();
//}

/**************************************************************************
 *函  数：void WS2812B_RedTroggle(void)
 *功  能：红灯快闪
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_RedTroggle(void)
{
	//	static uint8_t flag = 0;

	//	if(BATT_LowVoltageFlag)
	//	{
	//		if(flag)
	//		{
	//			flag = 0;
	//			WS2812B_Red();
	//		}
	//		else
	//		{
	//			flag = 1;
	//			WS2812B_OFF();
	//		}
	//	}
}

/**************************************************************************
*函  数：void WS2812B_Unlock(uint8_t N, uint8_t flag)
*功  能：飞机解锁指示彩色灯
*参  数：N:彩灯变换频率
		 flag:使能变换标志
*返回值：无
*备  注: 无
**************************************************************************/
void WS2812B_Unlock(uint8_t N, uint8_t flag)
{
	static uint8_t cnt = 0;
	if (flag && cnt++ > N)
	{
		cnt = 0;
		WS2812B_Rand();
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllRed(void)
 *功  能：红灯
 *参  数：无
 *返回值：无
 *备  注：无
 **************************************************************************/
void WS2812B_AllRed(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0, 0xFF, 0);
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllOrange(void)
 *功  能：橙灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_AllOrange(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0xA5, 0xFF, 0x00);
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllYellow(void)
 *功  能：黄灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_AllYellow(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0xFF, 0xFF, 0);
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllGreen(void)
 *功  能：绿灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_AllGreen(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0xFF, 0, 0);
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllCyan(void)
 *功  能：青灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_AllCyan(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0xFF, 0, 0xFF);
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllBlue(void)
 *功  能：蓝灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_AllBlue(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0, 0, 0xFF);
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllViolet(void)
 *功  能：紫灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_AllViolet(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0x00, 0xCD, 0xCD);
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllWhite(void)
 *功  能：白灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_AllWhite(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0xFF, 0xFF, 0xFF);
	}
}

/**************************************************************************
 *函  数：void WS2812B_AllOFF(void)
 *功  能：关灯
 *参  数：无
 *返回值：无
 *备  注: 无
 **************************************************************************/
void WS2812B_AllOFF(void)
{
	for (uint8_t i = 0; i < WS2812B_AMOUNT; i++)
	{
		WS2812B_SetColour(0, 0, 0);
	}
}
