#include "redmcsb_f1065_set_exec_base_pc34_compat.h"

void redmcsb_f1065_set_exec_base_pc34_compat(void)
{
}

const char *redmcsb_f1065_set_exec_base_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/AMIGALIB.C:837-844 defines "
           "F1065_SetExecBase only for enumerated Amiga media and EXETYPE "
           "variants; AMIGALIB.C:841-843 contains only the 68k instruction "
           "move.l (4).w,ExecBase(a5), loading the Amiga Exec address into "
           "the 68k global-base register. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}
