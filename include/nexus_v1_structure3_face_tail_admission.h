#ifndef NEXUS_V1_STRUCTURE3_FACE_TAIL_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_FACE_TAIL_ADMISSION_H

#include "nexus_v1_structure3_face_admission.h"

/* The final four bytes of one bounded Structure3b row are retained as an
 * opaque source suffix. They are not a material, flag, palette, or draw
 * admission. */
typedef struct {
    int valid;
    int entry_bound;
    int face_row_bound;
    int tail_bound;
    int no_draw_only;
    int material_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint64_t face_fnv1a64;
    uint32_t tail_offset;
    uint64_t tail_fnv1a64;
    uint8_t raw_bytes[4];
} Nexus_V1_Structure3FaceTailAdmissionReceipt;

int nexus_v1_structure3_face_tail_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    Nexus_V1_Structure3FaceTailAdmissionReceipt *out_receipt);

#endif
