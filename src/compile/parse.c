#include "parse.h"
#include <stdio.h>
#include <math.h>
#include "input.h"
#include "log.h"

bool adv_line()
{
	// Advance until _at least_ the newline
	while(!in_eof() && !NEWLINE(in_ptr()))
		in_adv(1);

	// Then continue advancing until we're past it
	while(!in_eof() && NEWLINE(in_ptr()))
		in_adv(1);

	return !in_eof();
}

bool adv_whitespace()
{
	while(!in_eof() && WHITESPACE(in_ptr()))
		in_adv(1);

	return !in_eof();
}

// Reads the next token on the line, skipping over whitespace
bool token_read(Token* out_token)
{
	adv_whitespace();
	return token_read_immedate(out_token);
}

// Reads the token located immedately at the current input ptr, without skipping whitespace
// If no token found (excluding whitespace), returns false
bool token_read_immedate(Token* out_token)
{
	if (in_eof())
		return false;

	if (NEWLINE(in_ptr()))
		return false;

	if (WHITESPACE(in_ptr()))
		return false;

	if (COMMENT(in_ptr()))
		return false;

	// Keywords
	if (ALPHA(in_ptr()))
	{
		out_token->type = TOKEN_KEYWORD;
		out_token->ptr = in_ptr();
		out_token->len = 0;

		while(!in_eof() && ALPHA(in_ptr()))
		{
			in_adv(1);
			out_token->len++;
		}

		return true;
	}

	// Number constants
	if (DIGIT(in_ptr()))
	{
		out_token->ptr = in_ptr();
		out_token->type = TOKEN_CONST;
		out_token->len = 0;

		if (in_ptr()[0] == '0')
		{
			// Hex constants
			if (strnicmp(in_ptr(), "0x", 2) == 0)
			{
				out_token->type = TOKEN_CONST_HEX;
				out_token->len += 2;
				in_adv(2);

				while(!in_eof() && HEX(in_ptr()))
				{
					in_adv(1);
					out_token->len++;
				}

				return true;
			}

			// Binary constants
			else if (strnicmp(in_ptr(), "0b", 2) == 0)
			{
				out_token->type = TOKEN_CONST_BIN;
				out_token->len += 2;
				in_adv(2);

				while(!in_eof() && BIN(in_ptr()))
				{
					in_adv(1);
					out_token->len++;
				}

				return true;
			}
		}

		// Regular :^)
		while(!in_eof() && DIGIT(in_ptr()))
		{
			in_adv(1);
			out_token->len++;
		}

		return true;
	}

	// Just return whatever symbol we found
	out_token->ptr = in_ptr();
	out_token->type = *in_ptr();
	out_token->len = 1;
	in_adv(1);
	return true;
}

Node* node_make(u32 size, Token token)
{
	Node* node = (Node*)malloc(size);
	memzero(node, size);

	node->token = token;
	return node;
}

void node_push(Node* base, Node* node)
{
	if (base)
	{
		while(base->next)
			base = base->next;

		node->prev = base;
		base->next = node;
	}
}

void parse_file(const char* file, Parse* p)
{
	memzero_t(*p);
	in_load(file);

	timer_push();
	log_writel(LOG_MEDIUM, "== Parsing '%s' ==", file);

	// Iterate through all lines in this file
	do
	{
		// Read all tokens on this line
		Token token;
		while(token_read(&token))
		{
			log_codeline(LOG_DEV, token);

			// Raw data
			if (token.type == '#')
			{
				Node_Raw* raw = parse_raw(token);
				if (raw)
					node_push(&p->root, (Node*)raw);

				adv_line();
				continue;
			}

			Token symbol;
			if (token_read_immedate(&symbol))
			{
				// We just read a label
				switch(symbol.type)
				{
					case ':':
						Node* lbl = parse_label(token);
						if (lbl)
							node_push(&p->root, lbl);

						break;

					default:
						error_at(symbol, "Unexpected token");
						adv_line();
						break;
				}
			}
			else
			{
				Node_Instruction* inst = parse_instruction(token);
				if (inst)
					node_push(&p->root, (Node*)inst);

				adv_line();
			}
		}
	} while(adv_line());

	log_writel(LOG_MEDIUM, "== Parse complete (%.2f ms) ==\n", timer_pop_ms());
}

Node* parse_expression(Token token)
{
	Node* result = NULL;

	switch(token.type)
	{
		case TOKEN_KEYWORD:
			result = node_make_t(Node, token);
			result->type = NODE_KEYWORD;
			break;

		case TOKEN_CONST:
		case TOKEN_CONST_BIN:
		case TOKEN_CONST_HEX:
			result = (Node*)parse_const(token);
			break;

		case '[':
			result = (Node*)parse_dereference(token);
			break;
	}

	return result;
}

Node* parse_label(Token token)
{
	log_writel(LOG_DEV, "LABL '%.*s'", token.len, token.ptr);
	Node* label = node_make_t(Node, token);
	label->type = NODE_LABEL;

	return label;
}

/* INSTRUCTION */
Node_Instruction* parse_instruction(Token token)
{
	log_writel(LOG_DEV, "INST '%.*s'", token.len, token.ptr);

	Node_Instruction* inst = node_make_t(Node_Instruction, token);
	inst->type = NODE_INST;

	// Read arguments
	Token arg_token;
	while(token_read(&arg_token))
	{
		Node* arg_node = parse_expression(arg_token);
		if (arg_node == NULL)
		{
			error_at(arg_token, "Unexpected token, expected instruction argument");
			return NULL;
		}

		//array_add(inst->args, arg_node);
		_array_add((Array*)&inst->args, sizeof(arg_node));
		inst->args.data[inst->args.count - 1] = arg_node;

		log_writel(LOG_DEV, " '%.*s'", arg_node->token.len, arg_node->token.ptr);

		Token separator;
		if (token_read(&separator))
		{
			if (separator.type != ',')
			{
				error_at(separator, "Unexpected token, expected ','");
				return NULL;
			}
		}
	}

	return inst;
}

/* CONST */
Node_Const* parse_const(Token token)
{
	Node_Const* cnst = node_make_t(Node_Const, token);
	cnst->type = NODE_CONST;

	switch(token.type)
	{
		case TOKEN_CONST:
			sscanf(token.ptr, "%llu", &cnst->value);
			cnst->size = ((u32)log2((f64)cnst->value) / 8) + 1;
			break;

		case TOKEN_CONST_HEX:
			sscanf(token.ptr + 2, "%llx", &cnst->value);
			cnst->size = ((token.len - 3) / 2) + 1;
			break;

		case TOKEN_CONST_BIN:
			error_at(token, "Sorry! Binary constants not implemented yet :(");
			break;
	}

	return cnst;
}

/* DEREFERENCE */
Node_Dereference* parse_dereference(Token token)
{
	Token expr_token;
	if (!token_read(&expr_token))
	{
		error_at(expr_token, "Expected dereference expression");
		return NULL;
	}

	Node_Dereference* deref = node_make_t(Node_Dereference, token);
	deref->type = NODE_DEREF;
	deref->expr = parse_expression(expr_token);

	// No expression parsed
	if (deref->expr == NULL)
	{
		error_at(expr_token, "Unexpected token, expected memory expression");
		return NULL;
	}

	// Read the closing ] token
	token_read(&expr_token);
	if (expr_token.type != ']')
	{
		error_at(token, "Bracket mismatch, expected ']'");
		return NULL;
	}

	deref->token = token_join(deref->token, expr_token);
	return deref;
}

/* RAW */
Node_Raw* parse_raw(Token token)
{
	Node_Raw* raw = node_make_t(Node_Raw, token);
	raw->type = NODE_RAW;

	Token expr_token;
	while(token_read(&expr_token))
	{
		Node* expr = parse_expression(expr_token);
		if (!expr)
		{
			error_at(expr_token, "Unexpected token, expected raw data value");
			break;
		}

		array_add(raw->values, expr);
	}

	return raw;
}