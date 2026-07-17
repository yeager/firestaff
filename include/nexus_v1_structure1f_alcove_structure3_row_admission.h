#ifndef NEXUS_V1_STRUCTURE1F_ALCOVE_STRUCTURE3_ROW_ADMISSION_H
#define NEXUS_V1_STRUCTURE1F_ALCOVE_STRUCTURE3_ROW_ADMISSION_H

#include "nexus_v1_structure1f_alcove_admission.h"
#include "nexus_v1_structure3_face_admission.h"
#include "nexus_v1_structure3_second_region_row_admission.h"

/* This is only an equality witness between the raw Structure1F selector byte
 * and one admitted Structure3 second-region row ordinal. It admits neither a
 * portal nor face, topology, geometry, material, texture, or draw semantics. */
typedef struct {
    int valid;
    int structure1f_record_bound;
    int structure3_entry_bound;
    int structure3_row_bound;
    int selector_row_ordinal_bound;
    int no_draw_only;
    int portal_semantics_permitted;
    int face_semantics_permitted;
    int topology_semantics_permitted;
    int geometry_semantics_permitted;
    int material_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t structure1f_record_offset;
    uint64_t structure1f_record_fnv1a64;
    uint8_t raw_selector;
    uint32_t structure3_row_ordinal;
    uint32_t structure3_row_offset;
    uint64_t structure3_row_fnv1a64;
} Nexus_V1_Structure1FAlcoveStructure3RowAdmissionReceipt;

int nexus_v1_structure1f_alcove_structure3_row_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FAlcoveAdmissionReceipt *alcove,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    const Nexus_V1_Structure3SecondRegionRowAdmissionReceipt *row,
    Nexus_V1_Structure1FAlcoveStructure3RowAdmissionReceipt *out_receipt);

#endif
