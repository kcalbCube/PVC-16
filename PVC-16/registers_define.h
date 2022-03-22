#pragma once

#include <map>
#include <string>
#include "map.h"


#define REGISTERS_LIST A, B, C, D, E, SI, BP, SP, IP
#define LREGISTERS_LIST AH, AL, BH, BL, CH, CL, DH, DL, EH, EL, SIH, SIL, BPH, BPL, SPH, SPL, IPH, IPL

namespace registers
{
	enum RegisterID : unsigned
	{
		REGISTERS_LIST,
		LREGISTERS_LIST,
		NO_REG
	};

	inline bool is16register(const RegisterID id)
	{
		return id < AH;
	}
#define _MAP(x) #x
	const static std::string registerId2registerName[] = 
	{
		MAP_LIST(_MAP, REGISTERS_LIST),
		MAP_LIST(_MAP, LREGISTERS_LIST)
	};
#undef _MAP

#define _MAP(x) {#x, x}
	const static std::map<std::string, RegisterID> registerName2registerId = 
	{
		MAP_LIST(_MAP, REGISTERS_LIST),
		MAP_LIST(_MAP, LREGISTERS_LIST)
	};
#undef _MAP
}