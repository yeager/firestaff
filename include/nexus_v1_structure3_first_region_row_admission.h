#ifndef NEXUS_V1_STRUCTURE3_FIRST_REGION_ROW_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_FIRST_REGION_ROW_ADMISSION_H

#include "nexus_v1_structure3_entry_admission.h"

/* One ordinal-selected first-region row is retained as raw source bytes.
 * It is not a coordinate, vertex, geometry, material, texture, or draw
 * admission. */
typedef struct {
    int valid;
    int entry_bound;
    int first_region_bound;
    int ordinal_row_bound;
    int no_draw_only;
    int coordinate_semantics_permitted;
    int geometry_semantics_permitted;
    int material_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t entry_offset;
    uint64_t entry_fnv1a64;
    uint32_t row_ordinal;
    uint32_t row_offset;
    uint64_t row_fnv1a64;
    uint8_t raw_bytes[12];
} Nexus_V1_Structure3FirstRegionRowAdmissionReceipt;

int nexus_v1_structure3_first_region_row_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    uint32_t row_ordinal,
    Nexus_V1_Structure3FirstRegionRowAdmissionReceipt *out_receipt);

#endif
