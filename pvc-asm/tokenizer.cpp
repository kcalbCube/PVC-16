#include "tokenizer.h"
#include <string>
#include <iostream>
#include <vector>

using namespace std::literals;

std::vector<Token> Tokenizer::coreTokenize(const std::string& src)
{
	const boost::escaped_list_separator<char> els("\\"s, " \n\t"s, R"(")");

    const boost::tokenizer tok(src, els);
    std::vector<Token> tokens;

    std::ranges::copy(tok, std::back_inserter(tokens));
    return tokens;
}

void Tokenizer::operatorTokenize(const std::vector<Token> src, std::vector<Token>& dest)
{
    dest.clear();

    for(auto&& t : src)
    {
        boost::tokenizer tok(t, boost::char_separator(",", "+-[]()@#%:h "));
        std::ranges::copy(tok, std::back_inserter(dest));
    }


    for(size_t i = 1; i < dest.size() - 1; ++i)
    {
	    if(dest[i] == " ")
	    {
            dest[i - 1] = dest[i - 1] + dest[i] + dest[i + 1];
            dest.erase(dest.begin() + i);
            dest.erase(dest.begin() + i--);
	    }
        else if(dest[i] == "%" || dest[i] == "@")
        {
            dest[i] += dest[i + 1];
            dest.erase(dest.begin() + i + 1);
        }
        else if(dest[i] == ":" || dest[i] == "h")
        {
            dest[i - 1] += dest[i];
            dest.erase(dest.begin() + i--);
        }
    }
}

std::vector<Token> Tokenizer::tokenize(const std::string& src)
{
    auto tokens = coreTokenize(src);
    operatorTokenize(tokens, tokens);
	return tokens;
}
