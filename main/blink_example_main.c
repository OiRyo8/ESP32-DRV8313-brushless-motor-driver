#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define IN1 GPIO_NUM_9
#define EN1 GPIO_NUM_10
#define IN2 GPIO_NUM_11
#define EN2 GPIO_NUM_12
#define IN3 GPIO_NUM_13
#define EN3 GPIO_NUM_14
#define NSLEEP GPIO_NUM_5
#define NFAULT GPIO_NUM_4


#define TAG "MOTOR"

void init()
{
	gpio_reset_pin(NFAULT);
	gpio_set_direction(NFAULT, GPIO_MODE_INPUT);
  
	gpio_reset_pin(NSLEEP);
	gpio_set_direction(NSLEEP, GPIO_MODE_OUTPUT);
  
	gpio_reset_pin(IN1);
	gpio_set_direction(IN1, GPIO_MODE_OUTPUT);
  
	gpio_reset_pin(EN1);
	gpio_set_direction(EN1, GPIO_MODE_OUTPUT);
  
	gpio_reset_pin(IN2);
	gpio_set_direction(IN2, GPIO_MODE_OUTPUT);
  
	gpio_reset_pin(EN2);
	gpio_set_direction(EN2, GPIO_MODE_OUTPUT);
  
	gpio_reset_pin(IN3);
	gpio_set_direction(IN3, GPIO_MODE_OUTPUT);
  
	gpio_reset_pin(EN3);
	gpio_set_direction(EN3, GPIO_MODE_OUTPUT);
}

void app_main(void)
{
	init();
  
	gpio_set_level(EN1, 1);
	gpio_set_level(EN2, 1);	
	gpio_set_level(NSLEEP, 1);
  
	while (1)
	{
		gpio_set_level(IN1, 1);
		vTaskDelay(pdMS_TO_TICKS(200));
		ESP_LOGI(TAG, "on");
		gpio_set_level(IN1, 0);
		vTaskDelay(pdMS_TO_TICKS(200));
		ESP_LOGI(TAG, "off");
	}
}