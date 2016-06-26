%{
#include "stdafx.h"
#include "asm.h"
#include "asm_yacc.h"
#define YYSTYPE asm_yacc_result
int yylex(YYSTYPE* lvalp, asm_yacc_proc* proc);
void yyerror(asm_yacc_proc* proc, const char* err);
%}

%define api.pure full
%param {asm_yacc_proc* proc}
%token ERR_TOKEN BRACKET_LEFT BRACKET_RIGHT NUMBER ADD SUB MUL DIV MOD SHL SHR AND XOR BOR SUB_LEFT SUB_RIGHT REG SPEC_REG PUSH POP PEEK PICK
%start term

%%

expr_1 : NUMBER { $$.data = $1.data; }
 | BRACKET_LEFT expr_7 BRACKET_RIGHT { $$.data = $2.data; }
;
expr_2 : expr_1 { $$.data = $1.data; }
 | ADD expr_1 { $$.data = $2.data; }
 | SUB expr_1 { $$.data = -$2.data; }
expr_3 : expr_2 { $$.data = $1.data; }
 | expr_3 MUL expr_2 { $$.data = $1.data * $3.data; }
 | expr_3 DIV expr_2 { $$.data = $1.data / $3.data; }
 | expr_3 MOD expr_2 { $$.data = $1.data % $3.data; }
;
expr_4 : expr_3 { $$.data = $1.data; }
 | expr_4 ADD expr_3 { $$.data = $1.data + $3.data; }
 | expr_4 SUB expr_3 { $$.data = $1.data - $3.data; }
;
expr_5 : expr_4 { $$.data = $1.data; }
 | expr_5 SHL expr_4 { $$.data = $1.data << $3.data; }
 | expr_5 SHR expr_4 { $$.data = $1.data >> $3.data; }
;
expr_6 : expr_5 { $$.data = $1.data; }
 | expr_6 AND expr_5 { $$.data = $1.data & $3.data; }
;
expr_7 : expr_6 { $$.data = $1.data; }
 | expr_7 XOR expr_6 { $$.data = $1.data ^ $3.data; }
;
expr_8 : expr_7 { $$.data = $1.data; }
 | expr_8 BOR expr_7 { $$.data = $1.data | $3.data; }
;
const : expr_8 { $$.data = $1.data; }
;
reg : REG { $$.res = $1.res; }
;
spec_reg : SPEC_REG { $$.res = $1.res; }
;
addr : SUB_LEFT const SUB_RIGHT { $$.res = 0x1E; $$.data = $2.data; }
;
addr_reg : SUB_LEFT reg SUB_RIGHT { $$.res = $2.res + 0x08; }
;
addr_reg_shift : SUB_LEFT reg const SUB_RIGHT { $$.res = $2.res + 0x10; $$.data = $3.data; }
 | reg SUB_LEFT const SUB_RIGHT { $$.res = $1.res + 0x10; $$.data = $3.data; }
;
stack_op : PUSH { if (proc->is_a) YYABORT; $$.res = 0x18; }
 | POP { if (!proc->is_a) YYABORT; $$.res = 0x18; }
 | PEEK { $$.res = 0x19; }
 | PICK const { $$.res = 0x1A; $$.data = $2.data; }
;
arg : const { $$.res = 0x1F; $$.data = $1.data; }
 | reg { $$.res = $1.res; }
 | spec_reg { $$.res = $1.res; }
 | addr { $$ = $1; }
 | addr_reg { $$.res = $1.res; }
 | addr_reg_shift { $$ = $1; }
 | stack_op { $$ = $1; }
term : arg { proc->result = $1; }

%%

uint8_t read_reg(char reg)
{
	reg = toupper(reg);
	switch (reg)
	{
		case 'A':
			return 0;
		case 'B':
			return 1;
		case 'C':
			return 2;
		case 'X':
			return 3;
		case 'Y':
			return 4;
		case 'Z':
			return 5;
		case 'I':
			return 6;
		case 'J':
			return 7;
	}
	throw(dcpu16_asm_error(dcpu16_asm::failbit));
}

uint16_t read_num(asm_yacc_proc& proc)
{
	const char *itr = proc.itr, *itr_end = proc.itr_end;
	char *num_end;
	uint16_t result = static_cast<uint16_t>(std::strtol(itr, &num_end, 0));
	if (num_end == itr || num_end > itr_end)
		throw(dcpu16_asm_error(dcpu16_asm::failbit));
	proc.itr = num_end;
	return result;
}

int yylex(YYSTYPE* lvalp, asm_yacc_proc* proc)
{
	const char *itr = proc->itr, *itr_end = proc->itr_end;
	while (itr != itr_end)
	{
		if (!isspace(*itr))
			break;
		itr++;
	}
	if (itr == itr_end)
		return 0;

	int result;
	switch (*itr)
	{
		case '(':
			result = BRACKET_LEFT;
			break;
		case ')':
			result = BRACKET_RIGHT;
			break;
		case '[':
			result = SUB_LEFT;
			break;
		case ']':
			result = SUB_RIGHT;
			break;
		case '+':
			result = ADD;
			break;
		case '-':
			result = SUB;
			break;
		case '*':
			result = MUL;
			break;
		case '/':
			result = DIV;
			break;
		case '%':
			result = MOD;
			break;
		case '<':
			itr++;
			if (*itr == '<')
				result = SHL;
			else
				return ERR_TOKEN;
			break;
		case '>':
			itr++;
			if (*itr == '>')
				result = SHL;
			else
				return ERR_TOKEN;
			break;
		case '&':
			result = AND;
			break;
		case '^':
			result = XOR;
			break;
		case '|':
			result = BOR;
			break;
		case 'A':
		case 'B':
		case 'C':
		case 'X':
		case 'Y':
		case 'Z':
		case 'I':
		case 'J':
		case 'a':
		case 'b':
		case 'c':
		case 'x':
		case 'y':
		case 'z':
		case 'i':
		case 'j':
			lvalp->res = read_reg(*itr);
			result = REG;
			break;
		case 'P':
		case 'p':
			itr++;
			if (*itr == 'C' || *itr == 'c')
			{
				lvalp->res = 0x1C;
				result = SPEC_REG;
			}
			else if (*itr == 'U' || *itr == 'u')
			{
				itr++;
				if (*itr != 'S' && *itr != 's')
					return ERR_TOKEN;
				itr++;
				if (*itr != 'H' && *itr != 'h')
					return ERR_TOKEN;
				result = PUSH;
			}
			else if (*itr == 'O' || *itr == 'o')
			{
				itr++;
				if (*itr != 'P' && *itr != 'p')
					return ERR_TOKEN;
				result = POP;
			}
			else if (*itr == 'E' || *itr == 'e')
			{
				itr++;
				if (*itr != 'E' && *itr != 'e')
					return ERR_TOKEN;
				itr++;
				if (*itr != 'K' && *itr != 'k')
					return ERR_TOKEN;
				result = PEEK;
			}
			else if (*itr == 'I' || *itr == 'i')
			{
				itr++;
				if (*itr != 'C' && *itr != 'c')
					return ERR_TOKEN;
				itr++;
				if (*itr != 'K' && *itr != 'k')
					return ERR_TOKEN;
				result = PICK;
			}
			else
				return ERR_TOKEN;
			break;
		case 'S':
		case 's':
			itr++;
			if (*itr == 'P' || *itr == 'p')
				result = SPEC_REG;
			else
				return ERR_TOKEN;
			lvalp->res = 0x1B;
			break;
		case 'E':
		case 'e':
			itr++;
			if (*itr == 'X' || *itr == 'x')
				result = SPEC_REG;
			else
				return ERR_TOKEN;
			lvalp->res = 0x1D;
			break;
		default:
			if (isdigit(*itr))
			{
				lvalp->data = read_num(*proc);
				return NUMBER;
			}
			return ERR_TOKEN;
	}
	itr++;
	proc->itr = itr;
	return result;
}

void yyerror(asm_yacc_proc* proc, const char* err)
{
	throw(dcpu16_asm_error(dcpu16_asm::failbit));
}
