#ifndef NEXUS_V1_STRUCTURE3_THIRD_REGION_ROW_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_THIRD_REGION_ROW_ADMISSION_H

#include "nexus_v1_structure3_entry_admission.h"

/* One ordinal-selected third-region row is retained as raw source bytes.
 * This does not identify it as a normal or permit vector, lighting, material,
 * texture, geometry, or draw semantics. */
typedef struct {
    int valid;
    int entry_bound;
    int third_region_bound;
    int ordinal_row_bound;
    int no_draw_only;
    int vector_semantics_permitted;
    int lighting_semantics_permitted;
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
} Nexus_V1_Structure3ThirdRegionRowAdmissionReceipt;

int nexus_v1_structure3_third_region_row_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    uint32_t row_ordinal,
    Nexus_V1_Structure3ThirdRegionRowAdmissionReceipt *out_receipt);

#endif
