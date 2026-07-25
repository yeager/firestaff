/* ReDMCSB MENU.C F0412 awards shifted F0304 XP before F0410 reports a
 * skill failure. No C11 action-disable or timeline event follows. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    int i;
    int sawXp = 0;
    (void)sawXp;

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    world.things = &things;
    world.newPartyMapIndex = -1;
    world.masterRng.seed = 1; /* First raw low-7 value is 38 > Wisdom+15. */
    world.party.championCount = 1;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;
    world.party.champions[0].mana.current = 100;
    world.party.champions[0].mana.maximum = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 0;
    /* F0303 gives an untrained Fire skill level 1; Fireball requires 6. */
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 8; /* Ful Ir: FIREBALL. */
    input.reserved = 3;    /* On power ordinal. */

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(world.timeline.count == 0);
    /* F0412 shifts 186 F0304 XP by the five missing F0303 levels. */
    assert(world.lifecycle.champions[0].skills20[DM1_SKILL_IDX_FIRE].experience == 5);
    assert(world.lifecycle.champions[0].skills20[DM1_SKILL_IDX_WIZARD].experience == 5);
    for (i = 0; i < result.emissionCount; ++i) {
        if (result.emissions[i].kind == EMIT_XP_AWARD) {
            assert(result.emissions[i].payload[0] == 0);
            assert(result.emissions[i].payload[1] == DM1_SKILL_IDX_FIRE);
            assert(result.emissions[i].payload[2] == 5);
            sawXp = 1;
        }
        assert(result.emissions[i].kind != EMIT_SPELL_EFFECT);
        assert(result.emissions[i].kind != EMIT_ACTION_DISABLED);
    }
    assert(sawXp == 1);

    printf("PASS dm1_v1_f0412_failure_xp_pc34_compat\n");
    return 0;
}
