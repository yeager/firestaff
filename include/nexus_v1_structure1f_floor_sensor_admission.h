#ifndef NEXUS_V1_STRUCTURE1F_FLOOR_SENSOR_ADMISSION_H
#define NEXUS_V1_STRUCTURE1F_FLOOR_SENSOR_ADMISSION_H

#include "nexus_v1_structure1f_directory_admission.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The Saturn/DMWeb Structure1F reference establishes family 2 as 0x12-tagged
 * sixteen-byte floor-sensor rows with direct 64x64 coordinates at bytes 1 and
 * 2. Bytes 3..15 remain source-ordered opaque data: no sensor, control,
 * destination, model, face, mesh, material, or draw relation is admitted. */
typedef struct {
    int valid;
    int directory_bound;
    int floor_sensor_record_bound;
    int coordinate_pair_bound;
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
    uint8_t x;
    uint8_t y;
    uint16_t cell_ordinal;
    uint8_t raw_payload[13];
} Nexus_V1_Structure1FFloorSensorAdmissionReceipt;

int nexus_v1_structure1f_floor_sensor_admit(
    const Nexus_V1_LevCorpusDirectLevelIdentity *identity,
    const uint8_t *dgn_data, int dgn_size,
    const Nexus_V1_Structure1FDirectoryAdmissionReceipt *directory,
    uint32_t record_index,
    Nexus_V1_Structure1FFloorSensorAdmissionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
