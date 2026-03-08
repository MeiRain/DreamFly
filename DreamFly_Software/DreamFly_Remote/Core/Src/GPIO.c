#include "GPIO.h"

void GPIO_DebugInit(void)
{
	//打开GPIO口时钟，先打开复用才能修改是否停用复用功能
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	//关闭JTAG，使能SWD调试口
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}
