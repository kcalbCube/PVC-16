#include "interrupt.h"

#include <iostream>

void interrupt(const uint8_t interrupt)
{
	switch(interrupt)
	{
	case HALT:
		isHalted = true;
		break;

	case DOUT:
		std::cout << static_cast<char>(readRegister(AL));
		break;
	default:

		break;
	}
}
