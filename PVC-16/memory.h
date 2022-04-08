#pragma once
#include <cstdint>
#include "registers.h"

using addr_t = uint_fast32_t;

constexpr auto ramSize = 0xFFFF;
class MemoryController
{
public:
	uint_fast8_t data[ramSize]{};

	void fill(uint8_t);

	void write8(addr_t, uint_fast8_t);

	void write16(addr_t, uint_fast16_t);

	[[nodiscard]] uint_fast8_t read8(addr_t) const;

	[[nodiscard]] uint_fast16_t read16(addr_t) const;

	[[maybe_unused]] int readInRegister(addr_t, registers::RegisterID) const;

	void writeFromRegister(addr_t, registers::RegisterID);
};

inline MemoryController mc;
