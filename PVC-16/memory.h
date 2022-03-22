#pragma once
#include <cstdint>
#include "registers.h"
#include <vector>

using addr_t = uint16_t;

constexpr auto ramSize = 0xFFFF;
class MemoryController
{
public:
	uint8_t data[ramSize]{};

	void fill(uint8_t);

	void write8(addr_t, uint8_t);

	void write16(addr_t, uint16_t);

	[[nodiscard]] uint8_t read8(addr_t) const;

	[[nodiscard]] uint16_t read16(addr_t) const;

	[[maybe_unused]] size_t readInRegister(addr_t, registers::RegisterID) const;

	void writeFromRegister(addr_t, registers::RegisterID);

	void write(addr_t, const std::vector<uint8_t>&);
};

inline MemoryController mc;