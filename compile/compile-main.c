#include <stdio.h>
#include <stdlib.h>
#include "compile.h"
#include "log.h"

int main(int argc, const char** argv)
{
	Node* node = parse_file(argv[1]);
	compile_node_tree(node, argv[2]);
}