#pragma once
#include <string>
#include <iostream>

int a16toi(const std::string& str);

void error(const std::string& file, size_t line, const std::string& msg);

size_t getErrorNumber(void);

void reserveLines(const std::string& file);
void writeLine(const std::string& file, std::string msg);

std::string& getNextLine(const std::string& file);

inline std::string curFile = ""; // fixme