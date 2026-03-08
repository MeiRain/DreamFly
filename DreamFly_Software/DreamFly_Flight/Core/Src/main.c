#include "Flight.h"
#include "Scheduler.h"

int main(void)
{
	Flight_AllInit();
//	flight();
	Scheduler_Init();
	
	while (1)
	{
		Scheduler_Loop();
	}
}
