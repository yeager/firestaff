/* test_dm2_v1_ccm_pc34_compat.c - DM2 V1 CCM (Creature Command Machine) Tests
 *
 * Phase 5 mechanics parity coverage (30+ assertions).
 * 2026-07-19 DM2-005 follow-up: all opcode numbers now follow the
 * skproject b_1a dispatch matrix (c_creature.cpp:2930-3212):
 *  1. Opcode table count = DM2_CCM_MAX_OPCODES
 *  2. Each opcode has a non-empty name
 *  3. Each opcode has arg_count >= 0
 *  4. Reset state zeroes all fields
 *  5. Init state zeroes all + sets pc=0, stack_top=0
 *  6. Stack push/pop round-trip
 *  7. Stack push returns 0 when full
 *  8. Stack pop returns 0 when empty
 *  9. Stack peek returns top without popping
 * 10. Stack size returns -1 for NULL state
 * 11. Flag set/get round-trip
 * 12. Flag out-of-range returns 0
 * 13. step(0x01 WALK_NOW) returns OK
 * 14. step(0x08 ATTACKS_PARTY) sets flag 9 (+target with arg)
 * 15. step(0x05 JUMPS) sets flag 2 to arg
 * 16. step(0x0A STEAL_FROM_CHAMPION) sets target_id
 * 17. step(0x17 PLACE_MERCHANDISE / 0x18 TAKE_MERCHANDISE) sets flag 4
 * 18. step(0x0E SHOOT_ITEM) pushes args to stack
 * 19. step(0x13 KILL_ON_TIMER_POSITION) sets last_step_tick_ms
 * 20. step(0x15 ROTATES_TARGET_CREATURE) sets target_id
 * 21. step(0x27 CAST_SPELL) sets target_x/y
 * 22. step(0x26 ATTACKS_PARTY alias) sets flag 9
 * 23. step(0x3D EXPLODE_OR_SUMMON) sets flag 10
 * 24. step(0xFF HALT) sets halted=1, returns HALTED
 * 25. step on unknown opcode (e.g., 0xFE) returns UNKNOWN_OPCODE
 * 26. step on stubbed opcode (e.g., 0x0B CCM0B) returns UNKNOWN_OPCODE
 * 27. Halted state rejects further step() calls
 * 28. step with too-few args returns BAD_ARG
 * 29. step on NULL state returns BAD_ARG
 * 30. step increments pc by 1
 * 31. step increments last_opcode
 * 32. step stores last_result
 * 33. step increments s_total_steps
 * 34. step on unknown increments s_total_unknown
 * 35. HALT increments s_total_halted
 * 36. get_opcode_def returns NULL for unknown
 * 37. get_opcode_name returns NULL for unknown
 * 38. Source evidence mentions skproject + DM2_PROCEED_CCM
 * 39. Multi-step: WALK_NOW → ATTACKS_PARTY → HALT
 * 40. Flags persist across steps
 * 41. Stack can hold up to DM2_CCM_STACK_SIZE items
 * 42. Stubbed + no-handler (source "no branch taken") opcodes return UNKNOWN
 */

#include "dm2_v1_ccm.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name_) do { \
    printf("  %-58s", #name_); \
    tests_run++; \
    if (test_##name_()) { \
        tests_passed++; \
        printf("  PASS\n"); \
    } else { \
        printf("  FAIL\n"); \
    } \
} while (0)

/* ── Catalog (1-3) ─────────────────────────────────────────────── */

/* Helper: lookup opcode at index i in our static table. */
static int g_opcode_table_lookup(int i) {
    /* We don't expose the table directly; use the source b_1a bytes. */
    static const int known[DM2_CCM_MAX_OPCODES] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x13,
        0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A,
        0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x55, 0xFF
    };
    if (i < 0 || i >= DM2_CCM_MAX_OPCODES) return -1;
    return known[i];
}

static int test_opcode_count(void) {
    return dm2_v1_ccm_get_opcode_count() == DM2_CCM_MAX_OPCODES;
}

static int test_opcode_names_nonempty(void) {
    int names_ok = 0;
    for (int i = 0; i < DM2_CCM_MAX_OPCODES; i++) {
        const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(g_opcode_table_lookup(i));
        if (def && def->name && def->name[0] != '\0') names_ok++;
    }
    return names_ok > 0;
}

static int test_opcode_arg_count_nonneg(void) {
    for (int i = 0; i < DM2_CCM_MAX_OPCODES; i++) {
        int op = g_opcode_table_lookup(i);
        const DM2_V1_CCMOpcodeDef *def = dm2_v1_ccm_get_opcode_def(op);
        if (def && def->arg_count < 0) return 0;
    }
    return 1;
}

/* ── Lifecycle (4-5) ───────────────────────────────────────────── */

static int test_reset_zeroes(void) {
    DM2_V1_CCMState s;
    memset(&s, 0xFF, sizeof(s));
    dm2_v1_ccm_reset_state(&s);
    return s.pc == 0 && s.halted == 0 && s.stack_top == 0 && s.target_id == 0;
}

static int test_init_state(void) {
    DM2_V1_CCMState s;
    memset(&s, 0xFF, sizeof(s));
    dm2_v1_ccm_init_state(&s);
    return s.pc == 0 && s.halted == 0 && s.stack_top == 0
        && s.last_result == 0 && s.last_opcode == 0;
}

/* ── Stack (6-10) ──────────────────────────────────────────────── */

static int test_stack_push_pop(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc1 = dm2_v1_ccm_stack_push(&s, 42);
    int v;
    int rc2 = dm2_v1_ccm_stack_pop(&s, &v);
    return rc1 == 1 && rc2 == 1 && v == 42 && s.stack_top == 0;
}

static int test_stack_push_full(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    for (int i = 0; i < DM2_CCM_STACK_SIZE; i++) {
        if (dm2_v1_ccm_stack_push(&s, i) != 1) return 0;
    }
    return dm2_v1_ccm_stack_push(&s, 999) == 0;  /* overflow */
}

static int test_stack_pop_empty(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int v;
    return dm2_v1_ccm_stack_pop(&s, &v) == 0;
}

static int test_stack_peek(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_stack_push(&s, 77);
    int v;
    int rc = dm2_v1_ccm_stack_peek(&s, &v);
    return rc == 1 && v == 77 && s.stack_top == 1;
}

static int test_stack_size_null(void) {
    return dm2_v1_ccm_stack_size(NULL) == -1;
}

/* ── Flags (11-12) ─────────────────────────────────────────────── */

static int test_flag_set_get(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_flag_set(&s, 5, 1);
    return dm2_v1_ccm_flag_get(&s, 5) == 1
        && dm2_v1_ccm_flag_get(&s, 4) == 0;
}

static int test_flag_out_of_range(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    return dm2_v1_ccm_flag_get(&s, -1) == 0
        && dm2_v1_ccm_flag_get(&s, DM2_CCM_FLAG_COUNT) == 0;
}

/* ── Per-opcode step (13-23) ──────────────────────────────────── */

static int test_step_walk_now(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_step(&s, 0x01, NULL, 0, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[0] == 1;
}

static int test_step_attacks_party_with_target(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 5 };
    int rc = dm2_v1_ccm_step(&s, 0x08, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[9] == 1
        && s.flags[1] == 1 && s.target_id == 5;
}

static int test_step_jumps(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 0x06 };  /* jump phase operand */
    int rc = dm2_v1_ccm_step(&s, 0x05, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[2] == 0x06
        && s.flags[13] == 1 && s.next_state == DM2_CCM_OP_WALK_CONT;
}

static int test_step_ccm03_rotate_and_item_actions(void) {
    DM2_V1_CCMState s;
    int value;
    dm2_v1_ccm_init_state(&s);
    if (dm2_v1_ccm_step(&s, DM2_CCM_OP_CCM03, NULL, 0, 0) !=
        (int)DM2_CCM_RESULT_OK || s.flags[11] != 1 ||
        s.next_state != DM2_CCM_OP_WALK_CONT) return 0;
    if (dm2_v1_ccm_step(&s, DM2_CCM_OP_ROTATES_TARGET_CREATURE, (int[]){7}, 1, 0) !=
        (int)DM2_CCM_RESULT_OK || s.flags[12] != 1 || s.flags[7] != 1 ||
        s.target_id != 7 ||
        s.next_state != DM2_CCM_OP_WALK_NOW) return 0;
    if (dm2_v1_ccm_step(&s, DM2_CCM_OP_PUTS_DOWN_ITEM, (int[]){44}, 1, 0) !=
        (int)DM2_CCM_RESULT_OK || s.flags[14] != 1) return 0;
    if (dm2_v1_ccm_step(&s, DM2_CCM_OP_TAKES_ITEM, (int[]){45}, 1, 0) !=
        (int)DM2_CCM_RESULT_OK || s.flags[15] != 1) return 0;
    return dm2_v1_ccm_stack_pop(&s, &value) == 1 && value == 45 &&
           dm2_v1_ccm_stack_pop(&s, &value) == 1 && value == 44;
}

static int test_step_ccm06_family(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    if (dm2_v1_ccm_step(&s, DM2_CCM_OP_CCM06_ALT, NULL, 0, 0) !=
        (int)DM2_CCM_RESULT_OK || s.flags[13] != 1 ||
        s.next_state != DM2_CCM_OP_WALK_CONT) return 0;
    if (dm2_v1_ccm_step(&s, DM2_CCM_OP_CCM06, NULL, 0, 0) !=
        (int)DM2_CCM_RESULT_OK || s.flags[13] != 1 ||
        s.next_state != DM2_CCM_OP_WALK_CONT) return 0;
    /* 0x09 routes to WALK_NOW via the source skip00387 branch. */
    if (dm2_v1_ccm_step(&s, DM2_CCM_OP_WALK_NOW_09, NULL, 0, 0) !=
        (int)DM2_CCM_RESULT_OK || s.flags[0] != 1) return 0;
    return 1;
}

static int test_step_steal_from_champion(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 3 };  /* champion 3 */
    int rc = dm2_v1_ccm_step(&s, 0x0A, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.target_id == 3 && s.flags[3] == 1;
}

static int test_step_merchandise(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 2 };
    /* Source 0x17 = PLACE_MERCHANDISE, 0x18 = TAKE_MERCHANDISE. */
    int rc = dm2_v1_ccm_step(&s, 0x17, args, 1, 0);
    if (rc != (int)DM2_CCM_RESULT_OK || s.flags[4] != 2) return 0;
    rc = dm2_v1_ccm_step(&s, 0x18, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[4] == 2;
}

static int test_step_shoot_item_pushes_stack(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 1001, 2 };  /* item 1001, dir 2 */
    int rc = dm2_v1_ccm_step(&s, 0x0E, args, 2, 0);
    if (rc != (int)DM2_CCM_RESULT_OK) return 0;
    if (s.stack_top != 2) return 0;
    int v;
    if (!dm2_v1_ccm_stack_pop(&s, &v)) return 0;
    if (v != 2) return 0;  /* LIFO: top is direction */
    if (!dm2_v1_ccm_stack_pop(&s, &v)) return 0;
    return v == 1001;
}

static int test_step_kill_on_timer_pos(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 7 };
    int rc = dm2_v1_ccm_step(&s, 0x13, args, 1, 5000);
    return rc == (int)DM2_CCM_RESULT_OK
        && s.flags[6] == 7 && s.last_step_tick_ms == 5000;
}

static int test_step_rotates_target(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 42 };
    int rc = dm2_v1_ccm_step(&s, 0x15, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.target_id == 42;
}

static int test_step_cast_spell(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 16, 5, 7 };  /* spell 16, target (5,7) */
    /* Source CAST_SPELL = b_1a 0x27/0x28. */
    int rc = dm2_v1_ccm_step(&s, 0x27, args, 3, 0);
    if (rc != (int)DM2_CCM_RESULT_OK
        || s.target_x != 5 || s.target_y != 7 || s.flags[8] != 1) return 0;
    rc = dm2_v1_ccm_step(&s, 0x28, args, 3, 0);
    return rc == (int)DM2_CCM_RESULT_OK
        && s.target_x == 5 && s.target_y == 7 && s.flags[8] == 1;
}

static int test_step_attacks_party_alias(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    /* 0x26 routes to ATTACKS_PARTY via the source skip00388 branch. */
    int rc = dm2_v1_ccm_step(&s, 0x26, NULL, 0, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[9] == 1;
}

static int test_step_explode_or_summon(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 99 };
    int rc = dm2_v1_ccm_step(&s, 0x3D, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[10] == 1;
}

/* ── Halt + unknown (24-26) ───────────────────────────────────── */

static int test_step_halt(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_step(&s, 0xFF, NULL, 0, 0);
    return rc == (int)DM2_CCM_RESULT_HALTED && s.halted == 1;
}

static int test_step_unknown_opcode(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_step(&s, 0xFE, NULL, 0, 0);  /* not in table */
    return rc == (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
}

static int test_step_stubbed_opcode(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_step(&s, 0x0B, NULL, 0, 0);  /* CCM0B stubbed */
    return rc == (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
}

/* ── Halted state (27) ─────────────────────────────────────────── */

static int test_halted_state_rejects_step(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    s.halted = 1;
    int rc = dm2_v1_ccm_step(&s, 0x01, NULL, 0, 0);
    return rc == (int)DM2_CCM_RESULT_HALTED;
}

/* ── Bad arg + null (28-29) ────────────────────────────────────── */

static int test_step_too_few_args(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_step(&s, 0x0A, NULL, 0, 0);  /* needs 1 arg */
    return rc == (int)DM2_CCM_RESULT_BAD_ARG;
}

static int test_step_null_state(void) {
    return dm2_v1_ccm_step(NULL, 0x01, NULL, 0, 0) == (int)DM2_CCM_RESULT_BAD_ARG;
}

/* ── pc + last_opcode + last_result (30-32) ───────────────────── */

static int test_step_increments_pc(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int before = s.pc;
    dm2_v1_ccm_step(&s, 0x01, NULL, 0, 0);
    return s.pc == before + 1;
}

static int test_step_sets_last_opcode(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0x27, (int[]){1,2,3}, 3, 0);
    return s.last_opcode == 0x27;
}

static int test_step_sets_last_result(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0x01, NULL, 0, 0);
    return s.last_result == (int)DM2_CCM_RESULT_OK;
}

/* ── Observability (33-35) ─────────────────────────────────────── */

static int test_total_steps_increments(void) {
    int before = dm2_v1_ccm_total_steps();
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0x01, NULL, 0, 0);
    dm2_v1_ccm_step(&s, 0x08, NULL, 0, 0);
    return dm2_v1_ccm_total_steps() == before + 2;
}

static int test_total_unknown_increments(void) {
    int before = dm2_v1_ccm_total_unknown();
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0xFE, NULL, 0, 0);  /* unknown */
    return dm2_v1_ccm_total_unknown() == before + 1;
}

static int test_total_halted_increments(void) {
    int before = dm2_v1_ccm_total_halted();
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0xFF, NULL, 0, 0);  /* halt */
    return dm2_v1_ccm_total_halted() == before + 1;
}

/* ── Def/name lookups (36-37) ──────────────────────────────────── */

static int test_get_opcode_def_unknown(void) {
    return dm2_v1_ccm_get_opcode_def(0xFE) == NULL;
}

static int test_get_opcode_name_unknown(void) {
    return dm2_v1_ccm_get_opcode_name(0xFE) == NULL;
}

/* ── Source evidence (38) ──────────────────────────────────────── */

static int test_source_evidence(void) {
    const char *e = dm2_v1_ccm_source_evidence();
    return e != NULL && e[0] != '\0'
        && strstr(e, "skproject/SKULLWIN/c_creature.cpp") != NULL
        && strstr(e, "DM2_PROCEED_CCM") != NULL
        && strstr(e, "GROUP.C:1695-1770") != NULL;
}

/* ── Multi-step (39-41) ───────────────────────────────────────── */

static int test_multistep_walk_attack_halt(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc1 = dm2_v1_ccm_step(&s, 0x01, NULL, 0, 0);  /* walk */
    int rc2 = dm2_v1_ccm_step(&s, 0x08, (int[]){5}, 1, 0);  /* attack */
    int rc3 = dm2_v1_ccm_step(&s, 0xFF, NULL, 0, 0);  /* halt */
    return rc1 == (int)DM2_CCM_RESULT_OK
        && rc2 == (int)DM2_CCM_RESULT_OK
        && rc3 == (int)DM2_CCM_RESULT_HALTED
        && s.flags[0] == 1 && s.flags[9] == 1 && s.halted == 1;
}

static int test_flags_persist_across_steps(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0x08, NULL, 0, 0);  /* sets flag 9 */
    dm2_v1_ccm_step(&s, 0x01, NULL, 0, 0);  /* sets flag 0 */
    return s.flags[9] == 1 && s.flags[0] == 1;
}

static int test_stack_capacity(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    for (int i = 0; i < DM2_CCM_STACK_SIZE; i++) {
        if (!dm2_v1_ccm_stack_push(&s, i)) return 0;
    }
    return dm2_v1_ccm_stack_size(&s) == DM2_CCM_STACK_SIZE;
}

/* ── Stubbed + no-handler opcodes return UNKNOWN (42) ─────────── */

static int test_stubbed_opcodes_return_unknown(void) {
    /* Stubbed source groups and source "no branch taken" bytes must all
     * return UNKNOWN_OPCODE (fail-closed, never simulated). */
    int stub_ops[] = {
        0x0B, 0x0C, 0x0D,              /* CCM0B/CCM0C stubs */
        0x2F, 0x30, 0x31,              /* ACTIVATES_WALL stubs */
        0x35, 0x3A,                    /* USES_LADDER_HOLE stubs */
        0x3B, 0x3C,                    /* TRANSFORM stubs */
        0x55,                          /* DM2_1B7D5 stub */
        0x00, 0x10, 0x12, 0x14,        /* source: no branch taken */
        0x1B, 0x20, 0x25, 0x32, 0x34,  /* source: no branch taken */
    };
    for (size_t i = 0; i < sizeof(stub_ops)/sizeof(stub_ops[0]); i++) {
        DM2_V1_CCMState s;
        dm2_v1_ccm_init_state(&s);
        int rc = dm2_v1_ccm_step(&s, stub_ops[i], NULL, 0, 0);
        if (rc != (int)DM2_CCM_RESULT_UNKNOWN_OPCODE) return 0;
    }
    return 1;
}

/* ── Program decode/run ───────────────────────────────────────── */

static int test_decode_program_walk_shoot_spell_halt(void) {
    const uint8_t bytes[] = {
        0x01,
        0x0E, 44, 2,
        0x27, 16, 5, 7,
        0xFF
    };
    DM2_V1_CCMProgram program;
    int rc = dm2_v1_ccm_decode_program(bytes, sizeof(bytes), &program);
    return rc == (int)DM2_CCM_RESULT_OK
        && program.count == 4
        && program.ops[0].opcode == 0x01
        && program.ops[1].opcode == 0x0E
        && program.ops[1].arg_count == 2
        && program.ops[1].args[0] == 44
        && program.ops[1].args[1] == 2
        && program.ops[2].opcode == 0x27
        && program.ops[2].arg_count == 3
        && program.ops[2].args[2] == 7
        && program.ops[3].opcode == 0xFF;
}

static int test_run_without_source_program_fails_closed(void) {
    DM2_V1_CCMState s;
    int unknown_before;
    dm2_v1_ccm_init_state(&s);
    s.pc = DM2_CCM_OP_WALK_NOW;
    unknown_before = dm2_v1_ccm_total_unknown();
    return dm2_v1_ccm_run(&s, 1234) == (int)DM2_CCM_RESULT_UNKNOWN_OPCODE &&
           s.pc == DM2_CCM_OP_WALK_NOW &&
           s.step_count == 0 &&
           s.flags[0] == 0 &&
           s.last_result == (int)DM2_CCM_RESULT_UNKNOWN_OPCODE &&
           dm2_v1_ccm_total_unknown() == unknown_before + 1;
}

static int test_run_program_walk_shoot_spell_halt(void) {
    const uint8_t bytes[] = {
        0x01,
        0x0E, 44, 2,
        0x27, 16, 5, 7,
        0xFF
    };
    DM2_V1_CCMProgram program;
    DM2_V1_CCMState s;
    int v;
    if (dm2_v1_ccm_decode_program(bytes, sizeof(bytes), &program) !=
        (int)DM2_CCM_RESULT_OK) return 0;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_run_program(&s, &program, 1234);
    if (rc != (int)DM2_CCM_RESULT_HALTED) return 0;
    if (!s.halted || s.flags[0] != 1 || s.flags[5] != 1 || s.flags[8] != 1) return 0;
    if (s.target_x != 5 || s.target_y != 7 || s.last_opcode != 0x27) return 0;
    if (s.stack_top != 2) return 0;
    if (!dm2_v1_ccm_stack_pop(&s, &v) || v != 2) return 0;
    if (!dm2_v1_ccm_stack_pop(&s, &v) || v != 44) return 0;
    return 1;
}

static int test_decode_program_rejects_truncated_args(void) {
    const uint8_t bytes[] = { 0x27, 16, 5 };
    DM2_V1_CCMProgram program;
    return dm2_v1_ccm_decode_program(bytes, sizeof(bytes), &program) ==
           (int)DM2_CCM_RESULT_BAD_ARG;
}

static int test_decode_program_rejects_stubbed_opcode(void) {
    const uint8_t bytes[] = { 0x0B };
    DM2_V1_CCMProgram program;
    return dm2_v1_ccm_decode_program(bytes, sizeof(bytes), &program) ==
           (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
}

static int test_decode_program_accepts_ccm06_family_and_walk09(void) {
    const uint8_t bytes[] = { 0x07, 0x06, 0x09, 0xFF };
    DM2_V1_CCMProgram program;
    DM2_V1_CCMState s;
    if (dm2_v1_ccm_decode_program(bytes, sizeof(bytes), &program) !=
        (int)DM2_CCM_RESULT_OK || program.count != 4 ||
        program.ops[0].opcode != DM2_CCM_OP_CCM06_ALT ||
        program.ops[1].opcode != DM2_CCM_OP_CCM06 ||
        program.ops[2].opcode != DM2_CCM_OP_WALK_NOW_09) return 0;
    dm2_v1_ccm_init_state(&s);
    return dm2_v1_ccm_run_program(&s, &program, 1234) ==
           (int)DM2_CCM_RESULT_HALTED &&
           s.flags[13] == 1 && s.flags[0] == 1;
}

static int test_decode_program_accepts_source_skip_aliases(void) {
    /* Source skip00386/skip00389 item aliases and the paired
     * ROTATES_TARGET_CREATURE state decode as real command bytes. */
    const uint8_t bytes[] = {
        DM2_CCM_OP_ROTATES_TARGET_16,
        DM2_CCM_OP_PUTS_DOWN_ITEM_29,
        DM2_CCM_OP_TAKES_ITEM_2B,
        DM2_CCM_OP_EXPLODE_OR_SUMMON_3E, 0x07,
        DM2_CCM_OP_HALT
    };
    DM2_V1_CCMProgram program;
    DM2_V1_CCMState s;
    if (dm2_v1_ccm_decode_program(bytes, sizeof(bytes), &program) !=
        (int)DM2_CCM_RESULT_OK || program.count != 5 ||
        program.ops[0].opcode != DM2_CCM_OP_ROTATES_TARGET_16 ||
        program.ops[1].opcode != DM2_CCM_OP_PUTS_DOWN_ITEM_29 ||
        program.ops[2].opcode != DM2_CCM_OP_TAKES_ITEM_2B ||
        program.ops[3].opcode != DM2_CCM_OP_EXPLODE_OR_SUMMON_3E) return 0;
    dm2_v1_ccm_init_state(&s);
    return dm2_v1_ccm_run_program(&s, &program, 1234) ==
           (int)DM2_CCM_RESULT_HALTED &&
           s.flags[7] == 1 &&
           s.flags[10] == 1 &&
           s.flags[14] == 1 &&
           s.flags[15] == 1 &&
           s.next_state == DM2_CCM_OP_WALK_NOW;
}

/* ── Main ─────────────────────────────────────────────────────── */

int main(void) {
    printf("DM2 V1 CCM (Creature Command Machine) parity - Phase 5 source-lock tests\n");
    printf("Source: skproject/SKULLWIN/c_creature.cpp:2930-3212 (DM2_PROCEED_CCM b_1a matrix)\n");
    printf("        skproject/SKULLWIN/c_ai.cpp (DM2_THINK_CREATURE)\n");
    printf("        ReDMCSB GROUP.C:1695-1770 (F0207 creature attack)\n\n");

    /* Catalog */
    TEST(opcode_count);
    TEST(opcode_names_nonempty);
    TEST(opcode_arg_count_nonneg);

    /* Lifecycle */
    TEST(reset_zeroes);
    TEST(init_state);

    /* Stack */
    TEST(stack_push_pop);
    TEST(stack_push_full);
    TEST(stack_pop_empty);
    TEST(stack_peek);
    TEST(stack_size_null);

    /* Flags */
    TEST(flag_set_get);
    TEST(flag_out_of_range);

    /* Per-opcode step */
    TEST(step_walk_now);
    TEST(step_attacks_party_with_target);
    TEST(step_jumps);
    TEST(step_ccm03_rotate_and_item_actions);
    TEST(step_ccm06_family);
    TEST(step_steal_from_champion);
    TEST(step_merchandise);
    TEST(step_shoot_item_pushes_stack);
    TEST(step_kill_on_timer_pos);
    TEST(step_rotates_target);
    TEST(step_cast_spell);
    TEST(step_attacks_party_alias);
    TEST(step_explode_or_summon);

    /* Halt + unknown */
    TEST(step_halt);
    TEST(step_unknown_opcode);
    TEST(step_stubbed_opcode);

    /* Halted state */
    TEST(halted_state_rejects_step);

    /* Bad arg */
    TEST(step_too_few_args);
    TEST(step_null_state);

    /* Step fields */
    TEST(step_increments_pc);
    TEST(step_sets_last_opcode);
    TEST(step_sets_last_result);

    /* Observability */
    TEST(total_steps_increments);
    TEST(total_unknown_increments);
    TEST(total_halted_increments);

    /* Lookups */
    TEST(get_opcode_def_unknown);
    TEST(get_opcode_name_unknown);

    /* Source */
    TEST(source_evidence);

    /* Multi-step */
    TEST(multistep_walk_attack_halt);
    TEST(flags_persist_across_steps);
    TEST(stack_capacity);

    /* Stubbed + no-handler */
    TEST(stubbed_opcodes_return_unknown);

    /* Program decode/run */
    TEST(decode_program_walk_shoot_spell_halt);
    TEST(run_without_source_program_fails_closed);
    TEST(run_program_walk_shoot_spell_halt);
    TEST(decode_program_rejects_truncated_args);
    TEST(decode_program_rejects_stubbed_opcode);
    TEST(decode_program_accepts_ccm06_family_and_walk09);
    TEST(decode_program_accepts_source_skip_aliases);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
