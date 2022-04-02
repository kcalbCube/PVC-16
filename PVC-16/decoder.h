#pragma once
#include <cstdint>
#include <bit>
#include "opcode.h"
#include "vmflags.h"
#include <deque>

class Decoder
{
	uint16_t readAddress(SIB sib, uint16_t disp);
public:
	void processRR	(Opcode opcode, registers::RegisterID r1, registers::RegisterID r2);
	void processRC	(Opcode opcode, registers::RegisterID r1, uint16_t constant);
	void processRM	(Opcode opcode, registers::RegisterID r1, uint16_t addr);
	void processR	(Opcode opcode, registers::RegisterID r1);
	void processMR	(Opcode opcode, uint16_t addr			, registers::RegisterID r1);
	void processMM	(Opcode opcode, uint16_t addr1			, uint16_t addr2);
	void processMC	(Opcode opcode, uint16_t addr			, uint16_t constant);
	void processMC8	(Opcode opcode, uint16_t addr			, uint8_t constant);
	void processM	(Opcode opcode, uint16_t addr);
	void processC8	(Opcode opcode, uint8_t  constant);
	void processC	(Opcode opcode, uint16_t constant);
	void processCC	(Opcode opcode, uint16_t c1				, uint16_t c2);
	void processC8C	(Opcode opcode, uint8_t  c1				, uint16_t c2);
	void processJO	(Opcode opcode);
	void processMCC8 (Opcode opcode, uint16_t addr			, uint16_t c1, uint8_t c2);

#ifdef ENABLE_OPCODE_STATISTICS
	struct StatisticsElement
	{
		Opcode opcode;
		int_fast16_t prefixes = 0;
		unsigned nanosec = 0;
	};
	std::deque<StatisticsElement> statistics;
#endif

	uint_fast16_t prefix = 0;
	int prefixes = 0;
	void process(void);
};



