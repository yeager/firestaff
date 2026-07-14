#include "redmcsb_f1083_allocate_724_bytes_pc34_compat.h"

void redmcsb_f1083_allocate_724_bytes_pc34_compat(void)
{
}

const char *redmcsb_f1083_allocate_724_bytes_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:4 guards this source with "
           "MEDIA754_A36M_A31E_A31M_A33M_A35E_A35M_AU1E_AU2E_AU2F_AU3E. "
           "AMIGINIT.C:259-266 defines F1083_Allocate724Bytes only for "
           "C03_GAME: it AllocMem(sizeof(AMISTRUCT), MEMF_CLEAR) into "
           "G3161_ac_Buffer724Bytes and issues F1050_AlertCSBSystemError "
           "0x80FF0010 on failure. AMIGA.H:99-108 defines AMISTRUCT as 724 "
           "bytes. No PC 3.4 branch or portable host behavior is supplied.";
}
