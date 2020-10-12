#pragma once

/* TOKEN PARSING */
enum
{
	TOKEN_KEYWORD = 256,
	TOKEN_CONST,
	TOKEN_CONST_HEX,
	TOKEN_CONST_BIN,
};

#define WHITESPACE(ptr) ((*(ptr)) == ' ' || (*(ptr)) == '\t')
#define NEWLINE(ptr) ((*(ptr)) == '\n' || (*(ptr)) == '\r')
#define ALPHA(ptr) (((*(ptr)) >= 'a' && (*(ptr)) <= 'z') || ((*(ptr)) >= 'A' && (*(ptr)) <= 'Z') || (*(ptr)) == '_' || (*(ptr)) == '-')
#define DIGIT(ptr) ((*(ptr)) >= '0' && (*(ptr)) <= '9')
#define HEX(ptr) (((*(ptr)) >= '0' && (*(ptr)) <= '9') || ((*(ptr)) >= 'A' && (*(ptr)) <= 'F') || ((*(ptr)) >= 'a' && (*(ptr)) <= 'f'))
#define BIN(ptr) ((*(ptr)) == '0' || (*(ptr)) == '1')

typedef struct
{
	u32 type;
	const char* ptr;
	u32 len;
} Token;

bool token_read(Token* out_token);

/* NODES */
enum
{
	NODE_NULL,
	NODE_INST,
	NODE_KEYWORD,
	NODE_CONST,
	NODE_CONST_HEX,
	NODE_CONST_BIN,
};

#define NODE_IMPL()\
u32 type;\
\
const char* src_path;\
const char* ptr;\
u32 len;\
\
Node* prev;\
Node* next

typedef struct Node_T Node;
typedef struct Node_T
{
	NODE_IMPL();
} Node;

typedef struct
{
	NODE_IMPL();

	u32 num_args;
	Node* args;
} Node_Instruction;

Node* parse_file(const char* file);

Node* node_add(u32 size, Node* base);
#define node_add_t(type, base) ((type*)node_add(sizeof(type), base))
Node* node_push(u32 size, Node* base);
#define node_push_t(type, base) ((type*)node_push(sizeof(type), base))