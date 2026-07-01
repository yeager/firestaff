/*
 * test_dm1_v1_orch_pending_damage_flow_pc34_compat.c
 *
 * Data-free end-to-end M10 gate for the DM1 creature/champion damage
 * handoff: pending CombatResult_Compat -> F0889 orchestrator step ->
 * ChampionState_Compat HP/wounds -> champion-down emission / party-dead
 * state. This closes the BUG-101 follow-up audit row without requiring
 * real DUNGEON.DAT/GRAPHICS.DAT or M11 rendering.
 *
 * Source lock:
 * - ReDMCSB CHAMPION.C:1814 F0320/F0321 damage application updates
 *   champion HP and wounds.
 * - ReDMCSB GAMELOOP.C:164-219 drains pending game-loop effects during
 *   the same source tick boundary that F0884 models.
 * - Firestaff M10 F0889_ORCH_ApplyPendingDamage_Compat is the live
 *   orchestrator bridge from F0737 damage application into world state.
 */

#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(const char* name, int ok) {
    if (!ok) {
        printf("FAIL %s\n", name);
        ++failures;
    } else {
        printf("PASS %s\n", name);
    }
}

static void init_world_with_one_champion(
    struct GameWorld_Compat* world,
    int championIndex,
    int hp,
    unsigned short wounds) {

    memset(world, 0, sizeof(*world));
    world->party.championCount = 1;
    world->party.activeChampionIndex = championIndex;
    world->party.champions[championIndex].present = 1;
    world->party.champions[championIndex].hp.current = (unsigned short)hp;
    world->party.champions[championIndex].hp.maximum = (unsigned short)hp;
    world->party.champions[championIndex].wounds = wounds;
}

static void test_nonlethal_pending_damage_applies_and_clears(void) {
    struct GameWorld_Compat world;
    struct TickResult_Compat result;

    init_world_with_one_champion(&world, 2, 100, 0x0001u);
    memset(&result, 0, sizeof(result));
    world.pendingCombat.damageApplied = 35;
    world.pendingCombat.woundMaskAdded = COMBAT_WOUND_TORSO;

    F0889_ORCH_ApplyPendingDamage_Compat(&world, &result);

    check("A1 nonlethal damage subtracts HP",
          world.party.champions[2].hp.current == 65);
    check("A2 wound mask is ORed through F0737",
          world.party.champions[2].wounds ==
              (unsigned short)(0x0001u | COMBAT_WOUND_TORSO));
    check("A3 pending combat is cleared",
          world.pendingCombat.damageApplied == 0 &&
          world.pendingCombat.woundMaskAdded == 0);
    check("A4 nonlethal damage emits no champion-down",
          result.emissionCount == 0);
    check("A5 party remains alive",
          world.partyDead == 0);
}

static void test_lethal_pending_damage_emits_champion_down_and_party_dead(void) {
    struct GameWorld_Compat world;
    struct TickResult_Compat result;

    init_world_with_one_champion(&world, 1, 40, 0);
    memset(&result, 0, sizeof(result));
    world.pendingCombat.damageApplied = 40;
    world.pendingCombat.woundMaskAdded = COMBAT_WOUND_HEAD;

    F0889_ORCH_ApplyPendingDamage_Compat(&world, &result);

    check("B1 lethal damage clamps HP to zero",
          world.party.champions[1].hp.current == 0);
    check("B2 lethal damage still records wound mask",
          world.party.champions[1].wounds == COMBAT_WOUND_HEAD);
    check("B3 lethal damage emits one champion-down",
          result.emissionCount == 1 &&
          result.emissions[0].kind == EMIT_CHAMPION_DOWN);
    check("B4 champion-down payload records active champion index",
          result.emissionCount == 1 &&
          result.emissions[0].payload[0] == 1);
    check("B5 party dead is set when no present champion remains alive",
          world.partyDead == 1);
    check("B6 pending combat cleared after lethal apply",
          world.pendingCombat.damageApplied == 0 &&
          world.pendingCombat.woundMaskAdded == 0);
}

static void test_no_pending_damage_preserves_party_state(void) {
    struct GameWorld_Compat world;
    struct TickResult_Compat result;

    init_world_with_one_champion(&world, 0, 77, 0x0008u);
    memset(&result, 0, sizeof(result));

    F0889_ORCH_ApplyPendingDamage_Compat(&world, &result);

    check("C1 no pending damage keeps HP",
          world.party.champions[0].hp.current == 77);
    check("C2 no pending damage keeps wounds",
          world.party.champions[0].wounds == 0x0008u);
    check("C3 no pending damage emits nothing",
          result.emissionCount == 0);
    check("C4 no pending damage does not mark party dead",
          world.partyDead == 0);
}

static void test_invalid_active_champion_does_not_clear_damage(void) {
    struct GameWorld_Compat world;
    struct TickResult_Compat result;

    init_world_with_one_champion(&world, 0, 90, 0);
    world.party.activeChampionIndex = -1;
    memset(&result, 0, sizeof(result));
    world.pendingCombat.damageApplied = 20;

    F0889_ORCH_ApplyPendingDamage_Compat(&world, &result);

    check("D1 invalid active index does not damage occupied slot",
          world.party.champions[0].hp.current == 90);
    check("D2 current F0889 contract still clears consumed pending damage",
          world.pendingCombat.damageApplied == 0);
    check("D3 invalid active index emits nothing",
          result.emissionCount == 0);
    check("D4 occupied living champion keeps party alive",
          world.partyDead == 0);
}

int main(void) {
    printf("test=dm1_v1_orch_pending_damage_flow_pc34_compat\n");

    test_nonlethal_pending_damage_applies_and_clears();
    test_lethal_pending_damage_emits_champion_down_and_party_dead();
    test_no_pending_damage_preserves_party_state();
    test_invalid_active_champion_does_not_clear_damage();

    printf("dm1V1OrchPendingDamageFlowOk=%u\n", failures == 0 ? 1u : 0u);
    return failures == 0 ? 0 : 1;
}
