#include <string>
#include <iostream>
#include <fstream>
#include "compiler.h"
#include "lexer.h"
#include "syntaxer.h"
#include "tokenizer.h"
#include <args.hxx>
#include <chrono>

using namespace std::literals;

int main(const int argc, char** args)
{
	args::ArgumentParser parser("PVC-16 assembler.");
	args::HelpFlag help(parser, "help", "Display this help menu.", { 'h', "help" });
	args::CompletionFlag completion(parser, { "complete" });
	args::ValueFlag<std::string> output(parser, "output", "Output file", { 'o', "output"});
	args::Positional<std::string> inputFile(parser, "input", "The input file");
	args::ValueFlagList<std::string> includeDirs(parser, "include", "The include directories", { 'I' });

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
	if (includeDirs)
		for (auto&& c : args::get(includeDirs))
			::includeDirs.push_back(c);

	auto start = std::chrono::high_resolution_clock::now();
	std::string fileName = curFile = args::get(inputFile);
	std::ifstream input(fileName);
	std::string source;
	reserveLines(fileName);

	std::getline(input, source, '\0');
	input.clear();
	input.seekg(0, std::ios::beg);

	while (std::getline(input, getNextLine(fileName)));

	input.close();


	auto tokens   = Tokenizer::tokenize(source);
	auto lexemas  = Lexer::lex(tokens);
	auto syntaxis = Syntaxer::syntaxParse(lexemas);

	Compiler compiler;
	compiler.compile(syntaxis);

	if (!getErrorNumber())
	{
		std::ofstream of(args::get(output), std::ios::binary);
		compiler.writeInOstream(of);
		of.flush();
		of.close();
	}
	auto elapsed = std::chrono::high_resolution_clock::now() - start;
	std::cout << std::endl << fileName << " compiled in " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms. " << getErrorNumber() << " error(s)" << std::endl;
	return 0;
}