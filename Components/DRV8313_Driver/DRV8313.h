#include <cstdint>

class DRV8313_Driver
{
public:
	struct InitSettings
	{
		uint8_t in1;
		uint8_t in2;
		uint8_t in3;
		uint8_t en1;
		uint8_t en2;
		uint8_t en3;
		uint8_t nfault;
		uint8_t nreset;
		uint8_t ncompo;
	};
	DRV8313_Driver(InitSettings settings)
		: settings(settings)
	{
	}
	
	//Переменные
	float duty_cycle;
	uint32_t step_delay_ms;
	uint8_t MULT;
	bool enable;
	
	void init_pin();
	void bldc_set_target(float);
	void bldc_run_servo(float, uint32_t, uint8_t);
	void bldc_control_task_servo(float duty_cycle, uint32_t step_delay_ms, uint8_t MULT);
	void bldc_commutate_sin(float field_degrees, uint8_t);
	void bldc_set_phase_pwm(float &duty_cycle, float offset, float field_degrees, uint8_t MULT, uint8_t phase);
	void servo_low();
	
	//Прерывание
	void setup_ncompo_isr();
	static void ncompo_low_task(void *pvParameter);
private: InitSettings settings;
	float duty1 = 0.0f;
	float duty2 = -1.0f;
	float duty3 = -1.0f;
	float rotor_degrees_current;
	float rotor_degrees_target;
};

extern DRV8313_Driver Motor;
