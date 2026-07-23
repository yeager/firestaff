#ifndef FIRESTAFF_CSB_V1_F1186_F1205_ANIM_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1186_F1205_ANIM_SOURCE_BOUNDARY_PC34_COMPAT_H

typedef struct {
    unsigned int function_number;
    const char *symbol;
    const char *redmcsb_anchor;
    int authentic_pc34_material_required;
    int runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F1186F1205AnimSourceBoundaryReceiptPc34;

int csb_v1_f1186_f1205_anim_source_boundary_admit_pc34(
    unsigned int function_number,
    CSB_V1_F1186F1205AnimSourceBoundaryReceiptPc34 *out_receipt);
const char *csb_v1_f1186_f1205_anim_source_boundary_evidence_pc34(void);

#endif
