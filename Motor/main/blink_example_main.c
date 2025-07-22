
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/spi_master.h"

#define IN1 GPIO_NUM_9
#define EN1 GPIO_NUM_10
#define IN2 GPIO_NUM_11
#define EN2 GPIO_NUM_12
#define IN3 GPIO_NUM_13
#define EN3 GPIO_NUM_14
#define NFAULT GPIO_NUM_5
#define NRESET GPIO_NUM_4

#define TAG "Motor"


// ���������� ���������� ��� ������ ����� ��������
volatile int currentStep = 0;
volatile uint32_t stepDelay = 40000; // �������� ����� ������ � �������������


void init_pin()
{
	gpio_reset_pin(NRESET);
	gpio_set_direction(NRESET, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin(NFAULT);
	gpio_set_direction(NRESET, GPIO_MODE_INPUT);
	
	
	gpio_reset_pin(IN1);
	gpio_set_direction(IN1, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin(EN1);
	gpio_set_direction(EN1, GPIO_MODE_OUTPUT);
	
	
	gpio_reset_pin(IN2);
	gpio_set_direction(IN2, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin(EN2);
	gpio_set_direction(EN2, GPIO_MODE_OUTPUT);

	
	// ��������� ��������
	gpio_set_level(NRESET, 1);
  
	// ��������� ��������� - ��� ���������
	gpio_set_level(EN1, 0);
	gpio_set_level(EN2, 0);
	gpio_set_level(EN3, 0);
}


// ������� ���������� ��� 6 ����� (��������������� ����������)
const uint8_t stepTable[6][6] = {
	// EN1, EN2, EN3, IN1, IN2, IN3
	{ 0, 1, 1, 0, 1, 0 },
	// ��� 1: ���� B �������� (HIGH), ���� C �������� (LOW)
	{ 1, 0, 1, 1, 0, 0 },
	// ��� 2: ���� A �������� (HIGH), ���� C �������� (LOW)
	{ 1, 1, 0, 1, 0, 1 },
	// ��� 3: ���� A �������� (HIGH), ���� B �������� (LOW)
	{ 0, 1, 1, 0, 0, 1 },
	// ��� 4: ���� B �������� (LOW), ���� C �������� (HIGH)
	{ 1, 0, 1, 0, 1, 1 },
	// ��� 5: ���� A �������� (LOW), ���� C �������� (HIGH)
	{ 1, 1, 0, 0, 1, 0 }  // ��� 6: ���� A �������� (LOW), ���� B �������� (HIGH)
};


// ������� ���������� ���� ����������
void process(int step) {
	gpio_set_level(EN1, stepTable[step][0]);
	gpio_set_level(EN2, stepTable[step][1]);
	gpio_set_level(EN3, stepTable[step][2]);
	gpio_set_level(IN1, stepTable[step][3]);
	gpio_set_level(IN2, stepTable[step][4]);
	gpio_set_level(IN3, stepTable[step][5]);
}


void app_main()
{
	
	init_pin();
	//motor.init
	while (1) {
		
		while (stepDelay > 5000)
		{			
			process(currentStep);
			
			stepDelay = stepDelay - 10;
	
			currentStep = (currentStep + 1) % 6;
    
			// �������� ��� ���������� ���������
			vTaskDelay(pdMS_TO_TICKS(stepDelay / 1000));
			
			ESP_LOGI(TAG, "STEP = %d", currentStep);
			ESP_LOGI(TAG, "nFault = %d", gpio_get_level(NFAULT));
			ESP_LOGI(TAG, "stepDelay = %lu", stepDelay);
		}
	
		
		//motor.process - ����������� ����
		// ��������� ������� ��� ����������
		process(currentStep);
    
		// ������� � ���������� ����
		currentStep = (currentStep + 1) % 6;
    
		// �������� ��� ���������� ���������
		vTaskDelay(pdMS_TO_TICKS(stepDelay / 1000)); // ������������ ������������ � ������������
		
		ESP_LOGI(TAG, "STEP = %d", currentStep);
		ESP_LOGI(TAG, "nFault = %d", gpio_get_level(NFAULT));
		ESP_LOGI(TAG, "stepDelay = %lu", stepDelay);
	}
}