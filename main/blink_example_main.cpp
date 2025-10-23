
#include "driver/gpio.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/mcpwm_prelude.h"
#include "DRV8313.h"

extern "C"
	
#define BUTTON_GPIO   GPIO_NUM_20
#define BUTTON_POLL_MS 100
#define BUTTON_POLL_TICKS (pdMS_TO_TICKS(BUTTON_POLL_MS))
	
void button_task(void *pvParameter) {
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = BUTTON_POLL_TICKS;

	// Инициализация переменной для первого вызова.
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		if (gpio_get_level(BUTTON_GPIO) == 0)
		{
			Motor.bldc_set_target(359);
			Motor.bldc_run_servo();
		} 
		vTaskDelayUntil(&xLastWakeTime, xPeriod);
	}
}


void app_main() {
	
gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);
gpio_set_pull_mode(BUTTON_GPIO, GPIO_PULLUP_ONLY);
	
Motor.init_pin();
	
xTaskCreate(button_task, "Button", 2048, NULL, 5, NULL);
	
}
