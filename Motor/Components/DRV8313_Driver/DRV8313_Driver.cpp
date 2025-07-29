#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "DRV8313.h"

//#define IN1 GPIO_NUM_9
//#define EN1 GPIO_NUM_10
//#define IN2 GPIO_NUM_11
//#define EN2 GPIO_NUM_12
//#define IN3 GPIO_NUM_13
//#define EN3 GPIO_NUM_14
//#define NFAULT GPIO_NUM_5
//#define NRESET GPIO_NUM_4

#define TAG "Motor"


// ���������� ���������� ��� ������ ����� ��������
volatile int currentStep = 0;
volatile uint32_t stepDelay0 = 40000;// �������� ����� ������ � �������������

volatile uint32_t stepDelay1 = 4000;

DRV8313_Driver::InitSettings mtr_init = {
	.in1 = GPIO_NUM_9,
	.in2 = GPIO_NUM_11,
	.in3 = GPIO_NUM_13,
	.en1 = GPIO_NUM_10,
	.en2 = GPIO_NUM_12,
	.en3 = GPIO_NUM_14,
	.nreset = GPIO_NUM_4,
	.nfault = GPIO_NUM_5,
	};

DRV8313_Driver Motor(mtr_init);

void DRV8313_Driver:: init_pin()
{
	gpio_reset_pin((gpio_num_t)settings.nreset);
	gpio_set_direction((gpio_num_t)settings.nreset, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.nfault);
	gpio_set_direction((gpio_num_t)settings.nfault, GPIO_MODE_INPUT);
	
	
	gpio_reset_pin((gpio_num_t)settings.in1);
	gpio_set_direction((gpio_num_t)settings.in1, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.en1);
	gpio_set_direction((gpio_num_t)settings.en1, GPIO_MODE_OUTPUT);
	
	
	gpio_reset_pin((gpio_num_t)settings.in2);
	gpio_set_direction((gpio_num_t)settings.in2, GPIO_MODE_OUTPUT);
	
	gpio_reset_pin((gpio_num_t)settings.en2);
	gpio_set_direction((gpio_num_t)settings.en2, GPIO_MODE_OUTPUT);

	
	// ��������� ��������
	gpio_set_level((gpio_num_t)settings.nreset, 1);
  
	// ��������� ��������� - ��� ���������
	gpio_set_level((gpio_num_t)settings.en1, 0);
	gpio_set_level((gpio_num_t)settings.en2, 0);
	gpio_set_level((gpio_num_t)settings.en3, 0);
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
	{ 1, 1, 0, 0, 1, 0 }  
		// ��� 6: ���� A �������� (LOW), ���� B �������� (HIGH)
};


// ������� ���������� ���� ����������
void DRV8313_Driver:: pin_activation(int step) {
	gpio_set_level((gpio_num_t)settings.en1, stepTable[step][0]);
	gpio_set_level((gpio_num_t)settings.en2, stepTable[step][1]);
	gpio_set_level((gpio_num_t)settings.en3, stepTable[step][2]);
	gpio_set_level((gpio_num_t)settings.in1, stepTable[step][3]);
	gpio_set_level((gpio_num_t)settings.in2, stepTable[step][4]);
	gpio_set_level((gpio_num_t)settings.in3, stepTable[step][5]);
}

void DRV8313_Driver:: acceleration(uint32_t stepDelay0, uint32_t stepDelay1)
{
	while (stepDelay0 > stepDelay1)
	{
		pin_activation(currentStep);
			
		stepDelay0 = stepDelay0 - 10;
	
		currentStep = (currentStep + 1) % 6;
    
		// �������� ��� ���������� ���������
		vTaskDelay(pdMS_TO_TICKS(stepDelay0 / 1000));
			
		ESP_LOGI(TAG, "STEP = %d", currentStep);
		ESP_LOGI(TAG, "nFault = %d", gpio_get_level((gpio_num_t)settings.nfault));
		ESP_LOGI(TAG, "stepDelay = %lu", stepDelay0);
	}
}

void DRV8313_Driver:: speed(uint32_t stepDelay1)
{
	// ��������� ������� ��� ����������
	pin_activation(currentStep);
    
	// ������� � ���������� ����
	currentStep = (currentStep + 1) % 6;
    
	// �������� ��� ���������� ���������
	vTaskDelay(pdMS_TO_TICKS(stepDelay1 / 1000)); // ������������ ������������ � ������������
		
	ESP_LOGI(TAG, "STEP = %d", currentStep);
	ESP_LOGI(TAG, "nFault = %d", gpio_get_level((gpio_num_t)settings.nfault));
	ESP_LOGI(TAG, "stepDelay = %lu", stepDelay1);
}
