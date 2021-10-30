#pragma once
#include <string>

inline int a16toi(const std::string& str)
{
	int result = 0;
	sscanf_s(str.c_str(), "%X%*c", &result);
	return result;
}
