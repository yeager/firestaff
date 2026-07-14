#include "redmcsb_f1047_close_libraries_pc34_compat.h"

bool redmcsb_f1047_close_libraries_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1047_close_libraries_source_evidence_pc34(void)
{
    return "ReDMCSB Toolchains/Common/Source/MAINLIB.C:4-60 guards "
           "F1047_CloseLibraries with "
           "MEDIA749_A36M_A31E_A31M_A33M_A35E_A35M_X31J. MAINLIB.C:52-58 "
           "closes MUSC, INT1, VDEO, and GRF1 only under MEDIA692_X31J, "
           "then calls F9009_ClosePRIM. "
           "Toolchains/Common/Source/GAMELOOP.C:334-336 calls F1047_CloseLibraries "
           "only in the same MEDIA749 route. No PC 3.4 branch or portable "
           "host adapter is supplied.";
}
