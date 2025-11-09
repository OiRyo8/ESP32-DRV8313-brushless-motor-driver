#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "DRV8313.h"
#include "driver/mcpwm_prelude.h"
#include "PWM.h"
#include "math.h"


#define TAG "Motor"

#define BLDC_STEPS 48 //Количество шагов на 360 градусов
#define STEP_DELAY 10 // Задержка между шагами (уменьшите для большей скорости)

volatile bool ncompo_low;

//Настройка пинов
DRV8313_Driver::InitSettings mtr_init = {
	.in1 = GPIO_NUM_14,
	.in2 = GPIO_NUM_12,
	.in3 = GPIO_NUM_10,
	.en1 = GPIO_NUM_13,
	.en2 = GPIO_NUM_11,
	.en3 = GPIO_NUM_9,
	.nfault = GPIO_NUM_4,
	.nreset = GPIO_NUM_5,
	.ncompo = GPIO_NUM_6,
};


DRV8313_Driver Motor(mtr_init);

//Инициализация пинов
void DRV8313_Driver::init_pin()
{
	ESP_LOGI(TAG, "Motor initialisation");
	
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
	
	rotor_degrees_current = 0;
	
	
	duty_cycle = 0.0f;
	step_delay_ms = STEP_DELAY;
	MULT = 100;
	gpio_set_level((gpio_num_t)settings.nreset, 0);
	vTaskDelay(pdMS_TO_TICKS(20));
	gpio_set_level((gpio_num_t)settings.nreset, 1);
	//xTaskCreate(ncompo_low_task, "ncompo_low", 2048, NULL, 5, NULL);
}


void DRV8313_Driver::bldc_set_target(float target)
{
	rotor_degrees_target = rotor_degrees_current + target;
}


void DRV8313_Driver::bldc_run_servo(float, uint32_t, uint8_t)
{
	if ((rotor_degrees_target != rotor_degrees_current) and (enable))
	{
		while (rotor_degrees_target != rotor_degrees_current) {
			Motor.bldc_control_task_servo(duty_cycle, step_delay_ms, MULT);
		}
	}
	else
	{ 
		while (rotor_degrees_target == rotor_degrees_current) {  //убрать цикл если нужно дердать сервопривод
			servo_low();
		}
		while (!enable) {
			servo_low(); 
		}
	}
}


void DRV8313_Driver::bldc_control_task_servo(float duty_cycle, uint32_t step_delay_ms, uint8_t MULT)
{
	bldc_commutate_sin(rotor_degrees_current * 7, MULT);
	if (rotor_degrees_target > rotor_degrees_current)
	{
		rotor_degrees_current += 1;
	}
	if (rotor_degrees_target < rotor_degrees_current)
	{
		rotor_degrees_current -= 1;
	}

	vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
}        


//Коммутация sin
void DRV8313_Driver::bldc_commutate_sin(float field_degrees, uint8_t MULT)
{
	bldc_set_phase_pwm(duty1, 0, field_degrees, MULT, 1);
	bldc_set_phase_pwm(duty2, 120, field_degrees, MULT, 2);
	bldc_set_phase_pwm(duty3, 240, field_degrees, MULT, 3);
}


//Задание фазы ШИМ
void DRV8313_Driver::bldc_set_phase_pwm(float &duty_cycle, float offset, float field_degrees, uint8_t MULT, uint8_t phase)
{	
		float radians = (field_degrees + offset) * (acos(-1) / 180);
		if (duty_cycle >= 0)
		{
			PWM.comparator_in(phase, duty_cycle);
			PWM.comparator_en(phase, duty_cycle);
			duty_cycle = MULT*(sin(radians));
		}
		else
		{
			PWM.comparator_in(phase, 0);
			PWM.comparator_en(phase, duty_cycle);
			duty_cycle = MULT*(sin(radians));
		}
}

void  DRV8313_Driver::servo_low()
{
	PWM.comparator_in(3, 0);
	PWM.comparator_in(2, 0);
	PWM.comparator_in(1, 0);
	PWM.comparator_en(3, 0);
	PWM.comparator_en(2, 0);
	PWM.comparator_en(1, 0);
}




//Обработка прерывания nCompo
//####################################

//Прерывание
void IRAM_ATTR ncompo_isr_handler(void* arg)
{
	ncompo_low = 1;
}


//Создание прерыванмя
void DRV8313_Driver::setup_ncompo_isr()
{
	// Настраиваем ncompo как вход с прерыванием
	gpio_config_t io_conf = {
		.pin_bit_mask = (1ULL << settings.ncompo),
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_NEGEDGE // Обычно нужно только по NEGEDGE (low)
	};
	gpio_config(&io_conf);
	
	// Привязываем обработчик
	gpio_install_isr_service(0);
	gpio_isr_handler_add(GPIO_NUM_6, ncompo_isr_handler, (void*) settings.ncompo);
}


//Функция защиты nCompo
void DRV8313_Driver::ncompo_low_task(void *pvParameter) {
	TickType_t xLastWakeTime;
	const TickType_t xPeriod = pdMS_TO_TICKS(100);

	// Инициализация переменной для первого вызова.
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		if (ncompo_low == 1)
		{
			ESP_LOGI(TAG, "Низкое значение NCOMPO");
			Motor.servo_low();
			vTaskDelay(pdMS_TO_TICKS(5));
			ncompo_low = 0;
		} 
		vTaskDelayUntil(&xLastWakeTime, xPeriod);
	}
}
//####################################







// Простая коммутация фаз (6-шаговая)
//void DRV8313_Driver::bldc_commutate_trapeze(uint8_t step, float duty_cycle)
//{
//switch (step % 6) {
//	case 0: // A high, B low, C off
//		PWM.comparator_in(3, 0);
//		PWM.comparator_in(2, duty_cycle);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_en(3, 100);
//		PWM.comparator_en(2, 100);
//		PWM.comparator_en(1, 0);
//		break;
//	case 1: // A high, C low, B off
//		PWM.comparator_in(3, 0);
//		PWM.comparator_in(2, 0);
//		PWM.comparator_in(1, duty_cycle);
//		PWM.comparator_en(3, 100);
//		PWM.comparator_en(2, 0);
//		PWM.comparator_en(1, 100);
//		break;
//	case 2: // B high, C low, A off
//		PWM.comparator_in(3, 0);
//		PWM.comparator_in(2, 0);
//		PWM.comparator_in(1, duty_cycle);
//		PWM.comparator_en(3, 0);
//		PWM.comparator_en(2, 100);
//		PWM.comparator_en(1, 100);
//		break;
//	case 3: // B high, A low, C off
//		PWM.comparator_in(3, duty_cycle);
//		PWM.comparator_in(2, 0);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_en(3, 100);
//		PWM.comparator_en(2, 100);
//		PWM.comparator_en(1, 0);
//		break;
//	case 4: // C high, A low, B off
//		PWM.comparator_in(3, duty_cycle);
//		PWM.comparator_in(2, 0);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_en(3, 100);
//		PWM.comparator_en(2, 0);
//		PWM.comparator_en(1, 100);
//		break;
//	case 5: // C high, B low, A off
//		PWM.comparator_in(3, 0);
//		PWM.comparator_in(2, duty_cycle);
//		PWM.comparator_in(1, 0);
//		PWM.comparator_en(3, 0);
//		PWM.comparator_en(2, 100);
//		PWM.comparator_en(1, 100);
//		break;
//	}
//}



// Задача для управления двигателем
//void DRV8313_Driver::bldc_control_task(uint8_t &step, float duty_cycle, uint32_t step_delay_ms, uint8_t MULT)
//{
//	ESP_LOGI(TAG, "Шаг коммутации: %d, Скважность: %.1f%%", step, duty_cycle);
//	if (MULT > 100)
//	{
//		bldc_commutate_trapeze(step, duty_cycle);
//	}
//	else
//	{
//		bldc_commutate_sin(step, MULT);
//	}
//	if (step < BLDC_STEPS - 1)
//	{
//		step++;
//	}
//	else
//	{
//		step = 0;
//	}
//	vTaskDelay(pdMS_TO_TICKS(step_delay_ms));
//}
