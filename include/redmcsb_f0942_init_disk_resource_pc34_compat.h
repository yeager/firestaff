#ifndef FIRESTAFF_REDMCSB_F0942_INIT_DISK_RESOURCE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0942_INIT_DISK_RESOURCE_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * EXEC.C F0942 exists only in the MEDIA442_A20E_A21E Amiga build and
 * acquires Amiga disk.resource, a DiscResourceUnit, and a message port.
 * No PC 3.4 route or portable host adapter is supplied, so it is not
 * applicable on this host.
 */
bool redmcsb_f0942_init_disk_resource_pc34_compat(void);

const char *redmcsb_f0942_init_disk_resource_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0942_INIT_DISK_RESOURCE_PC34_COMPAT_H */
