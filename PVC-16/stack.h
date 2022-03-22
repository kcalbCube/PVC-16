#pragma once
#include "registers_define.h"
namespace stack
{
	void pushf(void);
	void popf(void);
	void push(const registers::RegisterID);
	void push16(const uint16_t);
	void push8(const uint8_t);
	void pop(const registers::RegisterID);
	uint16_t pop16(void);
	uint8_t pop8(void);
}
