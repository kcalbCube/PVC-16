#include "memory.h"
#include <bit>

void MemoryController::fill(const uint8_t tofill)
{
	std::fill(std::begin(data), std::end(data), tofill);
}

void MemoryController::write8(const addr_t addr, const uint_fast8_t towrite)
{
	data[addr] = towrite & 0xFF;
}

void MemoryController::write16(const addr_t addr, const uint_fast16_t towrite)
{
	using T = struct { uint8_t h, l; };
	const auto [h, l] = std::bit_cast<T>(static_cast<uint16_t>(towrite));
	write8(addr		, h);
	write8(addr + 1 , l);

}

uint_fast8_t MemoryController::read8(const addr_t addr) const
{
	return data[addr] & 0xFF;
}

uint_fast16_t MemoryController::read16(const addr_t addr) const
{
	return (read8(addr) | (read8(addr + 1) << 8)) & 0xFFFF;
}

int MemoryController::readInRegister(const addr_t addr, const registers::RegisterID reg) const
{
	return is16register(reg) ?
		       (registers::write(reg, read16(addr)), 2)
			:  (registers::write(reg, read8(addr)) , 1);
}

void MemoryController::writeFromRegister(const addr_t addr, const registers::RegisterID reg)
{
	is16register(reg) ?
		write16(addr, read(reg))
	:	write8(addr, static_cast<uint8_t>(read(reg)));
}