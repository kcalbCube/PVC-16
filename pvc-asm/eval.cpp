#include "eval.h"
#include <deque>
#include <map>
#include "lexer.h"

void Expression::setLabel(LabelDefinition ld, uint16_t addr)
{
	if(data.index() == 0 && std::get<LabelUse>(data).label == ld.label)
		data = Constant(addr);
	if(left)
		left->setLabel(ld, addr);
	if(right)
		right->setLabel(ld, addr);
}

bool Expression::isConstexpr(void) const
{
	if(data.index() == 1)
		return true;
	if(data.index() == 0)
		return false;
	return (left ? left->isConstexpr() : true) && (right ? right->isConstexpr() : true);
}

unsigned Expression::evaluate(void) const
{
	if(data.index() == 0)
	{
		error(std::string("cannot evaluate expression: undefined symbol ") + std::get<LabelUse>(data).label);
		return 0;
	}

	if(data.index() == 1)
		return std::get<Constant>(data).constant;

#define EXPR_CHECK(check) if(!(check)) { error("bad expression syntax"); return 0; }
#define EXPR_GEN_OP(op) (isSigned() ? std::bit_cast<int>(op left->evaluate()) : (op left->evaluate()))
#define EXPR_GEN_OP2(op) (isSigned() ? (std::bit_cast<int>(left->evaluate()) op std::bit_cast<int>(right->evaluate())) : (left->evaluate() op right->evaluate()))
	switch(operation)
	{
	case Operation::NO:
		EXPR_CHECK(left);
		return EXPR_GEN_OP();
	case Operation::ADD:
		EXPR_CHECK(left && right);
		return EXPR_GEN_OP2(+);
	case Operation::SUB:
		EXPR_CHECK(left && right);
		return EXPR_GEN_OP2(-);
	case Operation::MUL:
		EXPR_CHECK(left && right);
		return EXPR_GEN_OP2(*);
	case Operation::DIV:
	{
		EXPR_CHECK(left && right);
		auto rv = right->evaluate();
		if(rv == 0)
		{
			error("division by zero in expression");
			return 0;
		}
		return EXPR_GEN_OP2(/);
	}
	case Operation::MOD:
	{
		EXPR_CHECK(left && right);
		auto rv = right->evaluate();
		if(rv == 0)
		{
			error("division by zero in expression");	
			return 0;
		}

		return EXPR_GEN_OP2(%);
	}
	case Operation::XOR:
		EXPR_CHECK(left && right);
		return EXPR_GEN_OP2(^);
	case Operation::AND:
		EXPR_CHECK(left && right);
		return EXPR_GEN_OP2(&);
	case Operation::NOT:
		EXPR_CHECK(left);
		return EXPR_GEN_OP(~);
	case Operation::SHL:
		EXPR_CHECK(left && right);
		return EXPR_GEN_OP2(<<);
	case Operation::SHR:
		EXPR_CHECK(left && right);
		return EXPR_GEN_OP2(>>);
	}
	return 0;
}
#undef EXPR_CHECK
#undef EXPR_GEN_OP
#undef EXPR_GEN_OP2

Expression::Expression(const Constant& constant)
	: data{constant} {}

Expression::Expression(const LabelUse& labelUse)
	: data{labelUse} {}

Expression::Expression(const std::vector<Lexema>& lexemas)
{
	std::deque<Expression> output;
	std::deque<Lexema> operators;
	Expression* cur = new Expression();

	std::map<std::string, Operation> string2operation = 
	{
		{"+", Operation::ADD},
		{"-", Operation::SUB},
		{"*", Operation::MUL},
		{"/", Operation::DIV},
		{"%", Operation::MOD},
		{"^", Operation::XOR},
		{"~", Operation::NOT},
		{"&", Operation::AND},
		{"|", Operation::OR},
		{"<", Operation::SHL},
		{">", Operation::SHR},
	};

	for(auto&& lexema : lexemas)
	{
		bool operation = false;
		switch(lexema.id)
		{
			case LexemID::NUMBER:
				output.emplace_back(Constant(std::get<unsigned>(lexema.lexemas)));
				break;
			case LexemID::LABEL_USE:
				output.emplace_back(LabelUse(std::get<std::string>(lexema.lexemas)));
				break;
			case LexemID::OPERATION:
				auto&& op = std::get<std::string>(lexema.lexemas);
				if(false && output.size() == 1) // TODO: unary
				{
					if(!cur->left)
					{
						cur->left = new Expression(output[0]);
						cur->operation = string2operation[op];

						Expression tmp = *cur;
						cur = new Expression();
						cur->left = new Expression(tmp);
					}
					else
					{

					}
				}
				else // TODO: ()
				{
					operators.push_back(lexema);
				}
				operation = true;
				break;
		}

		if(!operation && !operators.empty())
		{
			if(!cur->left && output.size() == 2)
			{
				Expression* expr = new Expression();
				expr->left = new Expression(output[0]);
				expr->right = new Expression(output[1]);
				expr->operation = string2operation[std::get<std::string>(operators[0].lexemas)];
				cur = expr;
			}
			else if(output.size() == 1)
			{
				Expression* expr = new Expression();
				expr->left = cur;
				expr->right = new Expression(output[0]);
				expr->operation = string2operation[std::get<std::string>(operators[0].lexemas)];
				cur = expr;
			}
			else
				error("EXom1: operator miss in expression.");
			operators.clear();
			output.clear();
		}
		
	}

	*this = *cur;
	delete cur;
}

Expression& Expression::operator=(const Expression& other)
{
	if(this != &other)
	{
		operation = other.operation;
		left = other.left;
		right = other.right;
		type = other.type;
	}
	
	return *this;
}
