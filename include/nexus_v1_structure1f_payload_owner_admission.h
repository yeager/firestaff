#ifndef NEXUS_V1_STRUCTURE1F_PAYLOAD_OWNER_ADMISSION_H
#define NEXUS_V1_STRUCTURE1F_PAYLOAD_OWNER_ADMISSION_H

#include "nexus_v1_structure1f_directory_admission.h"

/* Retains the source owner of the opaque payload tail for direct-coordinate
 * Structure1F families. It does not relate that tail to Structure3 or admit
 * object, sensor, placement, geometry, material, texture, or draw meaning. */
typedef struct {
    int valid;
    int directory_bound;
    int family_record_bound;
    int payload_owner_bound;
    int no_draw_only;
    int structure3_relation_permitted;
    int object_semantics_permitted;
    int sensor_semantics_permitted;
    int placement_semantics_permitted;
    int geometry_semantics_permitted;
    int material_semantics_permitted;
    int texture_semantics_permitted;
    int draw_permitted;
    uint32_t level_index;
    uint64_t package_fnv1a64;
    uint32_t family_index;
    uint8_t source_tag;
    uint32_t record_index;
    uint32_t record_offset;
    uint64_t record_fnv1a64;
    uint32_t payload_offset;
    uint32_t payload_length;
    uint64_t payload_fnv1a64;
    uint8_t raw_payload[13];
} Nexus_V1_Structure1FPayloadOwnerAdmissionReceipt;

int nexus_v1_structure1f_payload_owner_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *directory,
    uint32_t family_index, uint32_t record_index,
    Nexus_V1_Structure1FPayloadOwnerAdmissionReceipt *out_receipt);

#endif
