#pragma once
#include <cstdint>
#include <map>
#include <vector>
#include "syntaxer.h"
#include <string>
#include "utility.h"

#include "../PVC-16/opcode.h"

class Compiler
{
	std::map<std::string, uint16_t> symbols;
	std::map<std::string, std::map<std::string, uint16_t>> localSymbols;
	std::map<std::string, std::vector<uint16_t>> delayedSymbols;
	std::string currentSymbol;

	std::vector<uint8_t> data;
	uint16_t ip = 0;

	uint16_t* findLabel(const std::string& label);
	void write(uint8_t data);
	void write16(uint16_t data);

	void write(uint16_t ip, uint8_t data);
	void write16(uint16_t ip, uint16_t data);
	void writeLabel(const std::string& label);
	void writeSIB(const Mnemonic& mnemonic, const IndirectAddress& ia);

	SIB generateSIB(const IndirectAddress& ia);
	bool isDispPresent(const IndirectAddress& ia);

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

