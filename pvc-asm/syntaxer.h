#pragma once
#include <memory>

#include "lexer.h"

struct LabelDefinition{ std::string label; };
struct LabelUse{ std::string label; };

struct Register{ std::string name; };
struct Constant{ int constant = 0; };
struct String { std::string string; };

struct IndirectAddress
{
	std::variant<Register, LabelUse> first; Constant second = {0};
	template<typename T1, typename T2>
	IndirectAddress(T1 first1, T2 second1)
		: first{first1}, second{second1} {};
	template<typename T>
	IndirectAddress(T first1)
		: first{first1} {}
};

struct Mnemonic{ std::string name; std::vector<std::variant<Register, Constant, LabelUse, IndirectAddress, String>> mnemonics;};

using SyntaxUnit = std::variant<Mnemonic, LabelDefinition>;

class Syntaxer
{
public:
	static std::vector<SyntaxUnit> syntaxParse(const std::vector<Lexema>& lexems);
};

