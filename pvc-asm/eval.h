#pragma once
#include <ranges>
#include <variant>
#include <vector>
#include <bit>
#include "utility.h"

class Lexema;
class Expression
{
public:
	enum class Operation
	{
		NO = '\0',
		ADD = '+',
		SUB = '-',
		DIV = '/',
		MUL = '*',
		MOD = '%',
		XOR = '^',
		AND = '&',
		NOT = '~',
		SHL = '<', // y
		SHR = '>',
		OR  = '|',
	} operation = Operation::NO;

	Expression* left = nullptr;
	Expression* right = nullptr;

	enum
	{
		U8,
		U16,
		I8,
		I16
	} type = U16;
	inline bool isSigned(void) const { return type >= I8; }
	inline bool isByte(void) const { return type == U8 || type == I8; }
	inline void toByte(void) { if(!isByte()) --((unsigned&)type); }
	bool isConstexpr(void) const ;
	std::variant<LabelUse, Constant, uint8_t> data = uint8_t(0);
	void setLabel(LabelDefinition, uint16_t);
	unsigned evaluate(void) const;

	Expression& operator=(const Expression&);

	Expression(void) = default;
	Expression(const Constant&);
	Expression(const LabelUse&);
	Expression(Expression*, Expression*);
	Expression(const std::vector<Lexema>&);
};