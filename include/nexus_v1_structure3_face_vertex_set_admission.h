#ifndef NEXUS_V1_STRUCTURE3_FACE_VERTEX_SET_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_FACE_VERTEX_SET_ADMISSION_H

#include "nexus_v1_structure3_face_vertex_admission.h"

/* The four raw Structure3b index slots are retained in source order. Repeated
 * rows stay repeated; this is neither a polygon nor a topology admission. */
typedef struct {
    int valid;
    int entry_bound;
    int face_row_bound;
    int ordered_slots_bound;
    int no_draw_only;
    int topology_semantics_permitted;
    int geometry_semantics_permitted;
    int material_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint64_t face_fnv1a64;
    Nexus_V1_Structure3FaceVertexAdmissionReceipt slots[4];
} Nexus_V1_Structure3FaceVertexSetAdmissionReceipt;

int nexus_v1_structure3_face_vertex_set_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    Nexus_V1_Structure3FaceVertexSetAdmissionReceipt *out_receipt);

#endif
