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
#include "device.h"

#ifdef ENABLE_VIDEO
#include <SDL.h>
#include <SDL_ttf.h>
#include "video.h"
#endif


void loadPVCObjFromFile(const std::string& fileName)
{
    std::ifstream input(fileName, std::ios::binary);

    auto string_split = [](const std::string& s, const char* delimiter) -> std::vector<std::string>
    {
        std::vector<std::string> output;

        boost::split(output, s, boost::is_any_of(delimiter));

        return output;
    };

    unsigned tableLen = 0;
    char tableLenBuffer[5]{};
    input.get(tableLenBuffer, 5);
    sscanf_s(tableLenBuffer, "%04X", &tableLen);
    std::string syms;
    std::copy_n(std::istream_iterator<char>(input), tableLen, std::back_inserter(syms));

    syms.pop_back(); // remove last ;

    for (auto&& c : string_split(syms, ";"))
    {
	    auto&& ss = string_split(c, ":");

        auto a16toi = [](const std::string& str) -> int
        {
            int result = 0;
            sscanf_s(str.c_str(), "%X", &result);
            return result;
        };


        int addr = a16toi(ss[1]);
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


    (void)std::copy(std::istream_iterator<uint8_t>(input), std::istream_iterator<uint8_t>(),
        mc.data + org);
    writeRegister(IP, org);
    input.close();
}

void start(void)
{
    DeviceController dc;
    ::dc = &dc;
    dc.addDevice(new DebugOutputDevice);
    dc.addDevice(new VideoController);
    dc.start();

    status.interrupt = 1;
#ifdef ENABLE_EXECUTION_TIME_CAPTURE
    const auto start = std::chrono::high_resolution_clock::now();
#endif
    while (!isHalted)
    {
        handleDelayedInterrupts();
        Decoder::process();
    }
#ifdef ENABLE_EXECUTION_TIME_CAPTURE
    const auto elapsed = std::chrono::high_resolution_clock::now() - start;
    const auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count();
    const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    std::cout << std::endl << microseconds << " microseconds (" << milliseconds << " ms) elapsed" << std::endl;
#endif
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
#undef main
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
#ifdef ENABLE_WORKFLOW
    args::Flag workflow(debug, "dworkflow", "Enables workflow", { "dworkflow" });
#endif
#ifdef ENABLE_EXECUTION_TIME_CAPTURE
    args::Flag executionTimeCapture(debug, "dexectime", "Captures execution time", { "dexectime" });
#endif

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
    
#ifdef ENABLE_WORKFLOW
    vmflags.workflowEnabled = workflow;
#endif
#ifdef ENABLE_EXECUTION_TIME_CAPTURE
    vmflags.captureExecutionTime = executionTimeCapture;
#endif
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