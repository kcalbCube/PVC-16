#pragma once
#include <cstdint>
#include "registers.h"

using addr_t = uint16_t;

constexpr auto ramSize = 16384;
class MemoryController
{
public:
	uint8_t data[ramSize]{};

	void fill(uint8_t tofill);

	void write8(addr_t addr, uint8_t towrite);

	void write16(addr_t addr, uint16_t towrite);

	[[nodiscard]] uint8_t read8(addr_t addr) const;

	[[nodiscard]] uint16_t read16(addr_t addr) const;

	[[maybe_unused]] size_t readInRegister(addr_t addr, RegisterID reg) const;

	void writeFromRegister(addr_t addr, RegisterID reg);

	void write(addr_t org, const std::vector<uint8_t>& dat);
};

inline MemoryController mc;