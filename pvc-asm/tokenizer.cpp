#include "tokenizer.h"
#include <string>
#include <iostream>
#include <vector>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/bind/bind.hpp>
using namespace std::literals;

std::vector<Token> Tokenizer::coreTokenize(std::string src)
{
	const boost::escaped_list_separator<char> els("\\"s, " \n\t"s, "\"';");

	boost::replace_all(src, "\"", R"("\")");
	boost::replace_all(src, "'", "'\\'");
	boost::replace_all(src, ";", ";\\;");
	boost::replace_all(src, "\\n", "\\\\n");
	boost::replace_all(src, "\n", "\\n");

	const boost::tokenizer tok(src, els);
	std::vector<Token> tokens;

	std::ranges::copy(tok, std::back_inserter(tokens));
	bool (std::string::*starts_with) (char const) const = &std::string::starts_with;
	(void)std::remove_if(tokens.begin(), tokens.end(), boost::bind(starts_with, boost::placeholders::_1, ';'));

	return tokens;
}

void Tokenizer::operatorTokenize(const std::vector<Token> src, std::vector<Token>& dest)
{
	dest.clear();

	for(auto&& t : src)
	{
		boost::tokenizer tok(t, boost::char_separator(",", "\'*/+-<>%^&|~[]()@#%:{}\n "));
		std::ranges::copy(tok, std::back_inserter(dest));
	}


	for(size_t i = 1; i < dest.size(); ++i)
	{
		if (dest[i] == " ")
		{
			dest[i - 1] = dest[i - 1] + dest[i] + dest[i + 1];
			dest.erase(dest.begin() + i);
			dest.erase(dest.begin() + i--);
		}
		else if (dest[i] == "\"")
		{
			do
			{
				dest[i] += dest[i + 1];
				dest.erase(dest.begin() + i + 1);
			} while (!dest[i].ends_with("\""));
		}
		else if (dest[i] == "%" || dest[i] == "@")
		{
			dest[i] += dest[i + 1];
			dest.erase(dest.begin() + i + 1);
		}
		else if (dest[i] == ":")
		{
			dest[i - 1] += dest[i];
			dest.erase(dest.begin() + i--);
		}
		else if (dest[i] == ";")
		{
			do
			{
				dest[i] += dest[i + 1];
				dest.erase(dest.begin() + i + 1);
			} while (!dest[i].ends_with(';'));

			dest.erase(dest.begin() + i);
		}
	}
}

std::vector<Token> Tokenizer::tokenize(const std::string& src)
{
	auto tokens = coreTokenize(src);
	operatorTokenize(tokens, tokens);
	for (auto&& c : tokens)
		if (c.find('"') == std::string::npos && c.find('\'') == std::string::npos)
			std::ranges::transform(c, c.begin(), [](char c) -> char { return std::toupper(c); });

	return tokens;
}
