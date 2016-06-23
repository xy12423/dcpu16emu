#ifndef _H_ASM
#define _H_ASM

#include "cpu.h"

class dcpu16_asm
{
public:
	enum asm_err
	{
		ERR_ASM_NOERR,
		ERR_ASM_OTHER,

		ERR_ASM_UNRECOGNIZED,
	};

	typedef std::ios_base::iostate iostate;
	typedef std::deque<uint16_t> buf_type;

	static constexpr iostate goodbit = std::ios_base::goodbit;
	static constexpr iostate badbit = std::ios_base::badbit;
	static constexpr iostate failbit = std::ios_base::failbit;
	static constexpr iostate eofbit = std::ios_base::eofbit;
	
	inline void ignore(size_t size) { buffer.erase(buffer.begin(), buffer.begin() + std::min(size, buffer.size())); }

	void read(uint16_t* out, size_t size);
	void read(std::string& out);
	void read(std::ostream& out);

	void write(const uint16_t* in, size_t size);
	void write(const std::string& in);
	void write(std::istream& in);

	const buf_type& rdbuf() const { return buffer; }
	buf_type& rdbuf() { return buffer; }
	void clear_buf() { buffer.clear(); }

	void clear() { state = 0; }
	bool good() const { return state == 0; }
	bool bad() const { return (state & badbit) != 0; }
	bool fail() const { return (state & failbit) != 0; }
	bool eof() const { return (state & eofbit) != 0; }

	size_t gcount() { return last_io_size; };
private:
	uint16_t read();
	void write(uint16_t in) { buffer.push_back(in); last_io_size += 1; };

	void check_eof() { if (buffer.empty()) state |= eofbit; }

	void dasm_arg(uint8_t arg, bool is_a, std::string& ret);

	buf_type buffer;
	size_t last_io_size;
	iostate state;
};

class dcpu16_asm_error :public std::runtime_error
{
public:
	dcpu16_asm_error(dcpu16_asm::iostate _state) :std::runtime_error("Internal assembler error"), state(_state) {}
	dcpu16_asm::iostate rdstate() { return state; }
private:
	dcpu16_asm::iostate state;
};

#endif	//_H_ASM
