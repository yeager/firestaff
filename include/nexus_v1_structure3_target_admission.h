#ifndef NEXUS_V1_STRUCTURE3_TARGET_ADMISSION_H
#define NEXUS_V1_STRUCTURE3_TARGET_ADMISSION_H

#include "nexus_v1_structure1a_field_admission.h"

typedef struct {
    int valid, field_bound, directory_bound, target_span_bound, no_draw_only;
    int face_semantics_permitted, mesh_semantics_permitted, texture_semantics_permitted, draw_permitted;
    uint32_t level_index; uint64_t package_fnv1a64;
    uint8_t structure3_model_index;
    uint32_t payload_offset, payload_length, directory_length;
    uint64_t payload_fnv1a64, directory_fnv1a64;
    uint32_t target_offset, target_length;
    uint64_t target_fnv1a64;
} Nexus_V1_Structure3TargetAdmissionReceipt;

int nexus_v1_structure3_target_admit(const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1AFieldAdmissionReceipt *field,
    uint32_t payload_offset, uint32_t payload_length,
    uint64_t expected_directory_fnv1a64,
    Nexus_V1_Structure3TargetAdmissionReceipt *out_receipt);
#endif
