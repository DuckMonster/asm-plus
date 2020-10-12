#include "compile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "input.h"
#include "parse.h"

Node* node_base = NULL;
Node* node_cur = NULL;

void compile_file(const char* path)
{
	log(LOG_MEDIUM, "Compiling '%s'", path);
	Node* node = parse_file(path);

	while(node)
	{
		printf("%.*s\n", node->len, node->ptr);
		node = node->next;
	}
}