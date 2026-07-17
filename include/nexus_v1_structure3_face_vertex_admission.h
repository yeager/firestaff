#ifndef NEXUS_V1_STRUCTURE3_FACE_VERTEX_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_FACE_VERTEX_ADMISSION_H

#include "nexus_v1_structure3_face_admission.h"

/* One Structure3b index is joined to its exact twelve-byte first-region row.
 * The row remains opaque; this is not a coordinate, vertex, geometry, or draw
 * admission. */
typedef struct {
    int valid;
    int entry_bound;
    int face_row_bound;
    int indexed_row_bound;
    int no_draw_only;
    int coordinate_semantics_permitted;
    int geometry_semantics_permitted;
    int material_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint64_t face_fnv1a64;
    uint32_t index_slot;
    uint16_t raw_index;
    uint32_t row_offset;
    uint64_t row_fnv1a64;
    uint8_t raw_bytes[12];
} Nexus_V1_Structure3FaceVertexAdmissionReceipt;

int nexus_v1_structure3_face_vertex_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    uint32_t index_slot,
    Nexus_V1_Structure3FaceVertexAdmissionReceipt *out_receipt);

#endif
