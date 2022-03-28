#include "utility.h"
#include <map>
#include <vector>
#include <string>
#include <fstream>

unsigned a16toi(const std::string& str)
{
	unsigned result = 0;
	sscanf(str.c_str(), "%X", &result);
	return result;
}

size_t errors = 0, warnings = 0;

std::map<std::string, std::vector<std::string>> lines;

std::string formatErrorPrefix(const std::string& curFile, size_t curLine)
{
	return (std::string("[") + curFile + ":") + std::to_string(curLine) + "] " + (lines[curFile][curLine]) + " ";
}
std::string formatErrorPrefix(const std::string& curFile)
{
	return std::string("[") + curFile + ": ";
}

void error(const std::string& file, size_t line, const std::string& msg)
{
	std::cerr << formatErrorPrefix(file, line) << "error: " << msg << std::endl;
	++errors;
}

void warning(const std::string& file, size_t line, const std::string& msg)
{
	std::cerr << formatErrorPrefix(file, line) << "warning: " << msg << std::endl;
	++warnings;
}

void warning(const std::string& file, const std::string& msg)
{
	std::cerr << formatErrorPrefix(file) << "warning: " << msg << std::endl;
	++warnings;
}

void warning(const std::string& msg)
{
	std::cerr << "warning: " << msg << std::endl;
	++warnings;
}

size_t getErrorNumber(void)
{
	return errors;
}

size_t getWarningNumber(void)
{
	return warnings;
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

std::string findInclude(const std::string& includeString)
{
	if (includeString.empty())
		return std::string();

	if (std::ifstream(includeString))
		return includeString;

	for (auto&& dir : includeDirs)
	{
		auto full = dir + '/' + includeString;
		if (std::ifstream(full))
			return full;
	}

	return std::string();

}