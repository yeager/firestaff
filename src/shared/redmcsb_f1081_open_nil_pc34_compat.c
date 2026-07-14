#include "redmcsb_f1081_open_nil_pc34_compat.h"

void redmcsb_f1081_open_nil_pc34_compat(void)
{
}

const char *redmcsb_f1081_open_nil_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:4 guards this source with "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU2G_AU3E. "
           "AMIGINIT.C:235-246 defines F1081_OpenNIL only for C03_GAME, "
           "C07_HINT, C01_SWOOSH, or AU1E C06_CEDT: it opens the Amiga DOS "
           "NIL: device with MODE_NEWFILE into G3159_ps_NIL1/G3160_ps_NIL2 "
           "and reports an Amiga alert on failure. No PC 3.4 branch or "
           "portable host behavior is supplied by the source.";
}
