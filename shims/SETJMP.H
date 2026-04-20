#ifndef REDMCSB_SHIM_SETJMP_H
#define REDMCSB_SHIM_SETJMP_H
#include <setjmp.h>
#ifndef _JBLEN
typedef int jmp_buf[1];
#endif
#endif
