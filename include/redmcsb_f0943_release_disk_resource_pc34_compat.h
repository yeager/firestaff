#ifndef FIRESTAFF_REDMCSB_F0943_RELEASE_DISK_RESOURCE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0943_RELEASE_DISK_RESOURCE_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EXEC.C F0943 exists only in the MEDIA442_A20E_A21E Amiga build and
 * releases an Amiga message port and DiscResourceUnit through Exec APIs.
 * No PC 3.4 route or portable host adapter is supplied, so it is not
 * applicable on this host.
 */
bool redmcsb_f0943_release_disk_resource_pc34_compat(void);

const char *redmcsb_f0943_release_disk_resource_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0943_RELEASE_DISK_RESOURCE_PC34_COMPAT_H */
