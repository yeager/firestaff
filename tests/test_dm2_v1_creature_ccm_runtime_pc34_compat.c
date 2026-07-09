/* test_dm2_v1_creature_ccm_runtime_pc34_compat.c
 *
 * DM2 V1 CCM runtime bridge: creature_tick now executes the real CCM
 * interpreter for the instance b_1a state and writes back the runtime
 * attack/cooldown state.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_creature.cpp DM2_PROCEED_CCM
 *   skproject/SKULLWIN/c_ai.cpp      DM2_THINK_CREATURE
 */

#include "dm2_v1_creature.h"

#include <stdio.h>
#include <string.h>

static int g_run;
static int g_pass;

#define CHECK(label_, expr_) do { \
    ++g_run; \
    if (expr_) { ++g_pass; } \
    else { fprintf(stderr, "FAIL: %s\n", (label_)); return 0; } \
} while (0)

static void install_mobile_melee_ai(void) {
#ifdef FIRESTAFF_DM2_CREATURE_TESTING
    DM2_AIDefinition spec;
    memset(&spec, 0, sizeof(spec));
    spec.BaseHP = 16;
    spec.AttacksSpells = AI_ATTACK_FLAGS__MELEE;
    dm2_v1_creature_test_set_ai_spec(DM2_AI_CAVE_BAT, &spec);
#endif
}

static int test_walk_tick_enters_attack_state(void) {
    DM2_V1_CreatureCCMTickObserver obs;
    int slot;

    install_mobile_melee_ai();
    dm2_v1_creature_reset_ccm_tick_observer();
    slot = dm2_v1_creature_spawn(DM2_AI_CAVE_BAT, 10, 11, 0, 1, 8);
    CHECK("spawn", slot >= 0);

    dm2_v1_creature_tick();
    CHECK("observer valid", dm2_v1_creature_last_ccm_tick(&obs) == 1);
    CHECK("walk opcode", obs.before_b_1a == DM2_CCM_WALK_NOW);
    CHECK("walk ccm opcode", obs.ccm_opcode == DM2_CCM_WALK_NOW);
    CHECK("walk flag", obs.ccm_flag_walk == 1);
    CHECK("attack state writeback", obs.after_b_1a == DM2_CCM_CREATURE_ATTACKS_PARTY);
    CHECK("cooldown unchanged on planning tick", obs.attack_cooldown_after == 0);
    return 1;
}

static int test_attack_tick_sets_cooldown(void) {
    DM2_V1_CreatureCCMTickObserver obs;

    dm2_v1_creature_tick();
    CHECK("observer valid", dm2_v1_creature_last_ccm_tick(&obs) == 1);
    CHECK("attack opcode", obs.before_b_1a == DM2_CCM_CREATURE_ATTACKS_PARTY);
    CHECK("attack ccm opcode", obs.ccm_opcode == DM2_CCM_CREATURE_ATTACKS_PARTY);
    CHECK("attack flag", obs.ccm_flag_attack_party == 1);
    CHECK("walk writeback", obs.after_b_1a == DM2_CCM_WALK_NOW);
    CHECK("cooldown set", obs.attack_cooldown_after == 18);
    return 1;
}

int main(void) {
    printf("DM2 V1 creature CCM runtime bridge\n");
    if (!test_walk_tick_enters_attack_state()) return 1;
    if (!test_attack_tick_sets_cooldown()) return 1;
    printf("%d/%d checks passed\n", g_pass, g_run);
    return 0;
}
