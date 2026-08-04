/*
 * dm2_v1_creature_xact_handlers_pc34_compat.c — DM2 creature XACT handler
 * implementations.
 *
 * Ports individual PROCEED_XACT_NN creature behavior functions from
 * skproject/SKULLWIN/c_ai.cpp. Each handler implements a specific creature
 * AI action (move, attack, cast, flee, patrol, guard, etc.).
 *
 * Source: skproject/SKULLWIN/c_ai.cpp
 */

#include "dm2_v1_creature_xact_handlers_pc34_compat.h"

#include <string.h>

/* ================================================================== */
/* XACT 56 — Move forward (c_ai.cpp:78-83)                           */
/* ================================================================== */

int dm2_v1_xact_move_forward(
    const DM2_V1_XactMoveForwardRequest *req,
    DM2_V1_XactMoveForwardReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->go_there) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:80 — CREATURE_GO_THERE(0x80, x, y, -1, -1, direction)
     * c_ai.cpp:81-82 — if result != 0, return -4; else return -2 */
    int16_t go_result = req->go_there(req->ctx,
        0x80, req->pos_x, req->pos_y, -1, -1, req->direction);

    receipt->result = (go_result != 0) ? -4 : -2;
    return 1;
}

/* ================================================================== */
/* XACT 57 — Random turn (c_ai.cpp:86-103)                           */
/* ================================================================== */

int dm2_v1_xact_random_turn(
    const DM2_V1_XactRandomTurnRequest *req,
    DM2_V1_XactRandomTurnReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->go_there) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:90-94 — random ±1 */
    int16_t rg1 = req->rand_b1e ? 1 : -1;
    int16_t dir = req->direction;

    /* c_ai.cpp:98 — try (dir + rg1) & 3 */
    int16_t try_dir = (rg1 + dir) & 0x3;

    /* c_ai.cpp:99 — first attempt */
    if (req->go_there(req->ctx,
            0x80, req->pos_x, req->pos_y, -1, -1, try_dir) != 0) {
        receipt->moved = 1;
        receipt->new_direction = try_dir;
        return 1;
    }

    /* c_ai.cpp:101 — second attempt: (dir - rg1) & 3 */
    int16_t try_dir2 = (dir - rg1) & 0x3;
    if (req->go_there(req->ctx,
            0x80, req->pos_x, req->pos_y, -1, -1, try_dir2) == 0) {
        /* c_ai.cpp:102 — call 19f0_0559(try_dir) to rotate */
        receipt->moved = 1;
        receipt->new_direction = try_dir;
        if (req->rotate_creature)
            req->rotate_creature(req->ctx, 0, try_dir);
    }

    return 1;
}

/* ================================================================== */
/* XACT 59/76 — Move to target (c_ai.cpp:106-116)                    */
/* ================================================================== */

int dm2_v1_xact_move_to_target(
    const DM2_V1_XactMoveToTargetRequest *req,
    DM2_V1_XactMoveToTargetReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:108-110 — resolve rg3 from v1e0572 or v1e07d8.w_04 */
    int16_t rg3 = req->v1e0572;
    if (rg3 == -1)
        rg3 = req->v1e07d8_w04;

    /* c_ai.cpp:111-113 — if v1e0574 != 0, check item handling */
    if (req->v1e0574 != 0) {
        if (!req->can_handle_item) {
            receipt->fail_closed = 1;
            return 0;
        }
        if (req->can_handle_item(req->ctx, rg3,
                req->possession_w00, 0xFF) != -2) {
            receipt->result = -2;
            return 1;
        }
    }

    /* c_ai.cpp:114 — 19f0_2165(0x80, pos_x, pos_y, target_x, target_y, -1, rg3)
     * Requires live projectile launcher. Fail-closed for actual launch. */
    receipt->result = -3;
    receipt->v1e056f = -3;
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 63 — Cast spell (c_ai.cpp:295-330)                           */
/* ================================================================== */

int dm2_v1_xact_cast_spell(
    const DM2_V1_XactCastSpellRequest *req,
    DM2_V1_XactCastSpellReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;
    receipt->result = -3;

    /* c_ai.cpp:300-302 — if v1e0572 == -1, return -3 */
    int16_t spell_type = req->v1e0572;
    if (spell_type == -1)
        return 1;

    /* c_ai.cpp:306-313 — resolve direction for spell targeting */
    int8_t rg2 = (int8_t)(req->v1e0574 & 0xFF);
    int16_t dir = req->direction;
    if ((uint8_t)rg2 != 0xFF)
        rg2 = (int8_t)(((uint8_t)rg2 + dir + 2) & 0x3);

    /* c_ai.cpp:314-317 — compute target tile */
    int16_t check_x = req->pos_x + dm2_v1_dir_delta_x[dir];
    int16_t check_y = req->pos_y + dm2_v1_dir_delta_y[dir];

    /* c_ai.cpp:319-327 — GET_CREATURE_AT, check if spell usable */
    if (!req->get_creature_at || !req->can_handle_item) {
        receipt->fail_closed = 1;
        return 0;
    }

    int16_t creature = req->get_creature_at(req->ctx,
        check_x, check_y, 0);
    if (creature != (int16_t)0xFFFF) {
        /* c_ai.cpp:322 — get creature record, check possession */
        if (!req->get_address_of_record) {
            receipt->fail_closed = 1;
            return 0;
        }
        void *rec = req->get_address_of_record(req->ctx, (uint16_t)creature);
        if (rec) {
            /* c_ai.cpp:322 — word_at(record, 2) is possession chain head */
            uint16_t poss_w00 = ((uint8_t *)rec)[2] |
                                (uint16_t)(((uint8_t *)rec)[3] << 8);
            /* c_ai.cpp:323-326 */
            int16_t can = req->can_handle_item(req->ctx, spell_type,
                poss_w00, (uint8_t)rg2);
            if (can != (int16_t)0xFFFE)
                receipt->result = -2;
        }
    }

    return 1;
}

/* ================================================================== */
/* XACT 64 — Shoot missile (c_ai.cpp:333-361)                        */
/* ================================================================== */

int dm2_v1_xact_shoot_missile(
    const DM2_V1_XactShootMissileRequest *req,
    DM2_V1_XactShootMissileReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;
    receipt->result = -3;

    /* c_ai.cpp:338 — if no possession, fail */
    if (req->possession_w00 == (uint16_t)0xFFFE)
        return 1;

    /* c_ai.cpp:340-341 — check v1e057c bit 3 */
    if ((req->v1e057c & 0x8) == 0)
        return 1;

    /* c_ai.cpp:345-346 — resolve item type */
    int16_t item_type = req->v1e0572;
    if (item_type == -1)
        item_type = 63;

    /* c_ai.cpp:348 — CREATURE_CAN_HANDLE_ITEM_IN check */
    if (!req->can_handle_item) {
        receipt->fail_closed = 1;
        return 0;
    }

    if (req->can_handle_item(req->ctx, item_type,
            req->possession_w00, 0xFF) == -2)
        return 1;  /* cannot handle = fail */

    /* c_ai.cpp:350-355 — fire missile via 19f0_2165.
     * Requires live launcher. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 65 — Flee (c_ai.cpp:364-398)                                 */
/* ================================================================== */

int dm2_v1_xact_flee(
    const DM2_V1_XactFleeRequest *req,
    DM2_V1_XactFleeReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:370-373 — compute 2 tiles ahead in facing direction */
    int16_t dir = req->direction;
    int16_t check_x = 2 * dm2_v1_dir_delta_x[dir] + req->pos_x;
    int16_t check_y = 2 * dm2_v1_dir_delta_y[dir] + req->pos_y;

    if (!req->get_creature_at) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:374-383 — check for creature 2 tiles ahead */
    int16_t creature = req->get_creature_at(req->ctx,
        check_x, check_y, 0);

    int blocking = 0;
    if (creature != -1) {
        if (req->query_ai_spec_flags) {
            uint16_t flags = req->query_ai_spec_flags(req->ctx,
                (uint16_t)creature);
            if ((flags & 1) != 0)
                blocking = 1;
        }
    }

    if (!blocking) {
        /* c_ai.cpp:388 — check if party is at the target location */
        if (req->party_map != 0 /* same map check */ ||
            check_x != req->party_x || check_y != req->party_y) {
            /* c_ai.cpp:391-392 — blocked, set b_1a = 29 */
            receipt->result = -4;
            receipt->b_1a = 29;
            return 1;
        }
    }

    /* c_ai.cpp:396-397 — can flee */
    receipt->result = -2;
    return 1;
}

/* ================================================================== */
/* XACT 66 — Attack ranged (c_ai.cpp:519-568)                        */
/* ================================================================== */

int dm2_v1_xact_attack_ranged(
    const DM2_V1_XactAttackRangedRequest *req,
    DM2_V1_XactAttackRangedReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:519-568 — complex attack with creature search,
     * spell fallback, and counter management.
     * Requires 14cd_2662, PROCEED_XACT_63. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 67 — Combat evaluation (c_ai.cpp:571-742)                    */
/* ================================================================== */

int dm2_v1_xact_combat_evaluation(
    const DM2_V1_XactCombatEvaluationRequest *req,
    DM2_V1_XactCombatEvaluationReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:571-742 — extremely complex combat evaluation.
     * Requires 14cd_2662, GET_CREATURE_AT, 14cd_2886,
     * query_48ae_0767, RAND16, RANDDIR, MIN, MAX.
     * Fail-closed until bound to live dungeon data. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 68 — Guard position (c_ai.cpp:745-825)                       */
/* ================================================================== */

int dm2_v1_xact_guard_position(
    const DM2_V1_XactGuardPositionRequest *req,
    DM2_V1_XactGuardPositionReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:745-825 — guard position evaluation.
     * Requires 14cd_2662, GET_CREATURE_AT, 14cd_2886,
     * query_48ae_0767. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 69 — Emit sound (c_ai.cpp:828-840)                           */
/* Fully implemented — pure arithmetic on creature state fields.      */
/* ================================================================== */

int dm2_v1_xact_emit_sound(
    const DM2_V1_XactEmitSoundRequest *req,
    DM2_V1_XactEmitSoundReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    int16_t dir = req->direction;
    if (dir < 0 || dir > 3) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:833 — target X: (pos_x + dir_dx[dir]) & 0x1f */
    receipt->target_x = (req->pos_x + dm2_v1_dir_delta_x[dir]) & 0x1F;

    /* c_ai.cpp:837 — target Y: (pos_y + dir_dy[dir]) & 0x1f */
    receipt->target_y = (req->pos_y + dm2_v1_dir_delta_y[dir]) & 0x1F;

    /* c_ai.cpp:838 — b_1d = CUTX8(v1e0572) */
    receipt->b_1d = (int8_t)(req->v1e0572 & 0xFF);

    /* c_ai.cpp:839 — b_1a = (b_1d == 1 ? 1 : 0) + 21 */
    receipt->b_1a = (receipt->b_1d == 1 ? 1 : 0) + 21;

    return 1;
}

/* ================================================================== */
/* XACT 70 — Face party (c_ai.cpp:843-894)                           */
/* ================================================================== */

int dm2_v1_xact_face_party(
    const DM2_V1_XactFacePartyRequest *req,
    DM2_V1_XactFacePartyReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;
    receipt->result = -3;

    /* c_ai.cpp:843-894 — face party with creature lookup.
     * Requires GET_CREATURE_AT, CREATURE_CAN_HANDLE_ITEM_IN,
     * GET_ADDRESS_OF_RECORD. Fail-closed for the creature check. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 71 — Patrol (c_ai.cpp:897-946)                               */
/* ================================================================== */

int dm2_v1_xact_patrol(
    const DM2_V1_XactPatrolRequest *req,
    DM2_V1_XactPatrolReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:897-946 — patrol logic requires possession chain
     * walking, 1c9a_078b, CREATURE_CAN_HANDLE_ITEM_IN, 19f0_2165.
     * Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 73 — Manage flags (c_ai.cpp:958-1064)                        */
/* ================================================================== */

int dm2_v1_xact_manage_flags(
    const DM2_V1_XactManageFlagsRequest *req,
    DM2_V1_XactManageFlagsReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:965-966 — check v1e0574 range */
    if ((uint16_t)req->v1e0574 < 3 ||
        ((uint16_t)req->v1e0574 >= 0x10 && (uint16_t)req->v1e0574 <= 0x12)) {
        /* c_ai.cpp:1026-1056 — simple flag set/clear/test */
        int flag2 = (req->v1e0574 & 0x10) == 0;
        int16_t op = req->v1e0574 & 0x0F;
        int16_t rg2 = 1 << (req->v1e0572 & 0xFF);
        int16_t old_w_0a = req->v1e054e_w0a;
        int flag1 = ((old_w_0a & rg2) == rg2) ? 1 : 0;

        int16_t new_w_0a = old_w_0a;
        if (op == 0)
            new_w_0a = old_w_0a & ~rg2;
        else if (op == 1)
            new_w_0a = old_w_0a | rg2;

        receipt->w_0a = new_w_0a;

        if (flag2 && new_w_0a != old_w_0a)
            receipt->b_1a = 51;

        /* c_ai.cpp:1059-1060 — result = (flag1 ? 1 : 0) - 3 */
        receipt->result = (int16_t)((flag1 ? 1 : 0) - 3);
        return 1;
    }

    if ((uint16_t)req->v1e0574 <= 4) {
        /* c_ai.cpp:973-1009 — hex entry flag management.
         * Requires xp_0a (AI spec hex entries). Fail-closed. */
        receipt->fail_closed = 1;
        return 1;
    }

    /* c_ai.cpp:1020-1022 — out of range */
    receipt->result = -3;
    return 1;
}

/* ================================================================== */
/* XACT 74 — Navigate (c_ai.cpp:1067-1174)                           */
/* ================================================================== */

int dm2_v1_xact_navigate(
    const DM2_V1_XactNavigateRequest *req,
    DM2_V1_XactNavigateReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1067-1174 — complex navigation with door opening,
     * pathfinding, and direction calculation.
     * Requires RAND, 1c9a_381c, CREATURE_GO_THERE,
     * CALC_VECTOR_DIR, 19f0_0559, RANDBIT. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 75 — Use item (c_ai.cpp:1470-1495)                           */
/* ================================================================== */

int dm2_v1_xact_use_item(
    const DM2_V1_XactUseItemRequest *req,
    DM2_V1_XactUseItemReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1470-1495 — use item from AI table.
     * Requires ai_14cd_10d2, 19f0_0891. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 77 — Follow (c_ai.cpp:1569-1608)                             */
/* ================================================================== */

int dm2_v1_xact_follow(
    const DM2_V1_XactFollowRequest *req,
    DM2_V1_XactFollowReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1569-1608 — follow via FIND_WALK_PATH.
     * Requires ai_14cd_0f3c, FIND_WALK_PATH. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 78 — Face direction (c_ai.cpp:1611-1626)                     */
/* ================================================================== */

int dm2_v1_xact_face_direction(
    const DM2_V1_XactFaceDirectionRequest *req,
    DM2_V1_XactFaceDirectionReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1615 — set v1e056f = -3 */
    receipt->v1e056f = -3;

    /* c_ai.cpp:1616-1617 — check if party on same map.
     * Requires CALC_VECTOR_DIR, map_0cee_04e5, 19f0_0559.
     * Fail-closed for the direction calculation. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 79 — Wander setup (c_ai.cpp:1629-1642)                       */
/* Fully implemented — pure random + arithmetic.                      */
/* ================================================================== */

int dm2_v1_xact_wander_setup(
    const DM2_V1_XactWanderSetupRequest *req,
    DM2_V1_XactWanderSetupReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1633 — rg3 = RANDBIT() ? 1 : 0
     * We use rand_b1e as the RANDBIT input */
    int8_t rg3 = (req->rand_bit & 1) ? 1 : 0;

    /* c_ai.cpp:1634 — b_1e = 0x82 */
    receipt->b_1e = (int8_t)0x82;

    /* c_ai.cpp:1635 — b_1a = (rg3 != 0 ? 1 : 0) + 39 */
    receipt->b_1a = (rg3 != 0 ? 1 : 0) + 39;

    /* c_ai.cpp:1636-1637 — b_1b = RANDDIR() (0-3) */
    receipt->b_1b = (int8_t)(req->rand_dir & 3);

    /* c_ai.cpp:1638-1639 — b_1c = (b_1b + rg3) & 3 */
    receipt->b_1c = (int8_t)((receipt->b_1b + rg3) & 0x3);

    /* c_ai.cpp:1641 — b_20 = 0 */
    receipt->b_20 = 0;

    return 1;
}

/* ================================================================== */
/* XACT 80 — Move flagged (c_ai.cpp:1645-1669)                       */
/* ================================================================== */

int dm2_v1_xact_move_flagged(
    const DM2_V1_XactMoveFlaggedRequest *req,
    DM2_V1_XactMoveFlaggedReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!req->go_there) {
        receipt->fail_closed = 1;
        return 0;
    }

    /* c_ai.cpp:1651-1656 — compute flags byte */
    int8_t flags = (req->v1e0572 != 0) ? 6 : 0;

    /* c_ai.cpp:1661-1662 — compute direction: (current + v1e0572) & 3 */
    int16_t move_dir = (req->direction + req->v1e0572) & 0x3;

    /* c_ai.cpp:1665-1666 — CREATURE_GO_THERE with modified flags */
    req->go_there(req->ctx,
        (uint16_t)flags, req->pos_x, req->pos_y, -1, -1, move_dir);

    /* c_ai.cpp:1668 — return v1e056f (set by go_there) */
    receipt->result = -3; /* default; real value from v1e056f */
    receipt->v1e056f = -3;
    return 1;
}

/* ================================================================== */
/* XACT 81 — Ranged attack (c_ai.cpp:1672-1676)                      */
/* ================================================================== */

int dm2_v1_xact_ranged_attack(
    const DM2_V1_XactRangedAttackRequest *req,
    DM2_V1_XactRangedAttackReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1674 — 19f0_2813 call. Requires live function. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 82 — Commerce (c_ai.cpp:1805-1936)                           */
/* ================================================================== */

int dm2_v1_xact_commerce(
    const DM2_V1_XactCommerceRequest *req,
    DM2_V1_XactCommerceReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1805-1936 — very complex commerce/trade.
     * Requires GET_CREATURE_AT, GET_ADDRESS_OF_RECORD, IS_CONTAINER_MONEYBOX,
     * 14cd_3582, query_48ae_0767, ALLOC_NEW_DBITEM, ADD_COIN_TO_WALLET,
     * APPEND_RECORD_TO, PROCEED_XACT_64. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 83 — Sleep check (c_ai.cpp:1939-1964)                        */
/* Mostly implemented — flag checks and arithmetic.                   */
/* ================================================================== */

int dm2_v1_xact_sleep_check(
    const DM2_V1_XactSleepCheckRequest *req,
    DM2_V1_XactSleepCheckReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1943-1946 — check w_0a bit 7 or v1e0572 != 0 */
    if ((req->v1e054e_w0a & 0x80) != 0 || req->v1e0572 != 0) {
        /* c_ai.cpp:1949 — b_1a = BETWEEN_VALUE(0, 2, v1e0572) + 0x23 */
        int16_t clamped = req->v1e0572;
        if (clamped < 0) clamped = 0;
        if (clamped > 2) clamped = 2;
        receipt->b_1a = (int8_t)(clamped + 0x23);

        receipt->result = -2;

        /* c_ai.cpp:1951-1957 — if w_0a bit 7 set and v1e0572 == 1, result = -4 */
        if ((req->v1e054e_w0a & 0x80) != 0) {
            if (req->v1e0572 == 1)
                receipt->result = -4;
        }
    } else {
        /* c_ai.cpp:1960-1961 — not sleeping */
        receipt->result = -3;
    }

    return 1;
}

/* ================================================================== */
/* XACT 84 — Consume item (c_ai.cpp:1967-2075)                       */
/* ================================================================== */

int dm2_v1_xact_consume_item(
    const DM2_V1_XactConsumeItemRequest *req,
    DM2_V1_XactConsumeItemReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1972-1974 — check possession exists */
    if (req->possession_w00 == (uint16_t)0xFFFE) {
        receipt->result = -3;
        return 1;
    }

    /* c_ai.cpp:1978-2075 — complex item consumption.
     * Requires GET_ADDRESS_OF_RECORD, QUERY_GDAT_FOOD_VALUE_FROM_RECORD,
     * CUT_RECORD_FROM, DEALLOC_RECORD, 19f0_2165, RAND. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* XACT 89 — Ranged special (c_ai.cpp:2129-2133)                     */
/* ================================================================== */

int dm2_v1_xact_ranged_special(
    const DM2_V1_XactRangedSpecialRequest *req,
    DM2_V1_XactRangedSpecialReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:2131-2132 — 19f0_0d10 call. Requires live function. Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}

/* ================================================================== */
/* Commerce helper: rebalance coin wallet (c_ai.cpp:1679-1802)        */
/* ================================================================== */

int dm2_v1_rebalance_coin_wallet(
    const DM2_V1_RebalanceCoinWalletRequest *req,
    DM2_V1_RebalanceCoinWalletReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!req) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_ai.cpp:1679-1802 — coin wallet rebalancing.
     * Requires COUNT_BY_COIN_TYPES, TAKE_COIN_FROM_WALLET,
     * DEALLOC_RECORD, ALLOC_NEW_DBITEM, ADD_COIN_TO_WALLET.
     * Fail-closed. */
    receipt->fail_closed = 1;
    return 1;
}
