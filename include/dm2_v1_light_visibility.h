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
    int16_t alternate_projection_x;
    int16_t alternate_projection_y;
    uint8_t alternate_projection_valid;
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
int dm2_v1_1c9a_light_visibility_project_teleporter(
    DM2_V1_1c9aLightVisibility *state, int destination_map,
    int destination_x, int destination_y);
int dm2_v1_1c9a_light_visibility_mark(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    unsigned depth);
int dm2_v1_1c9a_light_visibility_or_mask(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    uint8_t mask);
/* A step returns 1 with an admitted source cell, 0 for a blocked edge, or
 * -1 when its tile/record/teleporter branch is not implemented. */
typedef int (*DM2_V1_1c9aLightStep)(
    void *context, int map, int x, int y, int direction,
    int *next_map, int *next_x, int *next_y);
/* Traverses admitted source edges without claiming complete mode-8 coverage. */
int dm2_v1_1c9a_light_mode8_frontier(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, DM2_V1_1c9aLightStep step, void *context);
typedef int (*DM2_V1_1c9aLightTile)(
    void *context, int map, int x, int y, int distance,
    int16_t *ambient_delta, int16_t *darkness_delta);
/* Consume observed cells without claiming complete mode-7 source coverage. */
int dm2_v1_1c9a_light_mode7_observed_cells(
    DM2_V1_1c9aLightVisibility *state,
    DM2_V1_1c9aLightTile tile, void *context, unsigned *out_cells);
/* Only a completed mode-8/mode-7 pair can be consumed by c_light. */
int dm2_v1_1c9a_light_visibility_level_inputs(
    const DM2_V1_1c9aLightVisibility *state, int map,
    int16_t *v1e0974, int16_t *v1e0978, uint32_t *source_state_hash);

#ifdef __cplusplus
}
#endif

#endif
