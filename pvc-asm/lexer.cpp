#include "lexer.h"

#include "utility.h"

std::vector<Lexema> Lexer::lex(const std::vector<Token>& tokens)
{
	enum
	{
		NONE,
		INDIRECT_ADDRESS,
	} state = NONE;
	std::vector<Lexema> lexemas;
	Lexema indirectAddress;
	for (const auto& token : tokens)
	{
		Lexema lexema;
		if (token == "[")
		{
			assert(state != INDIRECT_ADDRESS);
			state = INDIRECT_ADDRESS;
			indirectAddress = Lexema(LexemID::INDIRECT_ADDRESS, std::vector<Lexema>());
			continue;
		}
		if (token == "]")
		{
			assert(state == INDIRECT_ADDRESS);
			state = NONE;
			lexema = indirectAddress;
		}
		else if (token[token.size() - 1] == 'H' && std::isxdigit(token[0]))
		{
			lexema = Lexema(LexemID::NUMBER, a16toi(token));
		}
		else if (token == "+" || token == "-")
		{
			assert(state == INDIRECT_ADDRESS);
			lexema = Lexema(LexemID::OPERATION, token);
		}
		else if (token.starts_with('%'))
		{
			lexema = Lexema(LexemID::REGISTER, token.substr(1));
		}
		else if (token.starts_with('@'))
		{
			lexema = Lexema(LexemID::LABEL_USE, token.substr(1));
		}
		else if (token[token.size() - 1] == ':')
		{
			assert(state != INDIRECT_ADDRESS);
			lexema = Lexema(LexemID::LABEL, token.substr(0, token.size() - 1));
		}
		else if (token.find('"') != std::string::npos)
		{
			lexema = Lexema(LexemID::STRING, token.substr(0, token.size()-1).substr(1));
		}
		else
			lexema = Lexema(LexemID::MNEMONIC, token);

		if (state == INDIRECT_ADDRESS)
			std::get<std::vector<Lexema>>(indirectAddress.lexemas).push_back(lexema);
		else
			lexemas.push_back(lexema);
	}

	return lexemas;
}
