#ifndef FIRESTAFF_CSB_V1_F1126_F1145_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1126_F1145_SOURCE_BOUNDARY_PC34_COMPAT_H
typedef struct {
    unsigned int function_number;
    const char *symbol;
    const char *source_anchor;
    int authentic_pc34_material_required;
    int runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F1126F1145SourceBoundaryReceiptPc34;
int csb_v1_f1126_f1145_source_boundary_admit_pc34(unsigned int function_number, CSB_V1_F1126F1145SourceBoundaryReceiptPc34 *out_receipt);
const char *csb_v1_f1126_f1145_source_boundary_evidence_pc34(void);
#endif
