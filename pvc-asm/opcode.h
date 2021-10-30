#pragma once
enum Opcode
{
	NOP,
	MOV_RR, // MOV_RR %dest8 %src8 | mov %a %b
	MOV_RC, // MOV_RC %dest8 [src]16 | mov %a 0xFFh

	MOV_RMC, // MOV_RMÑ %dest8 [src]16| mov %a FFh
	MOV_MCR, // MOV_MCR [dest]16 %src8 | mov FFh %a

	ADD, // ADD %dest8 %src8 | add %a %b
	SUB, // SUB %dest8 %src8 | sub %a %b

	INT, // INT %int8 | int 0h
};

#define m1628(src) static_cast<uint8_t>(src), static_cast<uint8_t>((src) >> 8)
#define m1628h(src) static_cast<uint8_t>(src)
#define m1628l(src) static_cast<uint8>((src) >> 8)
#define NOP() 0x00
#define MOV_RR(dest, src) MOV_RR, dest, src
#define MOV_RC(dest, src) MOV_RC, dest, m1628(src)
#define MOV_RMC(dest, src) MOV_RMC, dest, m1628(src)
#define MOV_MCR(dest, src) MOV_MCR, m1628(dest), src
#define ADD(dest, src) ADD, dest, src
#define SUB(dest, src) SUB, dest, src
#define INT(interrupt) INT, interrupt