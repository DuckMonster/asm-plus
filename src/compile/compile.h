#pragma once
#define MAX_REG 0x100
#define MAX_INST 0x100

#define MAX_SYMBOL 1024
#define MAX_SYMBOL_REF 1024

#include "parse.h"

void compile_node_tree(Node* base, const char* target_path);

// Registers
typedef struct
{
	const char* name;
	u8 code;
	u8 size;
} Register;
extern Register reg_list[MAX_REG];
bool resolve_register(Node* node, Register* out_reg);

// Instructions
typedef void (*Inst_Func)(Node_Instruction* inst);

typedef struct 
{
	const char* name;
	Inst_Func func;
} Instruction;
extern Instruction inst_list[MAX_INST];
void compile_instruction(Node_Instruction* inst);
bool resolve_instruction(Node_Instruction* node, Instruction* out_inst);

// Symbols
typedef struct
{
	Node* node;

	bool compiled;
	u64 addr;
} Symbol;

void preprocess_label(Node* lbl);
void compile_label(Node* lbl);
bool resolve_symbol(const char* name, u32 name_len, Symbol** out_symbol);

// Symbol reference
typedef struct
{
	Symbol* symbol;
	u32 offset;

	u64 replace_addr;
} Symbol_Reference;

bool resolve_symbol_ref(Node* node, Symbol_Reference* out_ref);
void defer_symbol_ref(Symbol_Reference ref);
void compile_symbol_ref(Symbol_Reference ref);

// Constants
typedef struct 
{
	u64 value;
	u8 size;
} Constant;

bool resolve_constant(Node* node, Constant* out_const);