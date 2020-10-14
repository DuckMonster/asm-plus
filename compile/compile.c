#include "compile.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "output.h"
#include "log.h"
#include "input.h"

Node* node_base = NULL;
Node* node_cur = NULL;

Symbol symbols[MAX_SYMBOL];
u32 num_symbols = 0;

Symbol_Reference symbol_refs[MAX_SYMBOL_REF];
u32 num_symbol_refs = 0;

void compile_node_tree(Node* base, const char* target_path)
{
	out_begin();

	/* PRE-PROCESS LABELS */
	log_writel(LOG_MEDIUM, "Preprocessing labels", base->src_path);

	Node* ptr = base;
	while(ptr)
	{
		switch(ptr->type)
		{
			case NODE_LABEL:
				preprocess_label(ptr);
				break;
		}

		ptr = ptr->next;
	}

	/* COMPILE NODES */
	log_writel(LOG_MEDIUM, "Compiling", base->src_path);

	ptr = base;
	while(ptr)
	{
		switch(ptr->type)
		{
			case NODE_INST:
				compile_instruction((Node_Instruction*)ptr);
				break;

			case NODE_LABEL:
				compile_label(ptr);
				break;
		}

		ptr = ptr->next;
	}

	/* COMPILE SYMBOL REFERENCES */
	log_writel(LOG_MEDIUM, "Compiling symbol references");
	for(u32 i=0; i<num_symbol_refs; ++i)
		compile_symbol_ref(symbol_refs[i]);

	if (error_count)
		log_writel(LOG_IMPORTANT, "Build failed (%d errors, %d warnings)", error_count, warning_count);
	else if (warning_count)
		log_writel(LOG_IMPORTANT, "Build successful (%d warnings)", warning_count);
	else
	{
		log_writel(LOG_IMPORTANT, "Build successful");
		out_flush(target_path);
	}

	out_end();
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

/* SYMBOLS */
void preprocess_label(Node* lbl)
{
	Symbol* sym;
	if (resolve_symbol(lbl->ptr, lbl->len, &sym))
	{
		error_at(
			lbl->ptr, lbl->len,
			"Symbol '%.*s' already defined on line %d",
			lbl->len, lbl->ptr, in_line_at(sym->node->ptr));
		return;
	}

	sym = &symbols[num_symbols++];
	memzero(sym, sizeof(Symbol));
	sym->node = (Node*)lbl;
}

void compile_label(Node* lbl)
{
	Symbol* sym;
	if (!resolve_symbol(lbl->ptr, lbl->len, &sym))
	{
		error_at(lbl->ptr, lbl->len, "Label '%.*s' did not compile to a defined symbol", lbl->len, lbl->ptr);
		return;
	}

	sym->compiled = true;
	sym->addr = out_offset();
	log_writel(LOG_TRIVIAL, "%.*s: 0x%04X", lbl->len, lbl->ptr, sym->addr);
}

bool resolve_symbol(const char* name, u32 name_len, Symbol** out_symbol)
{
	for(u32 i=0; i<num_symbols; ++i)
	{
		Symbol* sym = &symbols[i];
		if (sym->node->len == name_len && memcmp(sym->node->ptr, name, name_len) == 0)
		{
			*out_symbol = sym;
			return true;
		}
	}

	return false;
}

/* SYMBOL REFERENCES */
bool resolve_symbol_ref(Node* node, Symbol_Reference* out_ref)
{
	Symbol* sym;
	if (!resolve_symbol(node->ptr, node->len, &sym))
		return false;

	out_ref->symbol = sym;
	out_ref->offset = 0;
	out_ref->replace_addr = 0;
	return true;
}

void defer_symbol_ref(Symbol_Reference ref)
{
	symbol_refs[num_symbol_refs++] = ref;
}

void compile_symbol_ref(Symbol_Reference ref)
{
	out_seek(ref.replace_addr);
	out_write_u16((u16)ref.symbol->addr);

	log_writel(LOG_TRIVIAL, "0x%04X <- 0x%04X", ref.replace_addr, ref.symbol->addr);
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
