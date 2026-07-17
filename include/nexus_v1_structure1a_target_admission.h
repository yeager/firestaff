#ifndef NEXUS_V1_STRUCTURE1A_TARGET_ADMISSION_H
#define NEXUS_V1_STRUCTURE1A_TARGET_ADMISSION_H

#include "nexus_v1_structure1f_directory_admission.h"

typedef struct {
    uint8_t source_tag;
    uint32_t source_record_offset;
    uint64_t source_record_fnv1a64;
    uint16_t structure1a_index;
} Nexus_V1_Structure1ATargetReference;

/* This receipt proves one raw 0x20/0x21 -> Structure1A row boundary only.
 * It does not interpret any target byte as a model, face, mesh, material,
 * transform, texture, palette, pixel, or draw instruction. */
typedef struct {
    int valid;
    int directory_bound;
    int source_reference_bound;
    int target_record_bound;
    int no_draw_only;
    int face_semantics_permitted;
    int mesh_semantics_permitted;
    int material_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint8_t source_tag;
    uint32_t source_record_offset;
    uint64_t source_record_fnv1a64;
    uint16_t structure1a_index;
    uint32_t directory_offset;
    uint32_t directory_length;
    uint64_t directory_fnv1a64;
    uint32_t target_record_offset;
    uint32_t target_record_length;
    uint64_t target_record_fnv1a64;
} Nexus_V1_Structure1ATargetAdmissionReceipt;

int nexus_v1_structure1a_target_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *structure1f_directory,
    const Nexus_V1_Structure1ATargetReference *reference,
    Nexus_V1_Structure1ATargetAdmissionReceipt *out_receipt);

#endif
