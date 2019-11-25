#include <stdint.h>

void xassert_fail(const char *expr, const char *file, int line, uintmax_t value);

#define XASSERT(expr,value) ((expr) ? (void)value : xassert_fail(#expr, __FILE__, __LINE__, value))

