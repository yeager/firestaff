#include "redmcsb_f1087_restore_intuition_vectors_pc34_compat.h"

void redmcsb_f1087_restore_intuition_vectors_pc34_compat(void)
{
}

const char *redmcsb_f1087_restore_intuition_vectors_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:293-306 defines F1087_RestoreIntuitionVectors "
           "only for C03_GAME under the AMIGINIT.C:4 Amiga media guard. When "
           "G3164_B_ is set it restores the two intuition.library vectors and "
           "alerts 0x80FF0015 unless both replacements were still F1085. No "
           "PC 3.4 branch or portable host behavior is supplied by the source.";
}
