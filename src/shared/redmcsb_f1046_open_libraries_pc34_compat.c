#include "redmcsb_f1046_open_libraries_pc34_compat.h"

bool redmcsb_f1046_open_libraries_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1046_open_libraries_source_evidence_pc34(void)
{
    return "ReDMCSB MAINLIB.C:4-47 encloses F1046_OpenLibraries in "
           "MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J. MAINLIB.C:20-46 "
           "records G3134_PRIM_Executable, opens PRIM, and only in "
           "MEDIA692_X31J opens GRF1, VDEO, INT1, and MUSC before assigning "
           "FTL_APPB. GAMELOOP.C:318-320 calls F1046_OpenLibraries only in "
           "the same MEDIA749 route. No PC 3.4 branch, FTL library ABI, or "
           "portable host adapter is supplied.";
}
