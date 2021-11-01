#include "syntaxer.h"

#include "utility.h"

#include "../PVC-16/opcode.h"

uint16_t Mnemonic::describeMnemonics(void) const
{
	uint16_t result = 0;
	size_t i = 0;
	for (auto&& m : mnemonics)
		result += m.index() << i++ * 4;

	return result;
}

std::vector<SyntaxUnit> Syntaxer::syntaxParse(std::vector<Lexema>& lexems)
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

	for (auto& [id, lexemas] : lexems)
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
				switch (op.size())
				{
				case 1: // %a
					switch (op[0].id)
					{
					case LexemID::REGISTER:
						if (isSIBbase(registerName2registerId.at(std::get<std::string>(op[0].lexemas))))
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.base = Register(std::get<std::string>(op[0].lexemas))
								});
						else
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.index = Register(std::get<std::string>(op[0].lexemas))
								});

						break;
					case LexemID::LABEL_USE:
						mnemonic.mnemonics.emplace_back(IndirectAddress{
							.disp = LabelUse(std::get<std::string>(op[0].lexemas))
								});
						break;
					case LexemID::NUMBER:
						mnemonic.mnemonics.emplace_back(IndirectAddress{
							.disp = Constant(std::get<int>(op[0].lexemas))
								});
						break;
					default: abort();
					}
					break;
				case 3:// %a + %b
				{
					switch (const auto operation = std::get<std::string>(op[1].lexemas)[0]; operation)
					{
					case '+':
						if (op[0].id != LexemID::REGISTER)
							abort();
						switch (op[2].id)
						{
						case LexemID::REGISTER:
							if (isSIBbase(registerName2registerId.at(std::get<std::string>(op[0].lexemas))))
								mnemonic.mnemonics.emplace_back(IndirectAddress{
									.base = Register(std::get<std::string>(op[0].lexemas)),
									.index = Register(std::get<std::string>(op[2].lexemas))
									});
							else
								mnemonic.mnemonics.emplace_back(IndirectAddress{
									.base = Register(std::get<std::string>(op[2].lexemas)),
									.index = Register(std::get<std::string>(op[0].lexemas))
									});
							break;
						case LexemID::NUMBER:
							if (isSIBbase(registerName2registerId.at(std::get<std::string>(op[0].lexemas))))
								mnemonic.mnemonics.emplace_back(IndirectAddress{
									.base = Register(std::get<std::string>(op[0].lexemas)),
									.disp = Constant(std::get<int>(op[2].lexemas))
									});
							else
								mnemonic.mnemonics.emplace_back(IndirectAddress{
									.index = Register(std::get<std::string>(op[0].lexemas)),
									.disp = Constant(std::get<int>(op[2].lexemas))
									});
							break;
						case LexemID::LABEL_USE:
							if (isSIBbase(registerName2registerId.at(std::get<std::string>(op[0].lexemas))))
								mnemonic.mnemonics.emplace_back(IndirectAddress{
									.base = Register(std::get<std::string>(op[0].lexemas)),
									.disp = LabelUse(std::get<std::string>(op[2].lexemas))
									});
							else
								mnemonic.mnemonics.emplace_back(IndirectAddress{
								.index = Register(std::get<std::string>(op[0].lexemas)),
								.disp = LabelUse(std::get<std::string>(op[2].lexemas))
									});
							break;
						default: abort();
						}
						break;
					case '*':
					{
						if (op[0].id == LexemID::NUMBER && op[2].id == LexemID::REGISTER)
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.base = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<int>(op[0].lexemas))
								});
					}
					break;

					default: abort(); break;
					}
				}
					break;
				case 5: // 8 * %b + %a
#if 0
						if(std::get<std::string>(op[1].lexemas) == "+") // %a + 8 * %b
						{ // transform to normal format
							std::swap(op[0], op[4]); // %b + 8 * %a
							std::swap(op[1], op[3]); // %b * 8 + %a
							std::swap(op[0], op[2]); // 8 * %b + %a
						}
#endif
						switch (op[4].id)
						{
						case LexemID::REGISTER:
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.base = Register(std::get<std::string>(op[4].lexemas)),
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<int>(op[0].lexemas))
								});
							break;

						case LexemID::NUMBER:
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<int>(op[0].lexemas)),
								.disp = Constant(std::get<int>(op[4].lexemas))
								});
							break;

						case LexemID::LABEL_USE:
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<int>(op[0].lexemas)),
								.disp = LabelUse(std::get<std::string>(op[4].lexemas))
								});
							break;

						default: abort();
						}
					break;
				case 7: // 8 * %b + %a + FFh
#if 0
						if (std::get<std::string>(op[1].lexemas) == "+") // %a + 8 * %b + FFh
						{ // transform to normal format
							std::swap(op[0], op[4]); // %b + 8 * %a
							std::swap(op[1], op[3]); // %b * 8 + %a
							std::swap(op[0], op[2]); // 8 * %b + %a
						}
#endif
						if (op[6].id == LexemID::NUMBER)
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.base = Register(std::get<std::string>(op[4].lexemas)),
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<int>(op[0].lexemas)),
								.disp = Constant(std::get<int>(op[6].lexemas))
								});
						else if (op[6].id == LexemID::LABEL_USE)
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.base = Register(std::get<std::string>(op[4].lexemas)),
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<int>(op[0].lexemas)),
								.disp = LabelUse(std::get<std::string>(op[6].lexemas))
								});
						break;
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
		default: break;
		}
	
	}

	finishMnemonic();

	return mnemonics;
}
