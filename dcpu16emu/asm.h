#ifndef _H_ASM
#define _H_ASM

#include "cpu.h"

class dcpu16_asm_error :public std::runtime_error
{
public:
	dcpu16_asm_error() :std::runtime_error("Internal assembler error") {}
};

class dcpu16_asm
{
public:
	enum asm_err
	{
		ERR_ASM_NOERR,
		ERR_ASM_OTHER,

		ERR_ASM_UNRECOGNIZED,
	};

	inline void read(uint16_t* out, size_t size) {
		uint16_t *p = out, *end = out + std::min(size, buffer.size());
		for (; p < end; p++) {
			*p = buffer.front();
			buffer.pop_front();
		};
		last_io_size = p - out;
	};
	void read(std::string& out);
	void read(std::ostream& out);

	inline void write(const uint16_t* in, size_t size) {
		for (const uint16_t *p = in, *end = in + size; p < end; p++)
			buffer.push_back(*p);
		last_io_size = size;
	};
	void write(const std::string& in);
	void write(std::istream& in);

	void clear() { last_io_size = buffer.size(); buffer.clear(); }
    size_t size() { return buffer.size(); }
	size_t gcount() { return last_io_size; };
private:
	inline void read(uint16_t& out) { if (buffer.empty()) throw(dcpu16_asm_error()); out = buffer.front(); buffer.pop_front(); last_io_size += 1; };
	inline void write(uint16_t in) { buffer.push_back(in); last_io_size += 1; };

	void dasm_arg(uint8_t arg, bool is_a, std::string& ret);

	std::deque<uint16_t> buffer;
	size_t last_io_size;
};

#endif	//_H_ASM
