#include "redmcsb_f1084_free_724_bytes_pc34_compat.h"

void redmcsb_f1084_free_724_bytes_pc34_compat(void)
{
}

const char *redmcsb_f1084_free_724_bytes_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:267-275 defines F1084_Free724Bytes only for "
           "C03_GAME under the AMIGINIT.C:4 Amiga media guard: it FreeMem "
           "releases G3161_ac_Buffer724Bytes with sizeof(AMISTRUCT) when "
           "present and clears the pointer. No PC 3.4 branch or portable host "
           "behavior is supplied by the source.";
}
