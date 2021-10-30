#pragma once
#include <variant>
#include <vector>

#include "tokenizer.h"

enum class LexemID
{
	MNEMONIC,
	LABEL,
	INDIRECT_ADDRESS,
	OPERATION,
	NUMBER,
	LABEL_USE,
	REGISTER,
	STRING
};

struct Lexema
{
	LexemID id;
	std::variant<std::vector<Lexema>, int, std::string> lexemas;
};

class Lexer
{
public:

	static std::vector<Lexema> lex(const std::vector<Token>& tokens);
};

