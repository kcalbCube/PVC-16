#pragma once
#include <cstdint>
#include "registers_define.h"
enum Opcode
{
	NOP,
	MOV_RR,
	MOV_RC,

	MOV_RM,
	MOV_MR,

	ADD,
	SUB,

	ADD_C,
	SUB_C,

	INC,
	DEC,


	INT,

	CMP_RC,


	JMP,

	JZ,
	JNZ,

	JG,
	JNG,
	JGZ,

	JL,


	PUSH_R,
	PUSH_C,
	PUSH_C8,

	POP_R,
	POP,
	POP8,
	POP_M8,
	POP_M16,

	RET,
	CALL,


	BRK = 0xCC,
};

enum OpcodeFormat
{
	OPCODE,
	OPCODE_RR,
	OPCODE_RC,
	OPCODE_RM,
	OPCODE_MR,
	OPCODE_R,
	OPCODE_M,
	OPCODE_C,
	OPCODE_C8
};

#define OPCODEFORMAT_CASE(opcode, format) case opcode: return format; break
__forceinline OpcodeFormat getOpcodeFormat(Opcode opcode)
{
	switch (opcode)
	{
		OPCODEFORMAT_CASE(NOP	, OPCODE);
		OPCODEFORMAT_CASE(MOV_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(MOV_RC, OPCODE_RC);
		OPCODEFORMAT_CASE(MOV_RM, OPCODE_RM);
		OPCODEFORMAT_CASE(MOV_MR, OPCODE_MR);
		OPCODEFORMAT_CASE(ADD	, OPCODE_RR);
		OPCODEFORMAT_CASE(SUB	, OPCODE_RR);
		OPCODEFORMAT_CASE(ADD_C	, OPCODE_RC);
		OPCODEFORMAT_CASE(SUB_C	, OPCODE_RC);
		OPCODEFORMAT_CASE(INC	, OPCODE_R);
		OPCODEFORMAT_CASE(DEC	, OPCODE_R);
		OPCODEFORMAT_CASE(INT	, OPCODE_C8);
		OPCODEFORMAT_CASE(CMP_RC, OPCODE_RC);
		OPCODEFORMAT_CASE(JMP	, OPCODE_C);
		OPCODEFORMAT_CASE(JZ	, OPCODE_C);
		OPCODEFORMAT_CASE(JNZ	, OPCODE_C);
		OPCODEFORMAT_CASE(JG	, OPCODE_C);
		OPCODEFORMAT_CASE(JNG	, OPCODE_C);
		OPCODEFORMAT_CASE(JGZ	, OPCODE_C);
		OPCODEFORMAT_CASE(JL	, OPCODE_C);
		OPCODEFORMAT_CASE(CALL	, OPCODE_C);
		OPCODEFORMAT_CASE(PUSH_R, OPCODE_R);
		OPCODEFORMAT_CASE(PUSH_C, OPCODE_C);
		OPCODEFORMAT_CASE(PUSH_C8, OPCODE_C8);
		OPCODEFORMAT_CASE(POP_R	, OPCODE_R);
		OPCODEFORMAT_CASE(POP	, OPCODE);
		OPCODEFORMAT_CASE(POP8	, OPCODE);
		OPCODEFORMAT_CASE(POP_M8, OPCODE_M);
		OPCODEFORMAT_CASE(POP_M16, OPCODE_M);
		OPCODEFORMAT_CASE(RET	, OPCODE);

	default: return (OpcodeFormat)0xFF; break;
	}
}
#undef OPCODEFORMAT_CASE
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
		if(is16register(index) || index == NO_REG)
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