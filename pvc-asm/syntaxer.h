#pragma once
#include <memory>
#include <numeric>
#include "lexer.h"
#include <ranges>

struct LabelDefinition{ std::string label; };
struct LabelUse{ std::string label; };

struct Register{ std::string name; };
struct Constant{ int constant = 0; };
struct String { std::string string; };
struct Newline {};

struct IndirectAddress
{
	Register base{}; Register index{}; uint8_t scale = 1; std::variant<Constant, LabelUse> disp;
};

struct Mnemonic
{
	std::string name;
	std::vector<std::variant<Register, Constant, LabelUse, IndirectAddress, String>> mnemonics;

	unsigned line = 0;
	std::string file;
	uint16_t describeMnemonics(void) const;
};

enum MnemonicDescription
{
	REGISTER = 1, CONSTANT, LABEL, INDIRECT_ADDRESS, STRING
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

