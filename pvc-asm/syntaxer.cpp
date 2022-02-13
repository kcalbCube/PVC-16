#include "syntaxer.h"

#include "utility.h"

#include "../PVC-16/opcode.h"

uint16_t Mnemonic::describeMnemonics(void) const
{
	uint16_t result = 0, i = 0;
	for (auto&& m : mnemonics)
		result += (REGISTER + m.index()) << i++ * 4;

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

	for (auto& [id, lexemas, file, line] : lexems)
	{
		switch(id)
		{
			case LexemID::MNEMONIC:
				finishMnemonic();
				inMnemonic = true;
				mnemonic.name = std::get<std::string>(lexemas);
				mnemonic.file = file;
				mnemonic.line = line;
				break;
			case LexemID::LABEL:
				finishMnemonic();

				mnemonics.emplace_back(LabelDefinition(std::get<std::string>(lexemas)));
				break;

			case LexemID::INDIRECT_ADDRESS:
			{
				assert(inMnemonic);
				auto&& op = std::get<std::vector<Lexema>>(lexemas);
				try
				{
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
								.disp = Constant(std::get<unsigned>(op[0].lexemas))
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
								error(file, line, "indirect address syntax error");
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
										.disp = Constant(std::get<unsigned>(op[2].lexemas))
										});
								else
									mnemonic.mnemonics.emplace_back(IndirectAddress{
										.index = Register(std::get<std::string>(op[0].lexemas)),
										.disp = Constant(std::get<unsigned>(op[2].lexemas))
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
									.scale = static_cast<uint8_t>(std::get<unsigned>(op[0].lexemas))
									});
						}
						break;

						default:
							error(file, line, "bad indirect address syntax");
							break;
						}
					}
					break;
					case 5: // 8 * %b + %a
						switch (op[4].id)
						{
						case LexemID::REGISTER:
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.base = Register(std::get<std::string>(op[4].lexemas)),
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<unsigned>(op[0].lexemas))
								});
							break;

						case LexemID::NUMBER:
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<unsigned>(op[0].lexemas)),
								.disp = Constant(std::get<unsigned>(op[4].lexemas))
								});
							break;

						case LexemID::LABEL_USE:
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<unsigned>(op[0].lexemas)),
								.disp = LabelUse(std::get<std::string>(op[4].lexemas))
								});
							break;

						default:
							error(file, line, "bad indirect address syntax");
							break;
						}
						break;
					case 7: // 8 * %b + %a + FFh
						if (op[6].id == LexemID::NUMBER)
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.base = Register(std::get<std::string>(op[4].lexemas)),
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<unsigned>(op[0].lexemas)),
								.disp = Constant(std::get<unsigned>(op[6].lexemas))
								});
						else if (op[6].id == LexemID::LABEL_USE)
							mnemonic.mnemonics.emplace_back(IndirectAddress{
								.base = Register(std::get<std::string>(op[4].lexemas)),
								.index = Register(std::get<std::string>(op[2].lexemas)),
								.scale = static_cast<uint8_t>(std::get<unsigned>(op[0].lexemas)),
								.disp = LabelUse(std::get<std::string>(op[6].lexemas))
								});
						break;
					}
				}
				catch (std::bad_variant_access&)
				{
					error(file, line, "indirect address not match");
				};
					
			}
				break;
			case LexemID::NUMBER:
			{
				if(!inMnemonic)
					error(file, line, "number outside mnemonic.");
				mnemonic.mnemonics.emplace_back(Constant(std::get<unsigned>(lexemas)));
			}
			break;
			case LexemID::LABEL_USE:
				{
				if (!inMnemonic)
					error(file, line, "label use outside mnemonic.");
				mnemonic.mnemonics.emplace_back(LabelUse(std::get<std::string>(lexemas)));
				}
				break;
			case LexemID::REGISTER: 
			{
				if (!inMnemonic)
					error(file, line, "register outside mnemonic.");
				mnemonic.mnemonics.emplace_back(Register(std::get<std::string>(lexemas)));
			}
			break;

			case LexemID::STRING: 
			{
				if (!inMnemonic)
					error(file, line, "string outside mnemonic.");
				mnemonic.mnemonics.emplace_back(String(std::get<std::string>(lexemas)));
			}
			break;

		default: break;
		}
	
	}

	finishMnemonic();

	return mnemonics;
}
