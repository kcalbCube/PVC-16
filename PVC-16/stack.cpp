#include "stack.h"
#include "memory.h"

void StackController::push(const RegisterID rid)
{
	if (is16register(rid))
		push16(readRegister(rid));
	else
		push8(static_cast<uint8_t>(readRegister(rid)));
}

void StackController::push16(const uint16_t src)
{
	auto&& sp = getRegister16(SP);
	sp -= 2;
	mc.write16(sp, src);
}

void StackController::push8(const uint8_t src)
{
	mc.write16(--getRegister16(SP), src);
}

void StackController::pop(const RegisterID rid)
{
	auto&& sp = getRegister16(SP);
	(void)mc.readInRegister(sp, rid);
	sp += 1 + is16register(rid);
}

uint16_t StackController::pop16(void)
{
	auto&& sp = getRegister16(SP);
	const auto toreturn = mc.read16(sp);
	sp += 2;
	return toreturn;
}

uint8_t StackController::pop8(void)
{
	return mc.read8(getRegister16(SP)++);
}
