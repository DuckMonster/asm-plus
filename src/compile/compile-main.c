#include <stdio.h>
#include <stdlib.h>
#include "compile.h"
#include "log.h"
#include "output.h"
#include "coff.h"

int main(int argc, const char** argv)
{
	log_writel(LOG_IMPORTANT, "  %s", argv[1]);
	timer_push();

	Parse parse;
	parse_file(argv[1], &parse);

	Compile compile;
	compile_parsed(&parse, &compile);

	if (!error_count)
	{
		coff_write(argv[2], &compile);
		log_writel(LOG_IMPORTANT, "  %s => %s (%.2f ms)", argv[1], argv[2], timer_pop_ms());
	}

	system("pause");
	return;
}