#ifndef FIRESTAFF_REDMCSB_F0939_RELEASE_AMIGA_DATA_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0939_RELEASE_AMIGA_DATA_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EXEC.C F0939 is compiled only for MEDIA442_A20E_A21E and tears down
 * Amiga Exec/graphics resources before terminating the process. No PC 3.4
 * branch or portable host adapter is supplied, so release is not applicable.
 */
bool redmcsb_f0939_release_amiga_data_pc34_compat(void);

const char *redmcsb_f0939_release_amiga_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0939_RELEASE_AMIGA_DATA_PC34_COMPAT_H */
