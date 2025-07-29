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
		uint8_t nreset;
		uint8_t nfault;
	};
	DRV8313_Driver(InitSettings settings): settings(settings)
	{
	}
	void init_pin();
	void pin_activation(int step);
	void acceleration(uint32_t stepDelay0, uint32_t stepDelay1);
	void speed(uint32_t stepDelay1);
private: InitSettings settings;
};

extern DRV8313_Driver Motor;