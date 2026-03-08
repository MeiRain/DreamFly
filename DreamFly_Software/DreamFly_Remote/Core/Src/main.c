#include "Scheduler.h"
#include "Remote.h"

int main()
{
	Remote_AllInit();
	Scheduler_Init();

	while(1)
	{			
		Scheduler_Loop();
	}
}

/***************** (C) COPYRIGHT 2024 梅雨 *** END OF FILE *******************/
