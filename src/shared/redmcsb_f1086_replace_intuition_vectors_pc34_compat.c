#include "redmcsb_f1086_replace_intuition_vectors_pc34_compat.h"

void redmcsb_f1086_replace_intuition_vectors_pc34_compat(void)
{
}

const char *redmcsb_f1086_replace_intuition_vectors_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:283-291 defines F1086_ReplaceIntuitionVectors "
           "only for C03_GAME under the AMIGINIT.C:4 Amiga media guard. It "
           "uses SetFunction on intuition.library vectors -0x15C and -0x168 "
           "for AutoRequest and BuildSysRequest, stores their prior vectors, "
           "and sets G3164_B_. No PC 3.4 branch or portable host behavior is "
           "supplied by the source.";
}
