#include "compile.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "output.h"
#include "log.h"

Node* node_base = NULL;
Node* node_cur = NULL;

void compile_node_tree(Node* base, const char* target_path)
{
	out_begin(target_path);

	log_write(LOG_MEDIUM, "Compiling '%s'", base->src_path);

	while(base)
	{
		switch(base->type)
		{
			case NODE_INST:
				compile_instruction((Node_Instruction*)base);
				break;
		}

		base = base->next;
	}

	out_end();

	if (error_count)
		log_write(LOG_IMPORTANT, "Build failed (%d errors, %d warnings)", error_count, warning_count);
	else if (warning_count)
		log_write(LOG_IMPORTANT, "Build successful (%d warnings)", warning_count);
	else
		log_write(LOG_IMPORTANT, "Build successful");
}

// MOV
void compile_mov_reg_c(Register dst, Constant src)
{
	if (src.size > 1)
		warning_at(src.node->ptr, src.node->len, "Move constant bigger than 1 byte; will be truncated");

	log_write(LOG_TRIVIAL, "MOV %s 0x%X", dst.name, src.value);
	out_write_bits(0b01, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(0b110, 3);

	out_write_u8(src.value);
}

void compile_mov_reg_reg(Register dst, Register src)
{
	log_write(LOG_TRIVIAL, "MOV %s %s", dst.name, src.name);
	out_write_bits(0b01, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(src.code, 3);
}

void compile_mov(Node_Instruction* inst)
{
	if (inst->num_args != 2) 
	{
		error_at(inst->ptr, inst->len, "Invalid argument count; expected 2, found %d", inst->num_args);
		return;
	}

	Node* dst = inst->args;
	Node* src = dst->next;

	// r < ?
	if (dst->type == NODE_KEYWORD)
	{
		Register dst_r;
		resolve_register(dst, &dst_r);

		// r < r'
		if (src->type == NODE_KEYWORD)
		{
			Register src_r;
			resolve_register(src, &src_r);

			compile_mov_reg_reg(dst_r, src_r);
			return;
		}

		// r < n
		if (src->type == NODE_CONST || src->type == NODE_CONST_HEX || src->type == NODE_CONST_BIN)
		{
			Constant src_c;
			resolve_constant(src, &src_c);

			compile_mov_reg_c(dst_r, src_c);
			return;
		}
	}

	error_at(inst->ptr, inst->len, "Invalid mov arguments");
}

// JMP
void compile_jmp_c(Constant c)
{
	//out_write_bits()
}

void compile_jmp(Node_Instruction* inst)
{
	if (inst->num_args != 1)
	{
		error_at(inst->ptr, inst->len, "Invalid argument count; expected jump address", inst->num_args);
		return;
	}

	Constant dest;
	resolve_constant(inst->args, &dest);

	compile_jmp_c(dest);
}

void compile_instruction(Node_Instruction* inst)
{
	if (inst->len == 3)
	{
		const char* str = inst->ptr;
		if (strnicmp(str, "mov", 3) == 0) { compile_mov(inst); return; }
	}

	error_at(inst->ptr, inst->len, "Unknown instruction '%.*s'", inst->len, inst->ptr);
}

bool resolve_register(Node* node, Register* out_reg)
{
	out_reg->node = node;

	if (node->len == 1)
	{
		out_reg->size = 1;
		switch(node->ptr[0])
		{
			case 'A':
			case 'a':
				out_reg->name = "A";
				out_reg->code = 0b111;
				return true;

			case 'B':
			case 'b':
				out_reg->name = "B";
				out_reg->code = 0b000;
				return true;
		}
	}

	return false;
}

bool resolve_constant(Node* node, Constant* out_const)
{
	out_const->node = node;
	out_const->value = 0;

	switch(node->type)
	{
		case NODE_CONST:
			sscanf(node->ptr, "%d", &out_const->value);
			out_const->size = ((u32)log2(out_const->value) / 8) + 1;
			return true;

		case NODE_CONST_HEX:
			sscanf(node->ptr + 2, "%x", &out_const->value);
			out_const->size = ((node->len - 3) / 2) + 1;
			return true;
	}

	return false;
}