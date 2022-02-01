#pragma once
#include <cstdint>
#include <map>
#include <vector>
#include "syntaxer.h"
#include <string>

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

	SIB generateSIB(const IndirectAddress& ia);
	bool isDispPresent(const IndirectAddress& ia);

public:
	void subcompileMnemonic(const Mnemonic& mnemonic, const std::map<uint16_t, Opcode>& variants);
	void compileMnemonic(const Mnemonic& mnemonic);
	void compile(std::vector<SyntaxUnit>& syntax, std::ostream& output);
	void writeDisp(const IndirectAddress& ia);
};

