#pragma once
#include "parse.h"

typedef struct Compile_T Compile;

/* SYMBOL */
enum
{
	SYM_LOCAL,
	SYM_EXPORT,
	SYM_IMPORT,
};

typedef struct
{
	u32 index;
	Token token;
	u32 section;
	u32 ptr;

	u8 type;
} Symbol;
ARRAY_DEF(Symbol_Array, Symbol);

Symbol* compile_find_symbol(Compile* c, Token token);
Symbol* compile_add_symbol(Compile* c, Token token);
void compile_add_import(Compile* c, Token token);
void compile_add_export(Compile* c, Token token);
void compile_mark_symbol(Compile* c, Token token);

/* RELOCATION */
enum
{
	RELOC_FUNC,
	RELOC_MEM,
};

typedef struct
{
	u32 sym_index;
	u32 ptr;
	u8 type;
} Relocation;
ARRAY_DEF(Relocation_Array, Relocation);

void compile_write_mem_relocation(Compile* c, Token token, u64 base);
void compile_write_func_relocation(Compile* c, Token token, u32 base);

/* SECTION */
enum
{
	SEC_CODE,
	SEC_READONLY,
};

typedef struct
{
	u32 index;
	Token token;
	u8* data;
	u32 data_size;

	Relocation_Array relocations;
} Section;
ARRAY_DEF(Section_Array, Section);

Section* compile_begin_section(Compile* c, Token token);
void compile_end_section(Compile* c);

/* INSTRUCTION */
#define MAX_INSTRUCTIONS 0x20
#define MAX_ARGS 5
enum
{
	ARG_NULL,
	ARG_REGISTER,
	ARG_MEMORY,
	ARG_CONST,
};
typedef void (*Instr_Func)(Node_Instruction* inst);

typedef struct
{
	const char* name;
	u8 num_args;
	u8 args[MAX_ARGS];
	Instr_Func func;
} Instruction;
extern Instruction instruction_list[MAX_INSTRUCTIONS];

void compile_instruction(Compile* c, Node_Instruction* inst_node);
bool resolve_instruction(Node_Instruction* inst_node, Instruction** out_inst);
u8 get_arg_type(Node* arg_node);

/* CONSTANT */
typedef struct
{
	u32 value;
	u32 size;
} Constant;
bool resolve_constant(Node* node, Constant* cnst);

/* REGISTER */
#define MAX_REGISTERS 0x10

typedef struct
{
	const char* name;
	u32 code;
} Register;
extern Register register_list[MAX_REGISTERS];

bool resolve_register(Node* node, Register** reg);

/* COMPILE */
typedef struct Compile_T
{
	Section_Array sections;
	Symbol_Array symbols;

	Section* cur_section;
} Compile;

void compile_parsed(Parse* parse, Compile* c);
void compile_raw(Compile* c, Node_Raw* raw);