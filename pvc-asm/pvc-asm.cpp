#include <boost/tokenizer.hpp>
#include <string>
#include <iostream>

#include "lexer.h"
#include "syntaxer.h"
#include "tokenizer.h"

using namespace std::literals;

int main(void)
{
    auto tokens = Tokenizer::tokenize("mov %a %b\nadd %a 5h\n mov [%c] [%bp+2]\ndata:\n\tdb \"Hello, world!\"\n@data");
    auto lexemas = Lexer::lex(tokens);
    auto syntaxis = Syntaxer::syntaxParse(lexemas);
    return 0;
}