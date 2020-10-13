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

	log_writel(LOG_MEDIUM, "Compiling '%s'", base->src_path);

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
		log_writel(LOG_IMPORTANT, "Build failed (%d errors, %d warnings)", error_count, warning_count);
	else if (warning_count)
		log_writel(LOG_IMPORTANT, "Build successful (%d warnings)", warning_count);
	else
		log_writel(LOG_IMPORTANT, "Build successful");
}

void compile_instruction(Node_Instruction* inst)
{
	// Find instruction in 'inst_list'
	for(u32 i=0; i<MAX_INST; ++i)
	{
		if (inst_list[i].name == NULL)
			break;

		if (strnicmp(inst->ptr, inst_list[i].name, inst->len) == 0)
		{
			inst_list[i].func(inst);
			return;
		}
	}

	error_at(inst->ptr, inst->len, "Unknown instruction '%.*s'", inst->len, inst->ptr);
}

void compile_label(Node_Label* lbl)
{
	lbl->value = out_offset();
}

bool resolve_register(Node* node, Register* out_reg)
{
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

bool resolve_instruction(Node_Instruction* inst, Instruction* out_inst)
{
	// Find instruction in 'inst_list'
	for(u32 i=0; i<MAX_INST; ++i)
	{
		if (inst_list[i].name == NULL)
			break;

		if (strnicmp(inst->ptr, inst_list[i].name, inst->len) == 0)
		{
			*out_inst = inst_list[i];
			return true;
		}
	}

	return false;
}

bool resolve_constant(Node* node, Constant* out_const)
{
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