#include "redmcsb_f1024_set_trap14_vector_error_processing_pc34_compat.h"

bool redmcsb_f1024_set_trap14_vector_error_processing_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1024_set_trap14_vector_error_processing_source_evidence_pc34(void)
{
    return "ReDMCSB FILE.C:806-836 and FILE.C:1091-1126 define "
           "F1024_SetTrap14VectorErrorProcessing only for X68000 builds. "
           "It installs vector 46 through DOS INTVCS, saves the 68000 A5 "
           "register, and its TRAP 14 handler increments G3091_i_ErrorCount. "
           "STARTUP2.C:1361-1364 calls it only in MEDIA607_X30J_X31J; "
           "CEDT023.C:181-192 calls it only in MEDIA692_X31J. No PC 3.4 "
           "branch or portable host adapter is supplied.";
}
