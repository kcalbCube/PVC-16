#include "decoder.h"

#include <iostream>

#include "interrupt.h"
#include "utility.h"
#include "stack.h"
#include "vmflags.h"
#include "bus.h"

#include <magic_enum.hpp>

uint16_t Decoder::readAddress(SIB sib, const uint16_t disp)
{
	return static_cast<uint16_t>((sib.index ? readRegister(getSIBindex(sib)) : 0) * (1 << sib.scale) +
		readRegister(getSIBbase(sib)) + disp);
}

void Decoder::processRR(Opcode opcode, RegisterID r1, RegisterID r2)
{
	switch (opcode)
	{
	case MOV_RR:
		writeRegister(r1, readRegister(r2));
		break;

	case ADD:
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) += readRegister(r2)));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) += readRegister(r2)));
		break;

	case SUB:
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) -= readRegister(r2)));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) -= readRegister(r2)));
		break;

	case MUL:
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) += readRegister(r2)));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) += readRegister(r2)));
		break;

	case DIV:
	{
		auto r2v = readRegister(r2);
		if (r2v == 0)
			interrupt(DE);
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) /= r2v));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) /= r2v));
		break;
	}
				
	}
}

void Decoder::processRC(Opcode opcode, RegisterID r1, uint16_t constant)
{
	switch (opcode)
	{
	case MOV_RC16:
		writeRegister(r1, constant);
		break;

	case CMP_RC:
	{
		const auto result = readRegister(r1) - constant;
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(result));
		else
			updateStatus(static_cast<uint8_t>(result));
		status.greater = result > 0;
	}
	break;

	case ADD_C16:
	{
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) += constant));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) += constant));
		break;
	}
	case SUB_C16:
	{
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) -= constant));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) -= constant));
		break;
	}

	case MUL_C16:
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) += constant));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) += constant));
		break;

	case DIV_C16:
	{
		/*
		if (constant == 0)
			interrupt(DE);
		*/
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) /= constant));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) /= constant));
		break;
	}

	case OUT_R:
		if (is16register(r1))
			busWrite16(constant, getRegister16(r1));
		else
			busWrite(constant, getRegister8(r1));
		break;

	case IN_R:
		if (is16register(r1))
			writeRegister(r1, busRead16(constant));
		else
			writeRegister(r1, busRead(constant));
		break;


	}
}

void Decoder::processRM(Opcode opcode, RegisterID r1, uint16_t addr)
{
	switch (opcode)
	{
	case MOV_RM:
		mc.readInRegister(addr, r1);
		break;
	case LEA:
		writeRegister(r1, addr);
		break;
	}
}

void Decoder::processR(Opcode opcode, RegisterID r1)
{
	switch (opcode)
	{
	case INC:
	{
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(++getRegister16(r1)));
		else
			updateStatus(static_cast<uint8_t>(++getRegister8(r1)));
		break;
	}
	case DEC:
	{
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(--getRegister16(r1)));
		else
			updateStatus(static_cast<uint8_t>(--getRegister8(r1)));
		break;
	}

	case POP_R:
	{
		StackController::pop(r1);
		break;
	}
	case PUSH_R:
	{
		StackController::push(r1);
		break;
	}
	}
}

void Decoder::processMR(Opcode opcode, uint16_t addr, RegisterID r1)
{
	switch (opcode)
	{
	case MOV_MR:
		mc.writeFromRegister(addr, r1);
		break;
	}
}

void Decoder::processM(Opcode opcode, uint16_t addr)
{
	switch (opcode)
	{
	case POP_M8:
	{
		mc.write8(addr, StackController::pop8());
	}
	break;

	case POP_M16:
	{
		mc.write16(addr, StackController::pop16());
	}
	break;


	}
}

void Decoder::processC8(Opcode opcode, uint8_t constant)
{
	switch (opcode)
	{
	case PUSH_C8:
	{
		StackController::push8(constant);
	}
	break;

	case INT:
	{
		interrupt(constant);
	}
	break;
	}
}

void Decoder::processC(Opcode opcode, uint16_t constant)
{
	switch (opcode)
	{
		case PUSH_C16:
		{
			StackController::push16(constant);
		}
		break;

		case JMP:
		{
			writeRegister(IP, constant);
		}
		break;
		case JZ:
		{
			if (status.zero)
				writeRegister(IP, constant);
		}
		break;
		case JNZ:
		{
			if (!status.zero)
				writeRegister(IP, constant);
		}
		break;
		case JG:
		{
			if (status.greater)
				writeRegister(IP, constant);
		}
		break;
		case JNG:
		{
			if (!status.greater)
				writeRegister(IP, constant);
		}
		break;
		case JGZ:
		{
			if (status.greater || status.zero)
				writeRegister(IP, constant);
		}
		break;
		case JL:
		{
			if (!status.greater && !status.zero)
				writeRegister(IP, constant);
		}
		break;


		case CALL:
		{
			auto& ip = getRegister16(IP);
			StackController::push16(ip);
			ip = constant;
		}
		break;
	}
}

void Decoder::processJO(Opcode opcode)
{
	switch (opcode)
	{
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
	case BRK:
	{
		registersDump();
	}
	break;
	case NOP: break;
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
		busWrite(constant, mc.read16(addr));
		break;
	case IN_M8:
		mc.write8(addr, busRead(constant));
		break;
	case IN_M16:
		mc.write16(addr, busRead16(constant));
		break;
	}
}

void Decoder::processMC8(Opcode opcode, uint16_t addr, uint8_t constant)
{
	switch (opcode)
	{
	case MOV_MC8:
		mc.write8(addr, constant);
		break;
	}
}

void Decoder::process(void)
{
	auto&& ip = getRegister16(IP); 
	auto opcode = static_cast<Opcode>(mc.read8(ip++));
	
#ifdef ENABLE_WORKFLOW
	if (vmflags.workflowEnabled)
	{
		if(auto&& opc = magic_enum::enum_name(opcode); opc.empty())
			printf("%04X: %02-6X ", (unsigned int)(ip - 1), (unsigned int)opcode);
		else
			printf("%04X: %-6s ", (unsigned int)(ip - 1), std::string(opc).c_str());
	}
#endif

	switch (getOpcodeFormat(opcode))
	{
	case OPCODE_RR:
	{
		const auto r1 = static_cast<RegisterID>(mc.read8(ip++));
		const auto r2 = static_cast<RegisterID>(mc.read8(ip++));
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%%%s %%%s\n", registerId2registerName[r1].c_str(), registerId2registerName[r2].c_str());
#endif
		processRR(opcode, r1, r2);
	}
	break;

	case OPCODE_RC:
	{
		const auto r1 = static_cast<RegisterID>(mc.read8(ip++));
		const auto c = mc.read16(ip);
		ip += 2;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%%%s %04X\n", registerId2registerName[r1].c_str(), (unsigned int)c);
#endif
		processRC(opcode, r1, c);
	}
	break;

	case OPCODE_R:
	{
		const auto r1 = static_cast<RegisterID>(mc.read8(ip++));
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%%%s\n", registerId2registerName[r1].c_str());
#endif
		processR(opcode, r1);
	}
	break;

	case OPCODE_RM:
	{
		const auto r1 = static_cast<RegisterID>(mc.read8(ip++));
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));

		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		const uint16_t addr = readAddress(sib, disp);
		ip += sib.disp * 2;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%%%s %s{%04X}\n", registerId2registerName[r1].c_str(), renderIndirectAddress(sib, disp).c_str(), (unsigned int)addr);
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
		ip += sib.disp * 2;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %04X\n", renderIndirectAddress(sib, disp).c_str(), (unsigned int)addr, c);
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
		ip += sib.disp * 2;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %04X\n", renderIndirectAddress(sib, disp).c_str(), (unsigned int)addr, c);
#endif
		processMC8(opcode, addr, c);
	}
	break;

	case OPCODE_MR:
	{
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));
		const auto r1  = static_cast<RegisterID>(mc.read8(ip++));
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		const uint16_t addr = readAddress(sib, disp);
		ip += sib.disp * 2;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %%%s\n", renderIndirectAddress(sib, disp).c_str(), (unsigned int)addr, registerId2registerName[r1].c_str());
#endif
		processMR(opcode, addr, r1);

	}
	break;

	case OPCODE_MM:
	{
		const auto sib1 = std::bit_cast<SIB>(mc.read8(ip++));
		const auto sib2 = std::bit_cast<SIB>(mc.read8(ip++));
		const uint16_t disp1 = sib1.disp ? mc.read16(ip) : 0;
		ip += sib1.disp * 2;
		const uint16_t disp2 = sib2.disp ? mc.read16(ip) : 0;
		ip += sib2.disp * 2;
		const uint16_t addr1 = readAddress(sib1, disp1);
		const uint16_t addr2 = readAddress(sib1, disp1);
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %s{%04X}\n", renderIndirectAddress(sib1, disp1).c_str(), (unsigned int)addr1, renderIndirectAddress(sib2, disp2).c_str(), (unsigned int)addr2);
#endif
		processMM(opcode, addr1, addr2);

	}
	break;

	case OPCODE_C8:
	{
		const auto c8 = mc.read8(ip++);
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%02X\n", (unsigned int)c8);
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
			printf("%04X\n", (unsigned int)c);
#endif
		processC(opcode, c);
	}
	break;

	case OPCODE_CC:
	{
		const auto c1 = mc.read16(ip);
		ip += 2;
		const auto c2 = mc.read16(ip);
		ip += 2;
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("%04X %04X\n", (unsigned int)c1, (unsigned int)c2);
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
			printf("%02X %04X\n", (unsigned int)c1, (unsigned int)c2);
#endif
		processC8C(opcode, c1, c2);
	}
	break;

	case OPCODE:
#ifdef ENABLE_WORKFLOW
		if (vmflags.workflowEnabled)
			printf("\n");
#endif
		processJO(opcode);
		break;

	default:
		printf("not handled op %X\n", (int)opcode);
		break;
	}
}
