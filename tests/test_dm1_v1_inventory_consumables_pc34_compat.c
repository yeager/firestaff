#include "dm1_v1_inventory_consumables_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("ok %s=%d\n", label, got);
    return 1;
}

static DM1ConsumableChampionPc34 base_champion(void)
{
    DM1ConsumableChampionPc34 c;
    memset(&c, 0, sizeof(c));
    c.statistic[DM1_CONSUMABLE_STAT_STRENGTH] = 30;
    c.statistic[DM1_CONSUMABLE_STAT_DEXTERITY] = 31;
    c.statistic[DM1_CONSUMABLE_STAT_WISDOM] = 32;
    c.statistic[DM1_CONSUMABLE_STAT_VITALITY] = 33;
    c.currentHealth = 40;
    c.maximumHealth = 100;
    c.currentStamina = 50;
    c.maximumStamina = 120;
    c.currentMana = 80;
    c.maximumMana = 100;
    c.food = 1000;
    c.water = 1000;
    c.wounds = 0x00F0u;
    c.poisonDose = 12;
    return c;
}

int main(void)
{
    int ok = 1;
    DM1ConsumableChampionPc34 c;
    DM1ConsumableResultPc34 r;
    const uint16_t viMasks[] = { 0xFF0Fu };
    DM1_SoundSystem soundSystem;
    DM1ConsumableMouthAnimationFramePc34 mouthFrames[DM1_CONSUMABLE_MOUTH_ANIMATION_FRAME_COUNT_PC34];

    printf("probe=dm1_v1_inventory_consumables_pc34_compat\n");
    printf("sourceEvidence=%s\n", dm1_inventory_consumables_source_evidence_pc34());

    ok &= expect_int("apple amount", dm1_inventory_food_amount_from_icon_pc34(168), 500);
    ok &= expect_int("dragon steak amount", dm1_inventory_food_amount_from_icon_pc34(175), 1400);
    ok &= expect_int("junk type 29 icon", dm1_inventory_junk_food_icon_from_type_pc34(29), 168);
    ok &= expect_int("junk type 36 icon", dm1_inventory_junk_food_icon_from_type_pc34(36), 175);

    c = base_champion();
    ok &= expect_int("eat cheese", dm1_inventory_consume_food_junk_pc34(&c, 171, &r), 1);
    ok &= expect_int("cheese food cap", c.food, 1820);
    ok &= expect_int("food removes leader hand", r.removeLeaderHandObject, 1);
    ok &= expect_int("food mouth animation count",
                     dm1_inventory_consumables_mouth_animation_pc34(&r, mouthFrames,
                                                                    DM1_CONSUMABLE_MOUTH_ANIMATION_FRAME_COUNT_PC34),
                     4);
    ok &= expect_int("food mouth frame 0 icon", mouthFrames[0].iconIndex, 206);
    ok &= expect_int("food mouth frame 1 icon", mouthFrames[1].iconIndex, 205);
    ok &= expect_int("food mouth frame 2 icon", mouthFrames[2].iconIndex, 206);
    ok &= expect_int("food mouth frame 3 icon", mouthFrames[3].iconIndex, 205);
    ok &= expect_int("food mouth frame delay", mouthFrames[0].delayTicks, 8);

    c = base_champion();
    c.food = 1800;
    DM1_Sound_Init(&soundSystem);
    DM1_Sound_SetPartyPosition(&soundSystem, 4, 6, 0, 0);
    ok &= expect_int("eat dragon steak for sound route",
                     dm1_inventory_consume_food_junk_pc34(&c, 175, &r), 1);
    ok &= expect_int("food swallow sound flag", r.playSwallowSound, 1);
    ok &= expect_int("food swallow sound routed",
                     dm1_inventory_consumables_route_swallow_sound_pc34(&r, &soundSystem, 4, 6), 1);
    ok &= expect_int("food swallow sound index", soundSystem.lastPlayedSoundIndex, DM1_SND_SWALLOW);
    ok &= expect_int("food swallow sound request count", soundSystem.totalSoundRequests, 1);
    ok &= expect_int("food swallow sound played count", soundSystem.totalSoundsPlayed, 1);

    ok &= expect_int("eat dragon steak", dm1_inventory_consume_food_junk_pc34(&c, 175, &r), 1);
    ok &= expect_int("dragon steak caps at 2048", c.food, 2048);

    c = base_champion();
    ok &= expect_int("drink waterskin", dm1_inventory_consume_water_junk_pc34(&c, 9, 3, &r), 1);
    ok &= expect_int("waterskin water amount", c.water, 1800);
    ok &= expect_int("waterskin charge decremented", r.chargeCountAfter, 2);
    ok &= expect_int("waterskin swallow sound flag", r.playSwallowSound, 1);
    ok &= expect_int("waterskin stays in hand", r.removeLeaderHandObject, 0);
    ok &= expect_int("waterskin no mouth animation",
                     dm1_inventory_consumables_mouth_animation_pc34(&r, mouthFrames,
                                                                    DM1_CONSUMABLE_MOUTH_ANIMATION_FRAME_COUNT_PC34),
                     0);

    c = base_champion();
    ok &= expect_int("empty waterskin no consume", dm1_inventory_consume_water_junk_pc34(&c, 9, 0, &r), 0);
    ok &= expect_int("empty waterskin water unchanged", c.water, 1000);
    DM1_Sound_Init(&soundSystem);
    ok &= expect_int("empty waterskin no swallow route",
                     dm1_inventory_consumables_route_swallow_sound_pc34(&r, &soundSystem, 0, 0), 0);

    c = base_champion();
    ok &= expect_int("ROS potion", dm1_inventory_consume_potion_pc34(&c, 6, 80, 0, 0, &r), 1);
    ok &= expect_int("ROS dexterity delta", c.statistic[DM1_CONSUMABLE_STAT_DEXTERITY], 42);
    ok &= expect_int("potion type becomes empty flask", r.potionTypeAfter, 20);
    ok &= expect_int("potion power preserved", r.potionPowerAfter, 80);

    c = base_champion();
    ok &= expect_int("KU potion", dm1_inventory_consume_potion_pc34(&c, 7, 105, 0, 0, &r), 1);
    ok &= expect_int("KU strength delta", c.statistic[DM1_CONSUMABLE_STAT_STRENGTH], 38);

    c = base_champion();
    ok &= expect_int("DANE potion", dm1_inventory_consume_potion_pc34(&c, 8, 50, 0, 0, &r), 1);
    ok &= expect_int("DANE wisdom delta", c.statistic[DM1_CONSUMABLE_STAT_WISDOM], 42);

    c = base_champion();
    ok &= expect_int("NETA potion", dm1_inventory_consume_potion_pc34(&c, 9, 50, 0, 0, &r), 1);
    ok &= expect_int("NETA vitality delta", c.statistic[DM1_CONSUMABLE_STAT_VITALITY], 43);

    c = base_champion();
    ok &= expect_int("antivenin", dm1_inventory_consume_potion_pc34(&c, 10, 80, 0, 0, &r), 1);
    ok &= expect_int("antivenin clears poison", c.poisonDose, 0);

    c = base_champion();
    ok &= expect_int("MON potion", dm1_inventory_consume_potion_pc34(&c, 11, 80, 0, 0, &r), 1);
    ok &= expect_int("MON stamina restored", c.currentStamina, 74);

    c = base_champion();
    ok &= expect_int("YA potion", dm1_inventory_consume_potion_pc34(&c, 12, 50, 0, 0, &r), 1);
    ok &= expect_int("YA shield", c.shieldDefense, 15);
    ok &= expect_int("YA event type", r.eventType, 72);
    ok &= expect_int("YA event delay", r.eventDelay, 225);

    c = base_champion();
    ok &= expect_int("EE potion", dm1_inventory_consume_potion_pc34(&c, 13, 100, 0, 0, &r), 1);
    ok &= expect_int("EE mana restored", c.currentMana, 96);

    c = base_champion();
    ok &= expect_int("VI potion", dm1_inventory_consume_potion_pc34(&c, 14, 84, viMasks, 1, &r), 1);
    ok &= expect_int("VI health restored", c.currentHealth, 60);
    ok &= expect_int("VI wound mask applied", c.wounds, 0);

    c = base_champion();
    ok &= expect_int("unknown potion still consumed", dm1_inventory_consume_potion_pc34(&c, 99, 80, 0, 0, &r), 1);
    ok &= expect_int("unknown potion leaves strength", c.statistic[DM1_CONSUMABLE_STAT_STRENGTH], 30);
    ok &= expect_int("unknown potion becomes empty flask", r.potionTypeAfter, 20);
    ok &= expect_int("unknown potion power preserved", r.potionPowerAfter, 80);

    c = base_champion();
    ok &= expect_int("water flask potion", dm1_inventory_consume_potion_pc34(&c, 15, 80, 0, 0, &r), 1);
    ok &= expect_int("water flask amount capped", c.water, 2048);
    DM1_Sound_Init(&soundSystem);
    DM1_Sound_SetPartyPosition(&soundSystem, 7, 8, 0, 0);
    ok &= expect_int("potion swallow sound routed",
                     dm1_inventory_consumables_route_swallow_sound_pc34(&r, &soundSystem, 7, 8), 1);
    ok &= expect_int("potion swallow sound flag", r.playSwallowSound, 1);
    ok &= expect_int("potion swallow sound index", soundSystem.lastPlayedSoundIndex, DM1_SND_SWALLOW);
    ok &= expect_int("potion swallow sound played count", soundSystem.totalSoundsPlayed, 1);
    ok &= expect_int("potion no mouth animation",
                     dm1_inventory_consumables_mouth_animation_pc34(&r, mouthFrames,
                                                                    DM1_CONSUMABLE_MOUTH_ANIMATION_FRAME_COUNT_PC34),
                     0);

    printf("inventoryConsumablesInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
