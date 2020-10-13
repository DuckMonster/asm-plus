#pragma once
#define MAX_REG 0x100
#define MAX_INST 0x100

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

// Constants
typedef struct 
{
	u32 value;
	u8 size;
} Constant;
bool resolve_constant(Node* node, Constant* out_const);

// Labels
typedef struct
{
	const char* name;
	u32 name_len;

	u64 addr;
	u8 addr_size;

	Label* next;
} Label;

void compile_label(Node_Label* lbl);