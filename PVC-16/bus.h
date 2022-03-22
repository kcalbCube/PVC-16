#pragma once
#include <cstdint>
// TODO: move to bus namespace
constexpr size_t busSize = 0xFF + 1;
inline uint8_t bus[busSize]{};

void busWrite(size_t addr, uint8_t value);
void busWrite16(size_t addr, uint16_t value);
uint8_t busRead(size_t addr);
uint16_t busRead16(size_t addr);

enum
{
	BUS_DOUT = 0xE9,
	BUS_VIDEO_START = 0x20,
	BUS_VIDEO_END = 0x40,
};