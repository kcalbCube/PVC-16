#include "decoder.h"

#include <iostream>

#include "interrupt.h"
#include "utility.h"
#include "stack.h"

void process(void)
{
	switch (auto&& ip = getRegister16(IP); static_cast<Opcode>(mc.read8(ip++)))
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

	case BRK:
		{
		registersDump();
		}
		break;

	case ADD_C: 
		{
			const auto dest = static_cast<RegisterID>(mc.read8(ip++));
			writeRegister(dest, readRegister(dest) + mc.read16(ip), true);
			ip += 2;
		}
		break;
	case SUB_C:
		{
			const auto dest = static_cast<RegisterID>(mc.read8(ip++));
			writeRegister(dest, readRegister(dest) - mc.read16(ip), true);
			ip += 2;
		}
		break;
	case INC: 
		{
			const auto dest = static_cast<RegisterID>(mc.read8(ip++));
			writeRegister(dest, readRegister(dest) + 1, true);
		}
		break;
	case DEC:
		{
			const auto dest = static_cast<RegisterID>(mc.read8(ip++));
			writeRegister(dest, readRegister(dest) - 1, true);
		}
		break;
	case CMP_RC: 
		{
		const auto a = static_cast<RegisterID>(mc.read8(ip++));
		const auto b = mc.read16(ip);
		const auto result = readRegister(a) - b;
		if (isH(a))
			updateStatus(static_cast<uint16_t>(result));
		else
			updateStatus(static_cast<uint8_t>(result));
		status.greater = result > 0;

		ip += 2;
		}
		break;

	case JMP:
	{
		const auto dest = mc.read16(ip);
		ip += 2;
		writeRegister(IP, dest);
	}
		break;
	case JZ:
	{
		const auto dest = mc.read16(ip);
		ip += 2;
		if(status.zero)
			writeRegister(IP, dest);
	}
	break;
	case JNZ:
	{
		const auto dest = mc.read16(ip);
		ip += 2;
		if(!status.zero)
			writeRegister(IP, dest);
	}
	break;
	case JG:
	{
		const auto dest = mc.read16(ip);
		ip += 2;
		if(status.greater)
			writeRegister(IP, dest);
	}
	break;
	case JNG:
	{
		const auto dest = mc.read16(ip);
		ip += 2;
		if(!status.greater)
			writeRegister(IP, dest);
	}
	break;
	case JGZ:
	{
		const auto dest = mc.read16(ip);
		ip += 2;

		if(status.greater || status.zero)
			writeRegister(IP, dest);
	}
	break;
	case JL:
	{
		const auto dest = mc.read16(ip);
		ip += 2;

		if (!status.greater && !status.zero)
		{
			writeRegister(IP, dest);
		}
	}
	break;
	case PUSH_R:
	{
		StackController::push(static_cast<RegisterID>(mc.read8(ip++)));
	}
	break;
	case PUSH_C8:
	{
		StackController::push8(mc.read8(ip++));
	}
	break;

	case PUSH_C:
	{
		StackController::push16(mc.read16(ip)); ip += 2;
	}
	break;

	case POP_M8:
	{
		union
		{
			SIB sib;
			uint8_t u8;
		};
		u8 = mc.read8(ip++);
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		mc.write8(readAddress(sib, disp), StackController::pop8());
		if (sib.disp)
			ip += 2;
	}
	break;

	case POP_M16:
	{
		union
		{
			SIB sib;
			uint8_t u8;
		};
		u8 = mc.read8(ip++);
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		mc.write16(readAddress(sib, disp), StackController::pop16());
		if (sib.disp)
			ip += 2;
	}
	break;

	case POP_R:
	{
		StackController::pop(static_cast<RegisterID>(mc.read8(ip++)));
	}
	break;

	case POP:
	{
		(void)StackController::pop16();
	}
	break;
	case POP8:
	{
		(void)StackController::pop8();
	}
	break;

	case RET:
	{
		StackController::pop(IP);
	}
	break;

	case CALL:
	{
		auto dest = mc.read16(ip);
		ip += 2;
		StackController::push16(ip);
		writeRegister(IP, dest);
	}
	break;
	case NOP: break;
	default: break;
	}
}

uint16_t readAddress(SIB sib, const uint16_t disp)
{
	return static_cast<uint16_t>((sib.index ? readRegister(getSIBindex(sib)) : 0) * (1 << sib.scale) +
		readRegister(getSIBbase(sib)) + disp);
}
