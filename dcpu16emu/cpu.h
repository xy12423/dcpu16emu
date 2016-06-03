#ifndef _H_EMU
#define _H_EMU

#ifdef __GNUC__
#define _cdecl/* __attribute__((cdecl)) */
#endif

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
	};

	struct opcode
	{
		opcode(uint16_t code)
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
		typedef uint16_t(__cdecl *fHWInt)();

		uint16_t a, b, c, x, y;
		fHWInt interrupt;
	};
	static const hardware hd_empty;

	typedef uint16_t(__cdecl *fGetHWCount)();
	typedef hardware(__cdecl *fGetInfo)(int n);
	typedef void(__cdecl *fSetHandle)(void *, void *, void *, void *, void *);
	typedef uint32_t(__cdecl *fInit)();

	enum emu_err
	{
		_ERR_EMU_NOERR,
		_ERR_EMU_READ,
		_ERR_EMU_WRITE,
		_ERR_EMU_OTHER,
		_ERR_EMU_ITR_EMPTY,
		_ERR_EMU_ITR_OVERFLOW,
		_ERR_EMU_EXP_MEMOVFL
	};

	dcpu16();

	void run();
	int step();

private:
	int do_3(const opcode& code);
	int do_2(const opcode& code);
	int do_1(const opcode& code);

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
	bool pcOf = false;
	uint16_t sp, ex, ia;
	uint16_t int_que[256];
	uint8_t int_begin = 0, int_end = 0;
	bool int_enabled = true;
	std::vector<hardware> hw_table;
};

#endif // _H_EMU
