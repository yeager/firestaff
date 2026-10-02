#include "dm2_v1_light_visibility.h"

#include <stddef.h>
#include <string.h>

void dm2_v1_1c9a_light_visibility_reset(
    DM2_V1_1c9aLightVisibility *state, int current_map,
    int current_width, int alternate_map, int alternate_width)
{
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->current_map = current_map >= 0 && current_width > 0 &&
        current_width <= 32 ? (int16_t)current_map : -1;
    state->alternate_map = alternate_map >= 0 && alternate_width > 0 &&
        alternate_width <= 32 ? (int16_t)alternate_map : -1;
    state->current_width = state->current_map >= 0 ?
        (uint8_t)current_width : 0u;
    state->alternate_width = state->alternate_map >= 0 ?
        (uint8_t)alternate_width : 0u;
}

int dm2_v1_1c9a_light_visibility_mark(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    unsigned depth)
{
    uint8_t value;
    int wrote = 0;
    if (!state || x < 0 || y < 0 || y >= 32 || depth >= 255u)
        return 0;
    value = (uint8_t)(depth + 1u);
    /* SK1C9A.cpp action 27 indexes both buffers as (x << 5) + y. */
    if (map == state->current_map && x < state->current_width) {
        state->current[(size_t)x * 32u + (size_t)y] = value;
        wrote = 1;
    }
    if (map == state->alternate_map && x < state->alternate_width) {
        state->alternate[(size_t)x * 32u + (size_t)y] = value;
        wrote = 1;
    }
    return wrote;
}

int dm2_v1_1c9a_light_visibility_or_mask(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    uint8_t mask)
{
    int wrote = 0;
    if (!state || x < 0 || y < 0 || y >= 32 || !mask)
        return 0;
    /* SK1C9A.cpp action 0x1b's later light-mode branch ORs a direction
     * mask into the same cells written by the depth branch. */
    if (map == state->current_map && x < state->current_width) {
        state->current[(size_t)x * 32u + (size_t)y] |= mask;
        wrote = 1;
    }
    if (map == state->alternate_map && x < state->alternate_width) {
        state->alternate[(size_t)x * 32u + (size_t)y] |= mask;
        wrote = 1;
    }
    return wrote;
}

int dm2_v1_1c9a_light_visibility_level_inputs(
    const DM2_V1_1c9aLightVisibility *state, int map,
    int16_t *v1e0974, int16_t *v1e0978, uint32_t *source_state_hash)
{
    if (!state || !v1e0974 || !v1e0978 || !source_state_hash ||
        map != state->current_map || !state->mode8_complete ||
        !state->mode7_complete || !state->source_state_hash)
        return 0;
    *v1e0974 = state->v1e0974;
    *v1e0978 = state->v1e0978;
    *source_state_hash = state->source_state_hash;
    return 1;
}
