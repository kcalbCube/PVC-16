// PVC-16.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "unittesting.h"
#include "decoder.h"
#include "interrupt.h"
#include <fstream>

#include "utility.h"

int main(void)
{
    while(true)
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
            while (!isHalted)
            {
                process();
            }

            registersDump();
        }
        if (command == "load-dump")
        {
            size_t org = 0;
            std::string file;
            std::cin >> std::hex >> org;
            std::cin >> file;
            std::ifstream input(file, std::ios::binary);


            const size_t size = std::copy(std::istream_iterator<uint8_t>(input), std::istream_iterator<uint8_t>(),
                                          mc.data + org) - mc.data;
            hexDump("Loaded dump", mc.data + org, size, org);
            writeRegister(IP, static_cast<uint16_t>(org));
            input.close();
        }
        if (command == "load")
        {
            std::string file;
            std::cin >> file;
            std::ifstream input(file, std::ios::binary);

            auto string_split = [](const std::string& s, const char delimiter) -> std::vector<std::string>
            {
                size_t start = 0;
                size_t end = s.find_first_of(delimiter);

                std::vector<std::string> output;

                while (end != std::string::npos)
                {
                    output.emplace_back(s.substr(start, end - start));

                    if (end == std::string::npos)
                        break;

                    start = end + 1;
                    end = s.find_first_of(delimiter, start);
                }

                return output;
            };

            size_t size = 0;
            input >> size;

            std::string syms;
            std::copy_n(std::istream_iterator<char>(input), size, std::back_inserter(syms));
            
            for(auto&& c : string_split(syms, ';'))
            {
                int addr = 0;
                auto&& ss = string_split(c, ':');
                auto x = syms.substr(ss[0].size()+2);

                auto a16toi = [](const std::string& str) -> int
                {
                    int result = 0;
                    sscanf_s(str.c_str(), "%X%*c", &result);
                    return result;
                };


                addr = a16toi(x);
                if(ss[0] == "START")
                {
                    writeRegister(IP, addr);
                    break;
                }
            }

            input.readsome(reinterpret_cast<char*>(mc.data), 0xFFFF);
            //std::copy(std::istream_iterator<uint8_t>(input), std::istream_iterator<uint8_t>(), mc.data);
        }

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
