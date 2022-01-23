#pragma once
#include "registers.h"

class StackController
{
public:
	static void push(const RegisterID rid);
	static void push16(const uint16_t src);
	static void push8(const uint8_t src);
	static void pop(const RegisterID rid);
	static uint16_t pop16(void);
	static uint8_t pop8(void);
};