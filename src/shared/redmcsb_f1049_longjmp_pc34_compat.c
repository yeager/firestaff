#include "redmcsb_f1049_longjmp_pc34_compat.h"

bool redmcsb_f1049_longjmp_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1049_longjmp_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/DEFS.H:3208-3215 retains "
           "F1049_longjmp only in commented-out aliases for "
           "MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J and "
           "MEDIA764_AU1E_AU2E_AU3E. DEFS.H:3399-3408 defines jmp_buf "
           "for non-PC media sets and declares setjmp/longjmp only for "
           "MEDIA551_F20E_F20J_F31E_F31J. No PC 3.4 branch or portable "
           "F1049 adapter is supplied.";
}
