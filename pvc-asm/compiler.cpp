#include "compiler.h"
#include "utility.h"
#include <ostream>

#include "../PVC-16/opcode.h"
#include "../PVC-16/registers_define.h"

using namespace std::string_literals;

uint16_t* Compiler::findLabel(const std::string& label)
{
	if (localSymbols[currentSymbol].count(label))
		return &localSymbols[currentSymbol][label];
	if (symbols.count(label))
		return &symbols[label];
	return nullptr;
}

void Compiler::write(uint8_t data)
{
	if (this->data.size() <= ip)
		this->data.resize(ip+1);
	this->data[ip++] = data;
}

void Compiler::write16(uint16_t data)
{
	write(m1628h(data));
	write(m1628l(data));
}

void Compiler::write(uint16_t ip, uint8_t data)
{
	if (this->data.size() <= ip)
		this->data.resize(ip + 1);
	this->data[ip] = data;
}

void Compiler::write16(uint16_t ip, uint16_t data)
{
	write(ip  , m1628h(data));
	write(ip+1, m1628l(data));
}

void Compiler::writeLabel(const std::string& label)
{
	if (const auto lbl = findLabel(label); lbl == nullptr)
	{
		delayedSymbols[label].push_back(ip);
		ip += 2;
	}
	else
		write16(*lbl);
}

SIB Compiler::generateSIB(const IndirectAddress& ia)
{
	return SIB(ia.scale, ia.index.name.empty() ? NO_REG : registerName2registerId.at(ia.index.name),
		ia.base.name.empty() ? NO_REG : registerName2registerId.at(ia.base.name), isDispPresent(ia));
}

void Compiler::writeSIB(const Mnemonic& mnemonic, const IndirectAddress& ia)
{
	write(std::bit_cast<uint8_t>(generateSIB(ia)));
}

bool Compiler::isDispPresent(const IndirectAddress& ia)
{
	return ia.disp.index() || std::get<Constant>(ia.disp).constant;
}

inline void Compiler::writeDisp(const IndirectAddress& ia)
{
	std::visit(visit_overload{
		[&](const Constant constant) -> void
			{
				write16(constant.constant);
			},
		[&](const LabelUse& lu) -> void
			{
				writeLabel(lu.label);
			}
	}, ia.disp);
}

void Compiler::subcompileMnemonic(const Mnemonic& mnemonic, const std::map<uint16_t, Opcode>& variants)
{
	auto md = mnemonic.describeMnemonics();
	Opcode dmnemonic;
	try
	{
		dmnemonic = variants.at(md);
	}
	catch (std::out_of_range&)
	{
		error(mnemonic.file, mnemonic.line, "bad opcode syntax.");
		return;
	}

	data.reserve(ip + 5);
	auto startIp = ip;
	try 
	{
		write(dmnemonic);
		switch (md)
		{
		case constructDescription(REGISTER, REGISTER):
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[1]).name));
			break;
		case constructDescription(REGISTER, CONSTANT):
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write16(std::get<Constant>(mnemonic.mnemonics[1]).constant);
			break;
		case constructDescription(REGISTER, INDIRECT_ADDRESS):
		{
			auto&& ia = std::get<IndirectAddress>(mnemonic.mnemonics[1]);
			auto id = registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name);
			write(id);

			write(std::bit_cast<uint8_t>(generateSIB(ia)));
			if (isDispPresent(ia))
				writeDisp(ia);
		}
		break;
		case constructDescription(REGISTER, LABEL):
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			writeLabel(std::get<LabelUse>(mnemonic.mnemonics[1]).label);
			break;

		case constructDescription(INDIRECT_ADDRESS, REGISTER):
		{
			auto&& ia = std::get<IndirectAddress>(mnemonic.mnemonics[0]);
			write(std::bit_cast<uint8_t>(generateSIB(ia)));

			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[1]).name));

			if (isDispPresent(ia))
				writeDisp(ia);
		}
		break;

		case constructDescription(CONSTANT):
			if (getOpcodeFormat(dmnemonic) == OPCODE_C8)
				write((uint8_t)std::get<Constant>(mnemonic.mnemonics[0]).constant);
			else
				write16(std::get<Constant>(mnemonic.mnemonics[0]).constant);
			break;

		case constructDescription(LABEL):
			writeLabel(std::get<LabelUse>(mnemonic.mnemonics[0]).label);
			break;

		case constructDescription(REGISTER):
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			break;

		case constructDescription(INDIRECT_ADDRESS):
		{
			auto&& ia = std::get<IndirectAddress>(mnemonic.mnemonics[0]);

			write(std::bit_cast<uint8_t>(generateSIB(ia)));
			if (isDispPresent(ia))
				writeDisp(ia);
		}
		break;
		case constructDescription():
			break;

		default:
			error(mnemonic.file, mnemonic.line, "subcompiler constructor not present.");
			break;
		}
	}
	catch (std::out_of_range&)
	{
		error(mnemonic.file, mnemonic.line, "bad register");
		while (ip --> startIp)
			write(ip, 0x00);
		++ip;
	}
}

void Compiler::compileMnemonic(Mnemonic mnemonic)
{
	if(mnemonic.name == "INT")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(CONSTANT), INT},
			});
	}
	else if(mnemonic.name == "MOV")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(REGISTER, REGISTER), MOV_RR},
			{constructDescription(REGISTER, CONSTANT), MOV_RC16},
			{constructDescription(REGISTER, LABEL), MOV_RC16},
			{constructDescription(REGISTER, INDIRECT_ADDRESS), MOV_RM},
			{constructDescription(INDIRECT_ADDRESS, REGISTER), MOV_MR},
			{constructDescription(INDIRECT_ADDRESS, INDIRECT_ADDRESS), MOV_MM16},
			{constructDescription(INDIRECT_ADDRESS, CONSTANT), MOV_MC16},
			});
	}
	else if (mnemonic.name == "MOVB")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(INDIRECT_ADDRESS, INDIRECT_ADDRESS), MOV_MM8},
			{constructDescription(INDIRECT_ADDRESS, CONSTANT), MOV_MC8},
			});
	}
	else if(mnemonic.name == "ADD")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), ADD},
		{constructDescription(REGISTER, CONSTANT), ADD_C16},
		{constructDescription(REGISTER, LABEL), ADD_C16},
				});
	}
	else if(mnemonic.name == "SUB")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), SUB},
		{constructDescription(REGISTER, CONSTANT), SUB_C16},
		{constructDescription(REGISTER, LABEL), SUB_C16},
					});
	}
	else if (mnemonic.name == "DIV")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), DIV},
		{constructDescription(REGISTER, CONSTANT), DIV_C16},
		{constructDescription(REGISTER, LABEL), DIV_C16},
			});
	}
	else if (mnemonic.name == "MUL")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), MUL},
		{constructDescription(REGISTER, CONSTANT), MUL_C16},
		{constructDescription(REGISTER, LABEL), MUL_C16},
			});
	}
	else if (mnemonic.name == "LEA")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, INDIRECT_ADDRESS), LEA},
			});
	}
	else if(mnemonic.name == "INC")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER), INC},
					});
	}
	else if(mnemonic.name == "DEC")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER), DEC},
			});
	}
	else if(mnemonic.name[0] == 'J' || mnemonic.name == "CALL") // FIXME: can break if J-starting not-jump opcodes present.
	{
		static std::map<std::string, Opcode> map = {
#define MAKE_JUMP_OPCODE(x) {#x, x}
	MAP_LIST(MAKE_JUMP_OPCODE, JMP, JZ, JNZ, JG, JNG, JGZ, JL, CALL)
		};
#undef MAKE_JUMP_OPCODE
		auto mnc = map[mnemonic.name];
		subcompileMnemonic(mnemonic, {
			{constructDescription(CONSTANT), mnc},
			{constructDescription(LABEL), mnc}
			});
	}
	else if(mnemonic.name == "CMP")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(REGISTER, CONSTANT), CMP_RC},
			{constructDescription(REGISTER, LABEL), CMP_RC},
			});
	}
	else if (mnemonic.name == "PUSHB")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(CONSTANT), PUSH_C8},
			});
	}
	else if (mnemonic.name == "PUSH")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(CONSTANT), PUSH_C16},
			{constructDescription(LABEL), PUSH_C16},
			{constructDescription(REGISTER), PUSH_R},
			});
	}
	else if (mnemonic.name == "POP")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(INDIRECT_ADDRESS), POP_M16},
			{constructDescription(REGISTER), POP_R},
			{constructDescription(), POP}
			});
	}
	else if (mnemonic.name == "POPB")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(INDIRECT_ADDRESS), POP_M8},
			{constructDescription(), POP8}
			});
	}
	else if (mnemonic.name == "OUTB")
	{
	std::iter_swap(mnemonic.mnemonics.begin(), mnemonic.mnemonics.begin() + 1);
	subcompileMnemonic(mnemonic, {
		{constructDescription(CONSTANT, CONSTANT), OUT_C8},
		{constructDescription(INDIRECT_ADDRESS, CONSTANT), OUT_M8},
		{constructDescription(REGISTER, CONSTANT), OUT_R},
		});
	}
	else if (mnemonic.name == "OUT")
	{
	std::iter_swap(mnemonic.mnemonics.begin(), mnemonic.mnemonics.begin() + 1);
	subcompileMnemonic(mnemonic, {
		{constructDescription(CONSTANT, CONSTANT), OUT_C16},
		{constructDescription(INDIRECT_ADDRESS, CONSTANT), OUT_M16},
		{constructDescription(REGISTER, CONSTANT), OUT_R},
		});
	}
	else if (mnemonic.name == "INB")
	{
	subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, CONSTANT), IN_R},
		{constructDescription(INDIRECT_ADDRESS, CONSTANT), IN_M8}
		});
	}
	else if (mnemonic.name == "IN")
	{
	subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, CONSTANT), IN_R},
		{constructDescription(INDIRECT_ADDRESS, CONSTANT), IN_M16}
		});
	}
	else if (mnemonic.name == "RET")
	{
		subcompileMnemonic(mnemonic, {
				{constructDescription(), RET}
			});
	}
	else
		error(mnemonic.file, mnemonic.line, "unknown mnemonic.");

}

void Compiler::compile(std::vector<SyntaxUnit>& syntax)
{
	for (auto&& su : syntax)
		std::visit(visit_overload{
				[&](Mnemonic& mnemonic) -> void
				{
					if (mnemonic.name == ".ORG")
					{
						ip = static_cast<uint16_t>(std::get<Constant>(mnemonic.mnemonics[0]).constant);
						data.reserve(ip);
					}
					else if (mnemonic.name == "DB")
					{
						for(auto&& v : mnemonic.mnemonics)
						{
							std::visit(visit_overload
								{
									[&](Constant c) -> void
									{
										write(static_cast<uint8_t>(c.constant));
									},
									[&](LabelUse l) -> void
									{
										writeLabel(l.label);
									},
									[&](String s) -> void
									{
										for(auto&& c : s.string)
										{
											write(c);
										}
									},
									[](auto) {abort(); }
								}, v);
						}
					}
					else
						compileMnemonic(mnemonic);
				},
				[&](const LabelDefinition& ld) -> void
				{
					for (auto&& ds : delayedSymbols[ld.label])
							write16(ds, ip);
						delayedSymbols.erase(ld.label);
					if(ld.label.starts_with('.'))
						localSymbols[currentSymbol][ld.label] = ip;
					else
					{
						symbols[currentSymbol = ld.label] = ip;
						std::erase_if(delayedSymbols, [](auto x) -> bool
							{
								return x.first.starts_with('.');
							});
					}
				},
				[&](const Newline& nl) -> void {}
			}, su);
}

void Compiler::writeInOstream(std::ostream& output)
{
	char buffer[128];
	std::string syms;
	for (auto&& [label, address] : symbols)
	{
		sprintf_s(buffer, "%s:%04X;", label.c_str(), address);
		syms += buffer;
	}
	sprintf_s(buffer, "%04X", syms.size());
	output << buffer << syms;
	std::ranges::copy(data, std::ostream_iterator<char>(output));
}