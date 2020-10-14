#include "compile.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "output.h"
#include "log.h"
#include "input.h"

Node* node_base = NULL;
Node* node_cur = NULL;

Label root_label = { NULL, 0, 0, NULL };
Label_Lookup root_label_lookup = { NULL, 0, 0, 0, NULL };

void compile_node_tree(Node* base, const char* target_path)
{
	out_begin(target_path);

	/* COMPILE NODES */
	log_writel(LOG_MEDIUM, "Compiling '%s'", base->src_path);
	while(base)
	{
		switch(base->type)
		{
			case NODE_INST:
				compile_instruction((Node_Instruction*)base);
				break;

			case NODE_LABEL:
				compile_label((Node_Label*)base);
				break;
		}

		base = base->next;
	}

	/* RESOLVE LABEL LOOKUPS */
	log_writel(LOG_MEDIUM, "Resolving labels");

	out_end();

	if (error_count)
		log_writel(LOG_IMPORTANT, "Build failed (%d errors, %d warnings)", error_count, warning_count);
	else if (warning_count)
		log_writel(LOG_IMPORTANT, "Build successful (%d warnings)", warning_count);
	else
		log_writel(LOG_IMPORTANT, "Build successful");
}

/* REGISTERS */
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

/* INSTRUCTIONS */
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

/* LABELS */
void compile_label(Node_Label* node)
{
	Label exist_label;
	if (resolve_label(node->ptr, node->len, &exist_label))
	{
		error_at(
			exist_label.node->ptr, exist_label.node->len,
			"Label '%.*s' already defined on line %d",
			node->len, node->ptr, in_line_at(exist_label.node->ptr));
		return;
	}

	Label* last_lbl = &root_label;
	while(last_lbl->next)
		last_lbl = last_lbl->next;

	Label* new_lbl = malloc(sizeof(Label));
	memzero(new_lbl, sizeof(Label));
	new_lbl->node = (Node*)node;
	new_lbl->addr = out_offset();
	new_lbl->addr_size = 2;
	last_lbl->next = new_lbl;

	log_writel(LOG_TRIVIAL, "%.*s: 0x%04X", node->len, node->ptr, new_lbl->addr);
}

bool resolve_label(const char* name, u32 name_len, Label* out_label)
{
	Label* ptr = root_label.next;
	while(ptr)
	{
		if (ptr->node->len== name_len && memcmp(ptr->node->ptr, name, name_len))
		{
			*out_label = *ptr;
			return true;
		}

		ptr = ptr->next;
	}

	return false;
}

/* LABEL LOOKUPS */
void add_label_lookup(Node* node, u64 addr, u8 size)
{
	Label_Lookup* lookup = (Label_Lookup*)malloc(sizeof(Label_Lookup));
	memzero(lookup, sizeof(Label_Lookup));

	lookup->name = node->ptr;
	lookup->name_len = node->len;
	lookup->addr = addr;
	lookup->addr_size = size;

	// Find last
	Label_Lookup* ptr = &root_label_lookup;
	while(ptr->next)
	{
		ptr = ptr->next;
	}

	ptr->next = lookup;
	log_writel(LOG_TRIVIAL, "0x%04X (%d) <- '%.*s'", addr, size, node->len, node->ptr);
}

/* CONSTANTS */
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
