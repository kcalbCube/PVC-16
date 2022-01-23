#pragma once
#include <cstdint>
#include "registers_define.h"
enum Opcode
{
	NOP,
	MOV_RR, // MOV_RR %dest8 %src8
	MOV_RC, // MOV_RC %dest8 [src]16

	MOV_RM, // MOV_RMÑ %dest8 %sib8(src) ?disp(src)16
	MOV_MR, // MOV_MCR %sib8(dest) %src8 ?disp(dest)16

	ADD, // ADD %dest8 %src8
	SUB, // SUB %dest8 %src8

	ADD_C, // ADD %dest8 %src16
	SUB_C, // SUB %dest8 %src16

	INC, // INC %dest8
	DEC, // DEC %dest8


	INT, // INT %int8

	CMP_RC, // CMP %a8 %src16


	JMP, // JMP %dest16

	JZ, // [jumpop] %dest16 
	JNZ,

	JG,
	JNG,
	JGZ,

	BRK = 0xCC, // BRK
};

#pragma pack(push, 1)
struct SIB
{
	uint8_t scale : 2; // 1 << scale
	uint8_t index : 3; // H registers
	uint8_t base  : 2; // 01 - BP, 02 - SP, 03 - reserved
	uint8_t disp  : 1; // 0 - no displacement, 1 - displacement present.

	// fluffy constructing.
	SIB(uint8_t scale, RegisterID index, RegisterID base, uint8_t disp)
	{
		this->scale = static_cast<uint8_t>(log2(scale));
		if(isH(index) || index == NO_REG)
			this->index = index == NO_REG ? 0 : (index + 1);
		else
			this->index = (index - AL) / 2;

		switch(base)
		{
		case BP:
			this->base = 1;
			break;
		case SP:
			this->base = 2;
			break;
		case NO_REG:
			this->base = 0;
			break;
		default: abort();
		}
		this->disp = disp;
	}
	SIB(void) = default;
};
#pragma pack(pop)
static_assert(sizeof(SIB) == 1);

inline bool isSIBbase(const RegisterID id)
{
	return id == BP || id == SP;
}

inline bool isSIBindex(const RegisterID id)
{
	return id < BP;
}

inline RegisterID getSIBbase(const SIB& sib)
{
	constexpr static RegisterID table[] = { BP, SP };
	return sib.base? table[sib.base - 1] : BP;
}

inline RegisterID getSIBindex(const SIB& sib)
{
	return static_cast<RegisterID>(sib.index - 1);
}

#define m1628(src) static_cast<uint8_t>(src), static_cast<uint8_t>((src) >> 8)
#define m1628h(src) static_cast<uint8_t>(src)
#define m1628l(src) static_cast<uint8_t>((src) >> 8)