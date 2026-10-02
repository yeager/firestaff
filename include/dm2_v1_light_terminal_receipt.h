#ifndef FIRESTAFF_DM2_V1_LIGHT_TERMINAL_RECEIPT_H
#define FIRESTAFF_DM2_V1_LIGHT_TERMINAL_RECEIPT_H

#include "dm2_v1_light_visibility.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The walk core, not a caller's path-search return value, must produce this
 * after its queue is exhausted and every source branch has been classified.
 * FIND_WALK_PATH may return -1 after a complete no-path traversal. */
typedef struct {
    uint8_t valid;
    uint8_t action;
    uint8_t mode;
    uint8_t terminal_exhausted;
    uint8_t callbacks_authenticated;
    uint8_t unknown_branch_seen;
    uint8_t fallthrough_seen;
    uint8_t skipped_zero_radius;
    int16_t map;
    uint8_t x;
    uint8_t y;
    uint8_t radius;
    uint16_t rng_before;
    uint16_t rng_after;
    uint32_t ordered_node_hash;
    uint32_t source_edge_hash;
    uint32_t radius_source_receipt_hash;
    uint32_t result_hash;
} DM2_V1_LightWalkTerminalProof;

/* An observer must bind each callback outcome to the live tile/record/GDAT
 * evidence it used. A hash of coordinates or an edge cost is insufficient. */
typedef uint32_t (*DM2_V1_LightEdgeSourceReceipt)(
    void *context, int map, int x, int y, int direction, unsigned score,
    int cost, int next_map, int next_x, int next_y,
    int projection_map, int projection_x, int projection_y);
typedef uint32_t (*DM2_V1_LightNodeSourceReceipt)(
    void *context, int map, int x, int y, int direction,
    unsigned score, unsigned source_flags, uint8_t effective_flags,
    int16_t ambient_before, int16_t darkness_before,
    int16_t ambient_after, int16_t darkness_after);
typedef struct {
    DM2_V1_LightEdgeSourceReceipt edge;
    DM2_V1_LightNodeSourceReceipt node;
    void *context;
} DM2_V1_LightWalkSourceReceipts;

int dm2_v1_1c9a_light_mode8_frontier_with_proof(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, DM2_V1_1c9aLightStep step, void *context,
    uint16_t *walk_rng, const DM2_V1_LightWalkSourceReceipts *receipts,
    DM2_V1_LightWalkTerminalProof *proof);
int dm2_v1_1c9a_light_mode7_frontier_with_proof(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, int source_facing, unsigned source_radius,
    DM2_V1_1c9aLightStep step, DM2_V1_1c9aLightNodeAction on_node,
    void *context, uint16_t *walk_rng,
    const DM2_V1_LightWalkSourceReceipts *receipts,
    DM2_V1_LightWalkTerminalProof *proof);
/* sklight.cpp skips action 23 when the authenticated GDAT radius is zero. */
int dm2_v1_1c9a_light_mode7_zero_radius_proof(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, uint16_t walk_rng, uint32_t radius_source_receipt_hash,
    DM2_V1_LightWalkTerminalProof *proof);

/* Live source identities required by DM2_RECALC_LIGHT_LEVEL and both walks.
 * Zero is a missing authentication receipt, except for numeric values such
 * as savegame_light that may legitimately be zero. */
typedef struct {
    int16_t map;
    uint8_t x;
    uint8_t y;
    uint8_t radius;
    uint16_t savegame_light;
    uint16_t rng_final;
    uint32_t map_descriptor_hash;
    uint32_t dungeon_hash;
    uint32_t record_pool_hash;
    uint32_t gdat_hash;
    uint32_t party_inventory_hash;
    uint32_t weather_hash;
} DM2_V1_LightTerminalSourceInputs;

uint32_t dm2_v1_light_terminal_visibility_hash(
    const DM2_V1_1c9aLightVisibility *state);
uint32_t dm2_v1_light_terminal_accumulator_hash(
    int16_t v1e0974, int16_t v1e0978);

/* Fail closed: clears all completion fields first and commits a provenance
 * hash only for two matching, authenticated terminal source passes. The
 * producer of each proof remains responsible for classifying every branch. */
int dm2_v1_light_terminal_receipt_commit(
    DM2_V1_1c9aLightVisibility *state,
    const DM2_V1_LightWalkTerminalProof *mode8,
    const DM2_V1_LightWalkTerminalProof *mode7,
    const DM2_V1_LightTerminalSourceInputs *inputs);

#ifdef __cplusplus
}
#endif

#endif
