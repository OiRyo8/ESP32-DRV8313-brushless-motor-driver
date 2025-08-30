#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "DRV8313.h"
#include "driver/mcpwm_prelude.h"
#include "D:/VisualStudio/Motor/Motor/Components/PWM/PWM.h"
#include "math.h"


#define TAG "Motor"

#define IN1 GPIO_NUM_9
#define IN2 GPIO_NUM_11
#define IN3 GPIO_NUM_13
#define EN1 GPIO_NUM_10
#define EN2 GPIO_NUM_12
#define EN3 GPIO_NUM_14
#define NFAULT GPIO_NUM_4
#define NRESET GPIO_NUM_5
#define NCOMPO GPIO_NUM_6


// Глобальные переменные для обмена между задачами
volatile int currentStep = 0;
volatile uint32_t stepDelay0 = 40000; // Задержка между шагами в микросекундах
volatile uint32_t stepDelay1 = 4000;
float pi = acos(-1);


bool enable;

DRV8313_Driver::InitSettings mtr_init = {
	.in1 = GPIO_NUM_9,
	.in2 = GPIO_NUM_11,
	.in3 = GPIO_NUM_13,
	.en1 = GPIO_NUM_10,
	.en2 = GPIO_NUM_12,
	.en3 = GPIO_NUM_14,
	.nfault = GPIO_NUM_4,
	.nreset = GPIO_NUM_5,
	.ncompo = GPIO_NUM_6,
};

DRV8313_Driver Motor(mtr_init);


void DRV8313_Driver::init_pin()
{
	gpio_reset_pin((gpio_num_t)settings.nreset);
	gpio_set_direction((gpio_num_t)settings.nreset, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.nfault);
	gpio_set_direction((gpio_num_t)settings.nfault, GPIO_MODE_INPUT);
	
	gpio_reset_pin((gpio_num_t)settings.ncompo);
	gpio_set_direction((gpio_num_t)settings.ncompo, GPIO_MODE_INPUT);
	
	gpio_reset_pin((gpio_num_t)settings.in1);
	gpio_set_direction((gpio_num_t)settings.in1, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.en1);
	gpio_set_direction((gpio_num_t)settings.en1, GPIO_MODE_OUTPUT);
	
	
	gpio_reset_pin((gpio_num_t)settings.in2);
	gpio_set_direction((gpio_num_t)settings.in2, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.en2);
	gpio_set_direction((gpio_num_t)settings.en2, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.in2);
	gpio_set_direction((gpio_num_t)settings.in2, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.en3);
	gpio_set_direction((gpio_num_t)settings.en3, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.in3);
	gpio_set_direction((gpio_num_t)settings.in3, GPIO_MODE_OUTPUT);

	
	// Активация драйвера
	gpio_set_level((gpio_num_t)settings.nreset, 1);
  
	// Начальное состояние - все выключено
	gpio_set_level((gpio_num_t)settings.en1, 0);
	gpio_set_level((gpio_num_t)settings.en2, 0);
	gpio_set_level((gpio_num_t)settings.en3, 0);
	
	PWM.bldc_mcpwm_init();
}

void DRV8313_Driver::ncompo_enter_irq()
{
	Motor.enable = false;
}

void DRV8313_Driver::ncompo_exit_irq()
{
	Motor.enable = true;
}

// Простая коммутация фаз (6-шаговая)
void DRV8313_Driver::bldc_commutate_trapeze(uint8_t step, float duty_cycle)
{
switch (step % 6) {
	case 0: // A high, B low, C off
		PWM.comparator_in(3, 0);
		PWM.comparator_in(2, duty_cycle);
		PWM.comparator_in(1, 0);
		PWM.comparator_en(3, 100);
		PWM.comparator_en(2, 100);
		PWM.comparator_en(1, 0);
		break;
	case 1: // A high, C low, B off
		PWM.comparator_in(3, 0);
		PWM.comparator_in(2, 0);
		PWM.comparator_in(1, duty_cycle);
		PWM.comparator_en(3, 100);
		PWM.comparator_en(2, 0);
		PWM.comparator_en(1, 100);
		break;
	case 2: // B high, C low, A off
		PWM.comparator_in(3, 0);
		PWM.comparator_in(2, 0);
		PWM.comparator_in(1, duty_cycle);
		PWM.comparator_en(3, 0);
		PWM.comparator_en(2, 100);
		PWM.comparator_en(1, 100);
		break;
	case 3: // B high, A low, C off
		PWM.comparator_in(3, duty_cycle);
		PWM.comparator_in(2, 0);
		PWM.comparator_in(1, 0);
		PWM.comparator_en(3, 100);
		PWM.comparator_en(2, 100);
		PWM.comparator_en(1, 0);
		break;
	case 4: // C high, A low, B off
		PWM.comparator_in(3, duty_cycle);
		PWM.comparator_in(2, 0);
		PWM.comparator_in(1, 0);
		PWM.comparator_en(3, 100);
		PWM.comparator_en(2, 0);
		PWM.comparator_en(1, 100);
		break;
	case 5: // C high, B low, A off
		PWM.comparator_in(3, 0);
		PWM.comparator_in(2, duty_cycle);
		PWM.comparator_in(1, 0);
		PWM.comparator_en(3, 0);
		PWM.comparator_en(2, 100);
		PWM.comparator_en(1, 100);
		break;
	}
}

void DRV8313_Driver::bldc_commutate_sin(uint8_t step, uint8_t MULT, float &duty1, float &duty2, float &duty3)
{
	if (duty1 >= 0)
	{
		PWM.comparator_in(1, duty1);
		PWM.comparator_en(1, 100);
		duty1 = MULT*(1 + sin(360.0f*(step % 48) / 48)) / 2;
	}
	else
	{
		PWM.comparator_in(1, 0);
		PWM.comparator_en(1, duty1);
		duty1 = MULT*(1 + sin(360.0f*(step % 48) / 48)) / 2;
	}
	
	if (duty2 >= 0)
	{
		PWM.comparator_in(2, duty2);
		PWM.comparator_en(2, 100);
		duty2 = MULT*(1 + sin(-pi + 360.0f*(step % 48) / 48)) / 2;
	}
	else
	{
		PWM.comparator_in(2, 0);
		PWM.comparator_en(2, duty2);
		duty2 = MULT*(1 + sin(-pi + (360.0f*(step % 48) / 48))) / 2;
	}
	
	if (duty3 >= 0)
	{
		PWM.comparator_in(3, duty3);
		PWM.comparator_en(3, 100);
		duty3 = MULT*(1 + sin(2*pi + (360.0f*(step % 48) / 48))) / 2;
	}
	else
	{
		PWM.comparator_in(3, 0);
		PWM.comparator_en(3, duty3);
		duty3 = MULT*(1 + sin(2*pi + (360.0f*(step % 48) / 48))) / 2;
	}
}

// Задача для управления двигателем
void DRV8313_Driver::bldc_control_task(uint8_t &step, float duty_cycle, uint32_t step_delay_ms, uint8_t MULT, float &duty1, float &duty2, float &duty3)
{
	ESP_LOGI(TAG, "Шаг коммутации: %d, Скважность: %.1f%%", step, duty_cycle);
	if (MULT > 100)
	{
		bldc_commutate_trapeze(step, duty_cycle);
		step++;
	}
	else
	{
		bldc_commutate_sin(step, MULT, duty1, duty2, duty3);
		step++;
	}
	vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
}
        

void DRV8313_Driver::bldc_run(void)
{
	uint8_t step = 0;
	float duty_cycle = 0.0f; // Начальная скважность (10%)
	float duty1 = 0.0f;
	float duty2 = -1.0f;
	float duty3 = -1.0f;
//	float* duty1 = &d1;
//	float* duty2 = &d2;
//	float* duty3 = &d3;
	uint32_t step_delay_ms = 100; // Задержка между шагами (уменьшите для большей скорости)
	uint8_t MULT = 100;
	gpio_set_level(NRESET, 0);
	vTaskDelay(pdMS_TO_TICKS(20));
	gpio_set_level(NRESET, 1);
	
	while (1)
	{
		if (gpio_get_level(NCOMPO) == 0)
		{
			Motor.ncompo_enter_irq();
		}
		else
		{
			Motor.ncompo_exit_irq();
		}
		if (Motor.enable == true)
		{
			Motor.bldc_control_task(step, duty_cycle, step_delay_ms, MULT, duty1, duty2, duty3);
		}
		else {
			ESP_LOGI(TAG, "Низкое значение NCOMPO");
			PWM.comparator_in(3, 0);
			PWM.comparator_in(2, 0);
			PWM.comparator_in(1, 0);
			PWM.comparator_en(3, 0);
			PWM.comparator_en(2, 0);
			PWM.comparator_en(1, 0);
		}
	}
}
