#include <stdio.h>
#include <stdlib.h>
#include "compile.h"
#include "log.h"
#include "output.h"
#include "coff.h"

ARRAY_DEF(Int_Array, int);
ARRAY_DEF(Str_Array, const char*);

int main(int argc, const char** argv)
{
	{
		Int_Array arr;
		memzero(&arr, sizeof(arr));

		array_add(arr, 5);
		array_add(arr, 2);
		array_add(arr, 1);
		array_add(arr, 9);

/*
		array_foreach(arr)
		{
			printf("%d\n", *it);
		}
		*/
	}

	{
		Str_Array arr;
		memzero(&arr, sizeof(arr));

		array_add(arr, "Hello!");
		array_add(arr, "World!");
		array_add(arr, "FUCK YOU");

/*
		array_foreach(arr)
		{
			printf("'%s'\n", *it);
		}
*/
	}
	return;

	coff_write(argv[2]);
	return;

	timer_push();

	Node* node = parse_file(argv[1]);
	compile_node_tree(node, argv[2]);

	if (error_count)
		log_writel(LOG_IMPORTANT, "Build failed (%d errors, %d warnings) (%.2f ms)", error_count, warning_count, timer_pop_ms());
	else if (warning_count)
		log_writel(LOG_IMPORTANT, "Build successful (%d warnings) (%.2f ms)", warning_count, timer_pop_ms());
	else
	{
		log_writel(LOG_IMPORTANT, "Build successful (%.2f ms)", timer_pop_ms());
		out_flush(argv[2]);
	}
}