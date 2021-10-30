#pragma once
#include <boost/tokenizer.hpp>

using Token = std::string;

class Tokenizer
{
	/// <summary>
	/// handles quotes, spaces.
	/// </summary>
	static std::vector<Token> coreTokenize(const std::string& src);

	/// <summary>
	/// handles operators, merges some.
	/// </summary>
	///	<example>
	///	"[%ax+%bx]" => "[", "%ax", "+", "%bx", "]"
	///	</example>
	///	<param name="src">
	///	pbv
	///	</param>
	static void operatorTokenize(std::vector<Token> src, std::vector<Token>& dest);
public:
	static std::vector<Token> tokenize(const std::string& src);
};