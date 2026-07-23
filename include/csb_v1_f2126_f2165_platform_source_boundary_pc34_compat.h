#ifndef FIRESTAFF_CSB_V1_F2126_F2165_PLATFORM_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F2126_F2165_PLATFORM_SOURCE_BOUNDARY_PC34_COMPAT_H

typedef struct {
    unsigned int function_number;
    const char *redmcsb_anchor;
    const char *existing_owner_or_boundary;
    int authentic_pc34_material_required;
    int csb_runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F2126F2165PlatformSourceBoundaryReceiptPc34;

int csb_v1_f2126_f2165_platform_source_boundary_admit_pc34(
    unsigned int function_number,
    CSB_V1_F2126F2165PlatformSourceBoundaryReceiptPc34 *out_receipt);
const char *csb_v1_f2126_f2165_platform_source_boundary_evidence_pc34(void);

#endif
