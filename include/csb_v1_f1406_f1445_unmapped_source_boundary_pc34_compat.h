#ifndef FIRESTAFF_CSB_V1_F1406_F1445_UNMAPPED_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1406_F1445_UNMAPPED_SOURCE_BOUNDARY_PC34_COMPAT_H

typedef struct {
    unsigned int function_number;
    const char *redmcsb_callable_source;
    int authentic_pc34_material_required;
    int csb_runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F1406F1445UnmappedSourceBoundaryReceiptPc34;

int csb_v1_f1406_f1445_unmapped_source_boundary_admit_pc34(
    unsigned int function_number,
    CSB_V1_F1406F1445UnmappedSourceBoundaryReceiptPc34 *out_receipt);
const char *csb_v1_f1406_f1445_unmapped_source_boundary_evidence_pc34(void);

#endif
