#ifndef NEXUS_V1_STRUCTURE3_NORMAL_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_NORMAL_ADMISSION_H

#include "nexus_v1_structure3_face_admission.h"

/* The third counted 12-byte region is joined only to its same-ordinal face
 * row. Its bytes are deliberately opaque: no normal, vector, plane, lighting
 * or draw meaning is admitted. */
typedef struct {
    int valid;
    int entry_bound;
    int face_row_bound;
    int same_ordinal_row_bound;
    int no_draw_only;
    int geometry_semantics_permitted;
    int lighting_semantics_permitted;
    int material_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint64_t face_fnv1a64;
    uint32_t row_offset;
    uint64_t row_fnv1a64;
    uint8_t raw_bytes[12];
} Nexus_V1_Structure3NormalAdmissionReceipt;

int nexus_v1_structure3_normal_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    Nexus_V1_Structure3NormalAdmissionReceipt *out_receipt);

#endif
