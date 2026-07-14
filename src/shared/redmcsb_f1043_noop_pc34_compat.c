#include "redmcsb_f1043_noop_pc34_compat.h"

void redmcsb_f1043_noop_pc34_compat(void)
{
}

const char *redmcsb_f1043_noop_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:1087-1092 defines F1043_ only inside "
           "MEDIA746_A36M_A31E_A31M_A33M_A35E_A35M; its body is empty. "
           "INPUT.C:500-525 calls F1043_ only in that same Amiga route "
           "after palette-update vertical blanks for the Amiga+N and Amiga+M "
           "view switches. No PC 3.4 branch or portable host adapter is "
           "supplied.";
}
