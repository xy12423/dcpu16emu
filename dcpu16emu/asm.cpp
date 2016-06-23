#include "stdafx.h"
#include "asm.h"
#include "utils.h"

const char* op_str_1[] = {
	nullptr,		"NOP",			"PUSHA",		"POPA",
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
	"RET",
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
		ret.append(toHEX(read()));
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
				ret.append(toHEX(read()));
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
				ret.append(toHEX(read()));
				ret.push_back(']');
				break;
            }
			case 0x1F:
			{
				ret.append(toHEX(read()));
				break;
            }
			default:
				throw(dcpu16_asm_error(badbit));
		}
	}
}

uint16_t dcpu16_asm::read()
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
	for (size_t i = 0; i < last_io_size; ++out, ++itr)
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

void dcpu16_asm::read(std::ostream& out)
{
	std::string tmp;
	read(tmp);
	while (good())
	{
		tmp.push_back('\n');
		out.write(tmp.data(), tmp.size());
		tmp.clear();
		read(tmp);
	}
}

void dcpu16_asm::write(const uint16_t* in, size_t size)
{
	buffer.insert(buffer.end(), in, in + size);
	last_io_size = size;
}
