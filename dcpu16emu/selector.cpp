#include "stdafx.h"
#include "cpu.h"

int dcpu16::read_a(uint8_t a, operand& val)
{
	if (a > 0x3F)
		return -1;

	int cycle = 0;
	val.type = operand::VAL;
	if (0x0 <= a && a <= 0x7)
		val.val = reg[a];
	else if (0x8 <= a && a <= 0xF)
	{
		val.type = operand::PTR;
		val.val = reg[a - 0x8];
	}
	else if (0x10 <= a && a <= 0x17)
	{
		val.type = operand::PTR;
		val.val = static_cast<uint16_t>(reg[a - 0x10] + mem[pc++]);
		if (pc > 0xFFFF)
		{
			pcOf = true;
			pc = static_cast<uint16_t>(pc);
		}
		cycle = 1;
	}
	else if (0x20 <= a && a <= 0x3F)
		val.val = a - 0x21;
	else
	{
		switch (a)
		{
			case 0x18:
				val.val = mem[sp++];
				break;
			case 0x19:
				val.val = mem[sp];
				break;
			case 0x1A:
				val.type = operand::PTR;
				val.val = static_cast<uint16_t>(sp + mem[pc++]);
				if (pc > 0xFFFF)
				{
					pcOf = true;
					pc = static_cast<uint16_t>(pc);
				}
				cycle = 1;
				break;
			case 0x1B:
				val.val = sp;
				break;
			case 0x1C:
				val.val = pc;
				break;
			case 0x1D:
				val.val = ex;
				break;
			case 0x1E:
				val.type = operand::PTR;
				val.val = mem[pc++];
				if (pc > 0xFFFF)
				{
					pcOf = true;
					pc = static_cast<uint16_t>(pc);
				}
				cycle = 1;
				break;
			case 0x1F:
				val.type = operand::PTR;
				val.val = pc++;
				if (pc > 0xFFFF)
				{
					pcOf = true;
					pc = static_cast<uint16_t>(pc);
				}
				cycle = 1;
				break;
			default:
				return -1;
		}
	}
	return cycle;
}

int dcpu16::read_b(uint8_t b, operand& val)
{
	if (b > 0x1F)
		return -1;

	int cycle = 0;
	val.type = operand::VAL;
	if (0x0 <= b && b <= 0x7)
		val.val = reg[b];
	else if (0x8 <= b && b <= 0xF)
	{
		val.type = operand::PTR;
		val.val = reg[b - 0x8];
	}
	else if (0x10 <= b && b <= 0x17)
	{
		val.type = operand::PTR;
		val.val = static_cast<uint16_t>(reg[b - 0x10] + mem[pc++]);
		if (pc > 0xFFFF)
		{
			pcOf = true;
			pc = static_cast<uint16_t>(pc);
		}
		cycle = 1;
	}
	else
	{
		switch (b)
		{
			case 0x18:
				val.val = mem[sp];
				break;
			case 0x19:
				val.val = mem[sp];
				break;
			case 0x1A:
				val.type = operand::PTR;
				val.val = static_cast<uint16_t>(sp + mem[pc++]);
				if (pc > 0xFFFF)
				{
					pcOf = true;
					pc = static_cast<uint16_t>(pc);
				}
				cycle = 1;
				break;
			case 0x1B:
				val.val = sp;
				break;
			case 0x1C:
				val.val = pc;
				break;
			case 0x1D:
				val.val = ex;
				break;
			case 0x1E:
				val.type = operand::PTR;
				val.val = mem[pc++];
				if (pc > 0xFFFF)
				{
					pcOf = true;
					pc = static_cast<uint16_t>(pc);
				}
				cycle = 1;
				break;
			case 0x1F:
				val.type = operand::PTR;
				val.val = pc++;
				if (pc > 0xFFFF)
				{
					pcOf = true;
					pc = static_cast<uint16_t>(pc);
				}
				cycle = 1;
				break;
			default:
				return -1;
		}
	}
	return cycle;
}

int dcpu16::write_b(uint8_t b, const operand& old, uint16_t val)
{
	if (b > 0x1F)
		return -1;

	if (0x0 <= b && b <= 0x7)
		reg[b] = val;
	else if (0x8 <= b && b <= 0x17)
		mem[old.val] = val;
	else
	{
		switch (b)
		{
			case 0x18:
				mem[--sp] = val;
				break;
			case 0x19:
				mem[sp] = val;
				break;
			case 0x1A:
				mem[old.val] = val;
				break;
			case 0x1B:
				sp = val;
				break;
			case 0x1C:
				pc = val;
				break;
			case 0x1D:
				ex = val;
				break;
			case 0x1E:
			case 0x1F:
				mem[old.val] = val;
				break;
			default:
				return -1;
		}
	}
	return 0;
}
