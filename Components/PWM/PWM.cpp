#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "PWM.h"
#include "driver/mcpwm_prelude.h"

#define TAG "PWM"

// Настройки двигателя
#define PWM_FREQUENCY_HZ     20000   // Частота ШИМ (20 кГц)
#define PWM_RESOLUTION_HZ    1000000 // Разрешение таймера (1 МГц)
#define MOTOR_POLE_PAIRS     7       // Количество пар полюсов двигателя

// Глобальные переменные
static mcpwm_cmpr_handle_t comparator_in3 = NULL;
static mcpwm_cmpr_handle_t comparator_in2 = NULL;
static mcpwm_cmpr_handle_t comparator_in1 = NULL;
static mcpwm_cmpr_handle_t comparator_en3 = NULL;
static mcpwm_cmpr_handle_t comparator_en2 = NULL;
static mcpwm_cmpr_handle_t comparator_en1 = NULL;
static mcpwm_timer_handle_t timer = NULL;


DRV8313_PWM::InitSettings pwm_init = {
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

DRV8313_PWM PWM(pwm_init);

// Инициализация MCPWM
void DRV8313_PWM::bldc_mcpwm_init(void)
{
	ESP_LOGI(TAG, "Инициализация MCPWM для BLDC управления");
	// Конфигурация таймера
	mcpwm_timer_config_t timer_config = {
		.group_id = 0,
		.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
		.resolution_hz = PWM_RESOLUTION_HZ,
		.count_mode = MCPWM_TIMER_COUNT_MODE_UP,
		.period_ticks = PWM_RESOLUTION_HZ / PWM_FREQUENCY_HZ,
        
	};
	ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

	// Конфигурация операторов
	mcpwm_oper_handle_t oper_3 = NULL;
	mcpwm_oper_handle_t oper_2 = NULL;
	mcpwm_oper_handle_t oper_1 = NULL;
    
	mcpwm_operator_config_t operator_config = {
		.group_id = 0,
	};
	ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_3));
	ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_2));
	ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper_1));

	// Подключение операторов к таймеру
	ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_3, timer));
	ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_2, timer));
	ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_1, timer));

	// Конфигурация компараторов
	mcpwm_comparator_config_t comparator_config = {
		.flags = {
		.update_cmp_on_tez = true,
	}
	};
	ESP_ERROR_CHECK(mcpwm_new_comparator(oper_3, &comparator_config, &comparator_in3));
	ESP_ERROR_CHECK(mcpwm_new_comparator(oper_2, &comparator_config, &comparator_in2));
	ESP_ERROR_CHECK(mcpwm_new_comparator(oper_1, &comparator_config, &comparator_in1));
	ESP_ERROR_CHECK(mcpwm_new_comparator(oper_3, &comparator_config, &comparator_en3));
	ESP_ERROR_CHECK(mcpwm_new_comparator(oper_2, &comparator_config, &comparator_en2));
	ESP_ERROR_CHECK(mcpwm_new_comparator(oper_1, &comparator_config, &comparator_en1));

	// Конфигурация генераторов
	mcpwm_gen_handle_t gen_a_high = NULL;
	mcpwm_gen_handle_t gen_a_low = NULL;
	mcpwm_gen_handle_t gen_b_high = NULL;
	mcpwm_gen_handle_t gen_b_low = NULL;
	mcpwm_gen_handle_t gen_c_high = NULL;
	mcpwm_gen_handle_t gen_c_low = NULL;
    
	mcpwm_generator_config_t generator_config = {
	};
	
	generator_config.gen_gpio_num = settings.en1;
	ESP_ERROR_CHECK(mcpwm_new_generator(oper_3, &generator_config, &gen_a_high));
	generator_config.gen_gpio_num = settings.in1;
	ESP_ERROR_CHECK(mcpwm_new_generator(oper_3, &generator_config, &gen_a_low));
    
	generator_config.gen_gpio_num = settings.en2;
	ESP_ERROR_CHECK(mcpwm_new_generator(oper_2, &generator_config, &gen_b_high));
	generator_config.gen_gpio_num = settings.in1;
	ESP_ERROR_CHECK(mcpwm_new_generator(oper_2, &generator_config, &gen_b_low));
    
	generator_config.gen_gpio_num = settings.en3;
	ESP_ERROR_CHECK(mcpwm_new_generator(oper_1, &generator_config, &gen_c_high));
	generator_config.gen_gpio_num = settings.in3;
	ESP_ERROR_CHECK(mcpwm_new_generator(oper_1, &generator_config, &gen_c_low));

	// Настройка действий генераторов
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_a_high, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_a_high, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_in3, MCPWM_GEN_ACTION_LOW)));
	
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_a_low, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_a_low, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_en3, MCPWM_GEN_ACTION_LOW)));

	
	// Повторите для фаз B и C аналогично фазе A
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_b_high, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_b_high, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_in2, MCPWM_GEN_ACTION_LOW)));
	
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_b_low, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_b_low, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_en2, MCPWM_GEN_ACTION_LOW)));


	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_c_high, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_c_high, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_in1, MCPWM_GEN_ACTION_LOW)));
	
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(gen_c_low, MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
	ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(gen_c_low, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator_en1, MCPWM_GEN_ACTION_LOW)));

	// Запуск таймера
	ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
	ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
}

// Установка ШИМ для фазы (0-100%)
void DRV8313_PWM::set_phase_pwm(mcpwm_cmpr_handle_t comparator, float duty_cycle)
{
	uint32_t period_ticks = PWM_RESOLUTION_HZ / PWM_FREQUENCY_HZ;
	uint32_t compare_ticks = (period_ticks * duty_cycle) / 100;
	ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, compare_ticks));
}

void DRV8313_PWM::comparator_en(uint8_t number_out, float duty_cycle)
{
	switch (number_out)
	{
	case 1:
		set_phase_pwm(comparator_en1, duty_cycle);
	break;
	case 2:
		set_phase_pwm(comparator_en2, duty_cycle);
	break;
	case 3:
		set_phase_pwm(comparator_en3, duty_cycle);
	break;
	}
}

void DRV8313_PWM::comparator_in(uint8_t number_out, float duty_cycle)
{
	switch (number_out)
	{
	case 1:
		set_phase_pwm(comparator_in1, duty_cycle);
	break;
	case 2:
		set_phase_pwm(comparator_in2, duty_cycle);
	break;
	case 3:
		set_phase_pwm(comparator_in3, duty_cycle);
	break;
	}
}