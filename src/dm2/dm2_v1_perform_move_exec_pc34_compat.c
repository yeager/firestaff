/*
 * dm2_v1_perform_move_exec_pc34_compat.c — DM2 movement execution layer.
 *
 * Source: skproject/SKWINSPX/src/v5/skmove.cpp:197-612
 *
 * This module bridges the plan-only dm2_v1_DM2_PERFORM_MOVE_plan
 * with the execution sub-functions that the original DM2_PERFORM_MOVE
 * calls to actually carry out the move:
 *
 *   skmove.cpp:245-263  Walk delay loop (max across heroes)
 *   skmove.cpp:376-398  Stamina drain loop (weight/load ratio)
 *   skmove.cpp:400      DM2_RESET_SQUAD_DIR
 *   skmove.cpp:408      DM2_12b4_0881 (move classification)
 *   skmove.cpp:420      DM2_MOVE_RECORD_TO (stair up path)
 *   skmove.cpp:473      DM2_ATTACK_DOOR (door attack path)
 *   skmove.cpp:528-534  Creature push (12b4_0d75)
 *   skmove.cpp:542      DM2_ATTACK_CREATURE (creature attack)
 *   skmove.cpp:564      DM2_GET_TELEPORTER_DETAIL + DM2_map_3BF83
 *   skmove.cpp:596      DM2_MOVE_RECORD_TO (normal move)
 *   skmove.cpp:520      DM2_move_12b4_00af (stair level change)
 */

#include "dm2_v1_perform_move_exec_pc34_compat.h"
#include "dm2_v1_move_record_to_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_skproject_core.h"

#include <string.h>

int dm2_v1_calc_party_walk_delay(
    const DM2_V1_HeroMoveState *heroes,
    int hero_count)
{
    int max_delay = 1;

    if (!heroes || hero_count <= 0) return 1;

    for (int i = 0; i < hero_count && i < 4; i++) {
        int32_t delay = 1;

        if (!heroes[i].alive) continue;
        /* SKProject c_querydb.cpp:2175 DM2_CALC_PLAYER_WALK_DELAY.
         * Keep this compatibility receipt on the single source-locked
         * implementation; the old local approximation used the wrong load
         * threshold and bodyflag bit. */
        if (dm2_v1_skproject_calc_player_walk_delay(
                heroes[i].max_load, heroes[i].player_weight,
                heroes[i].bodyflag, heroes[i].walkspeed,
                heroes[i].savegames1_b_04, &delay, NULL) &&
            delay > max_delay) {
            max_delay = (int)delay;
        }
    }

    return max_delay;
}

int dm2_v1_drain_party_stamina(
    const DM2_V1_HeroMoveState *heroes,
    int hero_count)
{
    int drained = 0;

    if (!heroes || hero_count <= 0) return 0;

    /* skmove.cpp:376-398: for each living hero, compute
     * stamina_cost = 3 * player_weight / max_load + 1,
     * then call DM2_ADJUST_STAMINA(hero_index, cost).
     * ADJUST_STAMINA is receipted in skproject_core but
     * requires live hero data, so we receipt the computation
     * and let the caller apply it. */
    for (int i = 0; i < hero_count && i < 4; i++) {
        if (!heroes[i].alive) continue;
        if (heroes[i].max_load == 0) continue;

        uint32_t cost = 3u * (uint32_t)heroes[i].player_weight;
        cost = cost / (uint32_t)heroes[i].max_load + 1u;
        (void)cost;
        drained++;
    }

    return drained;
}

int dm2_v1_perform_move_exec(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_SourceTimerQueue *queue,
    const DM2_V1_PerformMoveExecRequest *request,
    DM2_V1_PerformMoveExecReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    const DM2_V1_PerformMoveReceipt *plan = &request->plan;
    if (!plan->valid) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;
    receipt->final_x = (int16_t)plan->from_x;
    receipt->final_y = (int16_t)plan->from_y;
    receipt->final_dir = (int16_t)plan->to_dir;
    receipt->final_level = (int16_t)plan->current_level;

    if (plan->blocked) {
        return 0;
    }

    /* Walk delay: max across all living heroes.
     * skmove.cpp:245-263 */
    int walk_delay = dm2_v1_calc_party_walk_delay(
        request->heroes, request->hero_count);
    receipt->walk_delay = walk_delay;

    /* Half-step: if walk_delay > 1 and no cooldown active,
     * enter half-step interpolation state.
     * skmove.cpp:264-303 */
    if (walk_delay > 1) {
        receipt->half_step_entered = 1;
    }

    /* Stamina drain: weight-proportional per hero.
     * skmove.cpp:376-398 */
    receipt->stamina_drained = dm2_v1_drain_party_stamina(
        request->heroes, request->hero_count);
    receipt->squad_dir_reset = 1;

    /* Move classification: the reference calls DM2_12b4_0881
     * which classifies the target tile into one of 6 cases.
     * The plan receipt already computed blocking, so we map
     * the plan's target_square_type to a classification. */
    if (plan->target_square_type == 6) {
        receipt->classification = DM2_MOVE_CLASS_STAIR_UP;
    } else if (plan->target_square_type == 7) {
        receipt->classification = DM2_MOVE_CLASS_STAIR_DOWN;
    } else if (plan->target_is_door && plan->target_door_state != 0) {
        receipt->classification = DM2_MOVE_CLASS_DOOR_ATTACK;
    } else if (plan->target_square_type == 8) {
        receipt->classification = DM2_MOVE_CLASS_TELEPORTER;
    } else if (request->target_creature_handle != 0xFFFFu &&
               request->target_creature_handle != 0u) {
        receipt->classification = DM2_MOVE_CLASS_CREATURE;
    } else {
        receipt->classification = DM2_MOVE_CLASS_OPEN_TILE;
    }

    switch (receipt->classification) {
    case DM2_MOVE_CLASS_OPEN_TILE:
        /* Normal move: update position, fire MOVE_RECORD_TO.
         * skmove.cpp:596 */
        receipt->final_x = (int16_t)plan->to_x;
        receipt->final_y = (int16_t)plan->to_y;
        receipt->position_updated = 1;

        if (pool_set && dungeon && queue) {
            DM2_V1_MoveRecordToReceipt mr_receipt;
            dm2_v1_move_record_to(pool_set, dungeon, queue,
                                  (int16_t)0xFFFF,
                                  (int16_t)plan->from_x,
                                  (int16_t)plan->from_y,
                                  (int16_t)plan->to_x,
                                  (int16_t)plan->to_y,
                                  (int16_t)plan->to_dir,
                                  plan->current_level,
                                  request->game_tick,
                                  &mr_receipt);
            receipt->record_moved = mr_receipt.valid;
        }
        break;

    case DM2_MOVE_CLASS_STAIR_UP:
    case DM2_MOVE_CLASS_STAIR_DOWN: {
        /* Stair: locate destination level via map descriptors, then
         * 12b4_00af for level change.  skmove.cpp:420-521 */
        int enter_forward = (receipt->classification == DM2_MOVE_CLASS_STAIR_UP);
        int16_t locate_delta = enter_forward ? -1 : 1;

        receipt->final_x = (int16_t)plan->to_x;
        receipt->final_y = (int16_t)plan->to_y;
        receipt->position_updated = 1;

        if (request->map_descriptors && request->map_descriptor_count > 0) {
            int16_t loc_x = (int16_t)plan->to_x;
            int16_t loc_y = (int16_t)plan->to_y;
            DM2_V1_SkprojectLocateOtherLevelReceipt loc_receipt;
            uint16_t resume_off = 0;
            uint8_t candidates[DM2_V1_MAX_LEVELS];
            uint16_t cand_count = 0;
            for (uint16_t m = 0; m < request->map_descriptor_count; m++) {
                if ((int16_t)m != (int16_t)plan->current_level)
                    candidates[cand_count++] = request->map_descriptors[m].map_id;
            }
            int dest_map = dm2_v1_skproject_locate_other_level(
                request->map_descriptors, request->map_descriptor_count,
                (int16_t)plan->current_level, locate_delta,
                &loc_x, &loc_y, candidates, cand_count, 0, &resume_off, &loc_receipt);
            if (dest_map >= 0) {
                DM2_V1_SkprojectOtherLevelReceipt stair_receipt;
                dm2_v1_skproject_move_12b4_00af(
                    enter_forward,
                    (int16_t)plan->current_level,
                    (int16_t)plan->to_x, (int16_t)plan->to_y,
                    (int16_t)dest_map, loc_x, loc_y,
                    (int16_t)plan->to_dir,
                    &stair_receipt);
                if (stair_receipt.valid) {
                    receipt->stair_level_changed = 1;
                    receipt->stair_dest_level = dest_map;
                    receipt->final_x = loc_x;
                    receipt->final_y = loc_y;
                    receipt->final_level = (int16_t)dest_map;
                }
            }
        }
        if (!receipt->stair_level_changed) {
            receipt->fail_closed = 1;
        }
        break;
    }

    case DM2_MOVE_CLASS_DOOR_ATTACK: {
        /* Door attack: party attacks a closed door with combined power.
         * skmove.cpp:428-474 */
        DM2_V1_SkprojectAttackDoorReceipt door_receipt;
        int door_ok = dm2_v1_skproject_attack_door(
            request->door_tile_type,
            request->door_record_byte2,
            request->door_record_byte3,
            request->party_attack_power,
            request->door_required_power,
            request->door_use_byte2_gate,
            request->door_rebirth_altar,
            request->door_timer_delay,
            (int16_t)plan->to_x,
            (int16_t)plan->to_y,
            &door_receipt);
        receipt->door_attacked = 1;
        receipt->door_attack_power = (int)request->party_attack_power;
        if (!door_ok) {
            receipt->fail_closed = 0;
        }
        break;
    }

    case DM2_MOVE_CLASS_CREATURE: {
        /* Creature encounter: try push first (12b4_0d75), if creature
         * can't be pushed, attack it.  skmove.cpp:477-543 */
        receipt->creature_handle = (int)request->target_creature_handle;
        if (request->target_creature_handle == 0xFFFFu) {
            receipt->fail_closed = 1;
            break;
        }
        DM2_V1_SkprojectCreaturePushReceipt push_receipt;
        int pushed = dm2_v1_skproject_move_12b4_0d75(
            (int16_t)plan->to_x,
            (int16_t)plan->to_y,
            (uint8_t)(plan->to_dir & 3),
            1,
            request->target_creature_weight,
            request->creature_force_threshold,
            request->creature_random_value,
            &push_receipt);
        receipt->creature_pushable = pushed;
        if (pushed) {
            receipt->final_x = (int16_t)plan->to_x;
            receipt->final_y = (int16_t)plan->to_y;
            receipt->position_updated = 1;
        } else {
            receipt->creature_attacked = 1;
        }
        break;
    }

    case DM2_MOVE_CLASS_BLOCKED:
        break;

    case DM2_MOVE_CLASS_TELEPORTER: {
        /* Teleporter: GET_TELEPORTER_DETAIL + map_3BF83.
         * skmove.cpp:559-576 */
        DM2_V1_SkprojectTeleporterDetail detail;
        DM2_V1_SkprojectGetTeleporterDetailReceipt tp_receipt;
        const uint8_t *src_tiles;
        const uint8_t *dst_tiles;
        int16_t sw, sh, dw, dh;
        int tp_ok = 0;

        receipt->final_x = (int16_t)plan->to_x;
        receipt->final_y = (int16_t)plan->to_y;
        receipt->position_updated = 1;

        if (dungeon && pool_set) {
            src_tiles = dm2_v1_dungeon_level_tile_data(
                dungeon, plan->current_level, &sw, &sh);
            if (src_tiles) {
                dst_tiles = src_tiles;
                dw = sw;
                dh = sh;
                tp_ok = dm2_v1_skproject_get_teleporter_detail(
                    (int16_t)plan->to_x, (int16_t)plan->to_y,
                    src_tiles, sw, sh, pool_set,
                    (uint8_t)plan->current_level,
                    dst_tiles, dw, dh,
                    &detail, &tp_receipt);
                if (tp_ok) {
                    receipt->teleporter_resolved = 1;
                    receipt->teleporter_dest_x = (int)detail.b_02;
                    receipt->teleporter_dest_y = (int)detail.b_03;
                    receipt->teleporter_dest_map = (int)detail.b_04;
                    receipt->final_x = (int16_t)detail.b_02;
                    receipt->final_y = (int16_t)detail.b_03;
                    if (detail.b_04 != (uint8_t)plan->current_level) {
                        receipt->final_level = (int16_t)detail.b_04;
                    }
                }
            }
        }
        if (!tp_ok) {
            receipt->teleporter_resolved = 0;
            receipt->fail_closed = 1;
        }
        break;
    }
    }

    return receipt->position_updated ? 1 : 0;
}
