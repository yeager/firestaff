#ifndef FIRESTAFF_CSB_V1_F1246_F1265_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1246_F1265_SOURCE_BOUNDARY_PC34_COMPAT_H

#include <stddef.h>

typedef struct {
    unsigned int symbol_number;
    const char *redmcsb_anchor;
    const char *source_class;
    int authentic_pc34_material_required;
    int runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F1246F1265SourceBoundaryReceiptPc34;

const CSB_V1_F1246F1265SourceBoundaryReceiptPc34 *
csb_v1_f1246_f1265_source_boundary_pc34(size_t *out_count);
const CSB_V1_F1246F1265SourceBoundaryReceiptPc34 *
csb_v1_f1246_f1265_source_boundary_find_pc34(unsigned int symbol_number);
const char *csb_v1_f1246_f1265_source_boundary_evidence_pc34(void);

#endif
