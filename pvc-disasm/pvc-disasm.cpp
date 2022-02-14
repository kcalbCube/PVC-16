// pvc-disasm.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <args.hxx>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include <map>
#include <string>
#include <ranges>
#include "../PVC-16/opcode.h"
#include "../PVC-16/utility.h"

#include <magic_enum.hpp>

int main(int argc, char** args)
{
    args::ArgumentParser parser("PVC-16 disassembler.");
    args::HelpFlag help(parser, "help", "Display this help menu.", { 'h', "help" });
    args::CompletionFlag completion(parser, { "complete" });
    args::ValueFlag<std::string> output(parser, "output", "Output file", { 'o', "output" });
    args::Positional<std::string> inputFile(parser, "input", "The input file");
    args::ValueFlag<std::string> label(parser, "label", "Label to start disasm", { 'l', "label" });
    args::ValueFlag<size_t> numBytes(parser, "numbytes", "Bytes to disasm", { 'n', "numbytes" });
    args::ValueFlag<size_t> startByte(parser, "start", "Start byte", { 's', "start" });
    args::Flag dumpLabels(parser, "dumplabels", "Dump labels from file", { 'l', "labels"});

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
    std::string fileName = args::get(inputFile);
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

    std::map<std::string, size_t> labels;
    for (auto&& c : string_split(syms, ";"))
    {
        auto&& ss = string_split(c, ":");

        auto a16toi = [](const std::string& str) -> int
        {
            int result = 0;
            sscanf_s(str.c_str(), "%X", &result);
            return result;
        };


       labels[ss[0]] = a16toi(ss[1]);
    }

    if(dumpLabels)
        for (std::string label : labels | std::views::keys)
            printf("%s: %04zX\n", label.c_str(), labels[label]);
    size_t startaddress = startByte ? (args::get(startByte)) : (label ? (labels[args::get(label)]) : labels["START"]);

    uint8_t data[0xFFFF]{};
    input.readsome(reinterpret_cast<char*>(data), 0xFFFF);
    input.close();

	auto read16 = [&data](auto addr) -> uint16_t { return static_cast<uint16_t>(data[addr] | (data[addr + 1] << 8)); };

    for (size_t i = startaddress; i < startaddress + args::get(numBytes);)
    {
		for (auto&& [label, addr] : labels)
			if (addr == i)
				printf("\n%s:\n", label.c_str());
		auto opcode = static_cast<Opcode>(data[i++]);

		if (auto&& opc = magic_enum::enum_name(opcode); opc.empty())
			printf("%04zX: %X ", (unsigned int)(i - 1), (unsigned int)opcode);
		else
			printf("%04zX: %s ", (unsigned int)(i - 1), std::string(opc).c_str());

		switch (getOpcodeFormat(opcode))
		{
		case OPCODE_RR:
		{
			const auto r1 = static_cast<RegisterID>(data[i++]);
			const auto r2 = static_cast<RegisterID>(data[i++]);
			printf("%%%s %%%s\n", registerId2registerName[r1].c_str(), registerId2registerName[r2].c_str());
		}
		break;

		case OPCODE_RC:
		{
			const auto r1 = static_cast<RegisterID>(data[i++]);
			const auto c = read16(i);
			i += 2;
			printf("%%%s %04zX\n", registerId2registerName[r1].c_str(), (unsigned int)c);
		}
		break;

		case OPCODE_R:
		{
			const auto r1 = static_cast<RegisterID>(data[i++]);
			printf("%%%s\n", registerId2registerName[r1].c_str());
		}
		break;

		case OPCODE_RM:
		{
			const auto r1 = static_cast<RegisterID>(data[i++]);
			const auto sib = std::bit_cast<SIB>(data[i++]);
			const auto disp = sib.disp ? read16(i) : 0;
			i += sib.disp * 2;
			printf("%%%s %s\n", registerId2registerName[r1].c_str(), renderIndirectAddress(sib, disp).c_str());
		}
		break;

		case OPCODE_MC:
		{
			const auto sib = std::bit_cast<SIB>(data[i++]);
			const auto c = read16(i);
			i += 2;
			const uint16_t disp = sib.disp ? read16(i) : 0;
			i += sib.disp * 2;
			printf("%s %04zX\n", renderIndirectAddress(sib, disp).c_str(), c);
		}
		break;

		case OPCODE_MC8:
		{
			const auto sib = std::bit_cast<SIB>(data[i++]);
			const auto c = data[i++];
			const uint16_t disp = sib.disp ? read16(i) : 0;
			i += sib.disp * 2;
			printf("%s %04zX\n", renderIndirectAddress(sib, disp).c_str(), c);
		}
		break;

		case OPCODE_MR:
		{
			const auto sib = std::bit_cast<SIB>(data[i++]);
			const auto r1 = static_cast<RegisterID>(data[i++]);
			const uint16_t disp = sib.disp ? read16(i) : 0;
			i += sib.disp * 2;
			printf("%s %%%s\n", renderIndirectAddress(sib, disp).c_str(), registerId2registerName[r1].c_str());

		}
		break;

		case OPCODE_MM:
		{
			const auto sib1 = std::bit_cast<SIB>(data[i++]);
			const auto sib2 = std::bit_cast<SIB>(data[i++]);
			const uint16_t disp1 = sib1.disp ? read16(i) : 0;
			i += sib1.disp * 2;
			const uint16_t disp2 = sib2.disp ? read16(i) : 0;
			i += sib2.disp * 2;
			printf("%s %s\n", renderIndirectAddress(sib1, disp1).c_str(), renderIndirectAddress(sib2, disp2).c_str());

		}
		break;

		case OPCODE_C8:
		{
			const auto c8 = data[i++];
			printf("%02zX\n", (unsigned int)c8);
		}
		break;

		case OPCODE_C:
		{
			const auto c = read16(i);
			i += 2;
			printf("%04zX\n", (unsigned int)c);
		}
		break;

		case OPCODE_CC:
		{
			const auto c1 = read16(i);
			i += 2;
			const auto c2 = read16(i);
			i += 2;
			printf("%04zX %04zX\n", (unsigned int)c1, (unsigned int)c2);
		}
		break;

		case OPCODE_C8C:
		{
			const auto c1 = data[i++];
			const auto c2 = read16(i);
			i += 2;
			printf("%02zX %04zX\n", (unsigned int)c1, (unsigned int)c2);
		}
		break;

		case OPCODE:
			printf("\n");
			break;

		case OPCODE_MCC8:
		{
			const auto sib = std::bit_cast<SIB>(data[i++]);
			const uint16_t c1 = read16(i); i += 2;
			const uint8_t c2 = data[i++];
			const uint16_t disp = sib.disp ? read16(i) : 0; i += sib.disp * 2;

			printf("%s %04zX %02zX\n", renderIndirectAddress(sib, disp).c_str(), (unsigned)c1, (unsigned)c2);
		}
		break;

		case OPCODE_INVALID:
			printf("unknown\n");
			break;
		}
    }


    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
