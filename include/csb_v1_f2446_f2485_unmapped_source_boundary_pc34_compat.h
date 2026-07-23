#ifndef FIRESTAFF_CSB_V1_F2446_F2485_UNMAPPED_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F2446_F2485_UNMAPPED_SOURCE_BOUNDARY_PC34_COMPAT_H

typedef struct {
    unsigned int symbol_number;
    const char *redmcsb_source_owner;
    int authentic_pc34_material_required;
    int csb_runtime_execution_blocked;
    int no_synthetic_viewport_behavior;
} CSB_V1_F2446F2485UnmappedSourceBoundaryReceiptPc34;

int csb_v1_f2446_f2485_unmapped_source_boundary_admit_pc34(
    unsigned int symbol_number,
    CSB_V1_F2446F2485UnmappedSourceBoundaryReceiptPc34 *out_receipt);
const char *csb_v1_f2446_f2485_unmapped_source_boundary_evidence_pc34(void);

#endif
