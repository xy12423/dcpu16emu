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
			throw(dcpu16_asm_error());
	}
	else
	{
		if (arg > 0x1F)
			throw(dcpu16_asm_error());
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
		uint16_t shift;
		read(shift);
		ret.append(toHEX(shift));
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
				uint16_t shift;
				read(shift);
				ret.append(toHEX(shift));
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
				uint16_t shift;
				read(shift);
				ret.append(toHEX(shift));
				ret.push_back(']');
				break;
            }
			case 0x1F:
			{
				uint16_t shift;
				read(shift);
				ret.append(toHEX(shift));
				break;
            }
			default:
				throw(dcpu16_asm_error());
		}
	}
}

void dcpu16_asm::read(std::string& out)
{
	last_io_size = 0;
	if (buffer.empty())
		return;
	dcpu16::instruction ins = buffer.front();
	buffer.pop_front();
	last_io_size += 1;

	try
	{
		uint8_t type = 0;
		uint8_t op = 0, b = 0, a = 0;
		if (ins.op != 0)
		{
			type = 3;
			op = ins.op;
			b = ins.b;
			a = ins.a;
		}
		else if (ins.b != 0)
		{
			type = 2;
			op = ins.b;
			a = ins.a;
		}
		else if (ins.a != 0)
		{
			type = 1;
			op = ins.a;
		}
		else
		{
			throw(dcpu16_asm_error());
		}

		switch (type)
		{
			case 3:
				if (op_str_3[op] == nullptr)
					throw(dcpu16_asm_error());
				out.append(op_str_3[op]);
				break;
			case 2:
				if (op_str_2[op] == nullptr)
					throw(dcpu16_asm_error());
				out.append(op_str_2[op]);
				break;
			case 1:
				if (op_str_1[op] == nullptr)
					throw(dcpu16_asm_error());
				out.append(op_str_1[op]);
				break;
		}

		switch (type)
		{
			case 3:
				out.push_back(' ');
				dasm_arg(b, false, out);
				out.append(", ");
				dasm_arg(a, true, out);
				break;
			case 2:
				out.push_back(' ');
				dasm_arg(a, true, out);
				break;
		}
	}
	catch (dcpu16_asm_error &)
	{
		out.append("DAT ");
		out.append(toHEX(ins));
	}
}

void dcpu16_asm::read(std::ostream& out)
{
	std::string tmp;
	read(tmp);
	out.write(tmp.data(), tmp.size());
}
