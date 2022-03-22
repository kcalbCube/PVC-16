#pragma once
#include <cstdint>

#include "mutility.h"
#include "registers_define.h"

enum Opcode
{
	NOP,
	MOV_RR,
	MOV_RC16,

	MOV_RM,
	MOV_MR,

	MOV_MC16,
	MOV_MC8,
	MOV_MM16,
	MOV_MM8,

	ADD,
	SUB,

	ADD_C16,
	SUB_C16,

	MUL,
	DIV,

	MUL_C16,
	DIV_C16,

	LEA,

	INC,
	DEC,


	INT,

	CMP_RC,
	CMP_RR,


	JMP,

	JZ,
	JNZ,

	JG,
	JNG,
	JGZ,

	JL,

	JA,
	JB,
	JNB,
	JBZ,


	PUSH_R,
	PUSH_C16,
	PUSH_C8,
	PUSHA,

	POPA,
	POP_R,
	POP,
	POP8,
	POP_M8,
	POP_M16,

	OUT_M8,
	OUT_M16,
	OUT_R,
	OUT_C8,
	OUT_C16,

	IN_M8,
	IN_M16,
	IN_R,

	NEG,

	RET,
	CALL,

	MEMSET,

	CLI,
	STI,

	LOOP,

	TEST_RC,
	TEST_RR,

	NOT,
	AND_RR,
	OR_RR,
	XOR_RR,
	SHL_RR,
	SHR_RR,
	MOD_RR,
	MOD_RC,

	REI,
	PUSHF,
	POPF,

	BRK = 0xCC,
};

enum OpcodeFormat
{
	OPCODE,
	OPCODE_RR,
	OPCODE_RC,
	OPCODE_RM,
	OPCODE_MR,
	OPCODE_MM,
	OPCODE_R,
	OPCODE_M,
	OPCODE_C,
	OPCODE_C8,
	OPCODE_MC,
	OPCODE_MC8,
	OPCODE_C8C,
	OPCODE_CC,
	OPCODE_MCC8,
	OPCODE_INVALID
};

#define OPCODEFORMAT_CASE(opcode, format) case opcode: return format; break
inline OpcodeFormat getOpcodeFormat(const Opcode opcode)
{
	switch (opcode)
	{
		OPCODEFORMAT_CASE(NOP, OPCODE);
		OPCODEFORMAT_CASE(MOV_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(MOV_RC16, OPCODE_RC);
		OPCODEFORMAT_CASE(MOV_RM, OPCODE_RM);
		OPCODEFORMAT_CASE(MOV_MR, OPCODE_MR);
		OPCODEFORMAT_CASE(MOV_MC16, OPCODE_MC);
		OPCODEFORMAT_CASE(MOV_MC8, OPCODE_MC8);
		OPCODEFORMAT_CASE(MOV_MM16, OPCODE_MM);
		OPCODEFORMAT_CASE(MOV_MM8, OPCODE_MM);
		OPCODEFORMAT_CASE(ADD, OPCODE_RR);
		OPCODEFORMAT_CASE(SUB, OPCODE_RR);
		OPCODEFORMAT_CASE(ADD_C16, OPCODE_RC);
		OPCODEFORMAT_CASE(SUB_C16, OPCODE_RC);
		OPCODEFORMAT_CASE(MUL, OPCODE_RR);
		OPCODEFORMAT_CASE(DIV, OPCODE_RR);
		OPCODEFORMAT_CASE(MUL_C16, OPCODE_RC);
		OPCODEFORMAT_CASE(DIV_C16, OPCODE_RC);
		OPCODEFORMAT_CASE(LEA, OPCODE_RM);
		OPCODEFORMAT_CASE(INC, OPCODE_R);
		OPCODEFORMAT_CASE(DEC, OPCODE_R);
		OPCODEFORMAT_CASE(INT, OPCODE_C8);
		OPCODEFORMAT_CASE(CMP_RC, OPCODE_RC);
		OPCODEFORMAT_CASE(JMP, OPCODE_C);
		OPCODEFORMAT_CASE(JZ, OPCODE_C);
		OPCODEFORMAT_CASE(JNZ, OPCODE_C);
		OPCODEFORMAT_CASE(JG, OPCODE_C);
		OPCODEFORMAT_CASE(JNG, OPCODE_C);
		OPCODEFORMAT_CASE(JGZ, OPCODE_C);
		OPCODEFORMAT_CASE(JL, OPCODE_C);
		OPCODEFORMAT_CASE(JB, OPCODE_C);
		OPCODEFORMAT_CASE(JNB, OPCODE_C);
		OPCODEFORMAT_CASE(JBZ, OPCODE_C);
		OPCODEFORMAT_CASE(JA, OPCODE_C);
		OPCODEFORMAT_CASE(CALL, OPCODE_C);
		OPCODEFORMAT_CASE(PUSH_R, OPCODE_R);
		OPCODEFORMAT_CASE(PUSH_C16, OPCODE_C);
		OPCODEFORMAT_CASE(PUSH_C8, OPCODE_C8);
		OPCODEFORMAT_CASE(POP_R, OPCODE_R);
		OPCODEFORMAT_CASE(POP, OPCODE);
		OPCODEFORMAT_CASE(POP8, OPCODE);
		OPCODEFORMAT_CASE(POP_M8, OPCODE_M);
		OPCODEFORMAT_CASE(POP_M16, OPCODE_M);
		OPCODEFORMAT_CASE(OUT_M8, OPCODE_MC);
		OPCODEFORMAT_CASE(OUT_M16, OPCODE_MC);
		OPCODEFORMAT_CASE(OUT_C8, OPCODE_C8C);
		OPCODEFORMAT_CASE(OUT_C16, OPCODE_CC);
		OPCODEFORMAT_CASE(OUT_R, OPCODE_RC);
		OPCODEFORMAT_CASE(IN_R, OPCODE_RC);
		OPCODEFORMAT_CASE(IN_M8, OPCODE_MC);
		OPCODEFORMAT_CASE(IN_M16, OPCODE_MC);
		OPCODEFORMAT_CASE(RET, OPCODE);
		OPCODEFORMAT_CASE(REI, OPCODE);
		OPCODEFORMAT_CASE(CLI, OPCODE);
		OPCODEFORMAT_CASE(STI, OPCODE);
		OPCODEFORMAT_CASE(PUSHF, OPCODE);
		OPCODEFORMAT_CASE(POPF, OPCODE);
		OPCODEFORMAT_CASE(POPA, OPCODE);
		OPCODEFORMAT_CASE(PUSHA, OPCODE);
		OPCODEFORMAT_CASE(NEG, OPCODE_R);
		OPCODEFORMAT_CASE(MEMSET, OPCODE_MCC8);
		OPCODEFORMAT_CASE(LOOP, OPCODE_RC);
		OPCODEFORMAT_CASE(TEST_RC, OPCODE_RC);
		OPCODEFORMAT_CASE(SHL_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(SHR_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(OR_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(CMP_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(MOD_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(MOD_RC, OPCODE_RC);
		OPCODEFORMAT_CASE(NOT, OPCODE_R);
		OPCODEFORMAT_CASE(AND_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(XOR_RR, OPCODE_RR);
		OPCODEFORMAT_CASE(TEST_RR, OPCODE_RR);
	default: return OPCODE_INVALID;
	}
	UNREACHABLE;
}
#undef OPCODEFORMAT_CASE


#pragma pack(push, 1)
struct SIB
{
	uint8_t scale : 2; // 1 << scale
	uint8_t index : 3; // H registers
	uint8_t base  : 2; // 01 - BP, 02 - SP, 03 - reserved
	uint8_t disp  : 1; // 0 - no displacement, 1 - displacement present.

	SIB(const uint8_t scale, const registers::RegisterID index, const registers::RegisterID base, const uint8_t disp) :
		scale(static_cast<uint8_t>(log2(scale)))
	{
		if (is16register(index) || index == registers::NO_REG)
			this->index = index == registers::NO_REG ? 0 : (index + 1);
		else
			this->index = (index - registers::AL) / 2;

		switch (base)
		{
		case registers::BP:
			this->base = 1;
			break;
		case registers::SP:
			this->base = 2;
			break;
		case registers::NO_REG:
			this->base = 0;
			break;
		default: UNREACHABLE;
		}
		this->disp = disp;
	}
	SIB(void) = default;

	static bool isBase(const registers::RegisterID id)
	{
		return id == registers::BP || id == registers::SP;
	}

	static bool isIndex(const registers::RegisterID id)
	{
		return id < registers::BP;
	}

	[[nodiscard]] registers::RegisterID getBase(void) const
	{
		constexpr registers::RegisterID table[] = { registers::BP, registers::SP };
		return base ? table[base - 1] : registers::NO_REG;
	}

	[[nodiscard]] registers::RegisterID getIndex(void) const
	{
		return static_cast<registers::RegisterID>(index - 1);
	}
};
#pragma pack(pop)
static_assert(sizeof(SIB) == 1);