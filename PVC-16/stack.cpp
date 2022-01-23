#include "stack.h"
#include "memory.h"

void StackController::push(const RegisterID rid)
{
	if (isH(rid))
		push16(readRegister(rid));
	else
		push8(readRegister(rid));
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
	mc.readInRegister(sp, rid);
	sp += 1 + isH(rid);
}

uint16_t StackController::pop16(void)
{
	auto&& sp = getRegister16(SP);
	auto toreturn = mc.read16(sp);
	sp += 2;
	return toreturn;
}

uint8_t StackController::pop8(void)
{
	return mc.read8(getRegister16(SP)++);
}
