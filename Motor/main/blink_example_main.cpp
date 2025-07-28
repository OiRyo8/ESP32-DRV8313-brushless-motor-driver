#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "DRV8313.h"


extern "C"
	
void app_main()
{
	Motor.init_pin();
	Motor.acceleration(50000, 5000);
	Motor.speed(5000);
}