#include "stdafx.h"
#include "cpu.h"

dcpu16::dcpu16()
	:mem(std::make_unique<uint16_t[]>(0x10000))
{
	memset(mem.get(), 0, sizeof(mem));
	memset(reg, 0, sizeof(reg));
	pc = 0;
	sp = ex = ia = 0;
}

bool _skip_incpc[0x40] = {
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	true, true, true, true, true, true, true, true,
	false, false, true, false, false, false, true, true,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false
};

void dcpu16::skipsingle()
{
	opcode code = mem[pc++];
	if (_skip_incpc[code.a])
		pc++;
	if (_skip_incpc[code.b])
		pc++;
	if (pc > 0xFFFF)
	{
		pcOf = true;
		pc = static_cast<uint16_t>(pc);
	}
}

int dcpu16::skip()
{
	opcode code = mem[pc++];
	int ret = 0;
	while (0x10 <= code.op && code.op <= 0x17)
	{
		if (_skip_incpc[code.a])
			pc++;
		if (_skip_incpc[code.b])
			pc++;
		if (pc > 0xFFFF)
		{
			pcOf = true;
			pc = static_cast<uint16_t>(pc);
		}
		code = mem[pc++];
		ret++;
	}
	if (_skip_incpc[code.a])
		pc++;
	if (_skip_incpc[code.b])
		pc++;
	if (pc > 0xFFFF)
	{
		pcOf = true;
		pc = static_cast<uint16_t>(pc);
	}
	ret++;
	return ret;
}

int dcpu16::add_itr(uint16_t _int)
{
	if (int_begin == static_cast<uint8_t>(int_end - 1))
		return _ERR_EMU_ITR_OVERFLOW;
	int_que[int_end++] = _int;
	return _ERR_EMU_NOERR;
}

int dcpu16::pop_itr(uint16_t &_int)
{
	if (int_begin == int_end)
		return _ERR_EMU_ITR_EMPTY;
	_int = int_que[int_begin++];
	return _ERR_EMU_NOERR;
}

int dcpu16::step()
{
	opcode code = mem[pc++];
	if (pc > 0xFFFF)
	{
		pcOf = true;
		pc &= 0xFFFF;
	}

	int cycle = do_3(code);
	if (cycle == -1)
		return _ERR_EMU_OTHER;
	if (int_enabled)
	{
		uint16_t int_val = 0;
		if (ia != 0 && pop_itr(int_val) != _ERR_EMU_ITR_EMPTY)
		{
			int_enabled = false;
			mem[--sp] = pc;
			mem[--sp] = reg[REG_A];
			pc = ia;
			reg[REG_A] = int_val;
		}
	}
	return cycle;
}

int dcpu16::do_1(const opcode& code)
{
	uint16_t op = code.a;
	if (op == 0)
		return -1;

	int cycle = -1;
	switch (op)
	{
		case 0x01:
			cycle = 1;
			break;
		case 0x02:
			for (int i = 0; i < 8; i++)
				mem[--sp] = reg[i];
			cycle = 4;
			break;
		case 0x03:
			for (int i = 7; i >= 0; i--)
				reg[i] = mem[sp++];
			cycle = 4;
			break;
		default:
			return -1;
	}
	return cycle;
}

int dcpu16::do_2(const opcode& code)
{
	uint8_t op = code.b;
	if (op == 0)
		return do_1(code);

	operand a_real;
	int cycle = read_a(code.a, a_real);
	if (cycle < 0)
		return _ERR_EMU_READ;

	uint16_t a = a_real.get(this);
	switch (op)
	{
		case 0x01:
			mem[--sp] = pc;
			pc = a;
			cycle += 3;
			break;
		case 0x02:
			pc = mem[sp++];
			sp += a;
			cycle += 3;
		case 0x08:
			if (ia != 0)
				add_itr(a);
			cycle += 4;
			break;
		case 0x09:
			write_b(code.a, a_real, ia);
			cycle += 1;
			break;
		case 0x0A:
			ia = a;
			cycle += 1;
			break;
		case 0x0B:
			int_enabled = true;
			reg[0] = mem[sp++];
			pc = mem[sp++];
			cycle += 3;
			break;
		case 0x0C:
			if (a == 0)
				int_enabled = true;
			else
				int_enabled = false;
			cycle += 2;
			break;
		case 0x10:
			write_b(code.a, a_real, static_cast<uint16_t>(hw_table.size()));
			cycle += 2;
			break;
		case 0x11:
			if (a < hw_table.size())
			{
				reg[0] = hw_table[a].a;
				reg[1] = hw_table[a].b;
				reg[2] = hw_table[a].c;
				reg[3] = hw_table[a].x;
				reg[4] = hw_table[a].y;
			}
			cycle += 4;
			break;
		case 0x12:
			if (a < hw_table.size())
				cycle += (4 + hw_table[a].interrupt());
			break;
		default:
			return -1;
	}
	return cycle;
}

int dcpu16::do_3(const opcode& code)
{
	if (code.op == 0x00)
		return do_2(code);

	operand a_real, b_real;
	int cycle_t, cycle = 0;
	cycle_t = read_b(code.b, b_real);
	if (cycle_t < 0)
		return _ERR_EMU_READ;
	cycle += cycle_t;
	cycle_t = read_a(code.a, a_real);
	if (cycle_t < 0)
		return _ERR_EMU_READ;
	cycle += cycle_t;

	uint32_t res = 0;
	uint16_t a = a_real.get(this), b = b_real.get(this);
	switch (code.op)
	{
		case 0x01:
			res = a;
			cycle += 1;
			break;
		case 0x02:
			res = b + a;
			ex = res >> 16;
			cycle += 2;
			break;
		case 0x03:
			res = b - a;
			ex = res >> 16;
			cycle += 2;
			break;
		case 0x04:
			res = b * a;
			ex = res >> 16;
			cycle += 2;
			break;
		case 0x05:
			res = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(b)) * static_cast<int16_t>(a));
			ex = res >> 16;
			cycle += 2;
			break;
		case 0x06:
			if (a == 0)
			{
				res = 0;
				ex = 0;
			}
			else
			{
				res = static_cast<uint16_t>(b / a);
				ex = static_cast<uint16_t>((static_cast<uint32_t>(b) << 16) / a);
			}
			cycle += 3;
			break;
		case 0x07:
			if (a == 0)
			{
				res = 0;
				ex = 0;
			}
			else
			{
				int16_t sa = static_cast<int16_t>(a), sb = static_cast<int16_t>(b);
				res = static_cast<uint16_t>(sb / sa);
				ex = static_cast<uint16_t>((static_cast<int32_t>(sb) << 16) / sa);
			}
			cycle += 3;
			break;
		case 0x08:
			if (a == 0)
				res = 0;
			else
				res = static_cast<uint16_t>(b % a);
			cycle += 3;
			break;
		case 0x09:
			if (a == 0)
				res = 0;
			else
			{
				int16_t sa = static_cast<int16_t>(a), sb = static_cast<int16_t>(b);
				res = static_cast<uint16_t>(sb % sa);
			}
			cycle += 3;
			break;
		case 0x0A:
			res = b & a;
			cycle += 1;
			break;
		case 0x0B:
			res = b | a;
			cycle += 1;
			break;
		case 0x0C:
			res = b ^ a;
			cycle += 1;
			break;
		case 0x0D:
			res = b >> a;
			ex = (static_cast<uint32_t>(b) << 16) >> a;
			cycle += 1;
			break;
		case 0x0E:
			res = static_cast<int16_t>(b) >> a;
			ex = (static_cast<int32_t>(b) << 16) >> a;
			cycle += 1;
			break;
		case 0x0F:
			res = b << a;
			ex = (b << a) >> 16;
			cycle += 1;
			break;
		case 0x10:
			cycle += 2;
			if ((b&a) == 0)
				cycle += skip();
			break;
		case 0x11:
			cycle += 2;
			if ((b&a) != 0)
				cycle += skip();
			break;
		case 0x12:
			cycle += 2;
			if (b != a)
				cycle += skip();
			break;
		case 0x13:
			cycle += 2;
			if (b == a)
				cycle += skip();
			break;
		case 0x14:
			cycle += 2;
			if (b <= a)
				cycle += skip();
			break;
		case 0x15:
			cycle += 2;
			if (static_cast<int16_t>(b) <= static_cast<int16_t>(a))
				cycle += skip();
			break;
		case 0x16:
			cycle += 2;
			if (b >= a)
				cycle += skip();
			break;
		case 0x17:
			cycle += 2;
			if (static_cast<int16_t>(b) >= static_cast<int16_t>(a))
				cycle += skip();
			break;
		case 0x1A:
			res = static_cast<uint32_t>(b) + a + ex;
			ex = (res >> 16 ? 1 : 0);
			cycle += 3;
			break;
		case 0x1B:
			res = static_cast<uint32_t>(b) - a + ex;
			ex = (res >> 16 ? 0xFFFF : 0);
			cycle += 3;
			break;
		case 0x1E:
			res = a;
			reg[REG_I]++;
			reg[REG_J]++;
			cycle += 2;
			break;
		case 0x1F:
			res = a;
			reg[REG_I]--;
			reg[REG_J]--;
			cycle += 2;
			break;
		default:
			return -1;
	}

	if (write_b(code.b, b_real, static_cast<uint16_t>(res)) != 0)
		return _ERR_EMU_WRITE;
	return cycle;
}
