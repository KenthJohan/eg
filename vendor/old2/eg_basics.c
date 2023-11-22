#include "eg_basics.h"
#include <stdint.h>
#include <stdio.h>
#include <stdint.h>



void eg_assert_soft(char const * file, int line, char const * e)
{
	fprintf(stderr, "%s:%i: %s\n", file, line, e);
}