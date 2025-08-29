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
	void init_pin();
	void ncompo_enter_irq();
	void ncompo_exit_irq();
	void bldc_commutate_step(uint8_t step, float duty_cycle);
	void bldc_control_task(uint8_t &step, float duty_cycle, uint32_t step_delay_ms);
	void bldc_run(void);
	bool enable;
private: InitSettings settings;
};

extern DRV8313_Driver Motor;
