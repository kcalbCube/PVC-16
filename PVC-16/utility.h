#pragma once
#include <cstdint>
#include <string>
#include "opcode.h"
void hexDump(const char* desc, void* addr, const size_t len, const size_t offset, const size_t lineSize = 16);

void registersDump(void);

// Disasm utilities
inline std::string renderIndirectAddress(const SIB sib, const uint16_t disp)
{
	std::string str = "[";
	char buffer[16]{};

	bool opRequired = false;
	if (sib.scale)
	{
		sprintf_s(buffer, "%d * ", 1 << sib.scale);
		str += buffer;
	}
	if (sib.base)
	{
		sprintf_s(buffer, opRequired ? "+ %%%s " : "%%%s ", registerId2registerName[sib.getBase()].c_str());
		str += buffer;
		opRequired = true;
	}
	if (sib.index)
	{
		sprintf_s(buffer, opRequired ? "+ %%%s " : "%%%s ", registerId2registerName[sib.getIndex()].c_str());
		str += buffer;
		opRequired = true;
	}
	if (sib.disp)
	{
		sprintf_s(buffer, opRequired ? "+ %04X " : "%04X ", disp);
		str += buffer;
		opRequired = true;
	}

	return str.substr(0, str.size() - 1) + "]";
}

// Memory utils
#define m1628(src) static_cast<uint8_t>(src), static_cast<uint8_t>((src) >> 8)
#define m1628h(src) static_cast<uint8_t>(src)
#define m1628l(src) static_cast<uint8_t>((src) >> 8)

