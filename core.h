#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#include "xassert.h"
#include "log.h"

#define PTR_FREE(p) do { free(p); (p)=NULL; } while(0)
#define PTR_ASSIGN(dst,src) do { (dst)=(src); (src)=NULL; } while(0)

// from iw util.c
void mac_addr_n2a(char *mac_addr, const unsigned char *arg);

#endif

