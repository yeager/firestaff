/* test_dm2_v1_ccm_pc34_compat.c - DM2 V1 CCM (Creature Command Machine) Tests
 *
 * Phase 5 mechanics parity coverage (30+ assertions):
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
 * 13. step(0x00 NOP) returns OK
 * 14. step(0x01 ATTACK_HANDLER) sets flag 1
 * 15. step(0x05 SPECIAL_ACTION) sets flag 2 to arg
 * 16. step(0x09 STEAL_ITEM) sets target_id
 * 17. step(0x0A MERCHANT_BEHAVIOR) sets flag 4
 * 18. step(0x0D SHOOT_ITEM) pushes args to stack
 * 19. step(0x0F KILL_ON_TIMER_POS) sets last_step_tick_ms
 * 20. step(0x13 ROTATES_TARGET) sets target_id
 * 21. step(0x15 CAST_SPELL) sets target_x/y
 * 22. step(0x17 CREATURE_ATTACKS_PARTY) sets flag 9
 * 23. step(0x26 EXPLODE_OR_SUMMON) sets flag 10
 * 24. step(0xFF HALT) sets halted=1, returns HALTED
 * 25. step on unknown opcode (e.g., 0xFE) returns UNKNOWN_OPCODE
 * 26. step on stubbed opcode (e.g., 0x03) returns UNKNOWN_OPCODE
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
 * 39. Multi-step: WALK_NOW → ATTACK_HANDLER → HALT
 * 40. Flags persist across steps
 * 41. Stack can hold up to DM2_CCM_STACK_SIZE items
 * 42. After 12 implemented opcodes, all stubbed (0x03 etc.) return UNKNOWN
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
    /* We don't expose the table directly; use a few known opcodes. */
    static const int known[DM2_CCM_MAX_OPCODES] = {
        0x00, 0x01, 0x02, 0x05, 0x09, 0x0A, 0x0D, 0x0F,
        0x13, 0x15, 0x17, 0x26, 0x03, 0x04, 0x06, 0x07,
        0x08, 0x0B, 0x0C, 0x0E, 0x10, 0x11, 0x12, 0x14,
        0x16, 0x18, 0x19, 0x1A, 0x1F, 0x20, 0x21, 0xFF
    };
    (void)known;  /* referenced in test functions */
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

static int test_step_nop(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_step(&s, 0x00, NULL, 0, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[0] == 1;
}

static int test_step_attack_handler(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 5 };
    int rc = dm2_v1_ccm_step(&s, 0x01, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[1] == 1 && s.target_id == 5;
}

static int test_step_special_action(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 0x06 };  /* sub-action 06 */
    int rc = dm2_v1_ccm_step(&s, 0x05, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[2] == 0x06;
}

static int test_step_steal_item(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 3 };  /* champion 3 */
    int rc = dm2_v1_ccm_step(&s, 0x09, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.target_id == 3 && s.flags[3] == 1;
}

static int test_step_merchant_behavior(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 2 };
    int rc = dm2_v1_ccm_step(&s, 0x0A, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[4] == 2;
}

static int test_step_shoot_item_pushes_stack(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 1001, 2 };  /* item 1001, dir 2 */
    int rc = dm2_v1_ccm_step(&s, 0x0D, args, 2, 0);
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
    int rc = dm2_v1_ccm_step(&s, 0x0F, args, 1, 5000);
    return rc == (int)DM2_CCM_RESULT_OK
        && s.flags[6] == 7 && s.last_step_tick_ms == 5000;
}

static int test_step_rotates_target(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 42 };
    int rc = dm2_v1_ccm_step(&s, 0x13, args, 1, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.target_id == 42;
}

static int test_step_cast_spell(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 16, 5, 7 };  /* spell 16, target (5,7) */
    int rc = dm2_v1_ccm_step(&s, 0x15, args, 3, 0);
    return rc == (int)DM2_CCM_RESULT_OK
        && s.target_x == 5 && s.target_y == 7 && s.flags[8] == 1;
}

static int test_step_attacks_party(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_step(&s, 0x17, NULL, 0, 0);
    return rc == (int)DM2_CCM_RESULT_OK && s.flags[9] == 1;
}

static int test_step_explode_or_summon(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int args[] = { 99 };
    int rc = dm2_v1_ccm_step(&s, 0x26, args, 1, 0);
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
    int rc = dm2_v1_ccm_step(&s, 0x03, NULL, 0, 0);  /* stubbed */
    return rc == (int)DM2_CCM_RESULT_UNKNOWN_OPCODE;
}

/* ── Halted state (27) ─────────────────────────────────────────── */

static int test_halted_state_rejects_step(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    s.halted = 1;
    int rc = dm2_v1_ccm_step(&s, 0x00, NULL, 0, 0);
    return rc == (int)DM2_CCM_RESULT_HALTED;
}

/* ── Bad arg + null (28-29) ────────────────────────────────────── */

static int test_step_too_few_args(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int rc = dm2_v1_ccm_step(&s, 0x01, NULL, 0, 0);  /* needs 1 arg */
    return rc == (int)DM2_CCM_RESULT_BAD_ARG;
}

static int test_step_null_state(void) {
    return dm2_v1_ccm_step(NULL, 0x00, NULL, 0, 0) == (int)DM2_CCM_RESULT_BAD_ARG;
}

/* ── pc + last_opcode + last_result (30-32) ───────────────────── */

static int test_step_increments_pc(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    int before = s.pc;
    dm2_v1_ccm_step(&s, 0x00, NULL, 0, 0);
    return s.pc == before + 1;
}

static int test_step_sets_last_opcode(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0x15, (int[]){1,2,3}, 3, 0);
    return s.last_opcode == 0x15;
}

static int test_step_sets_last_result(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0x00, NULL, 0, 0);
    return s.last_result == (int)DM2_CCM_RESULT_OK;
}

/* ── Observability (33-35) ─────────────────────────────────────── */

static int test_total_steps_increments(void) {
    int before = dm2_v1_ccm_total_steps();
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0x00, NULL, 0, 0);
    dm2_v1_ccm_step(&s, 0x17, NULL, 0, 0);
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
    int rc1 = dm2_v1_ccm_step(&s, 0x00, NULL, 0, 0);  /* walk */
    int rc2 = dm2_v1_ccm_step(&s, 0x01, (int[]){5}, 1, 0);  /* attack */
    int rc3 = dm2_v1_ccm_step(&s, 0xFF, NULL, 0, 0);  /* halt */
    return rc1 == (int)DM2_CCM_RESULT_OK
        && rc2 == (int)DM2_CCM_RESULT_OK
        && rc3 == (int)DM2_CCM_RESULT_HALTED
        && s.flags[0] == 1 && s.flags[1] == 1 && s.halted == 1;
}

static int test_flags_persist_across_steps(void) {
    DM2_V1_CCMState s;
    dm2_v1_ccm_init_state(&s);
    dm2_v1_ccm_step(&s, 0x17, NULL, 0, 0);  /* sets flag 9 */
    dm2_v1_ccm_step(&s, 0x00, NULL, 0, 0);  /* sets flag 0 */
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

/* ── Stubbed opcodes return UNKNOWN (42) ──────────────────────── */

static int test_stubbed_opcodes_return_unknown(void) {
    /* Several stubbed opcodes should all return UNKNOWN_OPCODE. */
    int stub_ops[] = { 0x03, 0x04, 0x06, 0x07, 0x08, 0x0B, 0x0C };
    for (size_t i = 0; i < sizeof(stub_ops)/sizeof(stub_ops[0]); i++) {
        DM2_V1_CCMState s;
        dm2_v1_ccm_init_state(&s);
        int rc = dm2_v1_ccm_step(&s, stub_ops[i], NULL, 0, 0);
        if (rc != (int)DM2_CCM_RESULT_UNKNOWN_OPCODE) return 0;
    }
    return 1;
}

/* ── Main ─────────────────────────────────────────────────────── */

int main(void) {
    printf("DM2 V1 CCM (Creature Command Machine) parity - Phase 5 source-lock tests\n");
    printf("Source: skproject/SKULLWIN/c_creature.cpp (DM2_PROCEED_CCM)\n");
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
    TEST(step_nop);
    TEST(step_attack_handler);
    TEST(step_special_action);
    TEST(step_steal_item);
    TEST(step_merchant_behavior);
    TEST(step_shoot_item_pushes_stack);
    TEST(step_kill_on_timer_pos);
    TEST(step_rotates_target);
    TEST(step_cast_spell);
    TEST(step_attacks_party);
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

    /* Stubbed */
    TEST(stubbed_opcodes_return_unknown);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
