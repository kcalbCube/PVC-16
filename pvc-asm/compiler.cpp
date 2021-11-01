#include "compiler.h"

#include <ostream>

#include "../PVC-16/opcode.h"
#include "../PVC-16/registers_define.h"

using namespace std::string_literals;

uint16_t Compiler::findLabel(const std::string& label)
{
	if (localSymbols[currentSymbol].count(label))
		return localSymbols[currentSymbol][label];
	if (symbols.count(label))
		return symbols[label];
	return 0xFFFF;
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
	if (const auto lbl = findLabel(label); lbl == 0xFFFF)
	{
		//if (label.starts_with('.'))
			//abort(); // TODO: add message
		delayedSymbols[label].push_back(ip);
		ip += 2;
	}
	else
		write16(lbl);
}

SIB Compiler::generateSIB(const IndirectAddress& ia, bool& isDisp)
{
	isDisp = ia.disp.index() || std::get<Constant>(ia.disp).constant;
	return SIB(ia.scale, ia.index.name.empty()? NO_REG : registerName2registerId.at(ia.index.name),
		ia.base.name.empty()? NO_REG : registerName2registerId.at(ia.base.name), isDisp);
}

void Compiler::compileMnemonic(const Mnemonic& mnemonic)
{
	if(mnemonic.name == "INT")
	{
		write(INT);
		write(std::get<Constant>(mnemonic.mnemonics[0]).constant);
	}
	else if(mnemonic.name == "MOV")
	{
		switch(mnemonic.describeMnemonics())
		{
		case constructDescription(REGISTER, REGISTER):
			{
			data.reserve(ip + 3);
			write(MOV_RR);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[1]).name));
			}
			break;

		case constructDescription(REGISTER, CONSTANT):
			{
			data.reserve(ip + 4);
			write(MOV_RC);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write16(std::get<Constant>(mnemonic.mnemonics[1]).constant);
			}
			break;

		case constructDescription(REGISTER, LABEL):
			{
			data.reserve(ip + 4);
			write(MOV_RC);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			writeLabel(std::get<LabelUse>(mnemonic.mnemonics[1]).label);
			}
			break;

		case constructDescription(REGISTER, INDIRECT_ADDRESS):
			{
			auto&& ia = std::get<IndirectAddress>(mnemonic.mnemonics[1]);
			auto id = registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name);
			write(MOV_RM);
			write(id);

			bool isDisp = false;
			write(std::bit_cast<uint8_t>(generateSIB(ia, isDisp)));
			if (isDisp)
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
			}
			break;

		case constructDescription(INDIRECT_ADDRESS, REGISTER):
		{
			auto&& ia = std::get<IndirectAddress>(mnemonic.mnemonics[0]);
			write(MOV_MR);

			bool isDisp = false;
			write(std::bit_cast<uint8_t>(generateSIB(ia, isDisp)));

			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[1]).name));

			if (isDisp)
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
		}
		break;

		default:
			break;

		}
	}
	else if(mnemonic.name == "ADD")
	{
		data.reserve(ip + 3);
		switch (mnemonic.describeMnemonics())
		{
		case constructDescription(REGISTER, REGISTER):
			write(ADD);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[1]).name));
			break;

		case constructDescription(REGISTER, CONSTANT):
			write(ADD_C);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write16(std::get<Constant>(mnemonic.mnemonics[1]).constant);
			break;

		case constructDescription(REGISTER, LABEL):
			write(ADD_C);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			writeLabel(std::get<LabelUse>(mnemonic.mnemonics[1]).label);
			break;

		default: abort();
		}
	}
	else if (mnemonic.name == "SUB")
	{
		data.reserve(ip + 3);
		switch (mnemonic.describeMnemonics())
		{
		case constructDescription(REGISTER, REGISTER):
			write(SUB);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[1]).name));
			break;

		case constructDescription(REGISTER, CONSTANT):
			write(SUB_C);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write16(std::get<Constant>(mnemonic.mnemonics[1]).constant);
			break;

		case constructDescription(REGISTER, LABEL):
			write(SUB_C);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			writeLabel(std::get<LabelUse>(mnemonic.mnemonics[1]).label);
			break;

		default: abort();
		}
	}
	else if(mnemonic.name == "INC")
	{
		write(INC);
		write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
	}
	else if(mnemonic.name == "DEC")
	{
		write(DEC);
		write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
	}
	else if(mnemonic.name[0] == 'J') // FIXME: can break if J-starting not-jump opcodes present.
	{
		static std::map<std::string, Opcode> map = {
#define MAKE_JUMP_OPCODE(x) {#x, x}
	MAP_LIST(MAKE_JUMP_OPCODE, JMP, JZ, JNZ, JG, JNG, JGZ)
		};
#undef MAKE_JUMP_OPCODE
		write(map[mnemonic.name]);
		switch(mnemonic.describeMnemonics())
		{
		case constructDescription(CONSTANT):
			write16(std::get<Constant>(mnemonic.mnemonics[0]).constant);
			break;

		case constructDescription(LABEL):
			writeLabel(std::get<LabelUse>(mnemonic.mnemonics[0]).label);
			break;

		default: abort();
		}
	}
	else if(mnemonic.name == "CMP")
	{
	switch (mnemonic.describeMnemonics())
	{
		case constructDescription(REGISTER, CONSTANT):
			write(CMP_RC);
			write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
			write16(std::get<Constant>(mnemonic.mnemonics[1]).constant);
			break;

		case constructDescription(REGISTER, REGISTER):
			//writeLabel(std::get<LabelUse>(mnemonic.mnemonics[0]).label);
			break;

		default: abort();
	}
	}
}

void Compiler::compile(std::vector<SyntaxUnit>& syntax, std::ostream& output)
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
				}
			}, su);

	std::string syms;
	for(auto&& [label, address] : symbols)
	{
		char buffer[128];
		sprintf_s(buffer, "%s:%04X;", label.c_str(), address);
		syms += buffer;
	}

	output << syms.size() << syms;
	std::ranges::copy(data, std::ostream_iterator<char>(output));
}
