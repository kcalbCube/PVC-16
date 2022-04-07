#include "bus.h"
#include "utility.h"

void busWrite(int addr, uint_fast8_t value)
{
	bus[addr] = value & 0xFF;
}

void busWrite16(int addr, uint_fast16_t value)
{
	busWrite(addr    , m1628h(value));
	busWrite(addr + 1, m1628l(value));
}

uint_fast8_t busRead(int addr)
{
	return bus[addr] & 0xFF;
}

uint_fast16_t busRead16(int addr)
{
	return (busRead(addr) | (busRead(addr + 1) << 8));
}
