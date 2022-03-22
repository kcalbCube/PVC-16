#include "decoder.h"

#include <iostream>

#include "interrupt.h"
#include "utility.h"
#include "stack.h"
#include "vmflags.h"
#include "bus.h"
#include <magic_enum.hpp>

#pragma warning(disable: 4062)

uint16_t Decoder::readAddress(SIB sib, const uint16_t disp)
{
	return static_cast<uint16_t>(read(sib.getIndex()) * (1 << sib.scale) +
		read(sib.getBase()) + disp);
}

void Decoder::processRR(Opcode opcode, registers::RegisterID r1, registers::RegisterID r2)
{
	switch (opcode)
	{
	case MOV_RR:
		write(r1, read(r2));
		break;

	case OR_RR:
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) | read(r2));
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) | read(r2));
		}
		break;

	case ADD:
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) + read(r2));
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) + read(r2));
		}
		break;

	case SUB:
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) - read(r2));
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) - read(r2));
		}
		break;

	case MUL:
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) * read(r2));
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) * read(r2));
		}
		break;

	case DIV:
	{
		const auto r2v = read(r2);
		if (r2v == 0)
		{
			interrupt(interrupts::DE);
			break;
		}
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) / r2v);
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) / r2v);
		}
		break;
	}

	case MOD_RR:
	{
		const auto r2v = read(r2);
		if (r2v == 0)
		{
			interrupt(interrupts::DE);
			break;
		}
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) % r2v);
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) % r2v);
		}
		break;
	}

	case SHL_RR:
	{
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) << read(r2));
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) << read(r2));
		}
		break;
	}
	case SHR_RR:
	{
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) >> read(r2));
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) >> read(r2));
		}
		break;
	}

	case AND_RR:
	{
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) & read(r2));
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) & read(r2));
		}
		break;
	}

	case CMP_RR:
	{
		const auto result = read(r1) - read(r2);
		if (is16register(r1))
			registers::updateStatus16(result);
		else
			registers::updateStatus8(result);
	}
	break;
	default:
		UNREACHABLE;

				
	}
}

void Decoder::processRC(Opcode opcode, registers::RegisterID r1, uint16_t constant)
{
	switch (opcode)
	{
	case MOV_RC16:
		write(r1, constant);
		break;

	case CMP_RC:
	{
		const auto result = read(r1) - constant;

		if (is16register(r1))
			registers::updateStatus16(result);
		else
			registers::updateStatus8(result);
	}
	break;

	case ADD_C16:
	{
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) + constant);
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) + constant);
		}
		break;
	}
	case SUB_C16:
	{
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) - constant);
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) - constant);
		}
		break;
	}

	case MUL_C16:
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) * constant);
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) * constant);
		}
		break;

	case DIV_C16:
	{
		if (constant == 0)
		{
			interrupt(interrupts::DE);
			break;
		}
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) / constant);
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) / constant);
		}
		break;
	}

	case MOD_RC:
	{
		if (constant == 0)
		{
			interrupt(interrupts::DE);
			break;
		}
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			r = registers::updateStatus16(static_cast<unsigned>(r) % constant);
		}
		else
		{
			auto&& r = get8(r1);
			r = registers::updateStatus8(static_cast<unsigned>(r) % constant);
		}
		break;
	}

	case OUT_R:
		if (is16register(r1))
			busWrite16(constant, get16(r1));
		else
			busWrite(constant, get8(r1));
		break;

	case IN_R:
		if (is16register(r1))
			write(r1, busRead16(constant));
		else
			write(r1, busRead(constant));
		break;

	case LOOP:
	{
		if (is16register(r1))
		{
			if (--get16(r1))
				write(registers::IP, constant);
		}
		else
		{
			if (--get8(r1))
				write(registers::IP, constant);
		}
		break;
	}

	case TEST_RC:
		registers::status.zero = read(r1) & constant;
		break;
	default:
		UNREACHABLE;

	}
}

void Decoder::processRM(Opcode opcode, registers::RegisterID r1, uint16_t addr)
{
	switch (opcode)
	{
	case MOV_RM:
		(void)mc.readInRegister(addr, r1);
		break;
	case LEA:
		write(r1, addr);
		break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processR(Opcode opcode, registers::RegisterID r1)
{
	switch (opcode)
	{
	case INC:
	{
		if (is16register(r1))
			registers::updateStatus16(++get16(r1));
		else
			registers::updateStatus8(++get8(r1));
		break;
	}

	case NOT:
	{
		if (is16register(r1))
		{
			auto&& r = get16(r1);
			registers::updateStatus16(r = ~r);
		}
		else
		{
			auto&& r = get8(r1);
			registers::updateStatus8(r = ~r);
		}
		break;
	}

	case DEC:
	{
		if (is16register(r1))
			registers::updateStatus16(--get16(r1));
		else
			registers::updateStatus8(--get8(r1));
		break;
	}
	case NEG:
	{
		if (is16register(r1))
			registers::updateStatus16(static_cast<unsigned>(reinterpret_cast<short&>(get16(r1)) *= -1));
		else
			registers::updateStatus8(static_cast<unsigned>(reinterpret_cast<int8_t&>(get8(r1)) *= -1));
		break;
	}

	case POP_R:
	{
		stack::pop(r1);
		break;
	}
	case PUSH_R:
	{
		stack::push(r1);
		break;
	}
	case POPF:
	{
		stack::popf();
		break;
	}
	case PUSHF:
	{
		stack::pushf();
		break;
	}
	case PUSHA:
	{
		stack::push(registers::A);
		stack::push(registers::B);
		stack::push(registers::C);
		stack::push(registers::D);
		stack::push(registers::E);
		stack::push(registers::SI);
		break;
	}
	case POPA:
	{
		stack::pop(registers::SI);
		stack::pop(registers::E);
		stack::pop(registers::D);
		stack::pop(registers::C);
		stack::pop(registers::B);
		stack::pop(registers::A);
		break;
	}
	default:
		UNREACHABLE;
	}
}

void Decoder::processMR(const Opcode opcode, const uint16_t addr, const registers::RegisterID r1)
{
	switch (opcode)
	{
	case MOV_MR:
		mc.writeFromRegister(addr, r1);
		break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processM(Opcode opcode, uint16_t addr)
{
	switch (opcode)
	{
	case POP_M8:
	{
		mc.write8(addr, stack::pop8());
	}
	break;

	case POP_M16:
	{
		mc.write16(addr, stack::pop16());
	}
	break;
	default:
		UNREACHABLE;

	}
}

void Decoder::processC8(Opcode opcode, uint8_t constant)
{
	switch (opcode)
	{
	case PUSH_C8:
	{
		stack::push8(constant);
	}
	break;

	case INT:
	{
		interrupts::interrupt(constant);
	}
	break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processC(Opcode opcode, uint16_t constant)
{
	using namespace registers;
	switch (opcode)
	{
		case PUSH_C16:
		{
			stack::push16(constant);
		}
		break;

		case JMP:
		{
			write(IP, constant);
		}
		break;
		case JZ:
		{
			if (status.zero)
				write(IP, constant);
		}
		break;
		case JNZ:
		{
			if (!status.zero)
				write(IP, constant);
		}
		break;
		case JG:
		{
			if (!status.zero && status.sign == status.overflow)
				write(IP, constant);
		}
		break;
		case JNG:
		{
			if (status.zero || status.sign != status.overflow)
				write(IP, constant);
		}
		break;
		case JGZ:
		{
			if (status.sign == status.overflow)
				write(IP, constant);
		}
		break;
		case JL:
		{
			if (status.sign != status.overflow)
				write(IP, constant);
		}
		break;
		case JB:
		{
			if (status.overflow)
				write(IP, constant);
		}
		break;
		case JBZ:
		{
			if (status.overflow || status.zero)
				write(IP, constant);
		}
		break;
		case JNB:
		{
			if (!status.overflow)
				write(IP, constant);
		}
		break;
		case JA:
		{
			if (!status.sign && !status.overflow)
				write(IP, constant);
		}
		break;
		case CALL:
		{
			auto& ip = get16(IP);
			stack::push16(ip);
			ip = constant;
		}
		break;

		default:
			UNREACHABLE;
	}
}

void Decoder::processJO(Opcode opcode)
{
	switch (opcode)
	{
	case POP:
	{
		(void)stack::pop16();
	}
	break;
	case POP8:
	{
		(void)stack::pop8();
	}
	break;
	case REI:
		stack::popf();
		stack::pop(registers::IP);
		break;
	case RET:
		stack::pop(registers::IP);
	break;
	case STI:
	{
		registers::status.interrupt = 1;
	}
	break;
	case CLI:
	{
		registers::status.interrupt = 0;
	}
	break;
	case BRK:
	{
		registersDump();
	}
	break;
	case NOP: break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processCC(Opcode opcode, uint16_t c1, uint16_t c2)
{
	switch (opcode)
	{
	case OUT_C16:
	{
		busWrite16(c2, c1);
	}
	break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processC8C(Opcode opcode, uint8_t c1, uint16_t c2)
{
	switch (opcode)
	{
	case OUT_C8:
	{
		busWrite(c2, c1);
	}
	break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processMM(Opcode opcode, uint16_t addr1, uint16_t addr2)
{
	switch (opcode)
	{
	case MOV_MM16:
	{
		mc.write16(addr1, mc.read16(addr2));
	}
	break;
	case MOV_MM8:
	{
		mc.write8(addr1, mc.read8(addr2));
	}
	break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processMC(Opcode opcode, uint16_t addr, uint16_t constant)
{
	switch (opcode)
	{
	case MOV_MC16:
		mc.write16(addr, constant);
		break;

	case OUT_M8:
		busWrite(constant, mc.read8(addr));
		break;
	case OUT_M16:
		busWrite16(constant, mc.read16(addr));
		break;
	case IN_M8:
		mc.write8(addr, busRead(constant));
		break;
	case IN_M16:
		mc.write16(addr, busRead16(constant));
		break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processMC8(Opcode opcode, uint16_t addr, uint8_t constant)
{
	switch (opcode)
	{
	case MOV_MC8:
		mc.write8(addr, constant);
		break;
	default:
		UNREACHABLE;
	}
}

void Decoder::processMCC8(Opcode opcode, uint16_t addr, uint16_t c1, uint8_t c2)
{
	switch (opcode)
	{
	case MEMSET:
		memset(mc.data + addr, c2, c1);
		break;
	default:
		UNREACHABLE;
	}
}

void Decoder::process(void)
{
	auto&& ip = get16(registers::IP);
	const auto opcode = static_cast<Opcode>(mc.read8(ip++));
	
#ifdef ENABLE_WORKFLOW
	if (vmflags.workflowEnabled)
	{
		if(auto&& opc = magic_enum::enum_name(opcode); opc.empty())
			printf("%04X: %X ", (ip - 1), opcode);
		else
			printf("%04X: %s ", (ip - 1), std::string(opc).c_str());
	}
#endif

	switch (getOpcodeFormat(opcode))
	{
	case OPCODE_RR:
	{
		const auto r1 = static_cast<registers::RegisterID>(mc.read8(ip++));
		const auto r2 = static_cast<registers::RegisterID>(mc.read8(ip++));
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%%%s %%%s\n", registers::registerId2registerName[r1].c_str(), registers::registerId2registerName[r2].c_str());
#endif
		processRR(opcode, r1, r2);
	}
	break;

	case OPCODE_RC:
	{
		const auto r1 = static_cast<registers::RegisterID>(mc.read8(ip++));
		const auto c = mc.read16(ip);
		ip += 2;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%%%s %04X\n", registers::registerId2registerName[r1].c_str(), c);
#endif
		processRC(opcode, r1, c);
	}
	break;

	case OPCODE_R:
	{
		const auto r1 = static_cast<registers::RegisterID>(mc.read8(ip++));
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%%%s\n", registers::registerId2registerName[r1].c_str());
#endif
		processR(opcode, r1);
	}
	break;

	case OPCODE_RM:
	{
		const auto r1 = static_cast<registers::RegisterID>(mc.read8(ip++));
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));

		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		const uint16_t addr = readAddress(sib, disp);
		ip += sib.disp ? 2 : 0;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%%%s %s{%04X}\n", registers::registerId2registerName[r1].c_str(), renderIndirectAddress(sib, disp).c_str(), addr);
#endif
		processRM(opcode, r1, addr);
	}
	break;

	case OPCODE_MC:
	{
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));
		const auto c = mc.read16(ip);
		ip += 2;
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		const uint16_t addr = readAddress(sib, disp);
		ip += sib.disp ? 2 : 0;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %04X\n", renderIndirectAddress(sib, disp).c_str(), addr, c);
#endif
		processMC(opcode, addr, c);
	}
	break;

	case OPCODE_MC8:
	{
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));
		const auto c = mc.read8(ip++);
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		const uint16_t addr = readAddress(sib, disp);
		ip += sib.disp ? 2 : 0;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %04X\n", renderIndirectAddress(sib, disp).c_str(), addr, c);
#endif
		processMC8(opcode, addr, c);
	}
	break;

	case OPCODE_MR:
	{
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));
		const auto r1  = static_cast<registers::RegisterID>(mc.read8(ip++));
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		const uint16_t addr = readAddress(sib, disp);
		ip += sib.disp ? 2 : 0;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %%%s\n", renderIndirectAddress(sib, disp).c_str(), addr, registers::registerId2registerName[r1].c_str());
#endif
		processMR(opcode, addr, r1);

	}
	break;

	case OPCODE_M:
	{
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		const uint16_t addr = readAddress(sib, disp);
		ip += sib.disp ? 2 : 0;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X}\n", renderIndirectAddress(sib, disp).c_str(), addr);
#endif
		processM(opcode, addr);

	}
	break;

	case OPCODE_MM:
	{
		const auto sib1 = std::bit_cast<SIB>(mc.read8(ip++));
		const auto sib2 = std::bit_cast<SIB>(mc.read8(ip++));
		const uint16_t disp1 = sib1.disp ? mc.read16(ip) : 0;
		ip += sib1.disp ? 2 : 0;
		const uint16_t disp2 = sib2.disp ? mc.read16(ip) : 0;
		ip += sib2.disp ? 2 : 0;
		const uint16_t addr1 = readAddress(sib1, disp1);
		const uint16_t addr2 = readAddress(sib2, disp2);
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %s{%04X}\n", renderIndirectAddress(sib1, disp1).c_str(), addr1, renderIndirectAddress(sib2, disp2).c_str(), addr2);
#endif
		processMM(opcode, addr1, addr2);

	}
	break;

	case OPCODE_C8:
	{
		const auto c8 = mc.read8(ip++);
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%02X\n", c8);
#endif
		processC8(opcode, c8);
	}
	break;

	case OPCODE_C:
	{
		const auto c = mc.read16(ip);
		ip += 2;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%04X\n", c);
#endif
		processC(opcode, c);
	}
	break;

	case OPCODE_CC:
	{
		const auto c1 = mc.read16(ip);
		const auto c2 = mc.read16(ip + 2);
		ip += 4;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%04X %04X\n", c1, c2);
#endif
		processCC(opcode, c1, c2);
	}
	break;

	case OPCODE_C8C:
	{
		const auto c1 = mc.read8(ip++);
		const auto c2 = mc.read16(ip);
		ip += 2;

#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%02X %04X\n", c1, c2);
#endif
		processC8C(opcode, c1, c2);
	}
	break;

	case OPCODE_MCC8:
	{
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));
		const uint16_t c1 = mc.read16(ip); ip += 2;
		const uint8_t c2 = mc.read8(ip++);
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		ip += sib.disp ? 2 : 0;

		const uint16_t addr = readAddress(sib, disp);

#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %04X %02X\n", renderIndirectAddress(sib, disp).c_str(), addr, c1, c2);
#endif
		processMCC8(opcode, addr, c1, c2);
	}
	break;

	case OPCODE:
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("\n");
#endif
		processJO(opcode);
		break;

	case OPCODE_INVALID:
		printf("%04X - not handled op %02X\n", ip-1, opcode);
		break;
	default:
		UNREACHABLE;
	}
}
