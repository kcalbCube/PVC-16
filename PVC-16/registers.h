#pragma once
#include <array>
#include <cassert>
#include <cstdint>
#include "registers_define.h"

union Register
{
	uint16_t u16;
	struct
	{
		uint8_t l, h;
	};
};

inline struct StatusRegister
{
	int zero = 0;
	int greater = 0;
	int sign = 0;
	int interruptEnable = 1;


} status;

inline uint16_t updateStatus(const uint16_t result)
{
	status.zero = result == 0x0;
	status.sign = result & (1 << 16);

	return result;
}

inline uint8_t updateStatus(const uint8_t result)
{
	status.zero = result == 0x0;
	status.sign = result & (1 << 8);

	return result;
}

inline Register registers[AH];

inline void writeRegister(const RegisterID id, const uint16_t value, bool shouldUpdateStatus = false)
{
	if (shouldUpdateStatus)
		updateStatus(value);

	if (is16register(id))
	{
		registers[static_cast<size_t>(id)].u16 = value;
		return;
	}
	const auto parent = static_cast<RegisterID>((id - AH) / 2);
	const auto isH = (id - AH) % 2 == 0;
	(isH ? registers[parent].h : registers[parent].l) = value;
}

inline uint16_t readRegister(const RegisterID id)
{
	if(is16register(id))
		return registers[static_cast<size_t>(id)].u16;
	const auto idSubAH = id - AH;
	const auto parent = static_cast<RegisterID>(idSubAH >> 1);
	const auto isH = !(idSubAH & 1);
	return isH ? registers[parent].h : registers[parent].l;
}

inline uint16_t& getRegister16(const RegisterID id)
{
	assert(is16register(id));
	return registers[static_cast<size_t>(id)].u16;
}

inline uint8_t& getRegister8(const RegisterID id)
{
	const auto idSubAH = id - AH;
	const auto parent = static_cast<RegisterID>(idSubAH >> 1);
	const auto isH = !(idSubAH & 1);
	return isH ? registers[parent].h : registers[parent].l;
}