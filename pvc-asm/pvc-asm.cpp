#include <string>
#include <iostream>
#include <fstream>

#include "compiler.h"
#include "lexer.h"
#include "syntaxer.h"
#include "tokenizer.h"
using namespace std::literals;

int main(const int argc, char** args)
{
    std::string inputFile;
    if (argc == 1)
        std::cin >> inputFile;
    else
    	inputFile = args[1];
    std::ifstream input(inputFile);
    std::string source; std::getline(input, source, '\0');
    input.close();

    auto tokens = Tokenizer::tokenize(source);
    auto lexemas = Lexer::lex(tokens);
    auto syntaxis = Syntaxer::syntaxParse(lexemas);
    std::ofstream of("output.bin", std::ios::binary);
    Compiler().compile(syntaxis, of);
    of.flush();
    of.close(); // 	mov [%bp + 4] 95
    return 0;
}