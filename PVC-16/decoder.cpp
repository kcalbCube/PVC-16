#include "decoder.h"

#include <iostream>

#include "interrupt.h"
#include "utility.h"
#include "stack.h"
#include "vmflags.h"

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
				
	}
}

void Decoder::processRC(Opcode opcode, RegisterID r1, uint16_t constant)
{
	switch (opcode)
	{
	case MOV_RC:
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

	case ADD_C:
	{
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) += constant));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) += constant));
		break;
	}
	case SUB_C:
	{
		if (is16register(r1))
			updateStatus(static_cast<uint16_t>(getRegister16(r1) -= constant));
		else
			updateStatus(static_cast<uint8_t>(getRegister8(r1) -= constant));
		break;
	}
	}
}

void Decoder::processRM(Opcode opcode, RegisterID r1, uint16_t addr)
{
	switch (opcode)
	{
	case MOV_RM:
		mc.readInRegister(addr, r1);
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
		case PUSH_C:
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

void Decoder::process(void)
{
	auto&& ip = getRegister16(IP); 
	auto opcode = static_cast<Opcode>(mc.read8(ip++));

	if (vmflags.workflowEnabled)
	{
		if(auto&& opc = magic_enum::enum_name(opcode); opc.empty())
			printf("%04X: %02-6X ", (unsigned int)(ip - 1), (unsigned int)opcode);
		else
			printf("%04X: %-6s ", (unsigned int)(ip - 1), std::string(opc).c_str());
		
	}

	switch (getOpcodeFormat(opcode))
	{
	case OPCODE_RR:
	{
		const auto r1 = static_cast<RegisterID>(mc.read8(ip++));
		const auto r2 = static_cast<RegisterID>(mc.read8(ip++));
		if (vmflags.workflowEnabled)
			printf("%%%s %%%s\n", registerId2registerName[r1].c_str(), registerId2registerName[r2].c_str());
		processRR(opcode, r1, r2);
	}
	break;

	case OPCODE_RC:
	{
		const auto r1 = static_cast<RegisterID>(mc.read8(ip++));
		const auto c = mc.read16(ip);
		ip += 2;
		if (vmflags.workflowEnabled)
			printf("%%%s %04X\n", registerId2registerName[r1].c_str(), (unsigned int)c);
		processRC(opcode, r1, c);
	}
	break;

	case OPCODE_R:
	{
		const auto r1 = static_cast<RegisterID>(mc.read8(ip++));
		if (vmflags.workflowEnabled)
			printf("%%%s\n", registerId2registerName[r1].c_str());
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
		if (vmflags.workflowEnabled)
			printf("%%%s %s{%04X}\n", registerId2registerName[r1].c_str(), renderIndirectAddress(sib, disp).c_str(), (unsigned int)addr);
		processRM(opcode, r1, addr);
	}
	break;

	case OPCODE_MR:
	{
		const auto sib = std::bit_cast<SIB>(mc.read8(ip++));
		const auto r1  = static_cast<RegisterID>(mc.read8(ip++));
		const uint16_t disp = sib.disp ? mc.read16(ip) : 0;
		const uint16_t addr = readAddress(sib, disp);
		ip += sib.disp * 2;
		if (vmflags.workflowEnabled)
			printf("%s{%04X} %%%s\n", renderIndirectAddress(sib, disp).c_str(), (unsigned int)addr, registerId2registerName[r1].c_str());
		processMR(opcode, addr, r1);

	}

	case OPCODE_C8:
	{
		const auto c8 = mc.read8(ip++);
		if (vmflags.workflowEnabled)
			printf("%02X\n", (unsigned int)c8);
		processC8(opcode, c8);
	}
	break;

	case OPCODE_C:
	{
		const auto c = mc.read16(ip);
		ip += 2;
		if (vmflags.workflowEnabled)
			printf("%02X\n", (unsigned int)c);
		processC(opcode, c);
	}
	break;

	case OPCODE:
		if (vmflags.workflowEnabled)
			printf("\n");
		processJO(opcode);
		break;

	default:
		printf("not handled op %X\n", (int)opcode);
		break;
	}
}
