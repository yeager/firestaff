#ifndef FIRESTAFF_CSB_V1_F0986_F1005_GRAPHICS_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0986_F1005_GRAPHICS_SOURCE_BOUNDARY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CSB_V1_F0986_F1005_LOCAL_LABEL_PC34 = 0,
    CSB_V1_F0986_F1005_NON_PC34_MEDIA_PC34 = 1,
    CSB_V1_F0986_F1005_EXISTING_OWNER_WITHOUT_CSB_ADMISSION_PC34 = 2,
    CSB_V1_F0986_F1005_UNNUMBERED_SOURCE_HELPER_PC34 = 3
} CSB_V1_F0986F1005SourceKindPc34;

typedef struct {
    unsigned int function_number;
    CSB_V1_F0986F1005SourceKindPc34 source_kind;
    const char *symbol;
    const char *source_anchor;
    const char *owner_or_rationale;
    int authentic_pc34_material_required;
    int runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F0986F1005SourceBoundaryReceiptPc34;

/* Always returns zero: this lane has no proven authenticated CSB PC34 owner. */
int csb_v1_f0986_f1005_source_boundary_admit_pc34(
    unsigned int function_number,
    CSB_V1_F0986F1005SourceBoundaryReceiptPc34 *out_receipt);

const char *csb_v1_f0986_f1005_source_boundary_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0986_F1005_GRAPHICS_SOURCE_BOUNDARY_PC34_COMPAT_H */
