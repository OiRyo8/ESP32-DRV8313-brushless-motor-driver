#include <cstdint>
#include "driver/mcpwm_types.h"

class DRV8313_PWM
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
	DRV8313_PWM(InitSettings settings)
		: settings(settings)
	{
	}
	void bldc_mcpwm_init(void);
	void set_phase_pwm(mcpwm_cmpr_handle_t comparator, float duty_cycle);
	void bldc_commutate_step(uint8_t step, float duty_cycle);
	void bldc_control_task(uint8_t &step, float, uint32_t);
	void bldc_run(void);
	void comparator_en(uint8_t, float duty_cycle);
	void comparator_in(uint8_t, float duty_cycle);
private: InitSettings settings;
};

extern DRV8313_PWM PWM;