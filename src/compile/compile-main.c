#include <stdio.h>
#include <stdlib.h>
#include "compile.h"
#include "log.h"
#include "output.h"

int main(int argc, const char** argv)
{
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