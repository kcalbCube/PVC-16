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

	uint16_t updateStatus16(const unsigned);

	uint8_t updateStatus8(const unsigned);

	void write(const RegisterID, const uint16_t, const bool shouldUpdateStatus = false);

	uint16_t read(const RegisterID);

	uint16_t& get16(const RegisterID);

	uint8_t& get8(const RegisterID);

	void inc(const RegisterID);
	void dec(const RegisterID);
}
