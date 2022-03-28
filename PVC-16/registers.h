#pragma once
#include <cstdint>
#include "registers_define.h"

namespace registers
{
	inline struct StatusRegister
	{
		unsigned
			zero
			, sign
			, overflow
			, interrupt;
	} status;

	typedef uint8_t PackedStatus;

	PackedStatus packStatus(void);
	void unpackStatus(PackedStatus);

	uint16_t updateStatus16(const unsigned result);

	uint8_t updateStatus8(const unsigned result);

	void write(const RegisterID id, const uint16_t value, const bool shouldUpdateStatus = false);

	uint16_t read(const RegisterID id);

	uint16_t& get16(const RegisterID id);

	uint8_t& get8(const RegisterID id);
}