#include "registers.h"
namespace registers
{
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

	PackedStatus packStatus(void)
	{
		PackedStatus ps{};
#define WRITE_PS(member, id) if(status. member) ps |= (1 << (id))
		WRITE_PS(zero		, 0);
		WRITE_PS(sign		, 1);
		WRITE_PS(overflow	, 2);
		WRITE_PS(interrupt	, 3);
#undef WRITE_PS
		return ps;
	}

	void unpackStatus(PackedStatus ps)
	{
#define READ_PS(member, id)	(status. member) = (ps & 1 << (id))
		READ_PS(zero	 , 0);
		READ_PS(sign	 , 1);
		READ_PS(overflow , 2);
		READ_PS(interrupt, 3);
#undef READ_PS
	}


	uint16_t updateStatus16(const unsigned result)
	{
		status.zero = result == 0x0;
		status.sign = result & 1 << 16;
		status.overflow = result & (~0xFFFF);

		return static_cast<uint16_t>(result);
	}

	uint8_t updateStatus8(const unsigned result)
	{
		status.zero = result == 0x0;
		status.sign = result & 1 << 8;
		status.overflow = result & (~0xFF);
		return static_cast<uint8_t>(result);
	}

	void write(const RegisterID id, const uint16_t value, const bool shouldUpdateStatus)
	{
		if (is16register(id))
		{
			if (shouldUpdateStatus)
				updateStatus16(value);
			get16(id) = value;
			return;
		}
		if (shouldUpdateStatus)
			updateStatus8(value);
		get8(id) = static_cast<uint8_t>(value);
	}

	uint16_t read(const RegisterID id)
	{
		if (id >= NO_REG)
			return 0;

		if (is16register(id))
			return get16(id);
		return get8(id);
	}

	uint16_t& get16(const RegisterID id)
	{
		return registers[static_cast<size_t>(id)].u16;
	}

	uint8_t& get8(const RegisterID id)
	{
		const auto idSubAH = id - AH;
		const auto parent = static_cast<RegisterID>(idSubAH / 2);
		const auto isH = !(idSubAH & 1);
		return isH ? registers[parent].h : registers[parent].l;
	}
}