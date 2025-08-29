#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "DRV8313.h"
#include "driver/mcpwm_prelude.h"
#include "D:/VisualStudio/Motor/Motor/Components/PWM/PWM.h"

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
void DRV8313_Driver::bldc_commutate_step(uint8_t step, float duty_cycle)
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


// Задача для управления двигателем
void DRV8313_Driver::bldc_control_task(uint8_t &step, float duty_cycle, uint32_t step_delay_ms)
{
	ESP_LOGI(TAG, "Шаг коммутации: %d, Скважность: %.1f%%", step, duty_cycle);
	bldc_commutate_step(step, duty_cycle);
	step++;
	vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
}
        

void DRV8313_Driver::bldc_run(void)
{
	uint8_t step = 0;
	float duty_cycle = 10.0f; // Начальная скважность (10%)
	uint32_t step_delay_ms = 10; // Задержка между шагами (уменьшите для большей скорости)
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
			Motor.bldc_control_task(step, duty_cycle, step_delay_ms);
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
