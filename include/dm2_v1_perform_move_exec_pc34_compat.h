#ifndef FIRESTAFF_DM2_V1_PERFORM_MOVE_EXEC_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_PERFORM_MOVE_EXEC_PC34_COMPAT_H

/*
 * dm2_v1_perform_move_exec_pc34_compat.h — DM2 movement execution layer.
 *
 * Bridges the plan receipt (dm2_v1_DM2_PERFORM_MOVE_plan) with the
 * execution sub-functions from skproject DM2_PERFORM_MOVE:
 *
 *   - Walk delay calculation (DM2_CALC_PLAYER_WALK_DELAY per hero)
 *   - Stamina drain on accepted moves (DM2_ADJUST_STAMINA)
 *   - Move classification (DM2_12b4_0881: 6-way switch)
 *   - Creature displacement (DM2_move_12b4_0d75, IS_CREATURE_MOVABLE_THERE)
 *   - Door attack on blocked door (DM2_ATTACK_DOOR)
 *   - Teleporter resolution (DM2_GET_TELEPORTER_DETAIL + DM2_map_3BF83)
 *   - Stair level change (DM2_move_12b4_00af)
 *   - Record relocation (DM2_MOVE_RECORD_TO)
 *   - Squad direction reset (DM2_RESET_SQUAD_DIR)
 *
 * Source: skproject/SKWINSPX/src/v5/skmove.cpp:197-612
 */

#include <stdint.h>

#include "dm2_v1_perform_move.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_timeline.h"
#include "dm2_v1_skproject_core.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Direction vector tables (N/E/S/W).
 * Source: skmove.cpp table1d3ff8/table1d3ffc */
#define DM2_DIR_DX_N  0
#define DM2_DIR_DX_E  1
#define DM2_DIR_DX_S  0
#define DM2_DIR_DX_W (-1)
#define DM2_DIR_DY_N (-1)
#define DM2_DIR_DY_E  0
#define DM2_DIR_DY_S  1
#define DM2_DIR_DY_W  0

/* Move classification results from DM2_12b4_0881.
 * Source: skmove.cpp:408-501 switch cases */
typedef enum {
    DM2_MOVE_CLASS_OPEN_TILE    = 0,
    DM2_MOVE_CLASS_STAIR_UP     = 1,
    DM2_MOVE_CLASS_DOOR_ATTACK  = 2,
    DM2_MOVE_CLASS_CREATURE     = 3,
    DM2_MOVE_CLASS_BLOCKED      = 4,
    DM2_MOVE_CLASS_TELEPORTER   = 5,
    DM2_MOVE_CLASS_STAIR_DOWN   = 6
} DM2_V1_MoveClassification;

typedef struct {
    int valid;
    int fail_closed;

    int walk_delay;
    int stamina_drained;
    int half_step_entered;

    DM2_V1_MoveClassification classification;
    int creature_handle;
    int creature_pushable;
    int creature_attacked;

    int door_attacked;
    int door_attack_power;

    int teleporter_resolved;
    int teleporter_dest_x;
    int teleporter_dest_y;
    int teleporter_dest_map;

    int stair_level_changed;
    int stair_dest_level;

    int record_moved;
    int squad_dir_reset;
    int position_updated;

    int16_t final_x;
    int16_t final_y;
    int16_t final_dir;
    int16_t final_level;
} DM2_V1_PerformMoveExecReceipt;

/* Hero state snapshot for walk delay / stamina calculation.
 * Caller resolves these from the live hero structs so the
 * execution layer does not depend on the unproven c_hero layout. */
typedef struct {
    int alive;
    uint16_t max_load;
    uint16_t player_weight;
    uint8_t bodyflag;
    int8_t walkspeed;
    uint8_t savegames1_b_04;
} DM2_V1_HeroMoveState;

typedef struct {
    DM2_V1_PerformMoveReceipt plan;
    int hero_count;
    DM2_V1_HeroMoveState heroes[4];
    int is_creature_displacement;
    int outdoor;
    uint32_t game_tick;
    const DM2_V1_SkprojectMapDescriptor *map_descriptors;
    uint16_t map_descriptor_count;

    /* Door attack context (skmove.cpp:428-474).
     * Caller resolves these from the door record on the target tile. */
    uint8_t door_tile_type;
    uint8_t door_record_byte2;
    uint8_t door_record_byte3;
    uint16_t door_required_power;
    int door_use_byte2_gate;
    int door_rebirth_altar;
    uint16_t door_timer_delay;
    uint16_t party_attack_power;

    /* Creature encounter context (skmove.cpp:477-543).
     * Caller resolves these from the creature on the target tile. */
    uint16_t target_creature_handle;
    uint16_t target_creature_weight;
    uint16_t creature_force_threshold;
    uint16_t creature_random_value;
} DM2_V1_PerformMoveExecRequest;

/* Execute a planned move.
 *
 * Takes the plan receipt from dm2_v1_DM2_PERFORM_MOVE_plan and
 * performs the execution sub-functions: walk delay, stamina drain,
 * creature handling, teleporter/stair transitions, and record
 * relocation.
 *
 * Sub-functions that require unproven data (creature AI queries,
 * record pool manipulation, GDAT lookups) remain fail-closed and
 * are documented in the receipt.
 *
 * Returns 1 if the move was accepted and position updated,
 * 0 if blocked or fail-closed.
 *
 * Source: skmove.cpp:197-612 DM2_PERFORM_MOVE */
int dm2_v1_perform_move_exec(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_SourceTimerQueue *queue,
    const DM2_V1_PerformMoveExecRequest *request,
    DM2_V1_PerformMoveExecReceipt *receipt);

/* Calculate the maximum walk delay across all living heroes.
 * Returns the delay value (minimum 1).
 *
 * Source: skmove.cpp:245-263 hero walk delay loop */
int dm2_v1_calc_party_walk_delay(
    const DM2_V1_HeroMoveState *heroes,
    int hero_count);

/* Drain stamina from all living heroes proportional to
 * encumbrance.  Returns the number of heroes drained.
 *
 * Source: skmove.cpp:376-398 stamina drain loop */
int dm2_v1_drain_party_stamina(
    const DM2_V1_HeroMoveState *heroes,
    int hero_count);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_PERFORM_MOVE_EXEC_PC34_COMPAT_H */
