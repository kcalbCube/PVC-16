// PVC-16.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include "unittesting.h"
#include "decoder.h"
#include "interrupt.h"
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <args.hxx>
#include "utility.h"
#include "vmflags.h"


void loadPVCObjFromFile(const std::string& fileName)
{
    std::ifstream input(fileName, std::ios::binary);

    auto string_split = [](const std::string& s, const char* delimiter) -> std::vector<std::string>
    {
        std::vector<std::string> output;

        boost::split(output, s, boost::is_any_of(delimiter));

        return output;
    };

    size_t size = 0;
    input >> size;

    std::string syms;
    std::copy_n(std::istream_iterator<char>(input), size, std::back_inserter(syms));

    for (auto&& c : string_split(syms, ";"))
    {
        int addr = 0;
        auto&& ss = string_split(c, ":");

        auto a16toi = [](const std::string& str) -> int
        {
            int result = 0;
            sscanf_s(str.c_str(), "%X", &result);
            return result;
        };


        addr = a16toi(ss[1]);
        if (ss[0] == "START")
        {
            writeRegister(IP, addr);
            break;
        }
    }

    input.readsome(reinterpret_cast<char*>(mc.data), 0xFFFF);
    input.close();
}

void loadDumpFromFile(const std::string& fileName, uint16_t org)
{
    std::string file;
    std::ifstream input(fileName, std::ios::binary);


    const size_t size = std::copy(std::istream_iterator<uint8_t>(input), std::istream_iterator<uint8_t>(),
        mc.data + org) - mc.data;
    writeRegister(IP, static_cast<uint16_t>(org));
    input.close();
}

void start(void)
{
    while (!isHalted)
        Decoder::process();
    registersDump();
}
void interactive(void)
{
    while (true)
    {
        std::string command;
        std::cin >> command;

        if (command == "help")
            std::cout <<
            "load-dump [org] [file] -- load plain data\n"
            "run -- start machine\n"
            "help -- display this message\n"
            "unit-test -- run unit-test subsystem\n"
            "dump [org] [len] -- dump data to console\n"
            "register-dump -- dump registers to console\n"
            "load [file] -- load pvc-asm file\n";
        if (command == "unit-test")
            UnitTester::work();
        if (command == "dump")
        {
            size_t org = 0, len = 0;
            std::cin >> std::hex >> org >> len;
            hexDump(nullptr, mc.data + org, len, org);
        }
        if (command == "register-dump")
        {
            registersDump();
        }
        if (command == "run")
        {
            start();
        }
        if (command == "load-dump")
        {
            std::string file; uint16_t org;
            std::cin >> file;
            std::cin >> org;
            loadDumpFromFile(file, org);
        }
        if (command == "load")
        {
            std::string file;
            std::cin >> file;
            loadPVCObjFromFile(file);
        }

    }
}
int main(int argc, char** argv)
{
    args::ArgumentParser parser("PVC-16 virtual machine.");
    args::HelpFlag help(parser, "help", "Display this help menu.", { 'h', "help" });
    args::CompletionFlag completion(parser, { "complete" });
    args::Group execution(parser, "Execution flags");
    args::Flag interactive(execution, "interactive", "Starts in interactive mode", { 'i', "interactive" });
    args::Flag loadRaw(execution, "raw", "Load raw", { "raw" });
    args::ValueFlag<int> offset(execution, "offset", "Loading offset in raw mode", { "offset" });
    args::Group debug(parser, "Debug flags");
    args::Flag workflow(debug, "dworkflow", "Enables workflow", { "dworkflow" });
    args::Positional<std::string> input(parser, "input", "The input file");

    try
    {
        parser.ParseCLI(argc, argv);
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

    vmflags.workflowEnabled = workflow;
    if (offset)
        vmflags.loadOffset = args::get(offset);
    if (interactive || !input)
        ::interactive();
    else if(input)
    {
        if (loadRaw)
            loadDumpFromFile(args::get(input), vmflags.loadOffset);
        else
            loadPVCObjFromFile(args::get(input));
        start();
    }
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
