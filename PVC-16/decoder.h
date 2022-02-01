#pragma once
#include <cstdint>

#include "opcode.h"

class Decoder
{
	static uint16_t readAddress(SIB sib, uint16_t disp);
public:
	static void processRR	(Opcode opcode, RegisterID r1, RegisterID r2);
	static void processRC	(Opcode opcode, RegisterID r1, uint16_t constant);
	static void processRM	(Opcode opcode, RegisterID r1, uint16_t addr);
	static void processR	(Opcode opcode, RegisterID r1);
	static void processMR	(Opcode opcode, uint16_t addr, RegisterID r1);
	static void processM	(Opcode opcode, uint16_t addr);
	static void processC8	(Opcode opcode, uint8_t constant);
	static void processC	(Opcode opcode, uint16_t constant);
	static void processJO	(Opcode opcode);

	static void process(void);
};



