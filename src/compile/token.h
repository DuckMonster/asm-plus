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
#define COMMENT(ptr) (*(ptr) == ';')

typedef struct
{
	u32 type;
	const char* ptr;
	u32 len;
} Token;

inline bool token_eq(Token a, Token b)
{
	return a.ptr == b.ptr || (a.len == b.len && strncmp(a.ptr, b.ptr, a.len) == 0);
}
inline bool token_cstr_cmp(Token tok, const char* str)
{
	u32 len = (u32)strlen(str);
	if (tok.len != len) return false;

	return memcmp(tok.ptr, str, len) == 0;
}
inline Token token_from_str(const char* str)
{
	Token token;
	token.ptr = str;
	token.len = (u32)strlen(str);

	return token;
}
inline const char* token_to_str(Token tok)
{
	static char buffer[1024];
	memcpy(buffer, tok.ptr, tok.len);
	buffer[tok.len] = 0;

	return buffer;
}