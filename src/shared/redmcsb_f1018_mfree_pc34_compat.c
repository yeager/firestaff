#include "redmcsb_f1018_mfree_pc34_compat.h"

bool redmcsb_f1018_mfree_pc34_compat(void)
{
    return false;
}

const char *redmcsb_f1018_mfree_source_evidence_pc34(void)
{
    return "ReDMCSB CEDT018.C F1018_Mfree is routed only through the "
           "X68000 media build and invokes the native Mfree service. No PC "
           "3.4 branch or portable host adapter is supplied.";
}
