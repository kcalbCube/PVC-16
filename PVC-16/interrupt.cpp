#include "interrupt.h"

void interrupt(const uint8_t interrupt)
{
	switch(interrupt)
	{
	case HALT:
		isHalted = true;
		break;
	default:

		break;
	}
}
