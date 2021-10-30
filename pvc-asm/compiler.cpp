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
		if (label.starts_with('.'))
			abort(); // TODO: add message
		delayedSymbols[label].push_back(ip);
		ip += 2;
	}
	else
		write16(lbl);
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
			std::visit(visit_overload{
					[](const Register& reg) -> void
					{

					},
					[&](const LabelUse& lu) -> void
					{
						if (ia.second.constant)
						{
							// TODO:
						}
						else
						{
							data.reserve(ip + 4);
							write(MOV_RMC);
							write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
							writeLabel(lu.label);
						}
					},
					[&](const Constant& constant) -> void
					{
						data.reserve(ip + 4);
						write(MOV_RMC);
						write(registerName2registerId.at(std::get<Register>(mnemonic.mnemonics[0]).name));
						write16(std::get<Constant>(mnemonic.mnemonics[1]).constant);
					}
				}, ia.first);
			}

		default:
			break;

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
					if(ld.label.starts_with('.'))
					{
						assert(!currentSymbol.empty());
						localSymbols[currentSymbol][ld.label] = ip;
						return;
					}

					for (auto&& ds : delayedSymbols[ld.label])
						write16(ds, ip);
					delayedSymbols.erase(ld.label);
					symbols[currentSymbol = ld.label] = ip;
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
