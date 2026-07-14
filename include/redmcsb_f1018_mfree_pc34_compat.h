#ifndef FIRESTAFF_REDMCSB_F1018_MFREE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1018_MFREE_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * CEDT018.C F1018_Mfree is an X68000-native Mfree release route. It has no
 * PC 3.4 branch or portable host adapter, so this boundary intentionally
 * performs no release operation.
 */
bool redmcsb_f1018_mfree_pc34_compat(void);

const char *redmcsb_f1018_mfree_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1018_MFREE_PC34_COMPAT_H */
