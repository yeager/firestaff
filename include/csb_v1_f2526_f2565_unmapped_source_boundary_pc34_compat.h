#ifndef FIRESTAFF_CSB_V1_F2526_F2565_UNMAPPED_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F2526_F2565_UNMAPPED_SOURCE_BOUNDARY_PC34_COMPAT_H

typedef struct {
    unsigned int inventory_number;
    const char *redmcsb_source_owner;
    int authentic_pc34_material_required;
    int csb_runtime_execution_blocked;
    int no_synthetic_platform_or_endgame_behavior;
} CSB_V1_F2526F2565UnmappedSourceBoundaryReceiptPc34;

int csb_v1_f2526_f2565_unmapped_source_boundary_admit_pc34(
    unsigned int inventory_number,
    CSB_V1_F2526F2565UnmappedSourceBoundaryReceiptPc34 *out_receipt);
const char *csb_v1_f2526_f2565_unmapped_source_boundary_evidence_pc34(void);

#endif
