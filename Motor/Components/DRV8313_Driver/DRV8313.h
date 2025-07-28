#include <cstdint>

class DRV8313_Driver
{
public:
	void init_pin();
	void pin_activation(int step);
	void acceleration(uint32_t stepDelay0, uint32_t stepDelay1);
	void speed(uint32_t stepDelay1);
};

extern DRV8313_Driver Motor;