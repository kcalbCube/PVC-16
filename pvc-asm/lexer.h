#pragma once
#include <variant>
#include <vector>
#include "tokenizer.h"
#include "eval.h"

enum class LexemID
{
	MNEMONIC,
	LABEL,
	INDIRECT_ADDRESS,
	OPERATION,
	NUMBER,
	LABEL_USE,
	REGISTER,
	STRING,
	NEWLINE,
	EXPR,
};

struct Lexema
{
	LexemID id;
	std::variant<std::vector<Lexema>, unsigned, std::string, Expression> lexemas;

	std::string file;
	unsigned line = 0;

	Lexema(LexemID id_, decltype(lexemas) lexemas_)
		: id{ id_ }, lexemas{ lexemas_ } {}
	Lexema(void) = default;
};

class Lexer
{
public:

	static std::vector<Lexema> lex(const std::vector<Token>& tokens);
};

