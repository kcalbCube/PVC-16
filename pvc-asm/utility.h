#pragma once
#include <string>
#include <iostream>

unsigned a16toi(const std::string& str);

void error(const std::string& file, size_t line, const std::string& msg);
void warning(const std::string& file, size_t line, const std::string& msg);
void warning(const std::string& file, const std::string& msg);
void warning(const std::string& msg);

size_t getErrorNumber(void);
size_t getWarningNumber(void);

void reserveLines(const std::string& file);
void writeLine(const std::string& file, std::string msg);

std::string& getNextLine(const std::string& file);

inline std::string curFile = ""; // fixme

#define m1628(src) static_cast<uint8_t>(src), static_cast<uint8_t>((src) >> 8)
#define m1628h(src) static_cast<uint8_t>(src)
#define m1628l(src) static_cast<uint8_t>((src) >> 8)