#ifndef CORE_H
#define CORE_H

#include <errno.h>

#include "xassert.h"
#include "log.h"

#define PTR_FREE(p) do { free(p); (p)=NULL; } while(0)
#define PTR_ASSIGN(dst,src) do { (dst)=(src); (src)=NULL; } while(0)

#endif

