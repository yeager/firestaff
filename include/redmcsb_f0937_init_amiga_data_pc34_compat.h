#ifndef FIRESTAFF_REDMCSB_F0937_INIT_AMIGA_DATA_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0937_INIT_AMIGA_DATA_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EXEC.C F0937 is compiled only for MEDIA442_A20E_A21E and directly uses
 * Amiga Exec, graphics.library, and chipset memory. No PC 3.4 branch or
 * portable host adapter is supplied, so initialization is not applicable.
 */
bool redmcsb_f0937_init_amiga_data_pc34_compat(void);

const char *redmcsb_f0937_init_amiga_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0937_INIT_AMIGA_DATA_PC34_COMPAT_H */
