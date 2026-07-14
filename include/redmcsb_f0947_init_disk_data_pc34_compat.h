#ifndef FIRESTAFF_REDMCSB_F0947_INIT_DISK_DATA_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0947_INIT_DISK_DATA_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EXEC.C F0947 exists only in the MEDIA442_A20E_A21E Amiga build. It
 * allocates chip memory for a track buffer and IOExtTD, creates an Amiga
 * message port, and installs that port as the request reply port. No PC 3.4
 * route or portable host adapter is supplied, so it is unavailable here.
 */
bool redmcsb_f0947_init_disk_data_pc34_compat(void);

const char *redmcsb_f0947_init_disk_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0947_INIT_DISK_DATA_PC34_COMPAT_H */
