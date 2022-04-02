#include "lexer.h"

#include "utility.h"
#include <boost/algorithm/string/replace.hpp>
#include "eval.h"

std::vector<Lexema> Lexer::lex(const std::vector<Token>& tokens)
{
	bool inIndirectAddress = false;
	bool inExpr = false;

	std::vector<Lexema> lexemas;
	Lexema indirectAddress, expr;
	unsigned line = 0;
	std::string file = curFile;

	for (const auto& token : tokens)
	{
		Lexema lexema;
		if (token == "[")
		{
			if (inIndirectAddress)
				error(file, line, "'[' inside the indirect address block. Perhaps ']' missed.");
			else
			{
				inIndirectAddress = true;
				indirectAddress = Lexema(LexemID::INDIRECT_ADDRESS, std::vector<Lexema>());
			}
			continue;
		}
		else if (token == "]")
		{
			if (!inIndirectAddress)
			{
				error(file, line, "unmatched ']'. Perhaps '[' missed.");
				continue;
			}
			inIndirectAddress = false;
			lexema = indirectAddress;
		}
		else if (token == "{")
		{
			if (inExpr)
				error(file, line, "'{' inside the expression block. Perhaps '}' missed.");
			else
			{
				inExpr = true;
				expr = Lexema(LexemID::EXPR, std::vector<Lexema>());
			}
			continue;
		}
		else if (token == "}")
		{
			if (!inExpr)
			{
				error(file, line, "unmatched '}'. Perhaps '{' missed.");
				continue;
			}
			inExpr = false;
			lexema = expr;
			Expression expression(std::get<std::vector<Lexema>>(expr.lexemas));
			if(expression.isConstexpr())
				lexema = Lexema(LexemID::NUMBER, expression.evaluate());
			else
				lexema = Lexema(LexemID::EXPR, expression);
			
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
		else if ([](auto token) -> bool 
		{ 
			for(auto&& c : {"+", "-", "*", "/", "%", "<<", ">>", "^", "|", "&", "~", "(", ")"}) 
				if(token == c) 
					return true;
			return false;
		}(token))
		{
			if (!inIndirectAddress && !inExpr)
			{
				error(file, line, "operator outside the expression block.");
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
			if (inIndirectAddress || inExpr)
			{
				error(file, line, "label definition inside the expression block.");
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

		if (inExpr)
		{
			expr.file = file;
			expr.line = line;
			std::get<std::vector<Lexema>>(expr.lexemas).emplace_back(lexema);
		}
		else if (inIndirectAddress)
		{
			indirectAddress.file = file;
			indirectAddress.line = line;
			std::get<std::vector<Lexema>>(indirectAddress.lexemas).emplace_back(lexema);
		}
		else
		{
			lexema.file = file;
			lexema.line = line;
			lexemas.emplace_back(lexema);
		}
	}

	return lexemas;
}
