/*
 * dm2_v1_tile_record_walk_pc34_compat.c — DM2-002 tile record-link walk
 * (c_map.cpp:61-69 + c_record.cpp:54-57) plus the two AI-stop callers
 * that were blocked on it: DM2_PROCEED_XACT_85 (c_ai.cpp:2078-2117) and
 * DM2_ACTIVATE_CREATURE_KILLER (c_tim_proc.cpp:2907-2988).
 */

#include "dm2_v1_tile_record_walk_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#include "dm2_v1_think_creature_pc34_compat.h"

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static long walk_budget_total(const DM2_V1_RecordPoolSet *set)
{
    long budget = 1;
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        budget += set->pools[i].record_count;
    }
    return budget;
}

int16_t dm2_v1_tile_record_link(const DM2_V1_DungeonData *dungeon,
                                int map, int x, int y)
{
    int first;

    if (dungeon == NULL) {
        return DM2_V1_RECORD_HANDLE_NULL;
    }
    /* The loader binding returns -1 both for cells without the 0x10
     * object flag (the source's OBJECT_END_MARKER head) and for
     * unresolvable lookups; both terminate a walk, and the marker
     * distinction is kept for receipts. */
    first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    if (first < 0) {
        return DM2_V1_RECORD_HANDLE_END;
    }
    return (int16_t)first;
}

int dm2_v1_tile_record_walk(const DM2_V1_RecordPoolSet *set,
                            const DM2_V1_DungeonData *dungeon,
                            int map, int x, int y,
                            DM2_V1_TileRecordVisitor visitor,
                            void *context,
                            DM2_V1_TileRecordWalkReceipt *receipt)
{
    DM2_V1_TileRecordWalkReceipt local;
    int16_t cursor;
    long budget;

    memset(&local, 0, sizeof(local));
    if (receipt == NULL) {
        receipt = &local;
    } else {
        *receipt = local;
    }

    if (set == NULL || !set->valid || dungeon == NULL ||
        visitor == NULL) {
        return 0;
    }

    cursor = dm2_v1_tile_record_link(dungeon, map, x, y);
    budget = walk_budget_total(set);

    for (;;) {
        const uint8_t *record;
        int16_t next;

        if (cursor == DM2_V1_RECORD_HANDLE_END) {
            receipt->ended_at_marker = 1;
            receipt->valid = 1;
            return 1;
        }
        if (cursor == DM2_V1_RECORD_HANDLE_NULL) {
            receipt->fail_closed = 1;
            return 0;
        }
        if (budget-- <= 0) {
            receipt->fail_closed = 1; /* corrupt chain: bounded */
            return 0;
        }
        record = dm2_v1_record_pool_address(set, cursor);
        if (record == NULL) {
            receipt->fail_closed = 1;
            return 0;
        }
        ++receipt->records_visited;
        if (visitor(context, cursor, record) != 0) {
            receipt->stopped_by_visitor = 1;
            receipt->valid = 1;
            return 1;
        }
        if (!dm2_v1_record_pool_next_link(set, cursor, &next)) {
            receipt->fail_closed = 1;
            return 0;
        }
        cursor = next;
    }
}

/* Resolve the acting creature's CAII slot through record byte@5,
 * exactly like the source's s350.creatures pointer. */
static uint8_t *creature_slot(DM2_V1_RecordPoolSet *pool_set,
                              DM2_V1_CaiiArray *caii,
                              int16_t creature_record)
{
    uint8_t *record;
    int slot_index;

    record = dm2_v1_record_pool_address_mut(pool_set, creature_record);
    if (record == NULL || record[5] == 0xffu) {
        return NULL;
    }
    slot_index = record[5];
    if (slot_index < 0 || slot_index >= caii->capacity) {
        return NULL;
    }
    return caii->slots + (size_t)slot_index * DM2_V1_CAII_SLOT_SIZE;
}

typedef struct {
    int db_break;      /* DB index > 3: the source walk break */
    int match_found;   /* DB2 word@2 pattern matched */
} Xact85Walk;

static int xact85_visitor(void *context, int16_t handle,
                          const uint8_t *record)
{
    Xact85Walk *walk = (Xact85Walk *)context;
    int db = dm2_v1_record_handle_pool(handle);

    /* c_ai.cpp:2090-2093: DB index above 3 breaks the walk. */
    if (db > 3) {
        walk->db_break = 1;
        return 1;
    }
    /* c_ai.cpp:2095-2107: DB2 records carry the match probe on word@2:
     * (w & 0x6) == 0x2 and ((w >> 11) & 0x1f) == 1. */
    if (db == 2) {
        uint16_t w = rd16(record + 2);
        if ((w & 0x6u) == 0x2u && ((w >> 11) & 0x1fu) == 0x1u) {
            walk->match_found = 1;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_proceed_xact_85(DM2_V1_RecordPoolSet *pool_set,
                           const DM2_V1_DungeonData *dungeon,
                           DM2_V1_CaiiArray *caii,
                           DM2_V1_SourceTimerQueue *queue,
                           int map_id,
                           unsigned long game_tick,
                           int16_t creature_record,
                           int x, int y,
                           DM2_V1_Xact85Receipt *receipt)
{
    DM2_V1_Xact85Receipt local;
    DM2_V1_TileRecordWalkReceipt walk_rc;
    Xact85Walk walk;
    uint8_t *slot;

    memset(&local, 0, sizeof(local));
    snprintf(local.source_evidence, sizeof(local.source_evidence),
             "skproject c_ai.cpp:2078-2117 DM2_PROCEED_XACT_85 over the "
             "DM2-002 tile record-link walk (c_map.cpp:61-69, "
             "c_record.cpp:54-57) + bound DM2_ai_13e4_0360 "
             "(c_ai.cpp:5912-5960)");
    if (receipt == NULL) {
        receipt = &local;
    } else {
        *receipt = local;
    }

    if (pool_set == NULL || !pool_set->valid || dungeon == NULL ||
        caii == NULL || !caii->valid || queue == NULL ||
        x < 0 || y < 0 || x > 0xff || y > 0xff) {
        return 0;
    }

    /* The slot byte@0x1a/0x1e writes land through record byte@5 exactly
     * like the source's s350.creatures; no slot fails closed. */
    slot = creature_slot(pool_set, caii, creature_record);
    if (slot == NULL) {
        return 0;
    }

    memset(&walk, 0, sizeof(walk));
    memset(&walk_rc, 0, sizeof(walk_rc));
    if (!dm2_v1_tile_record_walk(pool_set, dungeon, map_id, x, y,
                                 xact85_visitor, &walk, &walk_rc)) {
        receipt->walk_records = walk_rc.records_visited;
        receipt->walk_fail_closed = 1;
        return 0;
    }
    receipt->walk_records = walk_rc.records_visited;
    receipt->walk_db_break = walk.db_break;

    if (walk.match_found) {
        /* c_ai.cpp:2104-2107: b_1e = 1, b_1a = 59, return -2. */
        receipt->match_found = 1;
        slot[0x1e] = 1;
        slot[0x1a] = 59;
        receipt->slot_b1e_written = 1;
        receipt->slot_b1a = 59;
        receipt->return_code = -2;
        receipt->valid = 1;
        receipt->completed = 1;
        return 1;
    }

    /* c_ai.cpp:2113-2116: the walk end AI-stops the creature through
     * the bound DM2_ai_13e4_0360 (dir 0x13, argl0 1), then b_1a = 51
     * and return -3 — the source writes b_1a unconditionally after the
     * call. */
    {
        DM2_V1_CaiiAiTurnReceipt turn;
        memset(&turn, 0, sizeof(turn));
        receipt->ai_stop_invoked = 1;
        if (dm2_v1_caii_ai_13e4_0360(pool_set, dungeon, caii, queue,
                                     map_id, game_tick, creature_record,
                                     x, y, 0x13, 1, &turn)) {
            receipt->ai_stop_completed = turn.completed;
        }
        receipt->ai_stop_guard_denied = turn.guard_denied;
    }
    slot[0x1a] = 51;
    receipt->slot_b1a = 51;
    receipt->return_code = -3;
    receipt->valid = 1;
    receipt->completed = 1;
    return 1;
}

int dm2_v1_activate_creature_killer_walk(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    DM2_V1_DropRng *rng,
    int map_id,
    unsigned long game_tick,
    int map_width, int map_height,
    int mode, int type_filter,
    int ebx, int ecx,
    int center_x, int center_y,
    int action, int attack_flag,
    int target_x, int target_y,
    DM2_V1_CreatureKillerWalkReceipt *receipt)
{
    DM2_V1_CreatureKillerWalkReceipt local;
    int dxr, dyr, start_x, start_y, rows, vl_0c;
    uint16_t mode_w = (uint16_t)mode;
    uint16_t filter_w = (uint16_t)type_filter;

    memset(&local, 0, sizeof(local));
    snprintf(local.source_evidence, sizeof(local.source_evidence),
             "skproject c_tim_proc.cpp:2907-2988 "
             "DM2_ACTIVATE_CREATURE_KILLER (sweep 2925-2987, "
             "DM2_GET_CREATURE_AT c_querydb.cpp:1486-1507, DM2_1c9a_09b9 "
             "c_1c9a.cpp:5404-5414, bound DM2_ai_13e4_0360 + "
             "DM2_ATTACK_CREATURE)");
    if (receipt == NULL) {
        receipt = &local;
    } else {
        *receipt = local;
    }

    if (pool_set == NULL || !pool_set->valid || dungeon == NULL ||
        caii == NULL || !caii->valid || queue == NULL ||
        map_width <= 0 || map_height <= 0) {
        return 0;
    }

    /* c_tim_proc.cpp:2925-2934: radii from the truncated 16-bit
     * differences, then the sweep origin. */
    dxr = (int)(int16_t)(ebx - center_x);
    if (dxr < 0) {
        dxr = -dxr;
    }
    dyr = (int)(int16_t)(ecx - center_y);
    if (dyr < 0) {
        dyr = -dyr;
    }
    start_x = ebx - dxr;
    start_y = ecx - dyr;
    rows = 2 * dyr + 1;      /* vl_0c */
    {
        int cols = 2 * dxr + 1; /* vl_08 */

        /* c_tim_proc.cpp:2936-2987: outer loop pre-decrements vl_0c
         * and returns when it underflows; the inner loop pre-decrements
         * RG5 and breaks on underflow. */
        for (vl_0c = rows;;) {
            int rg5;

            --vl_0c;
            if (vl_0c < 0) {
                break;
            }
            for (rg5 = cols;;) {
                int x, y;
                int16_t rec;
                const uint8_t *record;

                --rg5;
                if (rg5 < 0) {
                    break;
                }
                x = start_x + rg5;
                if (x < 0 || x >= map_width) {
                    ++receipt->cells_out_of_bounds;
                    continue;
                }
                y = start_y + vl_0c;
                if (y < 0 || y >= map_height) {
                    ++receipt->cells_out_of_bounds;
                    continue;
                }
                ++receipt->cells_visited;

                /* DM2_GET_CREATURE_AT (c_querydb.cpp:1486-1507). */
                rec = dm2_v1_get_creature_at(pool_set, dungeon,
                                             map_id, x, y);
                if (rec == DM2_V1_RECORD_HANDLE_NULL) {
                    continue;
                }
                ++receipt->creatures_found;

                /* DM2_1c9a_09b9 (c_1c9a.cpp:5404-5414): record word@8
                 * must equal the filter word. */
                record = dm2_v1_record_pool_address(pool_set, rec);
                if (record == NULL) {
                    return 0;
                }
                if (rd16(record + 8) != filter_w) {
                    ++receipt->type_filter_miss;
                    continue;
                }

                if (action == 0xb) {
                    /* c_tim_proc.cpp:2961-2976: mode word 0/1 skip,
                     * 2 AI-stops, anything above aborts the sweep. */
                    if (mode_w < 1u) {
                        continue;
                    }
                    if (mode_w <= 1u) {
                        continue;
                    }
                    if (mode_w != 2u) {
                        receipt->aborted = 1;
                        receipt->valid = 1;
                        receipt->completed = 1;
                        return 1;
                    }
                    ++receipt->ai_stops;
                    {
                        DM2_V1_CaiiAiTurnReceipt turn;
                        memset(&turn, 0, sizeof(turn));
                        if (dm2_v1_caii_ai_13e4_0360(
                                pool_set, dungeon, caii, queue, map_id,
                                game_tick, rec, x, y, 0x13, 1, &turn)) {
                            receipt->ai_stop_completed += turn.completed;
                        }
                    }
                    continue;
                }
                if (action != 0x28) {
                    ++receipt->unknown_action;
                    continue;
                }
                /* c_tim_proc.cpp:2979-2986: attack word = RG6 low word,
                 * bit 0x8000 set when argw3 != 0. */
                {
                    uint32_t attack_word = (uint32_t)mode_w;
                    if (attack_flag != 0) {
                        attack_word |= 0x8000u;
                    }
                    ++receipt->attacks;
                    {
                        DM2_V1_CaiiAttackReceipt atk;
                        memset(&atk, 0, sizeof(atk));
                        if (dm2_v1_caii_attack_creature(
                                pool_set, dungeon, caii, queue, rng,
                                map_id, game_tick, rec, x, y,
                                target_x, target_y, attack_word, 0x64,
                                0, &atk)) {
                            ++receipt->attacks_completed;
                        }
                    }
                }
            }
        }
    }

    receipt->valid = 1;
    receipt->completed = 1;
    return 1;
}

const char *dm2_v1_tile_record_walk_source_evidence(void)
{
    return "skproject c_map.cpp:61-69 DM2_GET_TILE_RECORD_LINK, "
           "c_record.cpp:54-57 DM2_GET_NEXT_RECORD_LINK, "
           "c_ai.cpp:2078-2117 DM2_PROCEED_XACT_85, "
           "c_tim_proc.cpp:2907-2988 DM2_ACTIVATE_CREATURE_KILLER, "
           "c_1c9a.cpp:5404-5414 DM2_1c9a_09b9";
}
