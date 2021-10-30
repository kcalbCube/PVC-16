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
            std::cout << "load [org] [file]\nrun\ncompile [input] [output]\nhelp\nunit-test\ndump [org] [len]\nregister-dump;";
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
        if (command == "compile")
        {
            std::string file;
            std::cin >> file;
            std::cin >> file;
            std::ofstream output(file);

            auto data = std::vector<uint8_t>({ MOV_RC(A, 6),
                MOV_RC(B, 2),
                ADD(A, B),
                INT(0) });

            std::ranges::copy(data, std::ostream_iterator<char>(output));
            output.close();
        }
        if (command == "load")
        {
            size_t org = 0;
            std::string file;
            std::cin >> std::hex >> org;
            std::cin >> file;
            std::ifstream input(file, std::ios::binary);

            
            size_t size = std::copy(std::istream_iterator<uint8_t>(input), std::istream_iterator<uint8_t>(),
                mc.data + org) - mc.data;
            hexDump("Loaded dump", mc.data + org, size, org);
            writeRegister(IP, static_cast<uint16_t>(org));
            input.close();
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
