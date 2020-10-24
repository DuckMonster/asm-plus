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
	timer_push();
	out_begin();

	log_writel(LOG_IMPORTANT, in_filename());

	/* PRE-PROCESS LABELS */
	log_writel(LOG_MEDIUM, "Preprocessing labels");

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
	log_writel(LOG_MEDIUM, "Compiling");

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

			case NODE_RAW:
				compile_raw((Node_Raw*)ptr);
				break;
		}

		ptr = ptr->next;
	}

	/* COMPILE SYMBOL REFERENCES */
	log_writel(LOG_MEDIUM, "Compiling symbol references");
	for(u32 i=0; i<num_symbol_refs; ++i)
		compile_symbol_ref(symbol_refs[i]);

	log_writel(LOG_MEDIUM, "Compile done (%.2f ms)", timer_pop_ms());
}

void compile_raw(Node_Raw* raw)
{
	Node* val = raw->values;
	while(val)
	{
		switch(val->type)
		{
			case NODE_CONST:
				Constant cnst;
				if (!resolve_constant(val, &cnst))
				{
					error_at(val->ptr, val->len, "Unresolved constant value");
					break;
				}

				out_write(&cnst.value, cnst.size);
				break;

			default:
				error_at(val->ptr, val->len, "Invalid raw data");
				break;
		}

		val = val->next;
	}
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
void compile_instruction(Node_Instruction* node)
{
	Instruction inst;
	if (!resolve_instruction(node, &inst))
	{
		error_at(node->ptr, node->len, "Unknown instruction '%.*s'", node->len, node->ptr);
		return;
	}

	inst.func(node);
}

bool resolve_instruction(Node_Instruction* inst, Instruction* out_inst)
{
	// Find instruction in 'inst_list'
	for(u32 i=0; i<MAX_INST; ++i)
	{
		if (inst_list[i].name == NULL)
			break;

		// Name matches
		if (strnicmp(inst->ptr, inst_list[i].name, inst->len) == 0)
		{
			// Compare arguments
			Node* arg = inst->args;
			u32 index = 0;
			while(arg || inst_list[i].args[index])
			{
				// Too few arguments
				if (!arg)
					goto arg_match_fail;

				// Too many arguments
				if (!inst_list[i].args[index])
					goto arg_match_fail;

				u32 arg_type;
				if (!resolve_argument(arg, &arg_type))
				{
					error_at(arg->ptr, arg->len, "Unable to parse argument type");
					return false;
				}

				// Argument type mismatch
				if (inst_list[i].args[index] != arg_type)
					goto arg_match_fail;

				arg = arg->next;
				index++;
			}

			*out_inst = inst_list[i];
			return true;
		}

arg_match_fail:;
	}

	return false;
}

bool resolve_argument(Node* node, u32* out_arg)
{
	Symbol sym;
	Register reg;
	Constant cnst;

	switch(node->type)
	{
		case NODE_KEYWORD:
			if (resolve_register(node, &reg))
			{
				*out_arg = ARG_REG;
				return true;
			}
			break;

		case NODE_CONST:
			if (resolve_constant(node, &cnst))
			{
				*out_arg = ARG_CONST;
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

	if (num_symbols == MAX_SYMBOL)
		error("Too many symbols");

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

void write_symbol_ref(Symbol_Reference ref)
{
	if (num_symbol_refs == MAX_SYMBOL_REF)
		error("Too many symbol references");

	ref.replace_addr = out_offset();
	symbol_refs[num_symbol_refs++] = ref;

	out_write_u16(0);
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
	Node_Const* const_node = (Node_Const*)node;
	out_const->value = const_node->value;
	out_const->size = const_node->size;
	return true;
}
