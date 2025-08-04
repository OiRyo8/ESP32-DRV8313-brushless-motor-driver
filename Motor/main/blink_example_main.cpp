#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "DRV8313.h"
#include "driver/ledc.h"
#include "driver/mcpwm.h"

extern "C"

/*#define LEDC_GPIO      9
#define LEDC_CHANNEL   LEDC_CHANNEL_0
#define LEDC_TIMER     LEDC_TIMER_0
#define LEDC_MODE      LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES  LEDC_TIMER_13_BIT
#define LEDC_FREQ      5000*/

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


/*void app_main()
{
	Motor.init_pin();
	Motor.acceleration(50000, 5000);
	Motor.speed(5000);
}*/
	
	
#define IN1 GPIO_NUM_9
#define EN1 GPIO_NUM_10
#define IN2 GPIO_NUM_11
#define EN2 GPIO_NUM_12
#define IN3 GPIO_NUM_13
#define EN3 GPIO_NUM_14
#define NFAULT GPIO_NUM_5
#define NRESET GPIO_NUM_4  // Вывод GPIO, на который подается ШИМ

	void app_main(void)
	{	
		Motor.init_pin();
		
		// 1. Инициализация MCPWM
		mcpwm_config_t pwm_config = {
			.frequency = 1000, // Частота ШИМ = 1 кГц
			.cmpr_a = 50.0, // Начальная скважность канала A = 50%
			.duty_mode = MCPWM_DUTY_MODE_0,
			.counter_mode = MCPWM_UP_COUNTER,
		};
    
		// 2. Настройка модуля MCPWM (выбираем модуль 0 и таймер 0)
		mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    
		// 3. Привязываем вывод GPIO к каналу A таймера 0
		mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, IN1);

		float duty_cycle = 0.0;
		bool increasing = true;
		gpio_set_level(EN1, 1);
		gpio_set_level(EN2, 1);
		gpio_set_level(EN3, 1);
		
		while (1) {
			// 4. Плавно меняем скважность от 0% до 100% и обратно
			if (increasing) {
				duty_cycle += 5.0;
				if (duty_cycle >= 100.0) {
					duty_cycle = 100.0;
					increasing = false;
				}
			}
			else {
				duty_cycle -= 5.0;
				if (duty_cycle <= 0.0) {
					duty_cycle = 0.0;
					increasing = true;
				}
			}

			// 5. Устанавливаем новую скважность
			mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
        
			vTaskDelay(pdMS_TO_TICKS(100)); // Задержка 100 мс
		}
	}