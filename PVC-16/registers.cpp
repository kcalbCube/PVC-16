#include "registers.h"
namespace
{
	union
	{
		uint16_t u16;
		struct
		{
			uint8_t l, h;
		};
	} registers[AH];
}

uint16_t updateStatus16(const unsigned result)
{
	status.zero = result == 0x0;
	status.sign = result & 1 << 16;
	status.overflow = result != (result & 0xFFFF);

	return static_cast<uint16_t>(result);
}

uint8_t updateStatus8(const unsigned result)
{
	status.zero = result == 0x0;
	status.sign = result & 1 << 8;
	status.overflow = result != (result & 0xFF);
	return static_cast<uint8_t>(result);
}

void writeRegister(const RegisterID id, const uint16_t value, const bool shouldUpdateStatus)
{
	if (is16register(id))
	{
		if (shouldUpdateStatus)
			updateStatus16(value);
		registers[id].u16 = value;
		return;
	}
	if (shouldUpdateStatus)
		updateStatus8(value);
	const auto parent = static_cast<RegisterID>((id - AH) / 2);
	const auto isH = (id - AH) % 2 == 0;
	(isH ? registers[parent].h : registers[parent].l) = static_cast<uint8_t>(value);
}

uint16_t readRegister(const RegisterID id)
{
	if(is16register(id))
		return registers[static_cast<size_t>(id)].u16;
	const auto idSubAH = id - AH;
	const auto parent = static_cast<RegisterID>(idSubAH >> 1);
	const auto isH = !(idSubAH & 1);
	return isH ? registers[parent].h : registers[parent].l;
}

uint16_t& getRegister16(const RegisterID id)
{
	return registers[static_cast<size_t>(id)].u16;
}

uint8_t& getRegister8(const RegisterID id)
{
	const auto idSubAH = id - AH;
	const auto parent = static_cast<RegisterID>(idSubAH >> 1);
	const auto isH = !(idSubAH & 1);
	return isH ? registers[parent].h : registers[parent].l;
}
