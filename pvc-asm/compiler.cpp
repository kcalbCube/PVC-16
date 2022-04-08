#include "compiler.h"
#include "utility.h"
#include <ostream>
#include <deque>
#include <fstream>
#include <variant>

#include "../PVC-16/opcode.h"
#include "../PVC-16/registers_define.h"

using namespace std::string_literals;

uint16_t* Compiler::findLabel(const std::string& label)
{
	if (localSymbols[currentSymbol].contains(label))
		return &localSymbols[currentSymbol][label];
	if (symbols.contains(label))
		return &symbols[label];
	return nullptr;
}

void Compiler::write(const uint8_t data)
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

void Compiler::writeLabels(Expression& expression)
{
	for(auto&& l : localSymbols[currentSymbol])
		expression.setLabel(LabelDefinition(l.first), l.second);
	for(auto&& l : symbols)
		expression.setLabel(LabelDefinition(l.first), l.second);
}
void Compiler::writeExpression(Expression expression)
{
	writeLabels(expression);
	if(expression.isConstexpr())
	{
		if(expression.isByte())
			write(expression.evaluate());
		else
			write16(expression.evaluate());
	}
	else
	{
		expressions.emplace_back(expression, ip);
		ip += expression.isByte() ? 1 : 2;
	}
}

SIB Compiler::generateSIB(const IndirectAddress& ia)
{
	return SIB(ia.scale, ia.index.name.empty() ? registers::NO_REG : registers::registerName2registerId.at(ia.index.name),
		ia.base.name.empty() ? registers::NO_REG : registers::registerName2registerId.at(ia.base.name), isDispPresent(ia));
}

void Compiler::writeSIB(const Mnemonic& mnemonic, const IndirectAddress& ia)
{
	write(std::bit_cast<uint8_t>(generateSIB(ia)));
}

bool Compiler::isDispPresent(const IndirectAddress& ia)
{
	return ia.disp.index() || std::get<Constant>(ia.disp).constant;
}

void Compiler::writeDisp(const IndirectAddress& ia)
{
	std::visit(visit_overload{
		[&](const Constant constant) -> void
			{
				write16(static_cast<uint16_t>(constant.constant));
			},
		[&](const LabelUse& lu) -> void
			{
				writeLabel(lu.label);
			},
		[&](const Expression& expr) -> void
			{
				writeExpression(expr);
			}
	}, ia.disp);
}

void Compiler::subcompileMnemonic(const Mnemonic& mnemonic, const std::map<uint16_t, SCVariant>& variants)
{
	auto md = mnemonic.describeMnemonics();
	Opcode dmnemonic;
	uint8_t args;
	try
	{
		const auto& argsr = variants.at(md);
		dmnemonic = argsr.opcode;
		args = argsr.arg;
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
		size_t i = 0;
		std::deque<IndirectAddress> dispDeque;

		for (auto&& c : mnemonic.mnemonics)
		{
			switch (c.index())
			{
			case MI_REGISTER:
				write(registers::registerName2registerId.at(std::get<Register>(c).name));
				break;
			case MI_CONSTANT:
				if(args & (1 << i))
					write(static_cast<uint8_t>(std::get<Constant>(c).constant));
				else
					write16(static_cast<uint16_t>(std::get<Constant>(c).constant));
				break;
			case MI_LABELUSE:
				writeLabel(std::get<LabelUse>(c).label);
				break;
			case MI_INDIRECT_ADDRESS:
			{
				auto&& ia = std::get<IndirectAddress>(c);

				write(std::bit_cast<uint8_t>(generateSIB(ia)));
				if (isDispPresent(ia))
					dispDeque.push_back(ia);
			}
				break;
			case MI_EXPRESSION:
			{
				auto expression = std::get<Expression>(c);
				if(args & (1 << i))
					expression.toByte();

				writeExpression(expression);
			}
				break;

			}
			++i;
		}
		for(auto&& disp : dispDeque)
			writeDisp(disp);
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
	if(mnemonic.name == "NOP")
	{
		if(auto desc = mnemonic.describeMnemonics();
			desc == constructDescription(CONSTANT) ||
			desc == constructDescription(EXPRESSION))
		{
			int n = 0;
			if(desc == constructDescription(EXPRESSION))
			{
				auto&& expression = std::get<Expression>(mnemonic.mnemonics[0]);
				if(!expression.isConstexpr())
				{
					error(mnemonic.file, mnemonic.line, "Cannot evaluate expression");
					return;
				}
				n = expression.evaluate();
			}
			else
				n = std::get<Constant>(mnemonic.mnemonics[0]).constant;
			for(int i = 0; i < n; ++i)
				subcompileMnemonic(Mnemonic("NOP", {}, mnemonic.line, mnemonic.file), {{constructDescription(), NOP}});
		}
		else
			subcompileMnemonic(mnemonic, {{constructDescription(), NOP}});
	}
	else if(mnemonic.name == "INT")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(CONSTANT), {INT, ARG1_8}},
			{constructDescription(EXPRESSION), {INT, ARG1_8}},
			});
	}
	else if(mnemonic.name == "MOV")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(REGISTER, REGISTER), MOV_RR},
			{constructDescription(REGISTER, CONSTANT), MOV_RC16},
			{constructDescription(REGISTER, EXPRESSION), MOV_RC16},
			{constructDescription(REGISTER, LABEL), MOV_RC16},
			{constructDescription(REGISTER, INDIRECT_ADDRESS), MOV_RM},
			{constructDescription(INDIRECT_ADDRESS, REGISTER), MOV_MR},
			{constructDescription(INDIRECT_ADDRESS, INDIRECT_ADDRESS), MOV_MM16},
			{constructDescription(INDIRECT_ADDRESS, CONSTANT), MOV_MC16},
			{constructDescription(INDIRECT_ADDRESS, EXPRESSION), MOV_MC16},
			{constructDescription(INDIRECT_ADDRESS, LABEL), MOV_MC16},
			});
	}
	else if (mnemonic.name == "MOVB")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(INDIRECT_ADDRESS, INDIRECT_ADDRESS), {MOV_MM8, ARG2_8}},
			{constructDescription(INDIRECT_ADDRESS, CONSTANT), {MOV_MC8, ARG2_8}},
			{constructDescription(INDIRECT_ADDRESS, EXPRESSION), {MOV_MC8, ARG2_8}},
			});
	}
	else if (mnemonic.name == "SHL")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), SHL_RR},
		{constructDescription(REGISTER, CONSTANT), SHL_RC},
		{constructDescription(REGISTER, EXPRESSION), SHL_RC},
			});
	}
	else if (mnemonic.name == "SHR")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), SHR_RR},
		{constructDescription(REGISTER, CONSTANT), SHR_RC},
		{constructDescription(REGISTER, EXPRESSION), SHR_RC},
			});
	}
	else if (mnemonic.name == "OR")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), OR_RR},
		{constructDescription(REGISTER, CONSTANT), OR_RC},
		{constructDescription(REGISTER, EXPRESSION), OR_RC},
			});
	}
	else if (mnemonic.name == "AND")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), AND_RR},
		{constructDescription(REGISTER, CONSTANT), AND_RC},
		{constructDescription(REGISTER, EXPRESSION), AND_RC} 
			});
	}
	else if(mnemonic.name == "ADD")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), ADD},
		{constructDescription(REGISTER, CONSTANT), ADD_C16},
		{constructDescription(REGISTER, EXPRESSION), ADD_C16},
		{constructDescription(REGISTER, LABEL), ADD_C16},
				});
	}
	else if(mnemonic.name == "SUB")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), SUB},
		{constructDescription(REGISTER, CONSTANT), SUB_C16},
		{constructDescription(REGISTER, EXPRESSION), SUB_C16},
		{constructDescription(REGISTER, LABEL), SUB_C16},
					});
	}
	else if (mnemonic.name == "DIV")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), DIV},
		{constructDescription(REGISTER, CONSTANT), DIV_C16},
		{constructDescription(REGISTER, EXPRESSION), DIV_C16},
		{constructDescription(REGISTER, LABEL), DIV_C16},
			});
	}
	else if (mnemonic.name == "MOD")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), MOD_RR},
		{constructDescription(REGISTER, CONSTANT), MOD_RC},
		{constructDescription(REGISTER, EXPRESSION), MOD_RC},
		{constructDescription(REGISTER, LABEL), MOD_RC},
			});
	}
	else if (mnemonic.name == "MUL")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, REGISTER), MUL},
		{constructDescription(REGISTER, CONSTANT), MUL_C16},
		{constructDescription(REGISTER, EXPRESSION), MUL_C16},
		{constructDescription(REGISTER, LABEL), MUL_C16},
			});
	}
	else if (mnemonic.name == "LEA")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, INDIRECT_ADDRESS), LEA_RM},
		{constructDescription(INDIRECT_ADDRESS, INDIRECT_ADDRESS), LEA_MM}
			});
	}
	else if(mnemonic.name == "INC")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER), INC},
					});
	}
	else if (mnemonic.name == "NEG")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER), NEG},
			});
	}
	else if (mnemonic.name == "NOT")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER), NOT},
			});
	}
	else if(mnemonic.name == "DEC")
	{
		subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER), DEC},
			});
	}
#define CONSTRUCT_JUMP(name_, opcode_) else if(mnemonic.name == #name_) \
	subcompileMnemonic(mnemonic, {\
	{constructDescription(CONSTANT), opcode_},\
	{constructDescription(LABEL), opcode_},\
	{constructDescription(EXPRESSION), opcode_}})

	CONSTRUCT_JUMP(JMP, JMP);
	CONSTRUCT_JUMP(CALL, CALL);
#undef CONSTRUCT_JUMP
#define CONSTRUCT_PREFIX(name_, opcode_) else if(mnemonic.name == #name_) subcompileMnemonic(mnemonic, {{constructDescription(), opcode_}})
CONSTRUCT_PREFIX(CZ, CZ);
CONSTRUCT_PREFIX(CE, CZ);

CONSTRUCT_PREFIX(CNZ, CNZ);
CONSTRUCT_PREFIX(CNE, CNZ);

CONSTRUCT_PREFIX(CG, CG);
CONSTRUCT_PREFIX(CNLE, CG);
CONSTRUCT_PREFIX(CNLZ, CG);

CONSTRUCT_PREFIX(CLE, CNG);
CONSTRUCT_PREFIX(CLZ, CNG);
CONSTRUCT_PREFIX(CNG, CNG);

CONSTRUCT_PREFIX(CGE, CGZ);
CONSTRUCT_PREFIX(CGZ, CGZ);
CONSTRUCT_PREFIX(CNL, CGZ);

CONSTRUCT_PREFIX(CNGZ, CL);
CONSTRUCT_PREFIX(CNGE, CL);
CONSTRUCT_PREFIX(CL  , CL);

CONSTRUCT_PREFIX(CB, CB);
CONSTRUCT_PREFIX(CNAE, CB);
CONSTRUCT_PREFIX(CNAZ, CB);
CONSTRUCT_PREFIX(CC, CB);

CONSTRUCT_PREFIX(CNB, CNB);
CONSTRUCT_PREFIX(CAE, CNB);
CONSTRUCT_PREFIX(CAZ, CNB);
CONSTRUCT_PREFIX(CNC, CNB);

CONSTRUCT_PREFIX(CBE, CBZ);
CONSTRUCT_PREFIX(CBZ, CBZ);
CONSTRUCT_PREFIX(CNA, CBZ);

CONSTRUCT_PREFIX(CA, CA);
CONSTRUCT_PREFIX(CNBE, CA);
CONSTRUCT_PREFIX(CNBZ, CA);

CONSTRUCT_PREFIX(CAR, CAR);
CONSTRUCT_PREFIX(CNAR, CNAR);
CONSTRUCT_PREFIX(CCR, CCR);
CONSTRUCT_PREFIX(CNCR, CNCR);
#undef CONSTRUCT_PREFIX
	else if(mnemonic.name == "CMP")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(REGISTER, CONSTANT), CMP_RC},
			{constructDescription(REGISTER, LABEL), CMP_RC},
			{constructDescription(REGISTER, REGISTER), CMP_RR},
			{constructDescription(INDIRECT_ADDRESS, CONSTANT), CMP_MC},
			{constructDescription(INDIRECT_ADDRESS, REGISTER), CMP_MR},
			{constructDescription(INDIRECT_ADDRESS, INDIRECT_ADDRESS), CMP_MM},
			{constructDescription(REGISTER, INDIRECT_ADDRESS), CMP_RM}
			});
	} 
	else if(mnemonic.name == "CMPB")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(INDIRECT_ADDRESS, INDIRECT_ADDRESS), CMP_MM8},
			{constructDescription(INDIRECT_ADDRESS, CONSTANT), {CMP_MC8, ARG2_8}}
		});
	}
	else if (mnemonic.name == "LOOP")
	{
	subcompileMnemonic(mnemonic, {
		{constructDescription(REGISTER, CONSTANT), LOOP},
		{constructDescription(REGISTER, LABEL), LOOP},
		});
	}
	else if (mnemonic.name == "PUSHB")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(CONSTANT), {PUSH_C8, ARG1_8}},
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
		{constructDescription(CONSTANT, CONSTANT), {OUT_C8, ARG1_8}},
		{constructDescription(INDIRECT_ADDRESS, CONSTANT), OUT_M8},
		{constructDescription(REGISTER, CONSTANT), OUT_R},
		});
	}
	else if (mnemonic.name == "OUT")
	{
	std::iter_swap(mnemonic.mnemonics.begin(), mnemonic.mnemonics.begin() + 1);
	subcompileMnemonic(mnemonic, {
		{constructDescription(CONSTANT, CONSTANT), OUT_C16},
		{constructDescription(LABEL, CONSTANT), OUT_C16},
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
	else if (mnemonic.name == "RET" || mnemonic.name == "RES")
	{
		subcompileMnemonic(mnemonic, {
				{constructDescription(), RET}
			});
	}
	else if (mnemonic.name == "IRET" || mnemonic.name == "REI")
	{
		subcompileMnemonic(mnemonic, {
				{constructDescription(), REI}
			});
	}
	else if (mnemonic.name == "PUSHA")
	{
	subcompileMnemonic(mnemonic, {
			{constructDescription(), PUSHA}
		});
	}
	else if (mnemonic.name == "POPA")
	{
	subcompileMnemonic(mnemonic, {
			{constructDescription(), POPA}
		});
	}
	else if (mnemonic.name == "PUSHF")
	{
	subcompileMnemonic(mnemonic, {
			{constructDescription(), PUSHF}
		});
	}
	else if (mnemonic.name == "POPF")
	{
	subcompileMnemonic(mnemonic, {
			{constructDescription(), POPF}
		});
	}
	else if (mnemonic.name == "CLI")
	{
		subcompileMnemonic(mnemonic, {
				{constructDescription(), CLI}
			});
	}
	else if (mnemonic.name == "STI")
	{
		subcompileMnemonic(mnemonic, {
				{constructDescription(), STI}
			});
	}
	else if (mnemonic.name == "MEMSET")
	{
		subcompileMnemonic(mnemonic, {
			{constructDescription(INDIRECT_ADDRESS, CONSTANT, CONSTANT), {MEMSET, ARG3_8}}
			});
	}
	else if (mnemonic.name == "TEST")
	{
	subcompileMnemonic(mnemonic, {
			{constructDescription(REGISTER, CONSTANT), TEST_RC},
			{constructDescription(REGISTER, EXPRESSION), TEST_RC},
			});
	}
	else
		error(mnemonic.file, mnemonic.line, std::string("unknown mnemonic \"") + mnemonic.name + "\"");

}

void Compiler::compile(std::vector<SyntaxUnit>& syntax)
{
	for (size_t i = 0; i < syntax.size(); ++i)
		std::visit(visit_overload{
				[&](Mnemonic& mnemonic) -> void
				{
					if (mnemonic.name == ".ORG")
					{
						std::visit(visit_overload
						{
							[&](const Constant& c) -> void
							{
								ip = static_cast<uint16_t>(c.constant);
							},
							[&](Expression expr) -> void
							{
								writeLabels(expr);
								if(expr.isConstexpr())
									ip = static_cast<uint16_t>(expr.evaluate());
								else
									error(mnemonic.file, mnemonic.line, "cannot evaluate expression.");
							},
							[&](auto) -> void { error(mnemonic.file, mnemonic.line, "bad .org argument."); }
						}, mnemonic.mnemonics[0]);
						
						data.reserve(ip);
					}
					else if (mnemonic.name == "DB")
					{
						for (auto&& v : mnemonic.mnemonics)
						{
							std::visit(visit_overload
								{
									[&](const Constant& c) -> void
									{
										write(static_cast<uint8_t>(c.constant));
									},
									[&](const String& s) -> void
									{
										for (auto&& c : s.string)
											write(c);
									},
									[&](auto) -> void { error(mnemonic.file, mnemonic.line, "bad db argument."); }
								}, v);
						}
					}
					else if (mnemonic.name == "DW")
						for (auto&& v : mnemonic.mnemonics)
						{
							std::visit(visit_overload
								{
									[&](const Constant& c) -> void
									{
										write16(static_cast<uint16_t>(c.constant));
									},
									[&](const LabelUse& l) -> void
									{
										writeLabel(l.label);
									},
									[&](auto) -> void { error(mnemonic.file, mnemonic.line, "bad dw argument."); }
								}, v);
						}
					else
						compileMnemonic(mnemonic);
				},
				[&](const LabelDefinition& ld) -> void
				{
					for (auto&& ds : delayedSymbols[ld.label])
					{
						write16(ds, ip);
					}
					delayedSymbols.erase(ld.label);

					for(auto&& [expr, eip] : expressions)
						expr.setLabel(ld, ip);

					std::erase_if(expressions, [&](auto&& x) -> bool
					{
						auto&& expr = x.first;
						auto&& eip = x.second;

						if(expr.isConstexpr())
						{
							if(expr.isByte())
								write(eip, expr.evaluate());
							else
								write16(eip, expr.evaluate());
							return true;
						}
						return false;
					});
					
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
			}, syntax[i]);

	for (const auto& key : delayedSymbols | std::views::keys)
		error(std::string("undeclared symbol \"") + key + "\".");
}

void Compiler::writeInOstream(std::ostream& output)
{

	char buffer[128];
	std::string syms;

	std::vector<std::pair<std::string, uint16_t>> symbolsd;
	std::transform(symbols.begin(), symbols.end(), std::back_inserter(symbolsd), [](auto&& a) -> auto { return a; } );
	std::ranges::sort(symbolsd, [](const auto& a, const auto& b) -> bool {return a.second > b.second; } );

	for (auto&& [label, address] : symbolsd)
	{
		sprintf(buffer, "%s:%04X;", label.c_str(), address);
		syms += buffer;
	}
	sprintf(buffer, "%04zX", syms.size());
	output << buffer << syms;
	std::ranges::copy(data, std::ostream_iterator<char>(output));
}
