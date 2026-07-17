#ifndef NEXUS_V1_STRUCTURE3_ENTRY_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_ENTRY_ADMISSION_H

#include "nexus_v1_structure3_target_admission.h"

/* This binds only the parser-observed 40-byte Structure3 entry frame and its
 * three counted 12-byte byte ranges. The names mirror the source layout but
 * do not grant geometry, material, texture, transform, or draw semantics. */
typedef struct {
    int valid;
    int target_bound;
    int fixed_header_bound;
    int counted_regions_bound;
    int no_draw_only;
    int geometry_semantics_permitted;
    int material_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t target_offset;
    uint32_t target_length;
    uint64_t target_fnv1a64;
    uint32_t raw_tag;
    uint16_t first_region_count;
    uint16_t second_region_count;
    uint32_t first_region_offset;
    uint32_t second_region_offset;
    uint32_t third_region_offset;
    uint32_t first_region_length;
    uint32_t second_region_length;
    uint32_t third_region_length;
    uint64_t header_fnv1a64;
    uint64_t first_region_fnv1a64;
    uint64_t second_region_fnv1a64;
    uint64_t third_region_fnv1a64;
} Nexus_V1_Structure3EntryAdmissionReceipt;

int nexus_v1_structure3_entry_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3TargetAdmissionReceipt *target,
    Nexus_V1_Structure3EntryAdmissionReceipt *out_receipt);

#endif
