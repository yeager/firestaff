#ifndef FIRESTAFF_DM2_V1_LIGHT_VISIBILITY_H
#define FIRESTAFF_DM2_V1_LIGHT_VISIBILITY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* SKProject sklight.cpp::CHECK_RECOMPUTE_LIGHT owns two 32-stride visibility
 * buffers. SK1C9A.cpp action 27 writes depth + 1. These buffers alone never
 * authenticate a dynamic-light frame. */
typedef struct {
    int16_t current_map;
    int16_t alternate_map;
    uint8_t current_width;
    uint8_t alternate_width;
    uint8_t current[32u * 32u];
    uint8_t alternate[32u * 32u];
    int16_t v1e0974;
    int16_t v1e0978;
    uint8_t mode8_complete;
    uint8_t mode7_complete;
    uint32_t source_state_hash;
} DM2_V1_1c9aLightVisibility;

void dm2_v1_1c9a_light_visibility_reset(
    DM2_V1_1c9aLightVisibility *state, int current_map,
    int current_width, int alternate_map, int alternate_width);
int dm2_v1_1c9a_light_visibility_mark(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    unsigned depth);
int dm2_v1_1c9a_light_visibility_or_mask(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    uint8_t mask);
/* Only a completed mode-8/mode-7 pair can be consumed by c_light. */
int dm2_v1_1c9a_light_visibility_level_inputs(
    const DM2_V1_1c9aLightVisibility *state, int map,
    int16_t *v1e0974, int16_t *v1e0978, uint32_t *source_state_hash);

#ifdef __cplusplus
}
#endif

#endif
