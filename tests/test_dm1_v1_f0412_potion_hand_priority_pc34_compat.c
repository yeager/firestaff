/* ReDMCSB MENU.C F0411 scans action hand before ready hand for C195 empty
 * flasks. The CMD hint may be stale after a save/reload or hand mapping. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct DungeonPotion_Compat potions[2];
    struct TickInput_Compat input;
    struct TickResult_Compat result;

    memset(&world, 0, sizeof(world));
    memset(&things, 0, sizeof(things));
    memset(potions, 0, sizeof(potions));
    memset(&input, 0, sizeof(input));
    memset(&result, 0, sizeof(result));
    world.things = &things;
    world.newPartyMapIndex = -1;
    world.party.championCount = 1;
    world.party.champions[0].present = 1;
    world.party.champions[0].hp.current = 100;
    world.party.champions[0].hp.maximum = 100;
    world.party.champions[0].mana.current = 100;
    world.party.champions[0].mana.maximum = 100;
    world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 60;
    world.lifecycle.champions[0].skills20[DM1_SKILL_IDX_HEAL].experience = 5000;
    things.potions = potions;
    things.potionCount = 2;

    /* Both hands contain real empty flasks. The incoming mapping incorrectly
     * claims ready hand (slot 0), but F0411 must mutate action hand (slot 1). */
    potions[0].type = 20;
    potions[1].type = 20;
    world.party.champions[0].inventory[1] =
        (unsigned short)((THING_TYPE_POTION << 10) | 0);
    world.party.champions[0].inventory[0] =
        (unsigned short)((THING_TYPE_POTION << 10) | 1);
    input.command = CMD_CAST_SPELL;
    input.commandArg1 = 0;
    input.commandArg2 = 16; /* Ya: Stamina Potion. */
    input.reserved = 1;
    input.reserved2 = CMD_CAST_SPELL_RESERVED2_HAS_EMPTY_FLASK |
        (0u << CMD_CAST_SPELL_RESERVED2_EMPTY_FLASK_SLOT_SHIFT);

    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(potions[0].type == 11); /* C11 Stamina Potion from G0487 row 16. */
    assert(potions[0].power >= 40 && potions[0].power <= 55);
    assert(potions[1].type == 20);
    assert(world.party.champions[0].inventory[1] ==
           (unsigned short)((THING_TYPE_POTION << 10) | 0));
    assert(world.party.champions[0].inventory[0] ==
           (unsigned short)((THING_TYPE_POTION << 10) | 1));

    /* A stale action-map hint alone is not F0411 evidence. With neither
     * flask in hand, F0412 must stop at NEEDS_FLASK before its XP/F0330
     * common tail and before any timeline event is created. */
    world.party.champions[0].inventory[0] = THING_NONE;
    world.party.champions[0].inventory[1] = THING_NONE;
    world.timeline.count = 0;
    memset(&result, 0, sizeof(result));
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    assert(potions[0].type == 11);
    assert(potions[1].type == 20);
    assert(world.timeline.count == 0);
    {
        int i;
        for (i = 0; i < result.emissionCount; ++i) {
            assert(result.emissions[i].kind != EMIT_SPELL_EFFECT);
            assert(result.emissions[i].kind != EMIT_XP_AWARD);
        }
    }

    printf("PASS dm1_v1_f0412_potion_hand_priority_pc34_compat\n");
    return 0;
}
