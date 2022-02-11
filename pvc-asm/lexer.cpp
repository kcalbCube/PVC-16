#include "lexer.h"

#include "utility.h"
#include <boost/algorithm/string/replace.hpp>

std::vector<Lexema> Lexer::lex(const std::vector<Token>& tokens)
{
	enum
	{
		NONE,
		INDIRECT_ADDRESS,
	} state = NONE;
	std::vector<Lexema> lexemas;
	Lexema indirectAddress;
	unsigned line = 0;
	std::string file = curFile;

	for (const auto& token : tokens)
	{
		Lexema lexema;
		if (token == "[")
		{
			if (state == INDIRECT_ADDRESS)
				error(file, line, "'[' inside the indirect address block. Perhaps ']' missed.");
			else
			{
				state = INDIRECT_ADDRESS;
				indirectAddress = Lexema(LexemID::INDIRECT_ADDRESS, std::vector<Lexema>());
			}
			continue;
		}
		else if (token == "]")
		{
			if (state != INDIRECT_ADDRESS)
			{
				error(file, line, "unmatched ']'. Perhaps '[' missed.");
				continue;
			}
			state = NONE;
			lexema = indirectAddress;
		}
		else if (token == "\n")
		{
			++line;
			continue;
		}
		else if (token.ends_with('H') && std::isxdigit(token[0]))
		{
			lexema = Lexema(LexemID::NUMBER, a16toi(token));
		}
		else if (token == "+" || token == "-")
		{
			if (state != INDIRECT_ADDRESS)
			{
				error(file, line, "operator outside the indirect address block.");
				continue;
			}
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
		else if (token.ends_with(':'))
		{
			if (state == INDIRECT_ADDRESS)
			{
				error(file, line, "label definition inside the indirect address block. Perhaps ']' missed.");
				continue;
			}
			lexema = Lexema(LexemID::LABEL, token.substr(0, token.size() - 1));
		}
		else if (token.find('"') != std::string::npos)
		{
			auto string = token.substr(0, token.size() - 1).substr(1);
			boost::replace_all(string, "\\n", "\n");
			lexema = Lexema(LexemID::STRING, string);
		}
		else
			lexema = Lexema(LexemID::MNEMONIC, token);

		if (state == INDIRECT_ADDRESS)
		{
			indirectAddress.file = file;
			indirectAddress.line = line;
			std::get<std::vector<Lexema>>(indirectAddress.lexemas).push_back(lexema);
		}
		else
		{
			lexema.file = file;
			lexema.line = line;
			lexemas.push_back(lexema);
		}
	}

	return lexemas;
}
