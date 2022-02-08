#include "utility.h"
#include <map>
#include <vector>
#include <string>

int a16toi(const std::string& str)
{
	int result = 0;
	sscanf_s(str.c_str(), "%X%*c", &result);
	return result;
}

size_t errors = 0;

std::map<std::string, std::vector<std::string>> lines;

std::string formatErrorPrefix(const std::string& curFile, size_t curLine)
{
	return (std::string("[") + curFile + ":") + std::to_string(curLine) + "] " + (lines[curFile][curLine]) + " < ";
}

void error(const std::string& file, size_t line, const std::string& msg)
{
	std::cerr << formatErrorPrefix(file, line) << "error: " << msg << std::endl;
	++errors;
}

size_t getErrorNumber(void)
{
	return errors;
}

void reserveLines(const std::string& file)
{
	lines[file].reserve(512);
}

void writeLine(const std::string& file, std::string msg)
{
	lines[file].emplace_back(std::move(msg));
}

std::string& getNextLine(const std::string& file)
{
	return lines[file].emplace_back();
}