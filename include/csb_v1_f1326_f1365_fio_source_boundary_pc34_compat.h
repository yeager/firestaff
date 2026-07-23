#ifndef FIRESTAFF_CSB_V1_F1326_F1365_FIO_SOURCE_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1326_F1365_FIO_SOURCE_BOUNDARY_PC34_COMPAT_H

#include <stddef.h>

typedef struct {
    unsigned int function_number;
    const char *redmcsb_anchor;
    const char *existing_owner_or_boundary;
    int authentic_pc34_material_required;
    int csb_runtime_execution_blocked;
    int no_synthetic_ui_graphics_timing;
} CSB_V1_F1326F1365FioSourceBoundaryReceiptPc34;

const CSB_V1_F1326F1365FioSourceBoundaryReceiptPc34 *
csb_v1_f1326_f1365_fio_source_boundary_pc34(size_t *out_count);
const CSB_V1_F1326F1365FioSourceBoundaryReceiptPc34 *
csb_v1_f1326_f1365_fio_source_boundary_find_pc34(unsigned int function_number);
const char *csb_v1_f1326_f1365_fio_source_boundary_evidence_pc34(void);

#endif
