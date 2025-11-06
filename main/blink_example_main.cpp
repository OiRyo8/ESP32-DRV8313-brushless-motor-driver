
#include "driver/gpio.h"
#include "esp_log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/mcpwm_prelude.h"
#include "DRV8313.h"

	
#define BUTTON_GPIO1   GPIO_NUM_21
#define BUTTON_GPIO2   GPIO_NUM_37
#define BUTTON_GPIO3   GPIO_NUM_38
#define BUTTON_POLL_MS 100
#define BUTTON_POLL_TICKS (pdMS_TO_TICKS(BUTTON_POLL_MS))
	
extern DRV8313_Driver Motor;

void run_servo(void *)
{
	for (;;) {
		Motor.bldc_run_servo();
	}
}

void button_task(void *pvParameter) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	const TickType_t xPeriod = BUTTON_POLL_TICKS;
    
	// Новая переменная для хранения предыдущего состояния
	int prev_button_state1 = gpio_get_level(BUTTON_GPIO1);
	int prev_button_state2 = gpio_get_level(BUTTON_GPIO2); 
	int prev_button_state3 = gpio_get_level(BUTTON_GPIO3); 
	bool motor_enable_toggle = false;
	
	for (;;) {
		int current_button_state1 = gpio_get_level(BUTTON_GPIO1);
		int current_button_state2 = gpio_get_level(BUTTON_GPIO2);
		int current_button_state3 = gpio_get_level(BUTTON_GPIO3);

		// Условие: Кнопка была отпущена (HIGH) и стала нажата (LOW)
		if (prev_button_state1 == 1 && current_button_state1 == 0) 
		{
			ESP_LOGI("BUTTON", "Button Pressed! Running motor.");
			Motor.bldc_set_target(10);
			Motor.bldc_run_servo();
		} 
		if (prev_button_state2 == 1 && current_button_state2 == 0) 
		{
			ESP_LOGI("BUTTON", "Button Pressed! Running motor.");
			Motor.bldc_set_target(-10);
			Motor.bldc_run_servo();
		} 
		if (prev_button_state3 == 1 && current_button_state3 == 0) 
		{
			if (!motor_enable_toggle)
			{
				motor_enable_toggle = true;
				ESP_LOGI("BUTTON", "Button Pressed! Stop.");
				Motor.enable = !Motor.enable;
			}
		}
		else
		{
			motor_enable_toggle = false;
		}
       

		vTaskDelayUntil(&xLastWakeTime, xPeriod);
	}
}



extern "C"
void app_main() {
	
gpio_set_direction(BUTTON_GPIO1, GPIO_MODE_INPUT);
gpio_set_pull_mode(BUTTON_GPIO1, GPIO_PULLUP_ONLY);
gpio_set_direction(BUTTON_GPIO2, GPIO_MODE_INPUT);
gpio_set_pull_mode(BUTTON_GPIO2, GPIO_PULLUP_ONLY);
gpio_set_direction(BUTTON_GPIO3, GPIO_MODE_INPUT);
gpio_set_pull_mode(BUTTON_GPIO3, GPIO_PULLUP_ONLY);
	
Motor.init_pin();
xTaskCreate(run_servo, "Motor", 3072, NULL, 5, NULL);
xTaskCreate(button_task, "Button", 3072, NULL, 5, NULL);
	
}
