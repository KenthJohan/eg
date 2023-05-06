#include "eg_assert.h"
#include <stdio.h>

void eg_assert_soft(char const * file, int line, char const * e)
{
	fprintf(stderr, "%s:%i: %s\n", file, line, e);
}

