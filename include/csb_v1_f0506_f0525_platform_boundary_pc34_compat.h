#ifndef FIRESTAFF_CSB_V1_F0506_F0525_PLATFORM_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0506_F0525_PLATFORM_BOUNDARY_PC34_COMPAT_H

#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_runtime_plan.h"

typedef enum {
    CSB_V1_F0506_F0525_PLATFORM_REJECT_ARGUMENT = -1,
    CSB_V1_F0506_F0525_PLATFORM_REJECT_UNPROVEN_AMIGA_OWNER = -2
} CSB_V1_F0506F0525PlatformBoundaryResult;

typedef struct {
    int valid;
    uint32_t source_bound_mask;
    int amiga_owner_required;
    int floppy_owner_required;
    const char *source_evidence;
} CSB_V1_F0506F0525PlatformBoundaryReceiptPc34;

/* ReDMCSB AMIGA.H boundary: never emulates unavailable Amiga owners. */
int csb_v1_f0506_f0525_platform_boundary_receipt_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0506F0525PlatformBoundaryReceiptPc34 *out_receipt);

#endif
