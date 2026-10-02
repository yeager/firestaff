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

int dm2_v1_1c9a_light_visibility_project_teleporter(
    DM2_V1_1c9aLightVisibility *state, int destination_map,
    int destination_x, int destination_y)
{
    if (!state || destination_map != state->alternate_map ||
        destination_x < 0 || destination_x >= state->alternate_width ||
        destination_y < 0 || destination_y >= 32)
        return 0;
    state->alternate_projection_x = (int16_t)destination_x;
    state->alternate_projection_y = (int16_t)destination_y;
    state->alternate_projection_valid = 1u;
    return 1;
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
    else if (map == state->alternate_map &&
             state->alternate_projection_valid) {
        int projected_x = state->alternate_projection_valid ?
            state->alternate_projection_x : x;
        int projected_y = state->alternate_projection_valid ?
            state->alternate_projection_y : y;
        state->alternate[(size_t)projected_x * 32u +
                         (size_t)projected_y] = value;
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
    else if (map == state->alternate_map &&
             state->alternate_projection_valid) {
        int projected_x = state->alternate_projection_valid ?
            state->alternate_projection_x : x;
        int projected_y = state->alternate_projection_valid ?
            state->alternate_projection_y : y;
        state->alternate[(size_t)projected_x * 32u +
                         (size_t)projected_y] |= mask;
        wrote = 1;
    }
    return wrote;
}

int dm2_v1_1c9a_light_mode8_frontier(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, DM2_V1_1c9aLightStep step, void *context)
{
    typedef struct { uint8_t map, x, y, depth; } Cell;
    /* SK1C9A allocates xp_90 as 0x400 bytes and stores four-byte work
     * entries through byte-sized producer/consumer cursors. Keep this
     * partial traversal to the same 256 pending-node slots. */
    Cell queue[256u];
    _Static_assert(sizeof(queue) == 0x400u,
                   "SK1C9A work ring must be 0x400 bytes");
    uint8_t seen[2u * 32u * 32u] = {0};
    uint8_t head = 0u, tail = 0u;
    unsigned pending = 0u;
    int selector;
    if (!state) return 0;
    memset(state->current, 0, sizeof(state->current));
    memset(state->alternate, 0, sizeof(state->alternate));
    state->mode8_complete = 0u;
    state->mode7_complete = 0u;
    state->source_state_hash = 0u;
    if (!step || start_x < 0 || start_y < 0 || start_y >= 32 ||
        start_map != state->current_map ||
        start_x >= state->current_width)
        return 0;
    if (start_map > 255) return 0;
    queue[tail++] = (Cell){(uint8_t)start_map, (uint8_t)start_x,
                           (uint8_t)start_y, 0u};
    pending = 1u;
    seen[(size_t)start_x * 32u + (size_t)start_y] = 1u;
    while (pending != 0u) {
        Cell cell = queue[head++];
        --pending;
        if (!dm2_v1_1c9a_light_visibility_mark(
                state, cell.map, cell.x, cell.y, cell.depth))
            goto incomplete;
        /* CHECK_RECOMPUTE_LIGHT supplies action 0x1b with byte 0x19, a
         * maximum source depth of 25. Only an admitted source edge enters
         * this queue; unknown record/teleporter cases invalidate the pass. */
        if (cell.depth == 25u) continue;
        for (int direction = 0; direction < 4; ++direction) {
            int next_map = -1, next_x = -1, next_y = -1;
            int result = step(context, cell.map, cell.x, cell.y, direction,
                              &next_map, &next_x, &next_y);
            size_t index;
            if (result < 0) goto incomplete;
            if (result == 0) continue;
            selector = next_map == state->current_map ? 0 :
                       next_map == state->alternate_map ? 1 : -1;
            if (selector < 0 || next_map > 255 || next_x < 0 ||
                next_y < 0 || next_y >= 32 ||
                next_x >= (selector == 0 ? state->current_width :
                                          state->alternate_width))
                goto incomplete;
            index = (size_t)selector * 1024u + (size_t)next_x * 32u +
                    (size_t)next_y;
            if (seen[index]) continue;
            if (pending == 256u) goto incomplete;
            seen[index] = 1u;
            queue[tail++] = (Cell){(uint8_t)next_map, (uint8_t)next_x,
                                   (uint8_t)next_y,
                                   (uint8_t)(cell.depth + 1u)};
            ++pending;
        }
    }
    return 1;
incomplete:
    /* Retain observed cells for an incomplete mode-7 probe; completion
     * flags and source hash stay clear. */
    return 0;
}

int dm2_v1_1c9a_light_mode7_observed_cells(
    DM2_V1_1c9aLightVisibility *state,
    DM2_V1_1c9aLightTile tile, void *context, unsigned *out_cells)
{
    unsigned cells = 0u;
    if (out_cells) *out_cells = 0u;
    if (!state || !tile || state->current_map < 0) return 0;
    state->v1e0974 = 0;
    state->v1e0978 = 0;
    state->mode7_complete = 0u;
    state->source_state_hash = 0u;
    for (int x = 0; x < state->current_width; ++x) {
        for (int y = 0; y < 32; ++y) {
            uint8_t visit = state->current[(size_t)x * 32u + (size_t)y];
            int16_t ambient = 0, darkness = 0;
            if (!visit) continue;
            if (visit > 9u) goto incomplete;
            if (tile(context, state->current_map, x, y, visit - 1u,
                     &ambient, &darkness) < 0)
                goto incomplete;
            state->v1e0974 = (int16_t)(state->v1e0974 + ambient);
            state->v1e0978 = (int16_t)(state->v1e0978 + darkness);
            ++cells;
        }
    }
    if (out_cells) *out_cells = cells;
    return 1;
incomplete:
    if (out_cells) *out_cells = cells;
    return 0;
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
