#include <string>
#include <iostream>
#include <fstream>

#include "compiler.h"
#include "lexer.h"
#include "syntaxer.h"
#include "tokenizer.h"
#include <args.hxx>
using namespace std::literals;

int main(const int argc, char** args)
{
    args::ArgumentParser parser("PVC-16 assembler.");
    args::HelpFlag help(parser, "help", "Display this help menu.", { 'h', "help" });
    args::CompletionFlag completion(parser, { "complete" });
    args::ValueFlag<std::string> output(parser, "output", "Output file", { 'o', "output"});
    args::Positional<std::string> inputFile(parser, "input", "The input file");

    try
    {
        parser.ParseCLI(argc, args);
    }
    catch (const args::Completion& e)
    {
        std::cout << e.what();
        return 0;
    }
    catch (const args::Help&)
    {
        std::cout << parser;
        return 0;
    }
    catch (const args::ParseError& e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    std::ifstream input(args::get(inputFile));
    std::string source; std::getline(input, source, '\0');
    input.close();

    auto tokens = Tokenizer::tokenize(source);
    auto lexemas = Lexer::lex(tokens);
    auto syntaxis = Syntaxer::syntaxParse(lexemas);
    std::ofstream of(args::get(output), std::ios::binary);
    Compiler().compile(syntaxis, of);
    of.flush();
    of.close();
    return 0;
}