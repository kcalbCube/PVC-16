#pragma once
#include <cstdint>
#include "registers_define.h"

inline struct StatusRegister
{
	int zero:1;
	int greater:1;
	int sign:1;
	int interrupt:1;
	int overflow:1;


} status;

uint16_t updateStatus16(const unsigned result);

uint8_t updateStatus8(const unsigned result);

void writeRegister(const RegisterID id, const uint16_t value, const bool shouldUpdateStatus = false);

uint16_t readRegister(const RegisterID id);

uint16_t& getRegister16(const RegisterID id);

uint8_t& getRegister8(const RegisterID id);
