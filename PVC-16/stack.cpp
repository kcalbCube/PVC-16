#include "stack.h"
#include "memory.h"

namespace stack
{
	void pushf(void)
	{
		push8(registers::packStatus());
	}

	void popf(void)
	{
		registers::unpackStatus(pop8());
	}

	void push(const registers::RegisterID rid)
	{
		if (is16register(rid))
			push16(read(rid));
		else
			push8(static_cast<uint8_t>(read(rid)));
	}

	void push16(const uint16_t src)
	{
		auto&& sp = get16(registers::SP);
		sp -= 2;
		mc.write16(sp, src);
	}

	void push8(const uint8_t src)
	{
		mc.write16(--get16(registers::SP), src);
	}

	void pop(const registers::RegisterID rid)
	{
		auto&& sp = get16(registers::SP);
		(void)mc.readInRegister(sp, rid);
		sp += 1 + is16register(rid);
	}

	uint16_t pop16(void)
	{
		auto&& sp = get16(registers::SP);
		const auto toreturn = mc.read16(sp);
		sp += 2;
		return toreturn;
	}

	uint8_t pop8(void)
	{
		return mc.read8(get16(registers::SP)++);
	}
}