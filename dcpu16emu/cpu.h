#ifndef _H_EMU
#define _H_EMU

class dcpu16
{
public:
	enum registers
	{
		REG_A,
		REG_B,
		REG_C,
		REG_X,
		REG_Y,
		REG_Z,
		REG_I,
		REG_J,

		REG_PC,
		REG_SP,
		REG_EX,
		REG_IA,

		REG_END
	};

	struct instruction
	{
		instruction(uint16_t code)
			:op(code & 0x1F), b((code >> 5) & 0x1F), a(code >> 10)
		{}
		operator uint16_t()
		{
			return ((a << 10) | (b << 5)) | op;
		}

		uint8_t op, b, a;
	};

	struct operand
	{
		enum type_tp { PTR, VAL } type;

		operand() :type(VAL), val(0) {};
		operand(type_tp _type, uint16_t _val)
			:type(_type), val(_val)
		{}
		inline uint16_t get(dcpu16* _base)
		{
			return type == PTR ? _base->mem[val] : val;
		}

		uint16_t val;
	};
	friend struct operand;

	struct hardware
	{
		typedef uint16_t(*fHWInt)();
		typedef int32_t(*fInit)();

		hardware(uint16_t _a, uint16_t _b, uint16_t _c, uint16_t _x, uint16_t _y, fHWInt _int, fInit _init)
			:a(_a), b(_b), c(_c), x(_x), y(_y), interrupt(_int), init(_init)
		{};

		uint16_t a, b, c, x, y;
		fHWInt interrupt;
		fInit init;
	};

	enum emu_err
	{
		ERR_EMU_NOERR,
		ERR_EMU_OTHER,

		ERR_EMU_READ,
		ERR_EMU_WRITE,
		ERR_EMU_UNRECOGNIZED,

		ERR_EMU_ITR_EMPTY,
		ERR_EMU_ITR_OVERFLOW,

		ERR_EMU_EXP_MEMOVFL
	};

	dcpu16();

	int step();

	bool set_reg(int reg_id, uint16_t val);
	bool set_mem(uint16_t ptr, uint16_t val);
	bool get_reg(int reg_id, uint16_t& ret);
	bool get_mem(uint16_t ptr, uint16_t& ret);
	int interrupt(uint16_t _int) { return add_itr(_int); }
	void reset();

	void add_hardware(const hardware& hw) { hw_table.push_back(hw); };

	bool pcOf = false;

private:
	int do_3(const instruction& ins);
	int do_2(const instruction& ins);
	int do_1(const instruction& ins);

	int read_a(uint8_t a, operand& val);
	int read_b(uint8_t b, operand& val);
	int write_b(uint8_t b, const operand& old, uint16_t val);

	int add_itr(uint16_t _int);
	int pop_itr(uint16_t& _int);

	void skipsingle();
	int skip();

	std::unique_ptr<uint16_t[]> mem;
	uint16_t reg[8];
	uint32_t pc;
	uint16_t sp, ex, ia;
	uint16_t int_que[256];
	uint8_t int_begin = 0, int_end = 0;
	bool int_enabled = true;
	std::vector<hardware> hw_table;
};

#endif // _H_EMU
