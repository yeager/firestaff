#include "csb_v1_f1085_f1086_f1087_intuition_vector_helpers_pc34_compat.h"
#include "redmcsb_f1085_intuition_vector_replacement_pc34_compat.h"
#include "redmcsb_f1086_replace_intuition_vectors_pc34_compat.h"
#include "redmcsb_f1087_restore_intuition_vectors_pc34_compat.h"

int16_t csb_v1_f1085_intuition_vector_replacement_pc34_compat(void)
{
    return 0;
}

int16_t redmcsb_f1085_intuition_vector_replacement_pc34_compat(void)
{
    return csb_v1_f1085_intuition_vector_replacement_pc34_compat();
}

int16_t F1085_IntuitionVectorReplacement(void)
{
    return csb_v1_f1085_intuition_vector_replacement_pc34_compat();
}

const char *csb_v1_f1085_intuition_vector_replacement_source_evidence_pc34(
    void)
{
    return "ReDMCSB AMIGINIT.C:277 F1085_IntuitionVectorReplacement; "
           "source-defined zero callback, no Amiga Intuition vector behavior "
           "is claimed on PC34";
}

const char *redmcsb_f1085_intuition_vector_replacement_source_evidence_pc34(
    void)
{
    return csb_v1_f1085_intuition_vector_replacement_source_evidence_pc34();
}

void csb_v1_f1086_replace_intuition_vectors_pc34_compat(void)
{
}

void redmcsb_f1086_replace_intuition_vectors_pc34_compat(void)
{
    csb_v1_f1086_replace_intuition_vectors_pc34_compat();
}

void F1086_ReplaceIntuitionVectors(void)
{
    csb_v1_f1086_replace_intuition_vectors_pc34_compat();
}

const char *csb_v1_f1086_replace_intuition_vectors_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:283 F1086_ReplaceIntuitionVectors; "
           "Amiga-only Intuition vector replacement boundary, no PC34 "
           "portable host vector route";
}

const char *redmcsb_f1086_replace_intuition_vectors_source_evidence_pc34(void)
{
    return csb_v1_f1086_replace_intuition_vectors_source_evidence_pc34();
}

void csb_v1_f1087_restore_intuition_vectors_pc34_compat(void)
{
}

void redmcsb_f1087_restore_intuition_vectors_pc34_compat(void)
{
    csb_v1_f1087_restore_intuition_vectors_pc34_compat();
}

void F1087_RestoreIntuitionVectors(void)
{
    csb_v1_f1087_restore_intuition_vectors_pc34_compat();
}

const char *csb_v1_f1087_restore_intuition_vectors_source_evidence_pc34(void)
{
    return "ReDMCSB AMIGINIT.C:293 F1087_RestoreIntuitionVectors; "
           "Amiga-only Intuition vector restore boundary, no PC34 portable "
           "host vector route";
}

const char *redmcsb_f1087_restore_intuition_vectors_source_evidence_pc34(void)
{
    return csb_v1_f1087_restore_intuition_vectors_source_evidence_pc34();
}
