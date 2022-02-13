#pragma once
#include <algorithm>
#include <iostream>
#include "registers.h"
#include <numeric>
#include "stack.h"
#include "memory.h"

struct RegisterWrite2x8Read16
{
	bool work(void)
	{
		for(const auto reg: {REGISTERS_LIST})
		{
			writeRegister(static_cast<RegisterID>(reg * 2 + AH), 0xAF);
			writeRegister(static_cast<RegisterID>(reg * 2 + AL), 0xFA);

			if (readRegister(reg) != 0xAFFA)
				return false;
		}
		return true;
	}

	void cleanup(void)
	{
		for (const auto reg : { REGISTERS_LIST })
		{
			writeRegister(static_cast<RegisterID>(reg), 0x0);
		}
	}
};

struct RegisterWrite16Read2x8
{
	bool work(void)
	{
		for (const auto reg : { REGISTERS_LIST })
		{
			writeRegister(reg, 0xAFFA);

			if (const bool result = readRegister(static_cast<RegisterID>(reg * 2 + AH))
				&& readRegister(static_cast<RegisterID>(reg * 2 + AL)); !result)
				return false;
		}
		return true;
	}
	void cleanup(void)
	{
		for (const auto reg : { REGISTERS_LIST })
		{
			writeRegister(static_cast<RegisterID>(reg), 0x0);
		}
	}
};

struct MemoryWrite2x8Read16
{
	bool work(void)
	{
		std::vector<addr_t> addresses{};
		std::iota(std::begin(addresses), std::end(addresses), ramSize / 2);
		for (const addr_t i : addresses)
		{
			mc.write8(i * 2, 0xF1);
			mc.write8(i * 2 + 1, 0x1F);
			if (mc.read16(i * 2) != 0xF11F)
				return false;
		}
		return true;
	}
	void cleanup(void)
	{
		mc.fill(0);
	}
};

struct MemoryWrite16Read2x8
{
	bool work(void)
	{
		std::vector<addr_t> addresses{};
		std::iota(std::begin(addresses), std::end(addresses), ramSize / 2);
		for (const addr_t i : addresses)
		{
			mc.write16(i * 2, i);
			if (mc.read16(i * 2) != i)
				return false;
		}
		return true;
	}
	void cleanup(void)
	{
		mc.fill(0);
	}
};

struct StackWriteReadTest
{
	bool work(void)
	{
		writeRegister(SP, 0xF00);
		writeRegister(IP, 0xFA2);
		StackController::push8(0xCB);
		StackController::push16(0x8AF8);
		StackController::push16(0x7FA7);

		auto cSp = readRegister(SP);
		StackController::push(IP);
		writeRegister(IP, 0x00);

		bool result = true;

		StackController::pop(IP);
		result = result && (readRegister(IP) == 0xFA2);
		result = result && (StackController::pop16() == 0x7FA7);
		result = result && (StackController::pop16() == 0x8AF8);
		result = result && (StackController::pop8() == 0xCB);
		result = result && (readRegister(SP) == 0xF00);

		return result;
	}

	void cleanup(void)
	{
		mc.fill(0);
		writeRegister(SP, 0);
		writeRegister(IP, 0);
	}
};

struct UnitTester
{
	template<typename T>
	static void test(void)
	{
		std::cout << std::string(typeid(T).name()).substr(7) << ": " << (T().work() ? "PASSED" : "FAILED") << std::endl;
		T().cleanup();
	}
	static void work(void)
	{
		test<RegisterWrite2x8Read16>();
		test<RegisterWrite16Read2x8>();
		test<MemoryWrite2x8Read16>();
		test<MemoryWrite16Read2x8>();
		test<StackWriteReadTest>();
	}

	UnitTester(void) = delete;
};