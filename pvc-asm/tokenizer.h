#pragma once
#include <boost/tokenizer.hpp>

using Token = std::string;

class Tokenizer
{
	/// <summary>
	/// handles quotes, spaces,
	/// </summary>
	static std::vector<Token> coreTokenize(std::string src);

	/// <summary>
	/// handles operators, merges some, removes comments.
	/// </summary>
	///	<example>
	///	"[%ax+%bx]" => "[", "%ax", "+", "%bx", "]"
	///	</example>
	static void operatorTokenize(std::vector<Token> src, std::vector<Token>& dest);
public:
	static std::vector<Token> tokenize(const std::string& src);
};