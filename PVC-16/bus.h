#pragma once
#include <cstdint>
#include <cstddef>

// TODO: move to bus namespace
constexpr size_t busSize = 0xFF + 1;
inline uint_fast8_t bus[busSize]{};

void busWrite(int addr, uint_fast8_t value);
void busWrite16(int addr, uint_fast16_t value);
uint_fast8_t busRead(int addr);
uint_fast16_t busRead16(int addr);

enum
{
	BUS_DOUT = 0xE9,
	BUS_VIDEO_START = 0x20,
	BUS_VIDEO_END = 0x40,
};
