#ifndef FIRESTAFF_CSB_V1_F0846_F0865_UNMAPPED_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0846_F0865_UNMAPPED_BOUNDARY_PC34_COMPAT_H

#include <stdint.h>

typedef struct {
    uint16_t function_id;
    int source_callable_absent;
    int authentic_package_required;
    int runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
    const char *source_evidence;
} CSB_V1_F0846F0865UnmappedBoundaryReceiptPc34;

/* Always fails admission: ReDMCSB has no callable F0846-F0865 symbols. */
int csb_v1_f0846_f0865_unmapped_admit_pc34(
    uint16_t function_id,
    CSB_V1_F0846F0865UnmappedBoundaryReceiptPc34 *out_receipt);

#endif
