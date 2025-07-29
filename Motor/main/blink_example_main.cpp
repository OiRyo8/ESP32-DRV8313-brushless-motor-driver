#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "DRV8313.h"
#include "driver/ledc.h"


extern "C"

#define LEDC_GPIO      9
#define LEDC_CHANNEL   LEDC_CHANNEL_0
#define LEDC_TIMER     LEDC_TIMER_0
#define LEDC_MODE      LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES  LEDC_TIMER_13_BIT
#define LEDC_FREQ      5000

/*void app_main(void)
{
	// 1. Конфигурация таймера
	ledc_timer_config_t ledc_timer = {
		.speed_mode = LEDC_MODE,
		.duty_resolution = LEDC_DUTY_RES,
		.timer_num = LEDC_TIMER,
		.freq_hz = LEDC_FREQ,
		.clk_cfg = LEDC_AUTO_CLK,
	};
	ledc_timer_config(&ledc_timer);

	// 2. Конфигурация канала
	ledc_channel_config_t ledc_channel = {
		.gpio_num = LEDC_GPIO,
		.speed_mode = LEDC_MODE,
		.channel = LEDC_CHANNEL,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER,
		.duty = 0,
		.hpoint = 0
	};
	ledc_channel_config(&ledc_channel);

	// 3. Плавное изменение яркости (duty cycle)
	while (1) {
		// Увеличиваем яркость
		for (int duty = 0; duty < (1 << LEDC_DUTY_RES); duty += 100) {
			ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
			ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
        
		// Уменьшаем яркость
		for (int duty = (1 << LEDC_DUTY_RES) - 1; duty >= 0; duty -= 100) {
			ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, duty);
			ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
			vTaskDelay(10 / portTICK_PERIOD_MS);
		}
	}
}*/

	
void app_main()
{
	Motor.init_pin();
	Motor.acceleration(50000, 5000);
	Motor.speed(5000);
}