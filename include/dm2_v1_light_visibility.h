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

/* SK1C9A s_31_ac is the four-byte xp_bc cell written at x*128+y*4. */
typedef struct {
    uint8_t score;             /* vo_e8 reads this byte */
    uint8_t direction;         /* vw_f8; 0xff until source RNG is bound */
    uint16_t packed_position;  /* map:6, y:5, x:5 */
} DM2_V1_1c9aLightWorkNode;

int dm2_v1_1c9a_light_work_node_position(
    DM2_V1_1c9aLightWorkNode *node, int map, int x, int y);
/* Resolve an xp_90 packet through the active map's xp_bc grid. The ring
 * packet has coordinates and map only; its score is read from xp_bc. */
int dm2_v1_1c9a_light_work_node_score(
    const DM2_V1_1c9aLightWorkNode *grids,
    int current_map, int alternate_map, int map, int x, int y,
    uint8_t *out_score);

void dm2_v1_1c9a_light_visibility_reset(
    DM2_V1_1c9aLightVisibility *state, int current_map,
    int current_width, int alternate_map, int alternate_width);
int dm2_v1_1c9a_light_visibility_project_teleporter(
    DM2_V1_1c9aLightVisibility *state, int destination_map,
    int destination_x, int destination_y);
int dm2_v1_1c9a_light_visibility_mark(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    unsigned depth);
/* SK1C9A action 27: write direct coordinates on matching map planes, or
 * projected coordinates when this node has a teleporter to that plane. */
int dm2_v1_1c9a_light_visibility_mark_action27(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    int projection_map, int projection_x, int projection_y,
    unsigned score);
int dm2_v1_1c9a_light_visibility_or_mask(
    DM2_V1_1c9aLightVisibility *state, int map, int x, int y,
    uint8_t mask);
/* A step returns a positive source edge cost for an admitted cell, 0 for a
 * blocked edge, or -1 when its tile/record/teleporter branch is unknown. */
typedef int (*DM2_V1_1c9aLightStep)(
    void *context, int map, int x, int y, int direction,
    int *next_map, int *next_x, int *next_y,
    int *projection_map, int *projection_x, int *projection_y);
/* Traverses admitted source edges without claiming complete mode-8 coverage. */
int dm2_v1_1c9a_light_mode8_frontier(
    DM2_V1_1c9aLightVisibility *state, int start_map, int start_x,
    int start_y, DM2_V1_1c9aLightStep step, void *context);
/* SK1C9A v1d62ec is a separate 16-bit walk RNG, initialized to 1 by
 * dm2data.cpp. Call only at source branches that consume that state. */
uint16_t dm2_v1_1c9a_light_walk_rng_advance(uint16_t state);
typedef struct {
    uint16_t rng;
    uint8_t direction;
    uint8_t direction_delta;
    uint8_t attempts;
} DM2_V1_1c9aLightNodeDecision;
/* SK1C9A's branch after dequeuing a node with score <= vw_d4. */
DM2_V1_1c9aLightNodeDecision dm2_v1_1c9a_light_node_decision(
    uint16_t rng, unsigned node_score, int extended_search);
/* Both CHECK_RECOMPUTE_LIGHT action flags select five attempts; the
 * seven-attempt branch remains available to other FIND_WALK_PATH callers. */
int dm2_v1_1c9a_light_extended_search(uint16_t source_flags);
/* SK1C9A action prepass sets vl_48 only for these action types. */
int dm2_v1_1c9a_light_action_prefetches_start_teleporter(
    unsigned action);
/* Recursive GO_THERE(argw1=6) returns 0 for these source tile branches;
 * -1 means this bounded helper cannot establish admission. */
int dm2_v1_1c9a_light_arg6_destination_admission(uint8_t raw_tile);
uint8_t dm2_v1_1c9a_light_node_next_direction(
    const DM2_V1_1c9aLightNodeDecision *decision,
    uint8_t previous_direction);
uint8_t dm2_v1_1c9a_light_node_attempt_direction(
    const DM2_V1_1c9aLightNodeDecision *decision,
    uint16_t source_flags, unsigned attempt_index);
typedef int (*DM2_V1_1c9aLightTile)(
    void *context, int map, int x, int y, int distance,
    int16_t *ambient_delta, int16_t *darkness_delta);
/* Consume observed cells without claiming complete mode-7 source coverage. */
int dm2_v1_1c9a_light_mode7_observed_cells(
    DM2_V1_1c9aLightVisibility *state, unsigned source_radius,
    DM2_V1_1c9aLightTile tile, void *context, unsigned *out_cells);
/* Only a completed mode-8/mode-7 pair can be consumed by c_light. */
int dm2_v1_1c9a_light_visibility_level_inputs(
    const DM2_V1_1c9aLightVisibility *state, int map,
    int16_t *v1e0974, int16_t *v1e0978, uint32_t *source_state_hash);

#ifdef __cplusplus
}
#endif

#endif
