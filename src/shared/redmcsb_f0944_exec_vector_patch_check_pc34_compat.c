#include "redmcsb_f0944_exec_vector_patch_check_pc34_compat.h"

bool redmcsb_f0944_exec_vector_patch_check_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f0944_exec_vector_patch_check_source_evidence_pc34(void)
{
    return "ReDMCSB EXEC.C:4-16 guards F0944_ with "
           "MEDIA442_A20E_A21E; EXEC.C:408-431 obtains Exec from address "
           "4, reads its DoIO (-0x1C6), WaitIO (-0x1D8), and OpenDevice "
           "(-0x1BA) vectors, and sets G0727_B_ when any is below 0x80000. "
           "No PC 3.4 branch or portable host adapter is supplied.";
}
