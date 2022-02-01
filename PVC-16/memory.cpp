#include "memory.h"

void MemoryController::fill(uint8_t tofill)
{
	std::fill(std::begin(data), std::end(data), tofill);
}

void MemoryController::write8(const addr_t addr, const uint8_t towrite)
{
	data[addr] = towrite;
}

void MemoryController::write16(const addr_t addr, const uint16_t towrite)
{
	union
	{
		struct 
		{
			uint8_t h, l;
		};
		uint16_t u16;
	};
	u16 = towrite;
	write8(addr, h);
	write8(addr + 1, l);

}

uint8_t MemoryController::read8(const addr_t addr) const
{
	return data[addr];
}

uint16_t MemoryController::read16(const addr_t addr) const
{
	return static_cast<uint16_t>(read8(addr) | (read8(addr + 1) << 8));
}

size_t MemoryController::readInRegister(const addr_t addr, const RegisterID reg) const
{
	return is16register(reg) ?
		       (writeRegister(reg, read16(addr)), 2)
		       :   (writeRegister(reg, read8(addr)) , 1);
}

void MemoryController::writeFromRegister(const addr_t addr, const RegisterID reg)
{
	is16register(reg) ?
		write16(addr, readRegister(reg))
		:	write8(addr, readRegister(reg));
}

void MemoryController::write(const addr_t org, const std::vector<uint8_t>& dat)
{
	std::ranges::copy(dat, data + org);
}
