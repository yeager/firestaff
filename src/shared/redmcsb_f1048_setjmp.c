#include "redmcsb_f1048_setjmp.h"

const char *redmcsb_f1048_setjmp_source_evidence(void)
{
    return "ReDMCSB Toolchains/Common/Source/DEFS.H:3208-3215 contains "
           "the disabled MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J and "
           "MEDIA764_AU1E_AU2E_AU3E aliases from setjmp to F1048_setjmp. "
           "DEFS.H:3399-3408 supplies jmp_buf and the native setjmp "
           "declaration for MEDIA551_F20E_F20J_F31E_F31J.";
}
