#include "decoder.h"

#include <iostream>

#include "interrupt.h"

void process(void)
{
	switch (auto&& ip = getRegister16(IP); mc.read8(ip++))
	{
	case MOV_RC:
		{
			writeRegister(static_cast<RegisterID>(mc.read8(ip)), mc.read16(ip+1));
			ip += 3;
		}
		break;
	case MOV_RR:
		{
			writeRegister(static_cast<RegisterID>(mc.read8(ip)), readRegister(static_cast<RegisterID>(mc.read8(ip+1))));
			ip += 2;
		}
		break;

	case MOV_RM:
		{
			const auto id = static_cast<RegisterID>(mc.read8(ip));
			union
			{
				SIB sib;
				uint8_t u8;
			};
			u8 = mc.read8(ip + 1);

			const uint16_t disp = sib.disp ? mc.read16(ip + 2) : 0;
			(void)mc.readInRegister(readAddress(sib, disp), id);
			ip += 2 + sib.disp * 2;
		}
		break;

	case MOV_MR:
		{
			union
			{
				SIB sib;
				uint8_t u8;
			};
			u8 = mc.read8(ip);
			const auto id = static_cast<RegisterID>(mc.read8(ip + 1));

			const uint16_t disp = sib.disp ? mc.read16(ip + 2) : 0;
			mc.writeFromRegister(readAddress(sib, disp), id);
			ip += 2 + sib.disp * 2;
		}
		break;

	case ADD:
		{
			const auto dest = static_cast<RegisterID>(mc.read8(ip++));
			const auto src = static_cast<RegisterID>(mc.read8(ip++));
			writeRegister(dest, readRegister(dest) + readRegister(src), true);
		}
		break;

	case SUB:
		{
			const auto dest = static_cast<RegisterID>(mc.read8(ip++));
			const auto src = static_cast<RegisterID>(mc.read8(ip++));
			writeRegister(dest, readRegister(dest) - readRegister(src), true);
		}
		break;

	case INT:
		{
		interrupt(mc.read8(ip++));
		}
		break;
	default: break;
	}
}

uint16_t readAddress(SIB sib, const uint16_t disp)
{
	return static_cast<uint16_t>((sib.index ? readRegister(getSIBindex(sib)) : 0) * (1 << sib.scale) +
		readRegister(getSIBbase(sib)) + disp);
}
