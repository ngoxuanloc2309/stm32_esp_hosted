#ifndef __PLATFORM_WRAPPER_FIX_H
#define __PLATFORM_WRAPPER_FIX_H

#include "lwip/mem.h"

#ifdef mem_free
#undef mem_free
#endif

// #define mem_free(x) do { if(x) { hosted_free(x); (x) = NULL; } } while(0)

#endif