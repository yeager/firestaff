#ifndef NEXUS_V1_STRUCTURE3_FACE_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_FACE_ADMISSION_H

#include "nexus_v1_structure3_entry_admission.h"

/* A Structure3b row retains four bounded entry-local indexes and its final
 * raw control bytes. It is not a surface, winding, material, or draw claim. */
typedef struct {
    int valid;
    int entry_bound;
    int face_row_bound;
    int vertex_indexes_bound;
    int no_draw_only;
    int geometry_semantics_permitted;
    int material_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t entry_offset;
    uint64_t entry_fnv1a64;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint64_t face_fnv1a64;
    uint16_t vertex_indexes[4];
    uint8_t raw_control;
    uint8_t raw_auxiliary;
    uint16_t raw_fill_selector;
    int fourth_index_repeats_third;
} Nexus_V1_Structure3FaceAdmissionReceipt;

int nexus_v1_structure3_face_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    uint32_t face_ordinal, Nexus_V1_Structure3FaceAdmissionReceipt *out_receipt);

#endif
