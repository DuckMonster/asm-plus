#pragma once
#include "parse.h"

void compile_node_tree(Node* base, const char* target_path);
void compile_instruction(Node_Instruction* inst);

// Registers
typedef struct
{
	Node* node;

	const char* name;
	u8 code;
	u8 size;
} Register;

typedef struct 
{
	Node* node;

	u32 value;
	u8 size;
} Constant;

bool resolve_register(Node* node, Register* out_reg);
bool resolve_constant(Node* node, Constant* out_const);