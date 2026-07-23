#ifndef FIRESTAFF_CSB_V1_F0566_F0585_PLATFORM_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0566_F0585_PLATFORM_BOUNDARY_PC34_COMPAT_H

#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_runtime_plan.h"

typedef enum {
    CSB_V1_F0566_F0585_PLATFORM_REJECT_ARGUMENT = -1,
    CSB_V1_F0566_F0585_PLATFORM_REJECT_UNPROVEN_AMIGA_OWNER = -2
} CSB_V1_F0566F0585PlatformBoundaryResult;

typedef struct {
    int valid;
    uint32_t source_bound_mask;
    int viewport_owner_required;
    int interrupt_owner_required;
    int entrance_owner_required;
    const char *source_evidence;
} CSB_V1_F0566F0585PlatformBoundaryReceiptPc34;

/* ReDMCSB AMIGA.H boundary: does not emulate unavailable platform owners. */
int csb_v1_f0566_f0585_platform_boundary_receipt_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0566F0585PlatformBoundaryReceiptPc34 *out_receipt);

#endif
