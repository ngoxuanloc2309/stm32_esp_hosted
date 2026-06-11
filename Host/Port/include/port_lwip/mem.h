#ifndef __LWIP_MEM_H__
#define __LWIP_MEM_H__

#include_next <lwip/mem.h>

#ifdef mem_free
#undef mem_free
#endif
#define mem_free(x) do { if(x) { free(x); (x) = NULL; } } while(0)

#endif