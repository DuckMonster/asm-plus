#include "parse.h"
#include "input.h"
#include "log.h"

Node* parse_base = NULL;
Node* parse_cur = NULL;

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

Node* node_add(u32 size, Node* base)
{
	Node* new_node = (Node*)malloc(size);
	memzero(new_node, size);

	new_node->src_path = in_path();

	if (base != NULL)
	{
		new_node->prev = base;
		if (base->next)
			base->prev = new_node;

		new_node->next = base->next;
		base->next = new_node;
	}

	return new_node;
}

Node* node_push(u32 size, Node* base)
{
	Node* new_node = (Node*)malloc(size);
	memzero(new_node, size);

	new_node->src_path = in_path();

	if (base)
	{
		while(base->next)
			base = base->next;

		new_node->prev = base;
		base->next = new_node;
	}

	return new_node;
}

Node* parse_file(const char* file)
{
	in_load(file);
	log_writel(LOG_MEDIUM, "Parsing '%s'", file);
	parse_base = parse_cur = node_add_t(Node, NULL);

	// Iterate through all lines in this file
	do
	{
		// Read all tokens on this line
		Token token;
		while(token_read(&token))
		{
			Token symbol;
			if (token_read_immedate(&symbol))
			{
				// We just read a label
				switch(symbol.type)
				{
					case ':':
						parse_label(token);
						break;

					default:
						error_at(symbol.ptr, symbol.len, "Unexpected token");
						adv_line();
						break;
				}
			}
			else
			{
				parse_instruction(token);
			}
		}
	} while(adv_line());

	log_writel(LOG_MEDIUM, "", file);
	return parse_base;
}

Node_Instruction* parse_instruction(Token token)
{
	log_write(LOG_TRIVIAL, "INST '%.*s'", token.len, token.ptr);

	Node_Instruction* inst = node_push_t(Node_Instruction, parse_base);
	inst->ptr = token.ptr;
	inst->len = token.len;
	inst->type = NODE_INST;

	// Read arguments
	while(token_read(&token))
	{
		Node* arg = node_push_t(Node, inst->args);
		arg->ptr = token.ptr;
		arg->len = token.len;

		switch(token.type)
		{
			case TOKEN_KEYWORD: arg->type = NODE_KEYWORD; break;
			case TOKEN_CONST: arg->type = NODE_CONST; break;
			case TOKEN_CONST_HEX: arg->type = NODE_CONST_HEX; break;
			case TOKEN_CONST_BIN: arg->type = NODE_CONST_BIN; break;
		}

		if (!inst->args)
			inst->args = arg;

		log_write(LOG_TRIVIAL, " '%.*s'", token.len, token.ptr);
		inst->num_args++;

		// Read the separating comma
		if (token_read(&token))
		{
			if (token.type != ',')
			{
				error_at(token.ptr, token.len, "Unexpected token; expected ','");
				return inst;
			}
		}
	}

	log_writel(LOG_TRIVIAL, "");
	return inst;
}

Node_Label* parse_label(Token token)
{
	log_writel(LOG_TRIVIAL, "LABL '%.*s'", token.len, token.ptr);
	Node_Label* label = node_push_t(Node_Label, parse_base);
	label->type = NODE_LABEL;
	label->len = token.len;
	label->ptr = token.ptr;

	return label;
}