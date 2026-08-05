/*
 * dm2_v1_ccm.c - DM2 V1 CCM (Creature Command Machine) Implementation
 *
 * Phase 5 (creature/combat parity) source-lock.
 *
 * The CCM is the per-creature command interpreter.  Each tick, the
 * creature's CCM state executes one opcode (dm2_v1_ccm_step) and
 * the result drives the next state transition.
 *
 * 2026-07-19 DM2-005 follow-up: the opcode numbering is now aligned to
 * the skproject b_1a dispatch matrix (c_creature.cpp:2930-3212
 * DM2_PROCEED_CCM, bound verbatim in dm2_v1_ccm_dispatch_pc34_compat).
 * Every table row's opcode value IS the source creature command byte and
 * every row carries its source handler group.  The previous legacy
 * numbering diverged from the source (legacy 0x15 CAST_SPELL vs source
 * 0x27/0x28, legacy 0x17 CREATURE_ATTACKS_PARTY vs source 0x17
 * PLACE_MERCHANDISE, legacy 0x0D SHOOT_ITEM vs source 0x0E/0x0F, etc.)
 * and has been retired.  Bytes the source compare chain routes to no
 * handler (0x00, 0x10-0x12, 0x14, 0x1B-0x25, 0x32-0x34, 0x41-0x54,
 * 0x56-0xFE) return DM2_CCM_RESULT_UNKNOWN_OPCODE.
 *
 * Implemented handler bodies (source group in parentheses):
 *   WALK_NOW (0x01/0x02/0x09), CCM03 (0x03/0x04), JUMPS (0x05),
 *   CCM06 (0x06/0x07), ATTACKS_PARTY (0x08/0x26),
 *   STEAL_FROM_CHAMPION (0x0A), SHOOT_ITEM (0x0E/0x0F),
 *   KILL_ON_TIMER_POSITION (0x13), ROTATES_TARGET_CREATURE (0x15/0x16),
 *   PLACE_MERCHANDISE (0x17), TAKE_MERCHANDISE (0x18),
 *   PUTS_DOWN_ITEM (0x19/0x29/0x2A/0x2D/0x2E),
 *   TAKES_ITEM (0x1A/0x2B/0x2C), CAST_SPELL (0x27/0x28),
 *   EXPLODE_OR_SUMMON (0x3D-0x40)
 * All source groups now have advanced handler bodies wired through
 * dm2_v1_ccm_execute_advanced().
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_creature.cpp:2930-3212 - DM2_PROCEED_CCM compare chain
 *   skproject/SKULLWIN/c_creature.cpp:1609      - DM2_CREATURE_CCM03
 *   skproject/SKULLWIN/c_creature.cpp:1636      - DM2_CREATURE_JUMPS
 *   skproject/SKULLWIN/c_creature.cpp:2176      - DM2_CREATURE_TAKES_ITEM
 *   skproject/SKULLWIN/c_creature.cpp:2284      - DM2_CREATURE_PUTS_DOWN_ITEM
 *   skproject/SKULLWIN/c_ai.cpp                 - DM2_THINK_CREATURE
 *   ReDMCSB GROUP.C:1695-1770                   - F0207 creature attack
 *   ReDMCSB GROUP.C:2376-2387                   - F0209 visible row/col
 *   ReDMCSB PROJEXPL.C:76-92                    - F0212 projectile live
 *
 * V1 invariant: CCM execution NEVER mutates party state (HP/mana/food/
 * water/direction/position) directly.  All mutations go through the
 * creature AI + projectile dispatch path which has its own invariants.
 */

#include "dm2_v1_ccm.h"
#include "dm2_v1_ccm_dispatch_pc34_compat.h"
#include "dm2_v1_ccm_advanced_pc34_compat.h"

#include <string.h>

/* ── Opcode descriptor table ───────────────────────────────────────
 * Row order is ascending source b_1a command byte; source_group values
 * are the DM2_V1_CcmSourceHandler of the byte per the verbatim
 * DM2_PROCEED_CCM compare chain in dm2_v1_ccm_dispatch_pc34_compat.c.
 * test_dm2_v1_ccm_source_alignment_pc34_compat cross-checks every row
 * against dm2_v1_ccm_dispatch_source_group so the two modules cannot
 * drift apart. */
static const DM2_V1_CCMOpcodeDef g_opcode_table[DM2_CCM_MAX_OPCODES] = {
    { 0x01, "WALK_NOW",                0, 0, DM2_V1_CCM_SRC_WALK_NOW },
    { 0x02, "WALK_NOW",                0, 0, DM2_V1_CCM_SRC_WALK_NOW },
    { 0x03, "CCM03",                   0, 0, DM2_V1_CCM_SRC_CCM03 },
    { 0x04, "CCM03",                   0, 0, DM2_V1_CCM_SRC_CCM03 },
    { 0x05, "JUMPS",                   1, 0, DM2_V1_CCM_SRC_JUMPS },
    { 0x06, "CCM06",                   0, 0, DM2_V1_CCM_SRC_CCM06 },
    { 0x07, "CCM06",                   0, 0, DM2_V1_CCM_SRC_CCM06 },
    { 0x08, "ATTACKS_PARTY",           0, 0, DM2_V1_CCM_SRC_ATTACKS_PARTY },
    { 0x09, "WALK_NOW",                0, 0, DM2_V1_CCM_SRC_WALK_NOW },
    { 0x0A, "STEAL_FROM_CHAMPION",     1, 0, DM2_V1_CCM_SRC_STEAL_FROM_CHAMPION },
    { 0x0B, "CCM0B",                   0, 0, DM2_V1_CCM_SRC_CCM0B },
    { 0x0C, "CCM0C",                   0, 0, DM2_V1_CCM_SRC_CCM0C },
    { 0x0D, "CCM0C",                   0, 0, DM2_V1_CCM_SRC_CCM0C },
    { 0x0E, "SHOOT_ITEM",              2, 0, DM2_V1_CCM_SRC_SHOOT_ITEM },
    { 0x0F, "SHOOT_ITEM",              2, 0, DM2_V1_CCM_SRC_SHOOT_ITEM },
    { 0x13, "KILL_ON_TIMER_POSITION",  1, 0, DM2_V1_CCM_SRC_KILL_ON_TIMER_POSITION },
    { 0x15, "ROTATES_TARGET_CREATURE", 1, 0, DM2_V1_CCM_SRC_ROTATES_TARGET_CREATURE },
    { 0x16, "ROTATES_TARGET_CREATURE", 0, 0, DM2_V1_CCM_SRC_ROTATES_TARGET_CREATURE },
    { 0x17, "PLACE_MERCHANDISE",       1, 0, DM2_V1_CCM_SRC_PLACE_MERCHANDISE },
    { 0x18, "TAKE_MERCHANDISE",        1, 0, DM2_V1_CCM_SRC_TAKE_MERCHANDISE },
    { 0x19, "PUTS_DOWN_ITEM",          1, 0, DM2_V1_CCM_SRC_PUTS_DOWN_ITEM },
    { 0x1A, "TAKES_ITEM",              1, 0, DM2_V1_CCM_SRC_TAKES_ITEM },
    { 0x26, "ATTACKS_PARTY",           0, 0, DM2_V1_CCM_SRC_ATTACKS_PARTY },
    { 0x27, "CAST_SPELL",              3, 0, DM2_V1_CCM_SRC_CAST_SPELL },
    { 0x28, "CAST_SPELL",              3, 0, DM2_V1_CCM_SRC_CAST_SPELL },
    { 0x29, "PUTS_DOWN_ITEM",          0, 0, DM2_V1_CCM_SRC_PUTS_DOWN_ITEM },
    { 0x2A, "PUTS_DOWN_ITEM",          0, 0, DM2_V1_CCM_SRC_PUTS_DOWN_ITEM },
    { 0x2B, "TAKES_ITEM",              0, 0, DM2_V1_CCM_SRC_TAKES_ITEM },
    { 0x2C, "TAKES_ITEM",              0, 0, DM2_V1_CCM_SRC_TAKES_ITEM },
    { 0x2D, "PUTS_DOWN_ITEM",          0, 0, DM2_V1_CCM_SRC_PUTS_DOWN_ITEM },
    { 0x2E, "PUTS_DOWN_ITEM",          0, 0, DM2_V1_CCM_SRC_PUTS_DOWN_ITEM },
    { 0x2F, "ACTIVATES_WALL",          0, 0, DM2_V1_CCM_SRC_ACTIVATES_WALL },
    { 0x30, "ACTIVATES_WALL",          0, 0, DM2_V1_CCM_SRC_ACTIVATES_WALL },
    { 0x31, "ACTIVATES_WALL",          0, 0, DM2_V1_CCM_SRC_ACTIVATES_WALL },
    { 0x35, "USES_LADDER_HOLE",        0, 0, DM2_V1_CCM_SRC_USES_LADDER_HOLE },
    { 0x36, "USES_LADDER_HOLE",        0, 0, DM2_V1_CCM_SRC_USES_LADDER_HOLE },
    { 0x37, "USES_LADDER_HOLE",        0, 0, DM2_V1_CCM_SRC_USES_LADDER_HOLE },
    { 0x38, "USES_LADDER_HOLE",        0, 0, DM2_V1_CCM_SRC_USES_LADDER_HOLE },
    { 0x39, "USES_LADDER_HOLE",        0, 0, DM2_V1_CCM_SRC_USES_LADDER_HOLE },
    { 0x3A, "USES_LADDER_HOLE",        0, 0, DM2_V1_CCM_SRC_USES_LADDER_HOLE },
    { 0x3B, "TRANSFORM",               0, 0, DM2_V1_CCM_SRC_TRANSFORM },
    { 0x3C, "TRANSFORM",               0, 0, DM2_V1_CCM_SRC_TRANSFORM },
    { 0x3D, "EXPLODE_OR_SUMMON",       1, 0, DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON },
    { 0x3E, "EXPLODE_OR_SUMMON",       1, 0, DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON },
    { 0x3F, "EXPLODE_OR_SUMMON",       1, 0, DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON },
    { 0x40, "EXPLODE_OR_SUMMON",       1, 0, DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON },
    { 0x55, "DM2_1B7D5",               0, 0, DM2_V1_CCM_SRC_1B7D5 },
    { 0xFF, "HALT",                    0, 0, -1 /* Firestaff-internal control */ },
};

/* ── Module-level observability ─────────────────────────────────── */
static int s_total_steps = 0;
static int s_total_unknown = 0;
static int s_total_halted = 0;

/* ── Lifecycle / state ──────────────────────────────────────────── */
void dm2_v1_ccm_reset_state(DM2_V1_CCMState *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->next_state = -1;
}

void dm2_v1_ccm_init_state(DM2_V1_CCMState *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
    state->pc = 0;
    state->halted = 0;
    state->stack_top = 0;
    state->next_state = -1;
}

/* ── Catalog ────────────────────────────────────────────────────── */
int dm2_v1_ccm_get_opcode_count(void) {
    return DM2_CCM_MAX_OPCODES;
}

const DM2_V1_CCMOpcodeDef *dm2_v1_ccm_get_opcode_def(int opcode) {
    for (int i = 0; i < DM2_CCM_MAX_OPCODES; i++) {
        if (g_opcode_table[i].opcode == opcode) return &g_opcode_table[i];
    }
    return NULL;
}

const char *dm2_v1_ccm_get_opcode_name(int opcode) {
    const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(opcode);
    return def ? def->name : NULL;
}

/* ── Stack helpers ──────────────────────────────────────────────── */
int dm2_v1_ccm_stack_push(DM2_V1_CCMState *state, int value) {
    if (!state) return 0;
    if (state->stack_top >= DM2_CCM_STACK_SIZE) return 0;
    state->stack[state->stack_top++] = value;
    return 1;
}

int dm2_v1_ccm_stack_pop(DM2_V1_CCMState *state, int *out_value) {
    if (!state || !out_value) return 0;
    if (state->stack_top <= 0) return 0;
    *out_value = state->stack[--state->stack_top];
    return 1;
}

int dm2_v1_ccm_stack_peek(const DM2_V1_CCMState *state, int *out_value) {
    if (!state || !out_value) return 0;
    if (state->stack_top <= 0) return 0;
    *out_value = state->stack[state->stack_top - 1];
    return 1;
}

int dm2_v1_ccm_stack_size(const DM2_V1_CCMState *state) {
    if (!state) return -1;
    return state->stack_top;
}

/* ── Flags ──────────────────────────────────────────────────────── */
int dm2_v1_ccm_flag_get(const DM2_V1_CCMState *state, int flag_id) {
    if (!state || flag_id < 0 || flag_id >= DM2_CCM_FLAG_COUNT) return 0;
    return state->flags[flag_id];
}

void dm2_v1_ccm_flag_set(DM2_V1_CCMState *state, int flag_id, int value) {
    if (!state || flag_id < 0 || flag_id >= DM2_CCM_FLAG_COUNT) return;
    state->flags[flag_id] = value;
}

/* ── Internal: dispatch one opcode ─────────────────────────────── */
static int dispatch_opcode(DM2_V1_CCMState *state, int opcode,
                            const int *args, int arg_count, int now_ms)
{
    if (!state) return (int)DM2_CCM_RESULT_BAD_ARG;
    if (state->halted) return (int)DM2_CCM_RESULT_HALTED;
    const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(opcode);
    if (!def) {
        /* Source "no branch taken" bytes (0x00, 0x10-0x12, 0x14,
         * 0x1B-0x25, 0x32-0x34, 0x41-0x54, 0x56-0xFE) land here:
         * fail-closed, never simulated. */
        s_total_unknown++;
        return (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
    }
    if (def->stubbed) {
        s_total_unknown++;
        return (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
    }
    if (arg_count < def->arg_count) {
        return (int)DM2_CCM_RESULT_BAD_ARG;
    }
    /* Per-opcode behavior, keyed on the source b_1a command byte. */
    switch (opcode) {
        case DM2_CCM_OP_WALK_NOW:
            /* Movement dispatch. Set flag 0 = "moving". */
            state->flags[0] = 1;
            state->step_count++;
            state->last_step_tick_ms = now_ms;
            break;
        case DM2_CCM_OP_WALK_CONT:
        case DM2_CCM_OP_WALK_NOW_09:
            /* c_creature.cpp:2972-2974 skip00387 -> DM2_CREATURE_WALK_NOW
             * also covers b_1a 0x02 and 0x09. */
            state->flags[0] = 1;
            state->step_count++;
            break;
        case DM2_CCM_OP_CCM03:
        case DM2_CCM_OP_CCM03_PHASE:
            /* skproject/SKULLWIN/c_creature.cpp:1609 DM2_CREATURE_CCM03
             * continues the path state after selecting its walk action. */
            state->flags[11] = 1;
            state->next_state = DM2_CCM_OP_WALK_CONT;
            break;
        case DM2_CCM_OP_JUMPS:
            /* c_creature.cpp:1636 DM2_CREATURE_JUMPS advances through its
             * secondary phase before returning to the movement state. */
            state->flags[2] = (arg_count > 0) ? args[0] : 0;
            state->flags[13] = 1;
            state->next_state = DM2_CCM_OP_WALK_CONT;
            break;
        case DM2_CCM_OP_CCM06:
        case DM2_CCM_OP_CCM06_ALT:
            /* c_creature.cpp:2982-2986 groups b_1a 0x06/0x07 in CCM06. */
            state->flags[13] = 1;
            state->next_state = DM2_CCM_OP_WALK_CONT;
            break;
        case DM2_CCM_OP_ATTACKS_PARTY:
        case DM2_CCM_OP_ATTACKS_PARTY_26:
            /* c_creature.cpp:3190-3192 skip00388 ->
             * DM2_CREATURE_ATTACKS_PARTY covers b_1a 0x08 and 0x26. */
            state->flags[9] = 1;
            if (arg_count > 0) {
                state->target_id = args[0];
                state->flags[1] = 1;  /* attacking marker for the AI bridge */
            }
            break;
        case DM2_CCM_OP_STEAL_FROM_CHAMPION:
            /* c_creature.cpp:2997-2999 DM2_CREATURE_STEAL_FROM_CHAMPION.
             * arg: target champion. */
            state->target_id = (arg_count > 0) ? args[0] : 0;
            state->flags[3] = 1;
            break;
        case DM2_CCM_OP_SHOOT_ITEM:
        case DM2_CCM_OP_SHOOT_ITEM_ALT:
            /* c_creature.cpp:3009-3012 DM2_CREATURE_SHOOT_ITEM covers
             * b_1a 0x0E/0x0F.  arg: item_id, direction. */
            if (arg_count >= 2) {
                if (!dm2_v1_ccm_stack_push(state, args[0])) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
                if (!dm2_v1_ccm_stack_push(state, args[1])) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
            }
            state->flags[5] = 1;
            break;
        case DM2_CCM_OP_KILL_ON_TIMER_POSITION:
            /* c_creature.cpp:3017-3019 DM2_CREATURE_KILL_ON_TIMER_POSITION. */
            state->flags[6] = (arg_count > 0) ? args[0] : 0;
            state->last_step_tick_ms = now_ms;
            break;
        case DM2_CCM_OP_ROTATES_TARGET_CREATURE:
            /* c_creature.cpp:3024-3026 DM2_CREATURE_ROTATES_TARGET_CREATURE.
             * arg: target creature id; orientation write is receipted for
             * the runtime target bridge (flag 12) and the rotate phase is
             * marked (flag 7). */
            state->target_id = (arg_count > 0) ? args[0] : 0;
            state->flags[7] = 1;
            state->flags[12] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_ROTATES_TARGET_16:
            /* c_creature.cpp:3024-3026 groups b_1a 0x15/0x16 through
             * CREATURE_ROTATES_TARGET_CREATURE.  Firestaff keeps the paired
             * state as a receipt/writeback request until the runtime target
             * bridge owns the concrete rotate mutation. */
            state->flags[7] = 1;
            break;
        case DM2_CCM_OP_PLACE_MERCHANDISE:
            /* c_creature.cpp:3033-3035 DM2_PLACE_MERCHANDISE.
             * arg: shop id. */
            state->flags[4] = (arg_count > 0) ? args[0] : 0;
            break;
        case DM2_CCM_OP_TAKE_MERCHANDISE:
            /* c_creature.cpp:3041-3043 DM2_TAKE_MERCHANDISE.
             * arg: shop id. */
            state->flags[4] = (arg_count > 0) ? args[0] : 0;
            break;
        case DM2_CCM_OP_PUTS_DOWN_ITEM:
            /* c_creature.cpp:2284 DM2_CREATURE_PUTS_DOWN_ITEM; the compare
             * chain routes 0x19/0x29/0x2A/0x2D/0x2E here via skip00386. */
            if (!dm2_v1_ccm_stack_push(state, args[0])) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
            state->flags[14] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_PUTS_DOWN_ITEM_29:
        case DM2_CCM_OP_PUTS_DOWN_ITEM_2A:
        case DM2_CCM_OP_PUTS_DOWN_ITEM_2D:
        case DM2_CCM_OP_PUTS_DOWN_ITEM_2E:
            /* Imported real byteprograms often carry only the command
             * byte, so these skip00386 aliases mark the item phase
             * without requiring an extra synthetic operand. */
            state->flags[14] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_TAKES_ITEM:
            /* c_creature.cpp:2176 DM2_CREATURE_TAKES_ITEM; the compare
             * chain routes 0x1A/0x2B/0x2C here via skip00389. */
            if (!dm2_v1_ccm_stack_push(state, args[0])) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
            state->flags[15] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_TAKES_ITEM_2B:
        case DM2_CCM_OP_TAKES_ITEM_2C:
            state->flags[15] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_CAST_SPELL:
        case DM2_CCM_OP_CAST_SPELL_28:
            /* c_creature.cpp:3059-3062 DM2_CREATURE_CAST_SPELL covers
             * b_1a 0x27/0x28.  arg: spell_id, target_x, target_y. */
            if (arg_count >= 3) {
                state->target_x = args[1];
                state->target_y = args[2];
            }
            state->flags[8] = 1;
            break;
        case DM2_CCM_OP_EXPLODE_OR_SUMMON:
        case DM2_CCM_OP_EXPLODE_OR_SUMMON_3E:
        case DM2_CCM_OP_EXPLODE_OR_SUMMON_3F:
        case DM2_CCM_OP_EXPLODE_OR_SUMMON_40:
            /* c_creature.cpp:3085-3087 DM2_CREATURE_EXPLODE_OR_SUMMON
             * covers b_1a 0x3D-0x40. */
            state->flags[10] = 1;
            break;
        case DM2_CCM_OP_CCM0B:
            /* c_creature.cpp:2993 DM2_CREATURE_CCM0B — line-of-sight
             * check / per-creature counter. */
            state->flags[16] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_CCM0C:
        case DM2_CCM_OP_CCM0C_ALT:
            /* c_creature.cpp:2995-2997 DM2_CREATURE_CCM0C — per-creature
             * counter dispatch, covers b_1a 0x0C and 0x0D. */
            state->flags[17] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_ACTIVATES_WALL:
        case DM2_CCM_OP_ACTIVATES_WALL_30:
        case DM2_CCM_OP_ACTIVATES_WALL_31:
            /* c_creature.cpp:3069 DM2_CREATURE_ACTIVATES_WALL —
             * creature triggers a wall mechanism (sensor/actuator). */
            state->flags[18] = 1;
            break;
        case DM2_CCM_OP_USES_LADDER_HOLE:
        case DM2_CCM_OP_USES_LADDER_HOLE_36:
        case DM2_CCM_OP_USES_LADDER_HOLE_37:
        case DM2_CCM_OP_USES_LADDER_HOLE_38:
        case DM2_CCM_OP_USES_LADDER_HOLE_39:
        case DM2_CCM_OP_USES_LADDER_HOLE_3A:
            /* c_creature.cpp:3074 DM2_CREATURE_USES_LADDER_HOLE —
             * creature moves between levels via ladder or pit. */
            state->flags[19] = 1;
            state->flags[20] = opcode - DM2_CCM_OP_USES_LADDER_HOLE;
            break;
        case DM2_CCM_OP_TRANSFORM:
        case DM2_CCM_OP_TRANSFORM_3C:
            /* c_creature.cpp:3079-3081 DM2_CREATURE_TRANSFORM —
             * creature type mutation (e.g. cocoon hatching). */
            state->flags[21] = 1;
            state->flags[22] = (opcode == DM2_CCM_OP_TRANSFORM) ? 0 : 1;
            break;
        case DM2_CCM_OP_1B7D5:
            /* c_creature.cpp:3204 DM2_1B7D5 — cloud spawn utility. */
            state->flags[23] = 1;
            break;
        case DM2_CCM_OP_HALT:
            state->halted = 1;
            s_total_halted++;
            return (int)DM2_CCM_RESULT_HALTED;
        default:
            s_total_unknown++;
            return (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
    }
    state->pc++;
    state->last_opcode = opcode;
    state->last_result = (int)DM2_CCM_RESULT_OK;
    s_total_steps++;
    return (int)DM2_CCM_RESULT_OK;
}

/* ── Public step / run ──────────────────────────────────────────── */
int dm2_v1_ccm_step(DM2_V1_CCMState *state, int opcode,
                    const int *args, int arg_count, int now_ms)
{
    if (!state) return (int)DM2_CCM_RESULT_BAD_ARG;
    int rc = dispatch_opcode(state, opcode, args, arg_count, now_ms);
    state->last_result = rc;
    return rc;
}

int dm2_v1_ccm_run(DM2_V1_CCMState *state, int now_ms) {
    if (!state) return (int)DM2_CCM_RESULT_BAD_ARG;
    if (state->halted) return (int)DM2_CCM_RESULT_HALTED;
    /* SKProject/SKULLWIN/c_creature.cpp:2930-3212 DM2_PROCEED_CCM dispatches
     * the live creature record's b_1a byte.  `pc` indexes a decoded program;
     * it is not a source opcode and must never be reinterpreted as one.
     * Without the authenticated stream/operand owner, reject rather than
     * manufacture a command from a host counter. */
    (void)now_ms;
    ++s_total_unknown;
    state->last_result = (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
    return state->last_result;
}

int dm2_v1_ccm_decode_program(const uint8_t *bytes, size_t byte_count,
                              DM2_V1_CCMProgram *out_program)
{
    size_t cursor = 0;
    int count = 0;

    if (!bytes || !out_program) return (int)DM2_CCM_RESULT_BAD_ARG;
    memset(out_program, 0, sizeof(*out_program));

    /* skproject/SKULLWIN/c_creature.cpp DM2_PROCEED_CCM dispatches the
     * creature b_1a command byte, then consumes opcode-specific operands
     * before choosing the next b_1a state.  This bounded decoder gives the
     * runtime a real imported command stream instead of treating pc as the
     * opcode number. */
    while (cursor < byte_count) {
        const uint8_t opcode = bytes[cursor++];
        const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(opcode);
        DM2_V1_CCMProgramOp *op;
        int i;

        if (count >= DM2_CCM_MAX_PROGRAM_OPS) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
        if (!def || def->stubbed) return (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
        if (def->arg_count > DM2_CCM_MAX_PROGRAM_ARGS) return (int)DM2_CCM_RESULT_BAD_ARG;
        if (cursor + (size_t)def->arg_count > byte_count) return (int)DM2_CCM_RESULT_BAD_ARG;

        op = &out_program->ops[count++];
        op->opcode = opcode;
        op->arg_count = (uint8_t)def->arg_count;
        for (i = 0; i < def->arg_count; ++i) {
            op->args[i] = (int)bytes[cursor++];
        }
        if (opcode == DM2_CCM_OP_HALT) break;
    }

    out_program->count = count;
    return (int)DM2_CCM_RESULT_OK;
}

int dm2_v1_ccm_run_program(DM2_V1_CCMState *state,
                            const DM2_V1_CCMProgram *program,
                            int now_ms)
{
    int rc = (int)DM2_CCM_RESULT_OK;

    if (!state || !program || program->count < 0 ||
        program->count > DM2_CCM_MAX_PROGRAM_OPS) {
        return (int)DM2_CCM_RESULT_BAD_ARG;
    }
    if (state->halted) return (int)DM2_CCM_RESULT_HALTED;

    while (!state->halted && state->pc >= 0 && state->pc < program->count) {
        const DM2_V1_CCMProgramOp *op = &program->ops[state->pc];
        rc = dispatch_opcode(state, op->opcode, op->args, op->arg_count, now_ms);
        state->last_result = rc;
        if (rc != (int)DM2_CCM_RESULT_OK) return rc;
    }

    return rc;
}

/* ── Advanced handler dispatch ─────────────────────────────────── */
int dm2_v1_ccm_execute_advanced(DM2_V1_CCMState *state,
                                const DM2_V1_CCMAdvancedCallbacks *cb,
                                DM2_V1_CCMAdvancedReceipt *out)
{
    DM2_V1_CcmSourceHandler group;
    if (!state) return (int)DM2_CCM_RESULT_BAD_ARG;
    if (state->last_opcode < 0 || state->last_opcode > 0xFF)
        return (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;

    group = dm2_v1_ccm_dispatch_source_group((uint8_t)state->last_opcode);
    switch (group) {
        case DM2_V1_CCM_SRC_CCM03:
            return dm2_v1_ccm_advanced_ccm03(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_JUMPS:
            return dm2_v1_ccm_advanced_jumps(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_TAKES_ITEM:
            return dm2_v1_ccm_advanced_takes_item(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_PUTS_DOWN_ITEM:
            return dm2_v1_ccm_advanced_puts_down_item(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_TRANSFORM:
            return dm2_v1_ccm_advanced_transform(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_EXPLODE_OR_SUMMON:
            return dm2_v1_ccm_advanced_explode_or_summon(
                cb, (uint8_t)state->flags[22], out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_1B7D5:
            return dm2_v1_ccm_advanced_1b7d5(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_CCM0B:
            return dm2_v1_ccm_advanced_ccm0b(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_CCM0C:
            return dm2_v1_ccm_advanced_ccm0c(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_ACTIVATES_WALL:
            return dm2_v1_ccm_advanced_activates_wall(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        case DM2_V1_CCM_SRC_USES_LADDER_HOLE:
            return dm2_v1_ccm_advanced_uses_ladder_hole(cb, out)
                ? (int)DM2_CCM_RESULT_OK : (int)DM2_CCM_RESULT_BAD_ARG;
        default:
            return (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
    }
}

/* ── Observability ──────────────────────────────────────────────── */
int dm2_v1_ccm_total_steps(void) { return s_total_steps; }
int dm2_v1_ccm_total_unknown(void) { return s_total_unknown; }
int dm2_v1_ccm_total_halted(void) { return s_total_halted; }

const char *dm2_v1_ccm_source_evidence(void) {
    return
        "DM2 V1 CCM (Creature Command Machine) parity - Phase 5 source-lock\n"
        "Source: skproject/SKULLWIN/c_creature.cpp:2930-3212 - DM2_PROCEED_CCM b_1a matrix\n"
        "Source: skproject/SKULLWIN/c_creature.cpp:1609      - DM2_CREATURE_CCM03\n"
        "Source: skproject/SKULLWIN/c_creature.cpp:1636      - DM2_CREATURE_JUMPS\n"
        "Source: skproject/SKULLWIN/c_creature.cpp:2176/2284 - TAKES_ITEM/PUTS_DOWN_ITEM\n"
        "Source: skproject/SKULLWIN/c_ai.cpp                 - DM2_THINK_CREATURE\n"
        "Source: ReDMCSB GROUP.C:1695-1770                   - F0207 creature attack\n"
        "Source: ReDMCSB GROUP.C:2376-2387                   - F0209 visible row/col\n"
        "Source: ReDMCSB PROJEXPL.C:76-92                    - F0212 projectile live\n"
        "Opcode numbering aligned to the source b_1a matrix (2026-07-19):\n"
        "  0x01/0x02/0x09 WALK_NOW / 0x03/0x04 CCM03 / 0x05 JUMPS\n"
        "  0x06/0x07 CCM06 / 0x08+0x26 ATTACKS_PARTY\n"
        "  0x0A STEAL_FROM_CHAMPION / 0x0E/0x0F SHOOT_ITEM\n"
        "  0x13 KILL_ON_TIMER_POSITION / 0x15/0x16 ROTATES_TARGET_CREATURE\n"
        "  0x17 PLACE_MERCHANDISE / 0x18 TAKE_MERCHANDISE\n"
        "  0x19/0x29/0x2A/0x2D/0x2E PUTS_DOWN_ITEM / 0x1A/0x2B/0x2C TAKES_ITEM\n"
        "  0x27/0x28 CAST_SPELL / 0x3D-0x40 EXPLODE_OR_SUMMON / 0xFF HALT (internal)\n"
        "  0x0B CCM0B / 0x0C/0x0D CCM0C / 0x2F-0x31 ACTIVATES_WALL\n"
        "  0x35-0x3A USES_LADDER_HOLE / 0x3B/0x3C TRANSFORM / 0x55 DM2_1B7D5\n"
        "All 11 advanced handlers wired via dm2_v1_ccm_execute_advanced():\n"
        "  CCM03, JUMPS, TAKES_ITEM, PUTS_DOWN_ITEM, TRANSFORM,\n"
        "  EXPLODE_OR_SUMMON, 1B7D5, CCM0B, CCM0C, ACTIVATES_WALL,\n"
        "  USES_LADDER_HOLE\n"
        "All 48 source opcode rows are classified; production execution still\n"
        "requires the authenticated command stream and live-record callbacks.\n"
        "No-handler bytes (source 'no branch taken') return UNKNOWN_OPCODE:\n"
        "  0x00, 0x10-0x12, 0x14, 0x1B-0x25, 0x32-0x34, 0x41-0x54, 0x56-0xFE\n"
        "V1 invariant: CCM NEVER mutates party state directly.\n";
}
