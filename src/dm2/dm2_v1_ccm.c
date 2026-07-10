/*
 * dm2_v1_ccm.c - DM2 V1 CCM (Creature Command Machine) Implementation
 *
 * Phase 5 (creature/combat parity) source-lock.
 *
 * The CCM is the per-creature command interpreter.  Each tick, the
 * creature's CCM state executes one opcode (dm2_v1_ccm_step) and
 * the result drives the next state transition.
 *
 * This module implements a representative subset of CCM opcodes:
 *   28 opcodes (out of skproject's full ~200) are wired up:
 *     WALK_NOW, ATTACK_HANDLER, WALK_CONT, WALK_PATH, ROTATE_TO_TARGET,
 *     SPECIAL_ACTION, SPECIAL_06, SPECIAL_07, SPECIAL_08,
 *     STEAL_ITEM, MERCHANT_BEHAVIOR,
 *     PUTS_DOWN_ITEM, TAKES_ITEM, SHOOT_ITEM, KILL_ON_TIMER_POS, ROTATES_TARGET,
 *     CAST_SPELL, ROTATES_TARGET_16, CREATURE_ATTACKS_PARTY, ATTACK_DOOR,
 *     PUTS_DOWN_ITEM_19, TAKES_ITEM_1A, EXPLODE_OR_SUMMON
 *   All other opcodes return DM2_CCM_RESULT_UNKNOWN_OPCODE (documented
 *   stub for the remaining ~179 opcodes).
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_creature.cpp         - DM2_PROCEED_CCM
 *   skproject/SKULLWIN/c_ai.cpp              - DM2_THINK_CREATURE
 *   skproject/SKULLWIN/c_creature.cpp:130     - DM2_PROCEED_CCM dispatch
 *   ReDMCSB GROUP.C:1695-1770                 - F0207 creature attack
 *   ReDMCSB GROUP.C:2376-2387                 - F0209 visible row/col
 *   ReDMCSB PROJEXPL.C:76-92                  - F0212 projectile live
 *
 * V1 invariant: CCM execution NEVER mutates party state (HP/mana/food/
 * water/direction/position) directly.  All mutations go through the
 * creature AI + projectile dispatch path which has its own invariants.
 */

#include "dm2_v1_ccm.h"

#include <string.h>

/* ── Opcode descriptor table ─────────────────────────────────────── */
static const DM2_V1_CCMOpcodeDef g_opcode_table[DM2_CCM_MAX_OPCODES] = {
    { 0x00, "WALK_NOW",            0, 0 },
    { 0x01, "ATTACK_HANDLER",      1, 0 },
    { 0x02, "WALK_CONT",           0, 0 },
    { 0x03, "WALK_PATH",           0, 0 },
    { 0x04, "ROTATE_TO_TARGET",    1, 0 },
    { 0x05, "SPECIAL_ACTION",      1, 0 },  /* arg: sub-action 06/0B/0C */
    { 0x06, "SPECIAL_06",          0, 0 },
    { 0x09, "STEAL_ITEM",          1, 0 },  /* arg: target champion */
    { 0x0A, "MERCHANT_BEHAVIOR",   1, 0 },
    { 0x0B, "PUTS_DOWN_ITEM",      1, 0 },
    { 0x0C, "TAKES_ITEM",          1, 0 },
    { 0x0D, "SHOOT_ITEM",          2, 0 },  /* arg: item_id, direction */
    { 0x0F, "KILL_ON_TIMER_POS",   1, 0 },
    { 0x13, "ROTATES_TARGET",      1, 0 },  /* arg: target creature */
    { 0x15, "CAST_SPELL",          3, 0 },  /* arg: spell_id, target_x, target_y */
    { 0x17, "CREATURE_ATTACKS_PARTY", 0, 0 },
    { 0x26, "EXPLODE_OR_SUMMON",   1, 0 },
    /* Additional known CCM entries; stubbed rows remain explicit below. */
    { 0x07, "SPECIAL_07",          0, 0 },
    { 0x08, "SPECIAL_08",          0, 0 },
    { 0x0E, "SPECIAL_0E",          0, 1 },
    { 0x10, "PASSIVE_10",          0, 0 },
    { 0x11, "SPAWN_DEFERRED",      0, 0 },
    { 0x12, "PASSIVE_12",          0, 0 },
    { 0x14, "PASSIVE_14",          0, 0 },
    { 0x16, "ROTATES_TARGET_16",   0, 0 },
    { 0x18, "ATTACK_DOOR",         0, 0 },
    { 0x19, "PUTS_DOWN_ITEM_19",   0, 0 },
    { 0x1A, "TAKES_ITEM_1A",       0, 0 },
    { 0x1F, "SPECIAL_1F",          1, 1 },
    { 0x20, "SPECIAL_20",          1, 1 },
    { 0x21, "SPECIAL_21",          1, 1 },
    { 0xFF, "HALT",                0, 0 },
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
    /* Per-opcode behavior. */
    switch (opcode) {
        case DM2_CCM_OP_NOP:
            /* Movement dispatch. Set flag 0 = "moving". */
            state->flags[0] = 1;
            state->step_count++;
            state->last_step_tick_ms = now_ms;
            break;
        case DM2_CCM_OP_ATTACK_HANDLER:
            /* Delegate to creature attack pipeline. Set flag 1 = "attacking". */
            state->flags[1] = 1;
            state->target_id = (arg_count > 0) ? args[0] : 0;
            break;
        case DM2_CCM_OP_WALK_CONT:
            state->flags[0] = 1;
            state->step_count++;
            break;
        case DM2_CCM_OP_WALK_PATH:
            /* skproject/SKULLWIN/c_creature.cpp:1609 DM2_CREATURE_CCM03
             * continues the path state after selecting its walk action. */
            state->flags[11] = 1;
            state->next_state = DM2_CCM_OP_WALK_CONT;
            break;
        case DM2_CCM_OP_ROTATE_TO_TARGET:
            /* Orientation is written before the next think pass.  The
             * normalized direction is consumed by the live instance bridge. */
            state->target_id = args[0] & 3;
            state->flags[12] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_SPECIAL_ACTION:
            /* c_creature.cpp:1636 DM2_CREATURE_JUMPS advances through its
             * secondary phase before returning to the movement state. */
            state->flags[2] = (arg_count > 0) ? args[0] : 0;
            state->flags[13] = 1;
            state->next_state = DM2_CCM_OP_WALK_CONT;
            break;
        case DM2_CCM_OP_SPECIAL_06:
            /* c_creature.cpp:2969 groups 0x06/0x07 in CCM06. */
            state->flags[13] = 1;
            state->next_state = DM2_CCM_OP_WALK_CONT;
            break;
        case DM2_CCM_OP_SPECIAL_07:
            /* skproject/SKULLWIN/c_creature.cpp CCM06 family treats 0x07 as
             * the paired alternate special phase before returning to walk. */
            state->flags[13] = 1;
            state->next_state = DM2_CCM_OP_WALK_CONT;
            break;
        case DM2_CCM_OP_SPECIAL_08:
            /* skproject/SKULLWIN/c_creature.cpp routes 0x08 through the same
             * short special-state envelope, without direct party mutation. */
            state->flags[13] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_STEAL_ITEM:
            /* arg: target champion. */
            state->target_id = (arg_count > 0) ? args[0] : 0;
            state->flags[3] = 1;
            break;
        case DM2_CCM_OP_MERCHANT_BEHAVIOR:
            /* arg: shop_id. */
            state->flags[4] = (arg_count > 0) ? args[0] : 0;
            break;
        case DM2_CCM_OP_PUTS_DOWN_ITEM:
            /* c_creature.cpp:2284 DM2_CREATURE_PUTS_DOWN_ITEM. */
            if (!dm2_v1_ccm_stack_push(state, args[0])) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
            state->flags[14] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_TAKES_ITEM:
            /* c_creature.cpp:2176 DM2_CREATURE_TAKES_ITEM. */
            if (!dm2_v1_ccm_stack_push(state, args[0])) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
            state->flags[15] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_SHOOT_ITEM:
            /* arg: item_id, direction. */
            if (arg_count >= 2) {
                if (!dm2_v1_ccm_stack_push(state, args[0])) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
                if (!dm2_v1_ccm_stack_push(state, args[1])) return (int)DM2_CCM_RESULT_STACK_OVERFLOW;
            }
            state->flags[5] = 1;
            break;
        case DM2_CCM_OP_KILL_ON_TIMER_POS:
            state->flags[6] = (arg_count > 0) ? args[0] : 0;
            state->last_step_tick_ms = now_ms;
            break;
        case DM2_CCM_OP_PASSIVE_10:
        case DM2_CCM_OP_SPAWN_DEFERRED:
        case DM2_CCM_OP_PASSIVE_12:
        case DM2_CCM_OP_PASSIVE_14:
            /* skproject/SKWIN/SkWinCore.cpp PROCEED_CCM routes ccm10,
             * ccmSpawn, ccm12, and ccm14 to the shared ^15D5 break. They are
             * valid no-op/deferred states, not unknown opcodes. */
            state->flags[6] = opcode;
            state->last_step_tick_ms = now_ms;
            break;
        case DM2_CCM_OP_ROTATES_TARGET:
            state->target_id = (arg_count > 0) ? args[0] : 0;
            state->flags[7] = 1;
            break;
        case DM2_CCM_OP_ROTATES_TARGET_16:
            /* skproject PROCEED_CCM groups ccm15 and ccm16 through
             * CREATURE_ROTATES_TARGET_CREATURE. Firestaff keeps this as a
             * receipt/writeback request until the runtime target bridge owns
             * the concrete rotate mutation. */
            state->flags[7] = 1;
            break;
        case DM2_CCM_OP_CAST_SPELL:
            /* arg: spell_id, target_x, target_y. */
            if (arg_count >= 3) {
                state->target_x = args[1];
                state->target_y = args[2];
            }
            state->flags[8] = 1;
            break;
        case DM2_CCM_OP_CREATURE_ATTACKS_PARTY:
            state->flags[9] = 1;
            break;
        case DM2_CCM_OP_ATTACK_DOOR:
            /* skproject/SKULLWIN/c_creature.cpp door-attack CCM state only
             * marks the door-target phase here; door HP/effects stay owned
             * by the runtime door/projectile path. */
            state->flags[7] = 1;
            state->target_id = DM2_CCM_OP_ATTACK_DOOR;
            break;
        case DM2_CCM_OP_PUTS_DOWN_ITEM_19:
            /* skproject PROCEED_CCM routes ccm19 with the other put-item
             * states. Imported real byteprograms often carry only the command
             * byte, so this alias marks the item phase without requiring an
             * extra synthetic operand. */
            state->flags[14] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_TAKES_ITEM_1A:
            /* skproject PROCEED_CCM routes ccm1A through CREATURE_TAKES_ITEM. */
            state->flags[15] = 1;
            state->next_state = DM2_CCM_OP_WALK_NOW;
            break;
        case DM2_CCM_OP_EXPLODE_OR_SUMMON:
            state->flags[10] = 1;
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
    /* Simple "run" loop: dispatch next opcode at state->pc.
     * In a real implementation, the program would live in a per-creature
     * bytecode buffer.  Here we just step through known opcodes. */
    int opcode = state->pc;  /* pc acts as last-opcode for now */
    int rc = dispatch_opcode(state, opcode, NULL, 0, now_ms);
    state->last_result = rc;
    return rc;
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

/* ── Observability ──────────────────────────────────────────────── */
int dm2_v1_ccm_total_steps(void) { return s_total_steps; }
int dm2_v1_ccm_total_unknown(void) { return s_total_unknown; }
int dm2_v1_ccm_total_halted(void) { return s_total_halted; }

const char *dm2_v1_ccm_source_evidence(void) {
    return
        "DM2 V1 CCM (Creature Command Machine) parity - Phase 5 source-lock\n"
        "Source: skproject/SKULLWIN/c_creature.cpp         - DM2_PROCEED_CCM\n"
        "Source: skproject/SKULLWIN/c_ai.cpp              - DM2_THINK_CREATURE\n"
        "Source: skproject/SKULLWIN/c_creature.cpp:130     - DM2_PROCEED_CCM dispatch\n"
        "Source: ReDMCSB GROUP.C:1695-1770                 - F0207 creature attack\n"
        "Source: ReDMCSB GROUP.C:2376-2387                 - F0209 visible row/col\n"
        "Source: ReDMCSB PROJEXPL.C:76-92                  - F0212 projectile live\n"
        "Implemented opcodes (28 of ~200 in skproject):\n"
        "  0x00 WALK_NOW / 0x01 ATTACK_HANDLER / 0x02 WALK_CONT\n"
        "  0x03 WALK_PATH / 0x04 ROTATE_TO_TARGET / 0x05 SPECIAL_ACTION\n"
        "  0x06 SPECIAL_06 / 0x07 SPECIAL_07 / 0x08 SPECIAL_08\n"
        "  0x09 STEAL_ITEM / 0x0A MERCHANT_BEHAVIOR\n"
        "  0x0B PUTS_DOWN_ITEM / 0x0C TAKES_ITEM\n"
        "  0x0D SHOOT_ITEM / 0x0F KILL_ON_TIMER_POS\n"
        "  0x10 PASSIVE_10 / 0x11 SPAWN_DEFERRED / 0x12 PASSIVE_12\n"
        "  0x13 ROTATES_TARGET / 0x14 PASSIVE_14\n"
        "  0x15 CAST_SPELL / 0x16 ROTATES_TARGET_16\n"
        "  0x17 CREATURE_ATTACKS_PARTY / 0x18 ATTACK_DOOR\n"
        "  0x19 PUTS_DOWN_ITEM_19 / 0x1A TAKES_ITEM_1A\n"
        "  0x26 EXPLODE_OR_SUMMON / 0xFF HALT\n"
        "Stubbed opcodes (return DM2_CCM_RESULT_UNKNOWN_OPCODE):\n"
        "  0x0E/0x1F/0x20/0x21/0x25\n"
        "  + remaining ~179 opcodes in skproject/SKULLWIN/c_creature.cpp.\n"
        "V1 invariant: CCM NEVER mutates party state directly.\n";
}
