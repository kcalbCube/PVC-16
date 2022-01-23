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

static struct StatusRegister
{
	bool zero : 1;
	bool greater : 1;
	bool sign : 1;
	bool interruptEnable : 1;


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
	if (id < AH)
	{
		registers[static_cast<size_t>(id)].u16 = shouldUpdateStatus ? updateStatus(value) : value;
		return;
	}
	const auto parent = static_cast<RegisterID>((id - AH) / 2);
	const auto isH = (id - AH) % 2 == 0;
	(isH ? registers[parent].h : registers[parent].l) = 
		shouldUpdateStatus ? updateStatus(static_cast<uint8_t>(value)) : static_cast<uint8_t>(value);
}

inline uint16_t readRegister(const RegisterID id)
{
	if(isH(id))
		return registers[static_cast<size_t>(id)].u16;
	const auto parent = static_cast<RegisterID>((id - AH) / 2);
	const auto isH = (id - AH) % 2 == 0;
	return isH ? registers[parent].h : registers[parent].l;
}

inline uint16_t& getRegister16(const RegisterID id)
{
	assert(isH(id));
	return registers[static_cast<size_t>(id)].u16;
}

inline uint8_t& getRegister8(const RegisterID id)
{
	const auto parent = static_cast<RegisterID>((id - AH) / 2);
	const auto isH = (id - AH) % 2 == 0;
	return isH ? registers[parent].h : registers[parent].l;
}