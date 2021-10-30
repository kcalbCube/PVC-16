#include "syntaxer.h"

#include "utility.h"

std::vector<SyntaxUnit> Syntaxer::syntaxParse(const std::vector<Lexema>& lexems)
{
	std::vector<SyntaxUnit> mnemonics;

	Mnemonic mnemonic;

	bool inMnemonic = false;
	auto finishMnemonic = [&](void) -> void
	{
		if (mnemonic.name.empty() || !inMnemonic)
			return;
		mnemonics.emplace_back(mnemonic);
		mnemonic = Mnemonic();
		inMnemonic = false;
	};									 

	for (const auto& [id, lexemas] : lexems)
	{
		switch(id)
		{
			case LexemID::MNEMONIC:
				finishMnemonic();
				inMnemonic = true;
				mnemonic.name = std::get<std::string>(lexemas);
				break;
			case LexemID::LABEL:
				finishMnemonic();
				mnemonics.emplace_back(LabelDefinition(std::get<std::string>(lexemas)));
				break;

			case LexemID::INDIRECT_ADDRESS:
			{
				assert(inMnemonic);
				auto&& op = std::get<std::vector<Lexema>>(lexemas);
				if (op.size() == 1)
					if (op[0].id == LexemID::REGISTER)
						mnemonic.mnemonics.emplace_back(IndirectAddress(Register(std::get<std::string>(op[0].lexemas))));
					else if (op[0].id == LexemID::LABEL_USE)
						mnemonic.mnemonics.emplace_back(IndirectAddress(LabelUse(std::get<std::string>(op[0].lexemas))));
				if (op.size() == 3)
				{
					const auto operation = std::get<std::string>(op[1].lexemas)[0] == '+' ? 1 : -1;
					if (op[0].id == LexemID::REGISTER)
						mnemonic.mnemonics.emplace_back(IndirectAddress(Register(std::get<std::string>(op[0].lexemas)), 
						                                                std::get<int>(op[2].lexemas) * operation));
					else if (op[0].id == LexemID::LABEL_USE)
						mnemonic.mnemonics.emplace_back(IndirectAddress(LabelUse(std::get<std::string>(op[0].lexemas)),
						                                                std::get<int>(op[2].lexemas) * operation));
				}
			}
				break;
			case LexemID::NUMBER:
			{
				assert(inMnemonic);
				mnemonic.mnemonics.emplace_back(Constant(std::get<int>(lexemas)));
			}
			break;

			case LexemID::LABEL_USE:
				{
				assert(inMnemonic);
				mnemonic.mnemonics.emplace_back(LabelUse(std::get<std::string>(lexemas)));
				}
				break;

			case LexemID::REGISTER: 
			{
				assert(inMnemonic);
				mnemonic.mnemonics.emplace_back(Register(std::get<std::string>(lexemas)));
			}
			break;

			case LexemID::STRING: 
			{
				assert(inMnemonic);
				mnemonic.mnemonics.emplace_back(String(std::get<std::string>(lexemas)));
			}
			break;
		default: ;
		}
	
	}

	finishMnemonic();

	return mnemonics;
}
