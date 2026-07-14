#ifndef FIRESTAFF_REDMCSB_F0948_RELEASE_DISK_DATA_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0948_RELEASE_DISK_DATA_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EXEC.C F0948 exists only in the MEDIA442_A20E_A21E Amiga build and
 * releases Amiga trackdisk allocations and its reply port. No PC 3.4 route
 * or portable host adapter is supplied, so it is not applicable on this host.
 */
bool redmcsb_f0948_release_disk_data_pc34_compat(void);

const char *redmcsb_f0948_release_disk_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0948_RELEASE_DISK_DATA_PC34_COMPAT_H */
