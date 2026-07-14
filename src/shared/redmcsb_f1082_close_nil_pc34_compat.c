#include "redmcsb_f1082_close_nil_pc34_compat.h"

void redmcsb_f1082_close_nil_pc34_compat(void)
{
}

const char *redmcsb_f1082_close_nil_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:4 guards this source with "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU3E. "
           "AMIGINIT.C:248-257 defines F1082_CloseNIL beside F1081 only for "
           "the Amiga executable guard: it closes G3160_ps_NIL2 when present "
           "and clears both NIL handles. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}
