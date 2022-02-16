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
	int zero:1;
	int greater:1;
	int sign:1;
	int interrupt:1;
	int overflow:1;


} status;

inline uint16_t updateStatus16(const unsigned result)
{
	status.zero = result == 0x0;
	status.sign = result & (1 << 16);
	status.overflow = result != (result & 0xFFFF);

	return result;
}

inline uint8_t updateStatus8(const unsigned result)
{
	status.zero = result == 0x0;
	status.sign = result & (1 << 8);
	status.overflow = result != (result & 0xFF);
	return result;
}

inline Register registers[AH];

inline void writeRegister(const RegisterID id, const uint16_t value, bool shouldUpdateStatus = false)
{

	if (is16register(id))
	{
		if (shouldUpdateStatus)
			updateStatus16(value);
		registers[static_cast<size_t>(id)].u16 = value;
		return;
	}
	if (shouldUpdateStatus)
		updateStatus8(value);
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