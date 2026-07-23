#ifndef FIRESTAFF_CSB_V1_F1066_F1085_AMIGA_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1066_F1085_AMIGA_SOURCE_BOUNDARY_PC34_COMPAT_H

typedef enum {
    CSB_V1_F1066_F1085_EXISTING_OWNER_NO_CSB_ADMISSION_PC34 = 0,
    CSB_V1_F1066_F1085_PLATFORM_NONAPPLICABLE_PC34 = 1
} CSB_V1_F1066F1085SourceKindPc34;

typedef struct {
    unsigned int function_number;
    CSB_V1_F1066F1085SourceKindPc34 source_kind;
    const char *symbol;
    const char *source_anchor;
    int authentic_pc34_material_required;
    int runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F1066F1085SourceBoundaryReceiptPc34;

/* Always returns zero: this Amiga lane has no CSB PC34 runtime admission. */
int csb_v1_f1066_f1085_source_boundary_admit_pc34(
    unsigned int function_number,
    CSB_V1_F1066F1085SourceBoundaryReceiptPc34 *out_receipt);
const char *csb_v1_f1066_f1085_source_boundary_evidence_pc34(void);

#endif
