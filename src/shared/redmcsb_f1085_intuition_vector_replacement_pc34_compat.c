#include "redmcsb_f1085_intuition_vector_replacement_pc34_compat.h"

int16_t redmcsb_f1085_intuition_vector_replacement_pc34_compat(void)
{
    return 0;
}

const char *redmcsb_f1085_intuition_vector_replacement_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:277-281 defines F1085_IntuitionVectorReplacement "
           "only for C03_GAME under the AMIGINIT.C:4 Amiga media guard; its "
           "entire body returns 0. F1086 installs it in two intuition.library "
           "vectors. The callback result is source-defined, while vector "
           "installation has no PC 3.4 branch.";
}
