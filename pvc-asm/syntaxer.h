#pragma once
#include <memory>
#include <numeric>
#include "lexer.h"
#include <ranges>
#include <variant>
#include "utility.h"

struct IndirectAddress
{
	Register base{}; 
	Register index{}; 
	uint8_t scale = 1; 
	std::variant<Constant, LabelUse, Expression> disp;
};

struct Mnemonic
{
	std::string name;
	std::vector<std::variant<Register, Constant, LabelUse, IndirectAddress, String, Expression>> mnemonics;

	unsigned line = 0;
	std::string file;
	uint16_t describeMnemonics(void) const;
};

enum MnemonicIndex
{
	MI_REGISTER,
	MI_CONSTANT,
	MI_LABELUSE,
	MI_INDIRECT_ADDRESS,
	MI_STRING,
	MI_EXPRESSION
};

enum MnemonicDescription
{
	REGISTER = 1, CONSTANT, LABEL, INDIRECT_ADDRESS, STRING, EXPRESSION,
};

template<typename... Args>
constexpr uint16_t constructDescription(Args... args)
{
	size_t i = 0;
	auto&& r = { args... };
	return std::accumulate(r.begin(), r.end(), static_cast<uint16_t>(0), [&i]<typename T0>(T0 a, auto b) -> T0
	{
		return std::move(a) + static_cast<uint16_t>(b << i++ * 4);
	});
}

constexpr uint16_t constructDescription(void)
{
	return 0;
}

using SyntaxUnit = std::variant<Mnemonic, LabelDefinition, Newline>;

template<class... Ts> struct visit_overload : Ts... { using Ts::operator()...; };

class Syntaxer
{
public:
	static std::vector<SyntaxUnit> syntaxParse(std::vector<Lexema>& lexems);
};

