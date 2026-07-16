#ifndef FIRESTAFF_CSB_V1_F1085_F1086_F1087_INTUITION_VECTOR_HELPERS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1085_F1086_F1087_INTUITION_VECTOR_HELPERS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int16_t F1085_IntuitionVectorReplacement(void);

void F1086_ReplaceIntuitionVectors(void);

void F1087_RestoreIntuitionVectors(void);

int16_t csb_v1_f1085_intuition_vector_replacement_pc34_compat(void);
void csb_v1_f1086_replace_intuition_vectors_pc34_compat(void);
void csb_v1_f1087_restore_intuition_vectors_pc34_compat(void);

const char *csb_v1_f1085_intuition_vector_replacement_source_evidence_pc34(
    void);
const char *csb_v1_f1086_replace_intuition_vectors_source_evidence_pc34(void);
const char *csb_v1_f1087_restore_intuition_vectors_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1085_F1086_F1087_INTUITION_VECTOR_HELPERS_PC34_COMPAT_H */
