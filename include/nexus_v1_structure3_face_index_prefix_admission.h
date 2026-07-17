#ifndef NEXUS_V1_STRUCTURE3_FACE_INDEX_PREFIX_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_FACE_INDEX_PREFIX_ADMISSION_H

#include "nexus_v1_structure3_face_admission.h"

/* The first eight bytes of a bounded Structure3b row are retained only as a
 * raw index prefix. This admits neither topology nor geometry nor a draw. */
typedef struct {
    int valid;
    int entry_bound;
    int face_row_bound;
    int prefix_bound;
    int no_draw_only;
    int topology_semantics_permitted;
    int geometry_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint64_t face_fnv1a64;
    uint32_t prefix_offset;
    uint64_t prefix_fnv1a64;
    uint8_t raw_bytes[8];
} Nexus_V1_Structure3FaceIndexPrefixAdmissionReceipt;

int nexus_v1_structure3_face_index_prefix_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure3EntryAdmissionReceipt *entry,
    const Nexus_V1_Structure3FaceAdmissionReceipt *face,
    Nexus_V1_Structure3FaceIndexPrefixAdmissionReceipt *out_receipt);

#endif
