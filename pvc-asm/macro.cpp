#include "macro.h"
#include <string>
#include <iostream>
#include <fstream>
#include "utility.h"

void Macro::process(std::vector<Token>& tokens)
{
	for(int i = 0; i < tokens.size(); ++i)
	{
		auto&& token = tokens[i];
		if(token == ".INCLUDE")
		{
			auto includeFile = tokens[i+1].substr(1);
			includeFile.pop_back();
			if(false)
			{
				error(includeFile + ": bad .include argument");
				continue;
			}
			std::string oldCurFile = std::move(curFile);
			curFile = includeFile;

			std::ifstream input(includeFile);
			std::string source;
			reserveLines(includeFile);

			std::getline(input, source, '\0');
			input.clear();
			input.seekg(0, std::ios::beg);

			while (std::getline(input, getNextLine(includeFile)));

			auto ntokens = Tokenizer::tokenize(source);
			tokens.erase(std::begin(tokens) + i);
			tokens.erase(std::begin(tokens) + i);
			tokens.insert(std::begin(tokens) + i--, std::begin(ntokens), std::end(ntokens));

			curFile = std::move(oldCurFile);
		}
		else if(token == ".SET")
		{
			tokens.erase(std::begin(tokens) + i);
			std::string id = tokens[i];
			std::vector<Token> contains;
			tokens.erase(std::begin(tokens) + i);
			while(tokens[i] != "\n")
			{
				contains.push_back(tokens[i]);
				tokens.erase(std::begin(tokens) + i);
			};

			--i;
			sets[id] = contains;
		}
		else if(sets.contains(token))
		{
			auto&& m = sets[token];
			tokens.erase(std::begin(tokens) + i);
			tokens.insert(std::begin(tokens) + i--, std::begin(m), std::end(m));
		}
	}
}
