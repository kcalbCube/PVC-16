#pragma once
#include <algorithm>
#include <iostream>
#include "registers.h"
#include <numeric>
#include "stack.h"
#include "memory.h"
#include <cstring>

struct RegisterWrite2x8Read16
{
	bool work(void)
	{
		for (registers::RegisterID reg = registers::A; reg < registers::AH; ++(int&)reg)
		{
			write(static_cast<registers::RegisterID>(reg * 2 + registers::AH), 0xAF);
			write(static_cast<registers::RegisterID>(reg * 2 + registers::AL), 0xFA);

			if (read(reg) != 0xAFFA)
				return false;
		}
		return true;
	}

	void cleanup(void)
	{
		for (registers::RegisterID reg = registers::A; reg < registers::AH; ++(int&)reg)
			write(reg, 0x0);
	}
};

struct RegisterWrite16Read2x8
{
	bool work(void)
	{
		for (registers::RegisterID reg = registers::A; reg < registers::AH; ++(int&)reg)
		{
			write(reg, 0xAFFA);

			if (const bool result = read(static_cast<registers::RegisterID>((unsigned int)reg * 2 + (unsigned int)registers::AH))
				&& read(static_cast<registers::RegisterID>((unsigned int)reg * 2 + (unsigned int)registers::AL)); !result)
				return false;
		}
		return true;
	}
	void cleanup(void)
	{
		for (registers::RegisterID reg = registers::A; reg < registers::AH; ++(int&)reg)
		{
			write(reg, 0x0);
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
		write(registers::SP, 0xF00);
		write(registers::IP, 0xFA2);
		stack::push8(0xCB);
		stack::push16(0x8AF8);
		stack::push16(0x7FA7);

		auto cSp = read(registers::SP);
		stack::push(registers::IP);
		write(registers::IP, 0x00);

		bool result = true;

		stack::pop(registers::IP);
		result = result && (read(registers::IP) == 0xFA2);
		result = result && (stack::pop16() == 0x7FA7);
		result = result && (stack::pop16() == 0x8AF8);
		result = result && (stack::pop8() == 0xCB);
		result = result && (read(registers::SP) == 0xF00);

		return result;
	}

	void cleanup(void)
	{
		mc.fill(0);
		write(registers::SP, 0);
		write(registers::IP, 0);
	}
};

struct StackFPushFPop
{
	bool work(void)
	{
		write(registers::SP, 0xF00);
		memset(&registers::status, 0xFF, sizeof(registers::status));

		registers::status.interrupt = 1;
		registers::status.zero = 1;
		registers::status.overflow = 1;
		registers::status.sign = 1;

		auto oldStatus = registers::status;
		stack::pushf();
		memset(&registers::status, 0x00, sizeof(registers::status));
		stack::popf();

		bool result = true;

		result = result && (registers::status.interrupt);
		result = result && (registers::status.zero);
		result = result && (registers::status.overflow);
		result = result && (registers::status.sign);
		result = result && (read(registers::SP) == 0xF00);

		return result;

	}
	void cleanup(void)
	{
		mc.fill(0);
		write(registers::SP, 0);
		memset(&registers::status, 0x00, sizeof(registers::status));
		registers::status.interrupt = 1;
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
		test<StackFPushFPop>();
	}

	UnitTester(void) = delete;
};