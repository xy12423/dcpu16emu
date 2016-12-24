#include "stdafx.h"
#include "asm.h"
#include "asm_yacc.h"
#include "utils.h"

extern bool _skip_incpc[0x40];

const char* op_str_1[] = {
	nullptr,		"NOP",			nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
	nullptr,		nullptr,		nullptr,		nullptr,
};

const char* op_str_2[] = {
	nullptr,
	"JSR",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	"INT",
	"IAG",
	"IAS",
	"RFI",
	"IAQ",
	nullptr,
	nullptr,
	nullptr,
	"HWN",
	"HWQ",
	"HWI",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
};

const char* op_str_3[] = {
	nullptr,
	"SET",
	"ADD",
	"SUB",
	"MUL",
	"MLI",
	"DIV",
	"DVI",
	"MOD",
	"MDI",
	"AND",
	"BOR",
	"XOR",
	"SHR",
	"ASR",
	"SHL",
	"IFB",
	"IFC",
	"IFE",
	"IFN",
	"IFG",
	"IFA",
	"IFL",
	"IFU",
	nullptr,
	nullptr,
	"ADX",
	"SBX",
	nullptr,
	nullptr,
	"STI",
	"STD",
};

const char* reg_str[] = {
	"A",
	"B",
	"C",
	"X",
	"Y",
	"Z",
	"I",
	"J",
};

int8_t lexer_state_op[][26] = {
	{ 3,15,0,9,0,0,0,31,21,25,0,0,6,33,0,0,0,29,1,0,0,0,0,17,0,0, },
	{ 0,23,0,0,2,0,0,19,0,0,0,0,0,0,0,0,0,0,0,24,5,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,0,0,0,0,0, },
	{ 0,0,0,4,0,0,0,0,0,0,0,0,0,14,0,0,0,0,20,0,0,0,0,0,0,0, },
	{ 0,0,0,-2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-24,0,0, },
	{ 0,-3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,13,0,0,0,0,0,0,0,8,0,0,12,0,0,0,0,0,7,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,-4,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,-5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,10,0,0,0,0,0,0,0,0,0,0,0,0,11,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-6,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,-7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,-8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,-9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,-10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-11,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,18,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-12,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,-15,0,0,0,0,0,-13,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-14,0,0,0,0,0,0,0,0, },
	{ 28,0,0,0,0,22,0,0,0,0,0,0,0,27,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ -21,-16,-17,0,-18,0,-20,0,0,0,0,-22,0,-19,0,0,0,0,0,0,-23,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-25,0,0, },
	{ 0,0,0,-27,0,0,0,0,-26,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-28,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-29,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,-30,0,0,0,0,0,0,0,0,0,-33,0,-31,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,-32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0, },
	{ 0,0,0,0,0,0,0,0,-36,0,0,0,0,-34,0,0,-35,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,0,0,0,0,0,0,0,0,0,0,0, },
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-37,0,0,0,0,0,0,0,0,0,0, },
};
uint8_t lexer_state_op_final[] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,26,27,30,31,33,40,41,42,43,44,48,49,50,65, };

void dcpu16_asm::asm_ins(const char** itr, const char* itr_end, inter_instruction& ret)
{
	int state = 0;
	do
	{
		if (*itr == itr_end)
			throw(dcpu16_asm_error(failbit));
		if (!isalpha(**itr))
			throw(dcpu16_asm_error(failbit));
		state = lexer_state_op[state][toupper(**itr) - 'A'];
		if (state == 0)
			throw(dcpu16_asm_error(failbit));
		else if (state < 0)
		{
			uint8_t op = lexer_state_op_final[-state];
			if (op < 0x20)
			{
				ret.ins.op = op;
			}
			else if (op < 0x40)
			{
				ret.ins.op = 0;
				ret.ins.b = op & 0x1F;
			}
			else if (op < 0x80)
			{
				ret.ins.op = 0;
				ret.ins.b = 0;
				ret.ins.a = op & 0x3F;
			}
			else
			{
				throw(dcpu16_asm_error(badbit));
			}
		}
		(*itr)++;
	} while (state > 0);
}

void dcpu16_asm::asm_arg(const char* itr, const char* itr_end, bool is_a, inter_instruction& ret)
{
	asm_yacc_proc proc(itr, itr_end, is_a);
	if (yyparse(&proc) != 0)
		throw(dcpu16_asm_error(failbit));
	asm_yacc_result &result = proc.result;
	if (result.res == 0x1F)
	{
		if (is_a)
		{
			if (result.data == 0xFFFF)
				result.res = 0x20;
			else if (result.data < 0x1F)
				result.res = 0x21 + result.data;
		}
	}
	else if (0x10 <= result.res && result.res <= 0x17)
	{
		if (result.data == 0)
			result.res -= 0x8;
	}
	if (is_a)
	{
		ret.ins.a = result.res;
		ret.a = result.data;
	}
	else
	{
		ret.ins.b = result.res;
		ret.b = result.data;
	}
}

void dcpu16_asm::dasm_arg(uint8_t arg, bool is_a, std::string& ret)
{
	if (is_a)
	{
		if (arg > 0x3F)
			throw(dcpu16_asm_error(badbit));
	}
	else
	{
		if (arg > 0x1F)
			throw(dcpu16_asm_error(badbit));
	}

	if (0x0 <= arg && arg <= 0x7)
		ret.append(reg_str[arg]);
	else if (0x8 <= arg && arg <= 0xF)
	{
		ret.push_back('[');
		ret.append(reg_str[arg - 0x8]);
		ret.push_back(']');
	}
	else if (0x10 <= arg && arg <= 0x17)
	{
		ret.push_back('[');
		ret.append(reg_str[arg - 0x10]);
		ret.append(" + ");
		ret.append(toHEX(get()));
		ret.push_back(']');
	}
	else if (0x20 <= arg && arg <= 0x3F)
		ret.append(toHEX(arg - 0x21));
	else
	{
		switch (arg)
		{
			case 0x18:
				if (is_a)
					ret.append("POP");
				else
					ret.append("PUSH");
				break;
			case 0x19:
				ret.append("PEEK");
				break;
			case 0x1A:
				ret.append("PICK ");
				ret.append(toHEX(get()));
				break;
			case 0x1B:
				ret.append("SP");
				break;
			case 0x1C:
				ret.append("PC");
				break;
			case 0x1D:
				ret.append("EX");
				break;
			case 0x1E:
			{
				ret.push_back('[');
				ret.append(toHEX(get()));
				ret.push_back(']');
				break;
            }
			case 0x1F:
			{
				ret.append(toHEX(get()));
				break;
            }
			default:
				throw(dcpu16_asm_error(badbit));
		}
	}
}

uint16_t dcpu16_asm::get()
{
	if (buffer.empty())
		throw(dcpu16_asm_error(eofbit | failbit));
	uint16_t out = buffer.front();
	buffer.pop_front();
	last_io_size += 1;
	return out;
}

void dcpu16_asm::read(uint16_t* out, size_t size)
{
	last_io_size = std::min(size, buffer.size());
	buf_type::iterator itr = buffer.begin();
	for (uint16_t *out_end = out + last_io_size; out < out_end; ++out, ++itr)
		*out = *itr;
	buffer.erase(buffer.begin(), buffer.begin() + last_io_size);
	check_eof();
}

void dcpu16_asm::read(std::string& out)
{
	last_io_size = 0;
	if (buffer.empty())
	{
		state |= eofbit;
		state |= failbit;
		return;
	}
	dcpu16::instruction ins = buffer.front();
	buffer.pop_front();
	last_io_size += 1;

	try
	{
		uint8_t type = 0;
		if (ins.op != 0)
			type = 3;
		else if (ins.b != 0)
			type = 2;
		else if (ins.a != 0)
			type = 1;
		else
			throw(dcpu16_asm_error(failbit));

		switch (type)
		{
			case 3:
				if (op_str_3[ins.op] == nullptr)
					throw(dcpu16_asm_error(failbit));
				out.append(op_str_3[ins.op]);
				break;
			case 2:
				if (op_str_2[ins.b] == nullptr)
					throw(dcpu16_asm_error(failbit));
				out.append(op_str_2[ins.b]);
				break;
			case 1:
				if (op_str_1[ins.a] == nullptr)
					throw(dcpu16_asm_error(failbit));
				out.append(op_str_1[ins.a]);
				break;
		}

		switch (type)
		{
			case 3:
				out.push_back(' ');
				dasm_arg(ins.b, false, out);
				out.append(", ");
				dasm_arg(ins.a, true, out);
				break;
			case 2:
				out.push_back(' ');
				dasm_arg(ins.a, true, out);
				break;
		}

		check_eof();
	}
	catch (dcpu16_asm_error &ex)
	{
		out.append("DAT ");
		out.append(toHEX(ins));
		state |= ex.rdstate();
	}
}

void dcpu16_asm::write(const uint16_t* in, size_t size)
{
	buffer.insert(buffer.end(), in, in + size);
	last_io_size = size;
}

void dcpu16_asm::write(const std::string& in)
{
	last_io_size = 0;
	const char *itr = in.data(), *itr_end = in.data() + in.size();
	inter_instruction ins;
	try
	{
		asm_ins(&itr, itr_end, ins);
		while (itr != itr_end)
		{
			if (!isspace(*itr))
				break;
			itr++;
		}
		if (itr == itr_end)
		{
			if (ins.ins.op != 0 || ins.ins.b != 0)
				throw(dcpu16_asm_error(failbit));
			buffer.push_back(ins.ins);
			last_io_size += 1;
			return;
		}
		const char *itr_mid = itr;
		for (; itr_mid != itr_end; itr_mid++)
			if (*itr_mid == ',')
				break;
		if (itr_mid == itr_end)
		{
			if (ins.ins.op != 0)
				throw(dcpu16_asm_error(failbit));
			asm_arg(itr, itr_end, true, ins);
			buffer.push_back(ins.ins);
			last_io_size += 1;
			if (_skip_incpc[ins.ins.a])
			{
				buffer.push_back(ins.a);
				last_io_size += 1;
			}
			return;
		}
		asm_arg(itr, itr_mid, false, ins);
		asm_arg(itr_mid + 1, itr_end, true, ins);
		buffer.push_back(ins.ins);
		last_io_size += 1;
		if (_skip_incpc[ins.ins.b])
		{
			buffer.push_back(ins.b);
			last_io_size += 1;
		}
		if (_skip_incpc[ins.ins.a])
		{
			buffer.push_back(ins.a);
			last_io_size += 1;
		}
	}
	catch (dcpu16_asm_error &ex)
	{
		state |= ex.rdstate();
	}
}
