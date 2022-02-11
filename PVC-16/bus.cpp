#include "bus.h"
#include "utility.h"
//uint8_t bus[busSize];
//bool busWrites[busSize / busSector];

void busWrite(size_t addr, uint8_t value)
{
	bus[addr] = value;
}

void busWrite16(size_t addr, uint16_t value)
{
	busWrite(addr    , m1628h(value));
	busWrite(addr + 1, m1628l(value));
}

uint8_t busRead(size_t addr)
{
	return bus[addr];
}

uint16_t busRead16(size_t addr)
{
	return static_cast<uint16_t>(busRead(addr) | (busRead(addr + 1) << 8));
}