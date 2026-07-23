#ifndef FIRESTAFF_CSB_V1_F1006_F1025_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1006_F1025_SOURCE_BOUNDARY_PC34_COMPAT_H

typedef enum {
    CSB_V1_F1006_F1025_LOCAL_OR_UNNUMBERED_PC34 = 0,
    CSB_V1_F1006_F1025_EXISTING_OWNER_NO_CSB_ADMISSION_PC34 = 1,
    CSB_V1_F1006_F1025_PLATFORM_NONAPPLICABLE_PC34 = 2
} CSB_V1_F1006F1025SourceKindPc34;

typedef struct {
    unsigned int function_number;
    CSB_V1_F1006F1025SourceKindPc34 source_kind;
    const char *symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
    int authentic_pc34_material_required;
    int runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F1006F1025SourceBoundaryReceiptPc34;

/* Always returns zero: no CSB package-backed runtime consumer is proven. */
int csb_v1_f1006_f1025_source_boundary_admit_pc34(
    unsigned int function_number,
    CSB_V1_F1006F1025SourceBoundaryReceiptPc34 *out_receipt);
const char *csb_v1_f1006_f1025_source_boundary_evidence_pc34(void);

#endif
