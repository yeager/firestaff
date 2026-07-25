/*
 * Locks the F0412 failure tail through the real CMD_CAST_SPELL runtime path.
 * ReDMCSB MENU.C F0412:1835-1841 awards partial F0304 XP before returning
 * C00_SPELL_CAST_FAILURE for NEEDS_MORE_PRACTICE.
 */
#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "memory_combat_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void init_spell_world(struct GameWorld_Compat* world,
                             struct DungeonThings_Compat* things)
{
    int slot;

    memset(world, 0, sizeof(*world));
    memset(things, 0, sizeof(*things));
    world->things = things;
    world->party.championCount = 1;
    world->party.activeChampionIndex = 0;
    world->party.champions[0].present = 1;
    world->party.champions[0].hp.current = 100;
    world->party.champions[0].hp.maximum = 100;
    world->party.champions[0].mana.current = 100;
    world->party.champions[0].mana.maximum = 100;
    for (slot = 0; slot < CHAMPION_SLOT_COUNT; ++slot) {
        world->party.champions[0].inventory[slot] = THING_NONE;
    }
}

static void test_needs_practice_awards_partial_spell_xp(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int sawPartialXp = 0;
    (void)sawPartialXp;
    int i;

    init_spell_world(&world, &things);
    world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 0;
    assert(F0730_COMBAT_RngInit_Compat(&world.masterRng, 0u) == 1);

    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 8; /* On Ful Ir: G0487 Fireball / Fire skill. */
    input.reserved = 3;    /* On power: F0412 needs five missing levels. */

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);

    /* ReDMCSB MENU.C F0412:1835-1841 awards L1273 >> missingLevels
     * before the C00 failure return. Seed 0 produces rng8=1, so
     * (1 + (6 << 4) + (2 * 3 << 3) + 36) >> 5 == 5. */
    assert(world.lifecycle.champions[0]
               .skills20[DM1_SKILL_IDX_FIRE].experience == 5);
    assert(world.lifecycle.champions[0]
               .skills20[DM1_SKILL_IDX_WIZARD].experience == 5);
    assert(world.party.champions[0]
               .skillExperience[DM1_SKILL_IDX_WIZARD] == 5ul);
    assert(world.projectiles.count == 0);
    assert(world.timeline.count == 0);

    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_XP_AWARD &&
            result.emissions[i].payload[0] == 0 &&
            result.emissions[i].payload[1] == DM1_SKILL_IDX_FIRE &&
            result.emissions[i].payload[2] == 5 &&
            result.emissions[i].payload[3] == 0) {
            sawPartialXp = 1;
        }
        assert(result.emissions[i].kind != EMIT_SPELL_EFFECT);
        assert(result.emissions[i].kind != EMIT_ACTION_DISABLED);
    }
    assert(sawPartialXp == 1);
}

int main(void)
{
    test_needs_practice_awards_partial_spell_xp();
    puts("PASS dm1_v1_f0412_needs_practice_runtime_pc34_compat");
    return 0;
}
