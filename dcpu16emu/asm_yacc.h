#ifndef _H_ASM_YACC
#define _H_ASM_YACC

struct asm_yacc_result
{
	uint8_t res;
	uint16_t data;
};

struct asm_yacc_proc
{
	asm_yacc_proc(const char* _itr, const char* _itr_end, bool _is_a)
		:itr(_itr), itr_end(_itr_end), is_a(_is_a)
	{}

	const char *itr, *itr_end;
	bool is_a;
	asm_yacc_result result;
};

int yyparse(asm_yacc_proc* proc);

#endif
