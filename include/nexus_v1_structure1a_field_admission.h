#ifndef NEXUS_V1_STRUCTURE1A_FIELD_ADMISSION_H
#define NEXUS_V1_STRUCTURE1A_FIELD_ADMISSION_H

#include "nexus_v1_structure1a_target_admission.h"

typedef struct {
    int valid;
    int target_record_bound;
    int raw_kind_bound;
    int structure3_model_reference_bound;
    int raw_rotation_selector_bound;
    int no_draw_only;
    int face_semantics_permitted;
    int mesh_semantics_permitted;
    int material_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t target_record_offset;
    uint64_t target_record_fnv1a64;
    uint8_t raw_kind;
    uint8_t structure3_model_index;
    uint8_t raw_rotation_selector;
    uint8_t raw_tail[21];
} Nexus_V1_Structure1AFieldAdmissionReceipt;

int nexus_v1_structure1a_field_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1ATargetAdmissionReceipt *target,
    Nexus_V1_Structure1AFieldAdmissionReceipt *out_receipt);

#endif
