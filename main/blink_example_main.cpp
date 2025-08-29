#include "driver/gpio.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/mcpwm_prelude.h"
#include "DRV8313.h"

extern "C"

/*void app_main()
{
	Motor.init_pin();
	Motor.acceleration(50000, 5000);
	Motor.speed(5000);
}*/


void app_main()
{
	Motor.init_pin();
	
	// Создание задачи для управления двигателем
	Motor.bldc_run();
}