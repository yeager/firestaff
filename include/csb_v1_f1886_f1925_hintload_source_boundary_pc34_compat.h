#ifndef FIRESTAFF_CSB_V1_F1886_F1925_HINTLOAD_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1886_F1925_HINTLOAD_SOURCE_BOUNDARY_PC34_COMPAT_H

#include <stddef.h>

typedef struct {
    unsigned int function_number;
    const char *redmcsb_anchor;
    const char *existing_owner_or_boundary;
    int existing_authenticated_csb_owner;
    int authentic_pc34_material_required;
    int fail_closed_without_pc34;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F1886F1925HintloadSourceReceiptPc34;

const CSB_V1_F1886F1925HintloadSourceReceiptPc34 *
csb_v1_f1886_f1925_hintload_source_receipt_pc34(size_t *out_count);
const CSB_V1_F1886F1925HintloadSourceReceiptPc34 *
csb_v1_f1886_f1925_hintload_source_find_pc34(unsigned int function_number);
const char *csb_v1_f1886_f1925_hintload_source_evidence_pc34(void);

#endif
