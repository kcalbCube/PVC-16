#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

inline void hexDump(const char* desc, void* addr, const size_t len, const size_t offset, const size_t lineSize = 16)
{
    size_t i = 0;
    unsigned char buff[32];
    const auto* pc = static_cast<unsigned char*>(addr);

    // Output description if given.
    if (desc != nullptr)
        printf("%s:\n", desc);

    // Process every byte in the data.
    for (i = 0; i < len; ++i) 
    {
        // Multiple of 16 means new line (with line offset).

        if ((i % lineSize) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04X ", offset + i);
        }

        // Now the hex code for the specific character.
        printf(" %02X", pc[i]);

        buff[i % lineSize] = isprint(pc[i]) ? pc[i] : '.';
        buff[(i % lineSize) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while (i % lineSize != 0) 
    {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}

inline void registersDump(void)
{
	for(auto&& c : {REGISTERS_LIST})
        printf("%4s  %04X\n", registerId2registerName[c].c_str(), readRegister(c));
}

// Disasm utilities
inline std::string renderIndirectAddress(SIB sib, uint16_t disp)
{
    std::string str = "[";
    char buffer[9]{};
    buffer[8] = '\0';

    bool opRequired = false;
    if (sib.scale)
    {
        sprintf_s(buffer, "%d * ", 1 << sib.scale);
        str += buffer;
    }
    if (sib.base)
    {
        sprintf_s(buffer, opRequired ? "+ %%%s " : "%%%s ", registerId2registerName[getSIBbase(sib)].c_str());
        str += buffer;
        opRequired = true;
    }
    if (sib.index)
    {
        sprintf_s(buffer, opRequired ? "+ %%%s " : "%%%s ", registerId2registerName[getSIBindex(sib)].c_str());
        str += buffer;
        opRequired = true;
    }
    if (sib.disp)
    {
        sprintf_s(buffer, opRequired ? "+ %04X " : "%04X ", disp);
        str += buffer;
        opRequired = true;
    }

    return str.substr(0, str.size()-1) + "]";
}