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

	case MOV_RMC:
		{
			const auto id = static_cast<RegisterID>(mc.read8(ip));
			(void)mc.readInRegister(mc.read16(ip + 1), id);
			ip += 3;
		}
		break;

	case MOV_MCR:
		{
			const addr_t addr = mc.read16(ip);
			const auto id = static_cast<RegisterID>(mc.read8(ip + 2));
			mc.writeFromRegister(addr, id);
			ip += 3;
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
