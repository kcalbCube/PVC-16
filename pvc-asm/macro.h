#pragma once
#include "tokenizer.h"
#include <vector>
#include <map>
#include <string>

class Macro
{
public:
	std::map<std::string, std::vector<Token>> sets;
	void process(std::vector<Token>&);	
};


