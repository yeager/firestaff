#include "dm2_v1_light_visibility.h"

#include <stddef.h>
#include <string.h>

_Static_assert(sizeof(DM2_V1_1c9aLightWorkNode) == 4u,
               "SK1C9A xp_bc cells are four bytes");

int dm2_v1_1c9a_light_work_node_position(
    DM2_V1_1c9aLightWorkNode *node, int map, int x, int y)
{
    if (!node || map < 0 || map >= 64 || x < 0 || x >= 32 ||
        y < 0 || y >= 32)
        return 0;
    node->packed_position = (uint16_t)((map << 10) | (y << 5) | x);
    return 1;
}

int dm2_v1_1c9a_light_work_node_score(
    const DM2_V1_1c9aLightWorkNode *grids,
    int current_map, int alternate_map, int map, int x, int y,
    uint8_t *out_score)
{
    int plane;
    size_t index;
    if (!grids || !out_score || x < 0 || x >= 32 || y < 0 || y >= 32)
        return 0;
    plane = map == current_map ? 0 : map == alternate_map ? 1 : -1;
    if (plane < 0) return 0;
    index = (size_t)plane * 1024u + (size_t)x * 32u + (size_t)y;
    if (grids[index].packed_position !=
        (uint16_t)((map << 10) | (y << 5) | x))
        return 0;
    *out_score = grids[index].score;
    return 1;
}

uint16_t dm2_v1_1c9a_light_walk_rng_advance(uint16_t state)
{
    /* SK1C9A.cpp:9521-9540 shifts v1d62ec and XORs its high byte with
     * 0xb4 when the discarded low bit was set. */
    return (uint16_t)((state >> 1) ^ ((state & 1u) ? 0xb400u : 0u));
}

DM2_V1_1c9aLightNodeDecision dm2_v1_1c9a_light_node_decision(
    uint16_t rng, unsigned node_score, int extended_search)
{
    DM2_V1_1c9aLightNodeDecision decision;
    /* SK1C9A.cpp:9514-9547 uses the old low bit for vl_e0, advances
     * v1d62ec once, then takes the new low two bits for vw_f8. */
    decision.direction_delta = (uint8_t)((node_score -
        ((rng & 1u) == 0u ? 1u : 0u)) & 3u);
    decision.rng = dm2_v1_1c9a_light_walk_rng_advance(rng);
    decision.direction = (uint8_t)(decision.rng & 3u);
    decision.attempts = extended_search ? 7u : 5u;
    return decision;
}

int dm2_v1_1c9a_light_extended_search(uint16_t source_flags)
{
    /* SK1C9A.cpp:6642-6662 gates vl_94 by both source masks. */
    return (source_flags & 0x2000u) != 0u &&
           (source_flags & 0x118u) != 0u;
}

int dm2_v1_1c9a_light_action_prefetches_start_teleporter(
    unsigned action)
{
    /* SK1C9A.cpp:6720-6907 sets skip00557 only for 1, 3, 11, 12;
     * vl_48 gates the initial GET_TELEPORTER_DETAIL at :6704. */
    return action == 1u || action == 3u || action == 11u || action == 12u;
}

int dm2_v1_1c9a_light_arg6_destination_admission(uint8_t raw_tile)
{
    unsigned type = raw_tile >> 5;
    /* SK1C9A.cpp:2646-2820: argw1=6 starts vql_40 at zero. For
     * class 1/2 without bit 0x02, skip00481 leaves it zero, so the
     * capability mask check exits before any destination edge is made. */
    if ((type == 1u || type == 2u) && (raw_tile & 0x02u) == 0u)
        return 0;
    return -1;
}

uint8_t dm2_v1_1c9a_light_node_next_direction(
    const DM2_V1_1c9aLightNodeDecision *decision,
    uint8_t previous_direction)
{
    if (!decision) return 0xffu;
    return (uint8_t)((previous_direction + decision->direction_delta) & 3u);
}

uint8_t dm2_v1_1c9a_light_node_attempt_direction(
    const DM2_V1_1c9aLightNodeDecision *decision,
    uint16_t source_flags, unsigned attempt_index)
{
    uint8_t direction;
    if (!decision || (decision->attempts != 5u && decision->attempts != 7u) ||
        attempt_index >= (unsigned)decision->attempts - 1u)
        return 0xffu;
    direction = decision->direction;
    for (unsigned i = 0; i <= attempt_index; ++i)
        direction = dm2_v1_1c9a_light_node_next_direction(
            decision, direction);
    if (decision->attempts == 7u && attempt_index >= 4u) {
        if (attempt_index == 4u)
            return (source_flags & 0x108u) != 0u ? 5u : 0xffu;
        return (source_flags & 0x110u) != 0u ? 4u : 0xffu;
    }
    return direction;
}

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
    return dm2_v1_1c9a_light_visibility_mark_action27(
        state, map, x, y, -1, -1, -1, depth);
}

int dm2_v1_1c9a_light_visibility_mark_action27(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    int projection_map, int projection_x, int projection_y,
    unsigned score)
{
    uint8_t value;
    int wrote = 0;
    if (!state || x < 0 || y < 0 || y >= 32 || score >= 255u)
        return 0;
    value = (uint8_t)(score + 1u);
    /* SK1C9A.cpp action 27 indexes both buffers as (x << 5) + y. */
    if (map == state->current_map && x < state->current_width) {
        state->current[(size_t)x * 32u + (size_t)y] = value;
        wrote = 1;
    }
    else if (projection_map == state->current_map &&
             projection_x >= 0 && projection_x < state->current_width &&
             projection_y >= 0 && projection_y < 32) {
        state->current[(size_t)projection_x * 32u +
                       (size_t)projection_y] = value;
        wrote = 1;
    }
    if (map == state->alternate_map && x < state->alternate_width) {
        state->alternate[(size_t)x * 32u + (size_t)y] = value;
        wrote = 1;
    }
    else if (projection_map == state->alternate_map &&
             projection_x >= 0 && projection_x < state->alternate_width &&
             projection_y >= 0 && projection_y < 32) {
        state->alternate[(size_t)projection_x * 32u +
                         (size_t)projection_y] = value;
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

/* FIND_WALK_PATH owns the ring, score grid and RNG for each action. Keep
 * these cursors local to one invocation; the caller supplies the cursor so
 * consecutive light actions can share v1d62ec in source order. */
static int light_walk_core(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, DM2_V1_1c9aLightStep step, void *context,
    uint16_t *walk_rng, uint16_t source_flags, unsigned action,
    unsigned max_score, int source_facing,
    DM2_V1_1c9aLightNodeAction on_node)
{
    typedef struct { uint8_t x, y, map, reserved; } Cell;
    /* SK1C9A xp_90 stores x, y, map in four-byte work entries. Scores live
     * in the grid and vba_08 buckets, rather than in the work entry. */
    Cell queue[256u];
    uint8_t score_bucket[0x33u];
    _Static_assert(sizeof(queue) == 0x400u,
                   "SK1C9A work ring must be 0x400 bytes");
    DM2_V1_1c9aLightWorkNode work_grid[2u * 32u * 32u];
    uint8_t seen[2u * 32u * 32u];
    uint8_t head = 0u, tail = 0u;
    unsigned pending = 0u;
    unsigned lowest = 0u;
    int selector;
    if (!state || !walk_rng || max_score == 0u || max_score > 50u ||
        source_facing < 0 || source_facing > 3 ||
        (action != 0x1bu && action != 0x17u)) return 0;
    memset(work_grid, 0, sizeof(work_grid));
    memset(seen, 0, sizeof(seen));
    memset(score_bucket, 0, sizeof(score_bucket));
    if (action == 0x1bu) {
        memset(state->current, 0, sizeof(state->current));
        memset(state->alternate, 0, sizeof(state->alternate));
        state->mode8_complete = 0u;
    } else {
        state->v1e0974 = 0;
        state->v1e0978 = 0;
    }
    state->mode7_complete = 0u;
    state->source_state_hash = 0u;
    if (!step || start_x < 0 || start_y < 0 || start_y >= 32 ||
        start_map != state->current_map ||
        start_x >= state->current_width)
        return 0;
    if (start_map >= 64) return 0;
    {
        size_t start_index = (size_t)start_x * 32u + (size_t)start_y;
        seen[start_index] = 1u;
        work_grid[start_index].score = 1u;
        work_grid[start_index].direction = 0xffu;
        if (!dm2_v1_1c9a_light_work_node_position(
                &work_grid[start_index], start_map, start_x, start_y))
            goto incomplete;
    }
    /* The initial action sees vo_e8=0, then the source's default tile
     * cost 1 places the start cell into xp_90 for its first expansion. */
    if (action == 0x1bu) {
        if (!dm2_v1_1c9a_light_visibility_mark_action27(
                state, start_map, start_x, start_y, -1, -1, -1, 0u))
            goto incomplete;
    /* dm2data.cpp table1d62ee[23] = 0x4f. With the one action-23 button
     * used by CHECK_RECOMPUTE_LIGHT, SK1C9A ORs that value into vb_140
     * before both the prepass and the edge-loop cache refresh. */
    } else if (!on_node || on_node(context, start_map, start_x,
                                   start_y, -1, source_facing,
                                   0u, 3u, 0x4fu) < 0) {
        goto incomplete;
    }
    queue[tail] = (Cell){(uint8_t)start_x, (uint8_t)start_y,
                         (uint8_t)start_map, 0u};
    ++tail;
    score_bucket[1u] = 1u;
    pending = 1u;
    while (pending != 0u) {
        unsigned rotations = 0u;
        Cell cell;
        uint8_t score;
        DM2_V1_1c9aLightNodeDecision decision;
        int cell_selector;
        size_t cell_index;
        /* SK1C9A rotates higher-score xp_90 packets to the write cursor
         * while vba_08 still has packets at the current score. The cursors
         * wrap as bytes; no array compaction takes place. */
        {
            while (lowest < sizeof(score_bucket) && !score_bucket[lowest])
                ++lowest;
            if (lowest == sizeof(score_bucket)) goto incomplete;
            for (;;) {
                cell = queue[head];
                if (!dm2_v1_1c9a_light_work_node_score(
                        work_grid, state->current_map, state->alternate_map,
                        cell.map, cell.x, cell.y, &score))
                    goto incomplete;
                if (score <= lowest) break;
                if (++rotations >= pending) goto incomplete;
                queue[tail] = queue[head];
                ++head;
                ++tail;
            }
            if (!score_bucket[lowest]) goto incomplete;
            --score_bucket[lowest];
            ++head;
            --pending;
        }
        cell_selector = cell.map == state->current_map ? 0 : 1;
        cell_index = (size_t)cell_selector * 1024u +
                     (size_t)cell.x * 32u + (size_t)cell.y;
        if (!seen[cell_index]) goto incomplete;
        /* vo_e8 is the consumed xp_bc score. Action 27 writes vo_e8+1
         * at each admitted target before its edge cost enters xp_90. */
        decision = dm2_v1_1c9a_light_node_decision(
            *walk_rng, score,
            dm2_v1_1c9a_light_extended_search(source_flags));
        *walk_rng = decision.rng;
        for (unsigned attempt = 0u; attempt < decision.attempts - 1u;
             ++attempt) {
            int direction = dm2_v1_1c9a_light_node_attempt_direction(
                &decision, source_flags, attempt);
            int next_map = -1, next_x = -1, next_y = -1;
            int projection_map = -1, projection_x = -1, projection_y = -1;
            int result = step(context, cell.map, cell.x, cell.y, direction,
                              &next_map, &next_x, &next_y,
                              &projection_map, &projection_x,
                              &projection_y);
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
            if (action == 0x1bu) {
                if (!dm2_v1_1c9a_light_visibility_mark_action27(
                        state, next_map, next_x, next_y,
                        projection_map, projection_x, projection_y, score))
                    goto incomplete;
            } else if (on_node(context, next_map, next_x, next_y,
                               direction, source_facing,
                               score, 4u, 0x4fu) < 0) {
                goto incomplete;
            }
            if ((unsigned)score + (unsigned)result > max_score ||
                (seen[index] &&
                 (unsigned)score + (unsigned)result >=
                     work_grid[index].score))
                continue;
            if (pending == 256u ||
                score_bucket[(unsigned)score + (unsigned)result] == 255u)
                goto incomplete;
            seen[index] = 1u;
            work_grid[index].score = (uint8_t)(score + result);
            work_grid[index].direction = 0xffu;
            if (!dm2_v1_1c9a_light_work_node_position(
                    &work_grid[index], next_map, next_x, next_y))
                goto incomplete;
            queue[tail] = (Cell){(uint8_t)next_x, (uint8_t)next_y,
                                 (uint8_t)next_map, 0u};
            ++score_bucket[(unsigned)score + (unsigned)result];
            ++tail;
            ++pending;
        }
    }
    return 1;
incomplete:
    /* Retain observed cells for an incomplete mode-7 probe; completion
     * flags and source hash stay clear. */
    return 0;
}

int dm2_v1_1c9a_light_mode8_frontier_with_rng(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, DM2_V1_1c9aLightStep step, void *context,
    uint16_t *walk_rng)
{
    /* SK1C9A action 27 installs v1e0576=0x36e7 in its prepass. */
    return light_walk_core(state, start_map, start_x, start_y, step,
                           context, walk_rng, 0x36e7u, 0x1bu, 25u, 0,
                           NULL);
}

int dm2_v1_1c9a_light_mode7_frontier_with_rng(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, int source_facing, unsigned source_radius,
    DM2_V1_1c9aLightStep step,
    DM2_V1_1c9aLightNodeAction on_node, void *context,
    uint16_t *walk_rng)
{
    /* Action 23 installs v1e0576=0x227; its two call sites have distinct
     * source flags (3 at start, 4 on an admitted edge). */
    if (source_radius == 0u || !on_node) return 0;
    if (source_radius > 8u) source_radius = 8u;
    return light_walk_core(state, start_map, start_x, start_y, step,
                           context, walk_rng, 0x227u, 0x17u,
                           source_radius, source_facing, on_node);
}

int dm2_v1_1c9a_light_mode8_frontier(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, DM2_V1_1c9aLightStep step, void *context)
{
    uint16_t uncommitted_rng = 1u;
    return dm2_v1_1c9a_light_mode8_frontier_with_rng(
        state, start_map, start_x, start_y, step, context,
        &uncommitted_rng);
}

int dm2_v1_1c9a_light_mode7_observed_cells(
    DM2_V1_1c9aLightVisibility *state,
    unsigned source_radius, DM2_V1_1c9aLightTile tile, void *context,
    unsigned *out_cells)
{
    unsigned cells = 0u;
    if (out_cells) *out_cells = 0u;
    if (!state || !tile || state->current_map < 0) return 0;
    state->v1e0974 = 0;
    state->v1e0978 = 0;
    state->mode7_complete = 0u;
    state->source_state_hash = 0u;
    if (source_radius == 0u) return 1;
    if (source_radius > 8u) source_radius = 8u;
    for (int x = 0; x < state->current_width; ++x) {
        for (int y = 0; y < 32; ++y) {
            uint8_t visit = state->current[(size_t)x * 32u + (size_t)y];
            int16_t ambient = 0, darkness = 0;
            if (!visit) continue;
            /* sklight.cpp clamps the mode-7 action-0x17 radius to eight. */
            if (visit > source_radius + 1u) continue;
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
