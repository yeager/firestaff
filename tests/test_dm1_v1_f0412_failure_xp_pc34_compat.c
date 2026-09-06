/* ReDMCSB MENU.C F0412 awards shifted F0304 XP before F0410 reports a
 * skill failure. No C11 action-disable or timeline event follows. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "memory_tick_orchestrator_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"

int main(void)
{
    struct GameWorld_Compat world;
    struct DungeonThings_Compat things;
    struct TickInput_Compat input;
    struct TickResult_Compat result;
    struct RngState_Compat expectedRng;
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
    expectedRng = world.masterRng;
    (void)F0731_COMBAT_RngNextRaw_Compat(&expectedRng); /* F0412 XP sample. */
    assert((F0731_COMBAT_RngNextRaw_Compat(&expectedRng) & 127u) > 15u);
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
    assert(world.masterRng.seed == expectedRng.seed); /* Stop at first failed probe. */
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

    world.masterRng.seed = 1;
    memset(&result, 0, sizeof(result));
    input.commandArg2 = 17; /* Ya Bro Dain: Wisdom potion. */
    input.reserved = 6;
    input.reserved2 = CMD_CAST_SPELL_RESERVED2_HAS_EMPTY_FLASK;
    assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
    /* A failed practice gate precedes even a declared available flask:
     * no potion-power sample may be drawn (MENU.C:1837 before :1853). */
    assert(world.masterRng.seed == expectedRng.seed);
    assert(world.timeline.count == 0);
    for (i = 0; i < result.emissionCount; ++i) {
        assert(result.emissions[i].kind != EMIT_SPELL_EFFECT);
        assert(result.emissions[i].kind != EMIT_ACTION_DISABLED);
    }

    {
        DM1_ChampionSpellStats stats = {0};
        DM1_SpellF0412RuntimeReceipt receipt;
        uint8_t probes[9] = {0};
        stats.currentHealth = 100;
        stats.wisdom = 100;
        for (i = 0; i < 20; ++i) stats.skillLevels[i] = 1;
        probes[8] = 127;
        /* Mon Lightning: base 4 + power 6 - skill 1 requires nine probes. */
        assert(dm1_spell_f0412RuntimeReceiptForTableIndexWithProbeCount(
            5, 6, 0, &stats, 0, probes, 9, 0, 0, 0, &receipt));
        assert(receipt.requiredSkillLevel - receipt.skillLevel == 9);
        assert(receipt.failureType == DM1_FAILURE_NEEDS_MORE_PRACTICE);

        /* MENU.C F0412:1837 uses strictly greater-than: equality at the
         * capped threshold must pass all nine checks, including the last.
         * :1853 then uses an independent low-four-bit potion power sample.
         * This source-shaped receipt fixture is not an emulator capture. */
        memset(probes, 115, sizeof(probes));
        assert(dm1_spell_f0412PotionReceiptForTableIndexWithProbeCount(
            17, 6, 0, &stats, 127, 0xffff, 1, probes, 9, &receipt));
        assert(receipt.requiredSkillLevel - receipt.skillLevel == 9);
        assert(receipt.castResult == DM1_SPELL_CAST_SUCCESS);
        assert(receipt.failureType == -1);
        assert(receipt.potionPower == 255);
        probes[8] = 116;
        assert(dm1_spell_f0412PotionReceiptForTableIndexWithProbeCount(
            17, 6, 0, &stats, 0, 0xffff, 1, probes, 9, &receipt));
        assert(receipt.failureType == DM1_FAILURE_NEEDS_MORE_PRACTICE);
    }

    {
        unsigned seed;
        int probe;
        /* Find a deterministic source stream whose first eight checks pass
         * and ninth fails. The command must consume that ninth real draw. */
        for (seed = 0; seed < 10000; ++seed) {
            expectedRng.seed = seed;
            (void)F0731_COMBAT_RngNextRaw_Compat(&expectedRng);
            for (probe = 0; probe < 9; ++probe)
                if ((F0731_COMBAT_RngNextRaw_Compat(&expectedRng) & 127u) > 115u)
                    break;
            if (probe == 8) break;
        }
        assert(seed < 10000);
        world.masterRng.seed = seed;
        world.party.champions[0].attributes[CHAMPION_ATTR_WISDOM] = 100;
        input.commandArg2 = 5;
        input.reserved = 6;
        input.reserved2 = 0;
        memset(&result, 0, sizeof(result));
        assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
        assert(world.masterRng.seed == expectedRng.seed);
        assert(world.timeline.count == 0);
        assert(world.projectiles.count == 0);
        for (i = 0; i < result.emissionCount; ++i)
            assert(result.emissions[i].kind != EMIT_SPELL_EFFECT);
    }

    {
        unsigned seed;
        int probe;
        int experienceBefore;
        /* MENU.C:1836-1850: all nine checks precede the flask lookup;
         * absent flask returns before potion-power RNG and XP award. */
        for (seed = 0; seed < 10000; ++seed) {
            expectedRng.seed = seed;
            (void)F0731_COMBAT_RngNextRaw_Compat(&expectedRng);
            for (probe = 0; probe < 9; ++probe)
                if ((F0731_COMBAT_RngNextRaw_Compat(&expectedRng) & 127u) > 115u)
                    break;
            if (probe == 9) break;
        }
        assert(seed < 10000);
        world.masterRng.seed = seed;
        input.commandArg2 = 17; /* Mon Wisdom Potion: nine missing levels. */
        input.reserved = 6;
        input.reserved2 = 0; /* No flask. */
        experienceBefore = world.lifecycle.champions[0].skills20[DM1_SKILL_IDX_HEAL].experience;
        memset(&result, 0, sizeof(result));
        assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
        assert(world.masterRng.seed == expectedRng.seed);
        assert(world.lifecycle.champions[0].skills20[DM1_SKILL_IDX_HEAL].experience == experienceBefore);
        assert(world.timeline.count == 0);
        for (i = 0; i < result.emissionCount; ++i) {
            assert(result.emissions[i].kind != EMIT_XP_AWARD);
            assert(result.emissions[i].kind != EMIT_SPELL_EFFECT);
            assert(result.emissions[i].kind != EMIT_ACTION_DISABLED);
        }
        {
            struct DungeonPotion_Compat potion = {0};
            unsigned char rawPotion[4] = {0xff, 0xfe, 0, 20};
            unsigned expectedPower;
            /* Reuse the identical successful gate with an actual C08
             * owner in hand. Exactly one further draw makes potion power. */
            potion.type = 20;
            things.potions = &potion;
            things.potionCount = 1;
            things.rawThingData[THING_TYPE_POTION] = rawPotion;
            things.thingCounts[THING_TYPE_POTION] = 1;
            world.party.champions[0].inventory[0] =
                (unsigned short)(THING_TYPE_POTION << 10);
            expectedPower = 240u +
                ((F0731_COMBAT_RngNextRaw_Compat(&expectedRng) >> 8) & 15u);
            world.masterRng.seed = seed;
            input.reserved2 = CMD_CAST_SPELL_RESERVED2_HAS_EMPTY_FLASK;
            memset(&result, 0, sizeof(result));
            assert(F0888_ORCH_ApplyPlayerInput_Compat(&world, &input, &result) == 1);
            assert(world.masterRng.seed == expectedRng.seed);
            assert(potion.type == 8);
            assert(potion.power == expectedPower);
            assert(rawPotion[2] == expectedPower);
            assert(rawPotion[3] == 8);
            things.potions = NULL;
            things.rawThingData[THING_TYPE_POTION] = NULL;
        }
    }

    printf("PASS dm1_v1_f0412_failure_xp_pc34_compat\n");
    return 0;
}
