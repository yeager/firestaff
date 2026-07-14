#include "redmcsb_f1061_pre_unreferenced_pc34_compat.h"

void redmcsb_f1061_pre_unreferenced_pc34_compat(void)
{
}

const char *redmcsb_f1061_pre_unreferenced_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/READWRIT.C:73-88 defines "
           "F1061_Pre_Unreferenced only inside "
           "MEDIA626_A31E_A31M_A33M_A35M with NOCOPYPROTECTION disabled. "
           "READWRIT.C:79-86 includes FAKE3.C for MEDIA618_A31E, FAKE2.C "
           "for MEDIA657_A31M_A33M_A35E, or FAKE4.C for MEDIA742_A35M. "
           "FAKE2.C:1-17, FAKE3.C:1-19, and FAKE4.C:1-17 each guard a 32 "
           "byte long 68k asm block explicitly marked never executed; no "
           "portable behavior or PC 3.4 branch is supplied.";
}
