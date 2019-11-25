#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "xassert.h"

void xassert_fail(const char *expr, const char *file, int line, uintmax_t value)
{
	fprintf(stderr, "XASSET fail: %s:%d \"%s\" value=%#" PRIxMAX "\n", 
			file, line, expr, value);
	abort();
}

