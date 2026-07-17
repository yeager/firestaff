#ifndef NEXUS_V1_STRUCTURE1F_ITEM_ADMISSION_H
#define NEXUS_V1_STRUCTURE1F_ITEM_ADMISSION_H

#include "nexus_v1_structure1f_directory_admission.h"

typedef struct {
    int valid;
    int directory_bound;
    int item_record_bound;
    int coordinate_pair_bound;
    int no_draw_only;
    int mesh_semantics_permitted;
    int face_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t record_index;
    uint32_t record_offset;
    uint64_t record_fnv1a64;
    uint8_t source_tag;
    uint8_t x;
    uint8_t y;
    uint16_t cell_ordinal;
    uint8_t opaque_tail[5];
} Nexus_V1_Structure1FItemAdmissionReceipt;

/* The Saturn/DMWeb Structure1F reference identifies family 0 as eight-byte
 * item rows with documented 64x64 coordinates at bytes 1 and 2. Bytes 3..7
 * remain opaque here. */
int nexus_v1_structure1f_item_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *directory,
    uint32_t record_index, Nexus_V1_Structure1FItemAdmissionReceipt *out_receipt);

#endif
