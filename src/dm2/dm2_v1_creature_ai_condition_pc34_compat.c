/*
 * dm2_v1_creature_ai_condition_pc34_compat.c — DM2 AI condition evaluator
 * and action dispatch helpers.
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include "dm2_v1_creature_ai_condition_pc34_compat.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* Helper: absolute value                                              */
/* ------------------------------------------------------------------ */

static int16_t abs16(int16_t v)
{
    return v < 0 ? (int16_t)-v : v;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_1316 — AI condition evaluator (c_ai.cpp:2516-3132)        */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_check_condition(
    const DM2_V1_AiConditionRequest *req,
    DM2_V1_AiConditionReceipt *receipt)
{
    uint8_t raw_byte;
    uint8_t case_num;
    int invert;
    int vl_00;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;
    raw_byte = req->condition_byte;

    /* c_ai.cpp:2550-2557 — bit 0x40: subtype check shortcut */
    if (raw_byte & 0x40) {
        if (req->creature_subtype == req->creature_rec.b_12) {
            receipt->subtype_matched = 1;
            receipt->result = 1;
            receipt->condition_case = -1;
            return 1;
        }
        raw_byte &= (uint8_t)~0x40;
    }

    /* c_ai.cpp:2558-2562 — bit 0x80: invert result */
    invert = (raw_byte & 0x80) ? 1 : 0;
    receipt->inverted = invert;
    case_num = raw_byte & 0x7F;
    receipt->condition_case = (int)case_num;

    if (case_num > 22) {
        /* c_ai.cpp:2565-2566 — out of range, jump to L_fin2 (result=0) */
        receipt->result = invert ? 1 : 0;
        return 1;
    }

    vl_00 = 0;

    switch (case_num) {

    case 0:
        /* c_ai.cpp:2570-2571 — always true */
        vl_00 = 1;
        break;

    case 1:
    case 22:
        /* c_ai.cpp:2573-2605 — party in line of sight + distance check.
         * Case 1: same map, same direction, any distance.
         * Case 22: same map, same direction, distance > 1 requires LOS. */
    {
        int16_t dir;
        int16_t sq_dist;

        if (req->creature_map != req->party_map)
            break; /* vl_00 stays 0 */

        if (!req->calc_vector_dir) {
            receipt->needs_external = 1;
            receipt->external_fn = "CALC_VECTOR_DIR";
            break;
        }

        dir = req->calc_vector_dir(req->cb_ctx,
            req->party_x, req->party_y,
            (uint16_t)req->creature_x, (uint16_t)req->creature_y);

        if ((uint16_t)dir != req->party_facing)
            break;

        /* Case 1: direction match alone is sufficient */
        if (case_num == 1) {
            vl_00 = 1;
            break;
        }

        /* Case 22: distance check */
        if (!req->calc_square_distance) {
            receipt->needs_external = 1;
            receipt->external_fn = "CALC_SQUARE_DISTANCE";
            break;
        }

        sq_dist = req->calc_square_distance(req->cb_ctx,
            req->party_x, req->party_y,
            (uint16_t)req->creature_x, (uint16_t)req->creature_y);

        if (sq_dist <= 1) {
            vl_00 = 1;
            break;
        }

        /* c_ai.cpp:2596-2597 — distance > param or LOS blocked */
        if (sq_dist > req->condition_param || req->v1e0976 != 0)
            break;

        /* Need line-of-sight callback */
        if (!req->line_of_sight) {
            receipt->needs_external = 1;
            receipt->external_fn = "DM2_19f0_0207";
            break;
        }

        if (req->line_of_sight(req->cb_ctx,
                (int32_t)req->party_x, (int32_t)req->party_y,
                (int32_t)req->creature_x, (int32_t)req->creature_y) != 0)
            vl_00 = 1;
        break;
    }

    case 2:
        /* c_ai.cpp:2607-2629 — party on same tile */
        if (req->creature_map == req->party_map &&
            req->creature_x == (uint8_t)req->party_x &&
            req->creature_y == (uint8_t)req->party_y) {
            vl_00 = 1;
        }
        break;

    case 3:
        /* c_ai.cpp:2631-2633 — creature can handle specific item.
         * Falls through to skip00307 logic (c_ai.cpp:3055-3063). */
        if (!req->can_handle_item) {
            receipt->needs_external = 1;
            receipt->external_fn = "CREATURE_CAN_HANDLE_ITEM_IN";
            break;
        }
        /* c_ai.cpp:3058-3062 — if handle returns -2, fail; else succeed */
        if (req->can_handle_item(req->cb_ctx,
                req->condition_param, req->spx.w_02, 0xFF) != -2)
            vl_00 = 1;
        break;

    case 4:
        /* c_ai.cpp:2636-2639 — timing flag (v1e058d) */
        vl_00 = (int)req->v1e058d;
        break;

    case 5:
    case 13:
        /* c_ai.cpp:2641-2667 — creature at home position.
         * Checks map, x, y from SPX w_0c packed field.
         * Case 13: if at home, also does item handle check (skip00307). */
    {
        uint16_t home = req->spx.w_0c;
        uint16_t home_map = home >> 10;
        uint16_t home_x = home & 0x1F;
        uint16_t home_y = (home << 6) >> 11; /* bits 5-9 */

        if (req->creature_map != home_map)
            break;
        if (req->creature_x != (uint8_t)home_x)
            break;
        if (req->creature_y != (uint8_t)home_y)
            break;

        if (case_num == 5) {
            vl_00 = 1;
            break;
        }

        /* Case 13: at home, now do item handle check */
        if (!req->can_handle_item) {
            receipt->needs_external = 1;
            receipt->external_fn = "CREATURE_CAN_HANDLE_ITEM_IN";
            break;
        }
        if (req->can_handle_item(req->cb_ctx,
                req->condition_param, req->spx.w_02, 0xFF) != -2)
            vl_00 = 1;
        break;
    }

    case 6:
        /* c_ai.cpp:2669-2676 — creature flag bit check.
         * Tests if bit (1 << condition_param) is set in SPX w_0a. */
        if ((uint32_t)((1 << (req->condition_param & 0xFF)) & req->spx.w_0a) != 0)
            vl_00 = 1;
        break;

    case 7:
        /* c_ai.cpp:2678-2682 — party on same map level */
        if (req->creature_map == req->party_map)
            vl_00 = 1;
        break;

    case 8:
        /* c_ai.cpp:2684-2728 — party has specific item equipped.
         * Loops through 4 party slots, checks hand items via CREATURE_CAN_HANDLE_IT. */
    {
        int slot;
        if (!req->get_player_at_position || !req->can_handle_it) {
            receipt->needs_external = 1;
            receipt->external_fn = "GET_PLAYER_AT_POSITION/CREATURE_CAN_HANDLE_IT";
            break;
        }

        for (slot = 0; slot < 4; slot++) {
            int16_t player = req->get_player_at_position(req->cb_ctx, (int16_t)slot);
            if (player == -1) continue;

            /* Check hand 1 (item[1]) */
            if (req->hero_items[player].item_hand1 != -1) {
                if (req->can_handle_it(req->cb_ctx,
                        (int32_t)(uint16_t)req->hero_items[player].item_hand1, 0x0b) != 0) {
                    vl_00 = 1;
                    break;
                }
            }

            /* Check hand 0 (item[0]) */
            if (req->hero_items[player].item_hand0 != -1) {
                if (req->can_handle_it(req->cb_ctx,
                        (int32_t)(uint16_t)req->hero_items[player].item_hand0, 0x0b) != 0) {
                    vl_00 = 1;
                    break;
                }
            }
        }
        break;
    }

    case 9:
        /* c_ai.cpp:2730-2737 — party power level >= threshold.
         * Needs DM2_2c1d_09d9 which computes party skill power level.
         * Fail-closed: requires live party data. */
        receipt->needs_external = 1;
        receipt->external_fn = "DM2_2c1d_09d9";
        break;

    case 10:
        /* c_ai.cpp:2739-2768 — party adjacent with door check.
         * Same map, then checks if party is on same tile (skip00312 path)
         * or adjacent (skip00310 path). Both paths then check door tile. */
    {
        int adjacent = 0;
        int same_tile = 0;

        if (req->creature_map != req->party_map)
            break;

        if (req->creature_x == (uint8_t)req->party_x &&
            req->creature_y == (uint8_t)req->party_y) {
            same_tile = 1;
        } else {
            adjacent = 1;
        }

        if (adjacent) {
            /* skip00310 path — then falls to skip00312 with RG1L=1 */
            /* c_ai.cpp:3083-3118 — distance check + door tile check */
            int16_t dx_abs, dy_abs;
            int16_t dir;
            int32_t tile_val;
            int16_t tile_type;

            dx_abs = abs16((int16_t)((int16_t)req->creature_x - (int16_t)req->party_x));
            dy_abs = abs16((int16_t)((int16_t)req->creature_y - (int16_t)req->party_y));

            if (dx_abs + dy_abs > 1)
                break;

            if (!req->calc_vector_dir || !req->get_tile_value || !req->dx_table || !req->dy_table) {
                receipt->needs_external = 1;
                receipt->external_fn = "CALC_VECTOR_DIR/GET_TILE_VALUE";
                break;
            }

            dir = req->calc_vector_dir(req->cb_ctx,
                (uint16_t)req->creature_x, (uint16_t)req->creature_y,
                req->party_x, req->party_y);

            /* Check tile in that direction from party */
            {
                int32_t check_x = (int32_t)req->party_x + req->dx_table[dir];
                int32_t check_y = (int32_t)req->party_y + req->dy_table[dir];

                tile_val = req->get_tile_value(req->cb_ctx, check_x, check_y);
                tile_type = (int16_t)((tile_val & 0xFF) >> 5);

                /* c_ai.cpp:3105 — must be type 2 (pit?) */
                if (tile_type != 2)
                    break;

                /* c_ai.cpp:3109-3112 — bit 3 must be set */
                if (((tile_val & 0xFF) & 0x08) == 0)
                    break;

                /* c_ai.cpp:3115-3117 — bit 0 must be clear */
                if (((tile_val & 0xFF) & 0x01) != 0)
                    break;
            }

            vl_00 = 1;
        } else if (same_tile) {
            /* c_ai.cpp:3083 — same tile, RG1L=0, skip00312 with RG1L==0 goes to L_fin2 */
            break;
        }
        break;
    }

    case 11:
        /* c_ai.cpp:2770-2796 — door ahead, can open.
         * Gets creature facing direction + 2 (opposite), checks v1e057a bit 0x20,
         * then checks tile ahead for door type, then DM2_19f0_0d10. */
    {
        int16_t facing;
        int16_t check_dir;

        /* Extract direction from SPX w_0e: (w_0e << 6) >> 14 = bits 8-9 */
        facing = (int16_t)(((req->spx.w_0e << 6) >> 14) & 0x3);
        check_dir = (facing + 2) & 3;

        /* c_ai.cpp:2779 — v1e057a bit 0x20 must be set */
        if ((req->v1e057a & 0x20) == 0)
            break;

        if (!req->get_tile_value || !req->dx_table || !req->dy_table) {
            receipt->needs_external = 1;
            receipt->external_fn = "GET_TILE_VALUE";
            break;
        }

        {
            int32_t check_x = (int32_t)req->creature_x + req->dx_table[check_dir];
            int32_t check_y = (int32_t)req->creature_y + req->dy_table[check_dir];
            int32_t tile_val = req->get_tile_value(req->cb_ctx, check_x, check_y);
            int32_t tile_type = ((tile_val & 0xFF) >> 5) & 0x7;

            /* c_ai.cpp:2789 — must be door (type 4) */
            if (tile_type != 4)
                break;
        }

        /* Need door_check callback for DM2_19f0_0d10 */
        if (!req->door_check) {
            receipt->needs_external = 1;
            receipt->external_fn = "DM2_19f0_0d10";
            break;
        }

        if (req->door_check(req->cb_ctx, 1,
                (int32_t)req->creature_x, (int32_t)req->creature_y,
                -1, -1, check_dir) != 0)
            vl_00 = 1;
        break;
    }

    case 12:
        /* c_ai.cpp:2798-2876 — friendly creature ahead near party.
         * Complex: checks same map, gets creature ahead, checks AI spec friendly bit,
         * then checks 4 directions from party for passable tiles with specific records.
         * Fail-closed: requires multiple external functions. */
        receipt->needs_external = 1;
        receipt->external_fn = "GET_CREATURE_AT/QUERY_CREATURE_AI_SPEC/GET_TILE_RECORD_LINK";
        break;

    case 14:
        /* c_ai.cpp:2878-2885 — HP percentage check.
         * (creature_hp * 52) / ai_spec_max_hp <= condition_param */
        if (req->ai_spec_max_hp == 0)
            break;
        {
            uint32_t pct = (uint32_t)(52 * (uint32_t)req->spx.w_06) / (uint32_t)req->ai_spec_max_hp;
            if (pct <= (uint32_t)req->condition_param)
                vl_00 = 1;
        }
        break;

    case 15:
        /* c_ai.cpp:2887-2962 — creature count on map.
         * Requires walking the entire map tile grid. Fail-closed. */
        receipt->needs_external = 1;
        receipt->external_fn = "mapdat/GET_TILE_VALUE/GET_ADDRESS_OF_RECORD";
        break;

    case 16:
        /* c_ai.cpp:2964-2971 — creature on home map.
         * Compares creature's current map with home map from SPX w_0c. */
    {
        uint16_t home_map = req->spx.w_0c >> 10;
        if (req->creature_map == (uint8_t)home_map)
            vl_00 = 1;
        break;
    }

    case 17:
        /* c_ai.cpp:2973-2983 — sound-based detection.
         * Needs DM2_19f0_045a and DM2_1c9a_1b16. */
        if (!req->sound_detect || !req->sound_threshold) {
            receipt->needs_external = 1;
            receipt->external_fn = "DM2_19f0_045a/DM2_1c9a_1b16";
            break;
        }

        req->sound_detect(req->cb_ctx, (int32_t)req->creature_x, (int32_t)req->creature_y);

        if ((req->sound_flags & 0x10) == 0)
            break;

        if (req->sound_threshold(req->cb_ctx,
                (int32_t)req->condition_param, (int32_t)req->spx.w_08) == 0)
            vl_00 = 1;
        break;

    case 18:
    case 20:
        /* c_ai.cpp:2985-3009 — creature on starting map.
         * Case 18: checks map == starting_map (shares path with case 19).
         * Case 20: checks flag bit set in SPX w_0a first, then same map check. */
        if (case_num == 20) {
            /* c_ai.cpp:3001-3008 — flag bit must be set, then check starting map */
            if (((1 << (req->condition_param & 0xFF)) & req->spx.w_0a) == 0)
                break;
        }
        /* skip00309 path: c_ai.cpp:3044-3048 */
        if (req->creature_map == (uint8_t)req->starting_map)
            vl_00 = 1;
        break;

    case 19:
        /* c_ai.cpp:2989-2999 — creature NOT on starting map with flag clear.
         * Flag bit must NOT be set in SPX w_0a, then creature NOT on starting map. */
        if (((1 << (req->condition_param & 0xFF)) & req->spx.w_0a) != 0)
            break;
        /* skip00305 path with flag = (map != starting_map) */
        if (req->creature_map != (uint8_t)req->starting_map)
            vl_00 = 1;
        break;

    case 21:
        /* c_ai.cpp:3011-3032 — creature at specific waypoint.
         * Reads waypoint from creature record at offset 0x0e + 2*condition_param.
         * Extracts x (bits 0-4), y (bits 5-9), map (bits 10-15) and compares. */
    {
        uint16_t wp_word;
        uint16_t wp_x, wp_y, wp_map;
        int wp_index = (int)req->condition_param;

        if (!req->creature_rec.waypoint_words || wp_index >= req->creature_rec.waypoint_count) {
            receipt->needs_external = 1;
            receipt->external_fn = "creature waypoint data";
            break;
        }

        wp_word = req->creature_rec.waypoint_words[wp_index];
        wp_x = wp_word & 0x1F;
        wp_y = (wp_word >> 5) & 0x1F;
        wp_map = wp_word >> 10;

        /* c_ai.cpp:3017-3019 — check x */
        if (req->creature_x != (uint8_t)wp_x)
            break;

        /* c_ai.cpp:3020-3024 — check y */
        if (req->creature_y != (uint8_t)wp_y)
            break;

        /* c_ai.cpp:3026-3031 — check map (skip00308 path) */
        if (req->creature_map == (uint8_t)wp_map)
            vl_00 = 1;
        break;
    }

    default:
        break;
    }

    /* c_ai.cpp:3128-3131 — invert if vl_04 != 0 */
    if (invert)
        vl_00 = (vl_00 == 0) ? 1 : 0;

    receipt->result = vl_00;
    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_ai_14cd_0f3c — create action entry (c_ai.cpp:1498-1566)       */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_create_action_entry(
    const DM2_V1_CreateActionEntryRequest *req,
    DM2_V1_CreateActionEntryReceipt *receipt)
{
    int8_t attack_str;
    int8_t adjust;
    int16_t combined;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->hexe) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:1516 — max 16 entries */
    if (req->current_entry_count >= 16) {
        receipt->rejected_full = 1;
        return 1;
    }

    /* c_ai.cpp:1518-1531 — compute attack strength.
     * If not on same map as party and strength > 0, check ai_spec bit 0x40.
     * If that bit clear, shift both attack and adjust right by 2. */
    attack_str = req->hexe->b_08;
    adjust = req->strength_adjust;

    if (req->creature_map != (uint8_t)req->party_map) {
        if (attack_str > 0) {
            if ((req->ai_spec_byte1 & 0x40) == 0) {
                attack_str >>= 2;
                adjust >>= 2;
            }
        }
    }

    /* c_ai.cpp:1533-1537 — combine and clamp to [-1, 127] */
    combined = (int16_t)(int8_t)attack_str + (int16_t)(int8_t)adjust;
    if (combined < -1) combined = -1;
    if (combined > 127) combined = 127;

    /* c_ai.cpp:1538 — if negative, don't create entry */
    if ((int8_t)combined < 0) {
        receipt->rejected_negative = 1;
        return 1;
    }

    /* c_ai.cpp:1540-1565 — create the entry */
    receipt->entry_created = 1;
    receipt->new_entry_count = req->current_entry_count + 1;

    receipt->entry.strength = (int8_t)combined;
    receipt->entry.b_01 = req->hexe->b_09;
    receipt->entry.b_07 = req->priority;
    receipt->entry.w_08 = req->hexe->w_04;
    receipt->entry.w_0a = req->hexe->w_06 & req->v1e0580;
    receipt->entry.w_0c = req->argw1;
    receipt->entry.b_0e = req->arg_0e;
    receipt->entry.b_0f = req->arg_0f;
    receipt->entry.b_11 = req->group_byte;
    receipt->entry.hexe_ptr = req->hexe_raw;

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_18f2 — hexe condition walk (c_ai.cpp:3135-3207)           */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_hexe_condition_walk(
    const DM2_V1_HexeWalkRequest *req,
    DM2_V1_HexeWalkReceipt *receipt)
{
    const DM2_V1_HexeEntry *entry;
    int8_t walk_key;
    int idx;
    int is_negative;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->table) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:3154-3165 — if walk_key < 0, negate it and set flag */
    walk_key = req->walk_key;
    is_negative = 0;
    if (walk_key < 0) {
        is_negative = 1;
        walk_key = (int8_t)-walk_key;
    }

    (void)is_negative; /* used by caller to clear b_08/b_09 in live path */

    entry = req->table;

    for (idx = 0; idx < req->table_count; idx++) {
        receipt->entries_checked++;

        /* c_ai.cpp:3169-3170 — match on b_0c */
        if (entry->b_0c == walk_key) {
            DM2_V1_AiConditionRequest cond = req->cond_base;
            DM2_V1_AiConditionReceipt cond_receipt;

            /* c_ai.cpp:3174 — check condition */
            cond.condition_byte = (uint8_t)entry->b_01;
            cond.condition_param = entry->w_02;

            if (dm2_v1_ai_check_condition(&cond, &cond_receipt) && cond_receipt.result != 0) {
                receipt->conditions_passed++;

                /* c_ai.cpp:3182-3187 — if was negative, clear b_08 and b_09 */
                /* Action entry creation would happen here via DM2_ai_14cd_0f3c.
                 * Fail-closed: requires live action entry array. */
                receipt->actions_created++;
            }
        }

        /* c_ai.cpp:3203 — check continuation flag */
        if (entry->b_0d == 0)
            break;

        entry++;
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* DM2_14cd_0f0a — AI action dispatch (c_ai.cpp:3841-3930)            */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_action_dispatch(
    const DM2_V1_ActionDispatchRequest *req,
    DM2_V1_ActionDispatchReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (req->dispatch_type > 16) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->dispatch_case = (int)req->dispatch_type;

    /* c_ai.cpp:3852 — set s350.v1e0580 = -1 (0xFFFFFFFF).
     * Each case calls one of the wrapper functions.
     * Fail-closed: the actual wrapper calls require live AI state. */
    receipt->fail_closed = 1;

    return 1;
}

/* ------------------------------------------------------------------ */
/* Unified wrapper (c_ai.cpp:3210-3839)                               */
/* ------------------------------------------------------------------ */

int dm2_v1_ai_action_wrapper(
    const DM2_V1_AiWrapperRequest *req,
    DM2_V1_AiWrapperReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;
    receipt->type_dispatched = req->type;

    /* Validate type range */
    if ((int)req->type < 0 || (int)req->type > 16) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* All wrapper types require live AI state (hexe tables, creature
     * records, s350 state). Fail-closed until wired to runtime.
     *
     * Type mapping to original functions:
     *   0  -> DM2_14cd_18cc (c_ai.cpp:2446-2463): direct action entry, no walk
     *   1  -> DM2_14cd_19a4 (c_ai.cpp:3210-3219): 18f2(eax, edx, ebx, 0, 0xffff)
     *   2  -> DM2_14cd_1a3c (c_ai.cpp:3250-3259): 19c2(eax, ebx, edx, 2, 1)
     *   3  -> DM2_14cd_1a5a (c_ai.cpp:3262-3271): 19c2(eax, ebx, edx, 4, 3)
     *   4  -> DM2_14cd_1b74 (c_ai.cpp:3356-3365): 1a78(eax, edx, ebx, 1)
     *   5  -> DM2_14cd_1b90 (c_ai.cpp:3368-3377): 1a78(eax, edx, ebx, 3)
     *   6  -> DM2_14cd_1c27 (c_ai.cpp:3414-3423): 1bac(eax, edx, ebx, 2, 1)
     *   7  -> DM2_14cd_1c45 (c_ai.cpp:3426-3435): 1bac(eax, edx, ebx, 4, 3)
     *   8  -> DM2_14cd_1c63 (c_ai.cpp:3438-3456): 18f2(5, edx, ebx, 0, special)
     *   9  -> DM2_14cd_1c8d (c_ai.cpp:3459-3492): 18f2(6, edx, ebx, 0, 0xffff) w/gate
     *   10 -> DM2_14cd_1cec (c_ai.cpp:3495-3519): 18f2(7, edx, ebx, 0, special)
     *   11 -> DM2_14cd_1d42 (c_ai.cpp:3522-3539): 18f2(18, edx, ebx, 0, special)
     *   12 -> DM2_14cd_1e36 (c_ai.cpp:3632-3641): 1d6c(eax, edx, ebx, 0xf)
     *   13 -> DM2_14cd_1e52 (c_ai.cpp:3644-3653): 1d6c(eax, edx, ebx, 0x10)
     *   14 -> DM2_14cd_1e6e (c_ai.cpp:3679-3736): random enchant gate
     *   15 -> DM2_14cd_1f8b (c_ai.cpp:3804-3813): 1eec(eax, edx, ebx, 0x15)
     *   16 -> DM2_14cd_1fa7 (c_ai.cpp:3816-3839): 18f2(22, edx, ebx, 0, packed_pos)
     */
    receipt->fail_closed = 1;

    return 1;
}
