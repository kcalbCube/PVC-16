#pragma once
#include <cstdint>
#include <map>
#include <vector>
#include "syntaxer.h"
#include <string>
#include "utility.h"
#include "eval.h"
#include "../PVC-16/opcode.h"

class Compiler
{
	std::map<std::string, uint16_t> symbols;
	std::map<std::string, std::map<std::string, uint16_t>> localSymbols;
	std::map<std::string, std::vector<uint16_t>> delayedSymbols;
	std::vector<std::pair<Expression, uint16_t>> expressions;
	std::string currentSymbol;

	std::vector<uint8_t> data;
	uint16_t ip = 0;

	uint16_t* findLabel(const std::string&);
	void write(uint8_t);
	void write16(uint16_t);

	void write(uint16_t, uint8_t);
	void write16(uint16_t, uint16_t);
	void writeLabel(const std::string&);
	void writeLabels(Expression&);
	void writeExpression(Expression);
	void writeSIB(const Mnemonic&, const IndirectAddress&);

	SIB generateSIB(const IndirectAddress&);
	bool isDispPresent(const IndirectAddress&);

	struct BadRegister : std::exception {};

public:
	enum Arg : uint8_t
	{
		ARG1_8 = 1,
		ARG2_8 = (1 << 1),
		ARG3_8 = (1 << 2)
	};

	struct SCVariant
	{
		Opcode opcode;
		uint8_t arg = 0;

		SCVariant(Opcode opcode_) : opcode{ opcode_ } {}
		SCVariant(Opcode opcode_, uint8_t arg_) : opcode{ opcode_ }, arg{ arg_ } {}
		SCVariant(void) = default;
	};

	void subcompileMnemonic(const Mnemonic& mnemonic, const std::map<uint16_t, SCVariant>& variants);
	void compileMnemonic(Mnemonic mnemonic); // pbv
	void compile(std::vector<SyntaxUnit>& syntax);
	void writeDisp(const IndirectAddress& ia);

	void writeInOstream(std::ostream& output);

};

