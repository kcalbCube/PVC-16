#pragma once

#include <map>
#include <string>
#include "map.h"


#define REGISTERS_LIST A, B, C, D, E, SI, BP, SP, IP
#define LREGISTERS_LIST AH, AL, BH, BL, CH, CL, DH, DL, EH, EL, SIH, SIL, BPH, BPL, SPH, SPL, IPH, IPL

enum RegisterID
{
	REGISTERS_LIST,
	LREGISTERS_LIST,
	NO_REG
};

inline bool isH(const RegisterID id)
{
	return id < AH;
}

const static std::string registerId2registerName[] = {
#define _MAP(x) #x
	MAP_LIST(_MAP, REGISTERS_LIST),
	MAP_LIST(_MAP, LREGISTERS_LIST)
};
#undef _MAP

const static std::map<std::string, RegisterID> registerName2registerId = {
#define _MAP(x) {#x, x}
	MAP_LIST(_MAP, REGISTERS_LIST),
	MAP_LIST(_MAP, LREGISTERS_LIST)
};
#undef _MAP