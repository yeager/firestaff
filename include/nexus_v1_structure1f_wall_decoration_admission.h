#ifndef NEXUS_V1_STRUCTURE1F_WALL_DECORATION_ADMISSION_H
#define NEXUS_V1_STRUCTURE1F_WALL_DECORATION_ADMISSION_H

#include "nexus_v1_structure1f_directory_admission.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Family 4 is the 0x21-tagged, twelve-byte Structure1A-bound wall-decoration
 * row. Byte 1 is retained as a raw face selector and bytes 2..3 as a
 * big-endian Structure1A index. Their world, face, mesh and draw relations
 * remain unproved. */
typedef struct {
    int valid;
    int directory_bound;
    int wall_decoration_record_bound;
    int face_selector_bound;
    int structure1a_index_bound;
    int raw_layout_bound;
    int no_draw_only;
    int face_semantics_permitted;
    int mesh_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t record_index;
    uint32_t record_offset;
    uint64_t record_fnv1a64;
    uint8_t source_tag;
    uint8_t raw_face_selector;
    uint16_t raw_structure1a_index;
    uint8_t raw_payload[8];
} Nexus_V1_Structure1FWallDecorationAdmissionReceipt;

int nexus_v1_structure1f_wall_decoration_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *directory,
    uint32_t record_index,
    Nexus_V1_Structure1FWallDecorationAdmissionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
