#include "dm1_v1_inventory_consumables_pc34_compat.h"

#include <string.h>

enum {
    DM1_ICON_JUNK_WATER_PC34 = 8,
    DM1_ICON_JUNK_WATERSKIN_PC34 = 9,
    DM1_ICON_JUNK_APPLE_PC34 = 168,
    DM1_ICON_JUNK_IRON_KEY_PC34 = 176,
    DM1_JUNK_TYPE_APPLE_PC34 = 29,
    DM1_JUNK_TYPE_DRAGON_STEAK_PC34 = 36,
    DM1_EVENT_CHAMPION_SHIELD_PC34 = 72
};

enum {
    DM1_POTION_ROS_PC34 = 6,
    DM1_POTION_KU_PC34 = 7,
    DM1_POTION_DANE_PC34 = 8,
    DM1_POTION_NETA_PC34 = 9,
    DM1_POTION_ANTIVENIN_PC34 = 10,
    DM1_POTION_MON_PC34 = 11,
    DM1_POTION_YA_PC34 = 12,
    DM1_POTION_EE_PC34 = 13,
    DM1_POTION_VI_PC34 = 14,
    DM1_POTION_WATER_FLASK_PC34 = 15
};

static const int kFoodAmountsPc34[8] = {
    500, 600, 650, 820, 550, 350, 990, 1400
};

static int clamp_max_pc34(int value, int maximum)
{
    return value > maximum ? maximum : value;
}

static int max_int_pc34(int a, int b)
{
    return a > b ? a : b;
}

static void clear_result(DM1ConsumableResultPc34* outResult)
{
    if (outResult) {
        memset(outResult, 0, sizeof(*outResult));
        outResult->potionTypeAfter = -1;
        outResult->potionPowerAfter = -1;
        outResult->chargeCountAfter = -1;
        outResult->evidence = dm1_inventory_consumables_source_evidence_pc34();
    }
}

const char* dm1_inventory_consumables_source_evidence_pc34(void)
{
    return
        "PANEL.C:1743-1785 F0349 declares mouth-consumption locals; "
        "PANEL.C:1824-1844 gates mouth-allowed objects, water/waterskin charge use, and leader-hand removal; "
        "PANEL.C:1850-1917 applies potion effects and converts all potions to C20 empty flask without clearing Power; "
        "PANEL.C:1898-1910 makes VI heal wounds by applying M006_RANDOM(65536) masks, first max(1, Power/42) times, then one mask per retry for up to 10 tries; "
        "PANEL.C:1918-1919 applies G0242 food amounts for C168..C175 food icons capped at 2048; "
        "PANEL.C:1922-1945 clamps health/stamina, animates removed leader-hand food with C205+!(counter&1) for four 8-tick frames, and plays C08 swallow; "
        "DEFS.H:1-5 defines M006_RANDOM as F0027_MAIN_Get16bitRandomNumber and BASE.C:1688-1695 defines the PC 3.4 16-bit RNG step; "
        "DEFS.H:62-68 defines C08_SOUND_SWALLOW as sound 8 for champion eat/drink; DUNGEON.C:428-436 defines G0242 food amounts; "
        "DUNGEON.C:1108-1127 defines waterskin charge weight and empty-flask weight; "
        "DEFS.H:1468-1481,1517-1524,1891-1947 define potion, junk, and icon constants.";
}

int dm1_inventory_food_amount_from_icon_pc34(int iconIndex)
{
    if (iconIndex < DM1_ICON_JUNK_APPLE_PC34 || iconIndex >= DM1_ICON_JUNK_IRON_KEY_PC34) {
        return 0;
    }
    return kFoodAmountsPc34[iconIndex - DM1_ICON_JUNK_APPLE_PC34];
}

int dm1_inventory_junk_food_icon_from_type_pc34(int junkType)
{
    if (junkType < DM1_JUNK_TYPE_APPLE_PC34 || junkType > DM1_JUNK_TYPE_DRAGON_STEAK_PC34) {
        return -1;
    }
    return DM1_ICON_JUNK_APPLE_PC34 + (junkType - DM1_JUNK_TYPE_APPLE_PC34);
}

int dm1_inventory_consume_water_junk_pc34(DM1ConsumableChampionPc34* champion,
                                          int iconIndex,
                                          int chargeCount,
                                          DM1ConsumableResultPc34* outResult)
{
    clear_result(outResult);
    if (!champion || (iconIndex != DM1_ICON_JUNK_WATER_PC34 &&
                      iconIndex != DM1_ICON_JUNK_WATERSKIN_PC34)) {
        return 0;
    }
    if (chargeCount <= 0) {
        if (outResult) outResult->kind = DM1_CONSUMABLE_RESULT_WATER_JUNK;
        return 0;
    }
    champion->water = (int16_t)clamp_max_pc34((int)champion->water + 800,
                                               DM1_CONSUMABLE_FOOD_WATER_MAX_PC34);
    if (outResult) {
        outResult->kind = DM1_CONSUMABLE_RESULT_WATER_JUNK;
        outResult->consumed = 1;
        outResult->removeLeaderHandObject = 0;
        outResult->playSwallowSound = 1;
        outResult->amountApplied = 800;
        outResult->chargeCountAfter = chargeCount - 1;
    }
    return 1;
}

int dm1_inventory_consume_food_junk_pc34(DM1ConsumableChampionPc34* champion,
                                         int iconIndex,
                                         DM1ConsumableResultPc34* outResult)
{
    const int amount = dm1_inventory_food_amount_from_icon_pc34(iconIndex);
    clear_result(outResult);
    if (!champion || amount <= 0) {
        return 0;
    }
    champion->food = (int16_t)clamp_max_pc34((int)champion->food + amount,
                                              DM1_CONSUMABLE_FOOD_WATER_MAX_PC34);
    if (outResult) {
        outResult->kind = DM1_CONSUMABLE_RESULT_FOOD_JUNK;
        outResult->consumed = 1;
        outResult->removeLeaderHandObject = 1;
        outResult->playSwallowSound = 1;
        outResult->amountApplied = amount;
    }
    return 1;
}

int dm1_inventory_consume_potion_pc34(DM1ConsumableChampionPc34* champion,
                                      int potionType,
                                      int potionPower,
                                      const uint16_t* woundRandomMasks,
                                      int woundRandomMaskCount,
                                      DM1ConsumableResultPc34* outResult)
{
    int counter;
    int adjusted;
    int mana;
    int healIterations;
    uint16_t originalWounds;
    int randomIndex = 0;

    clear_result(outResult);
    if (!champion) {
        return 0;
    }

    counter = ((511 - potionPower) / (32 + ((potionPower + 1) / 8))) >> 1;
    if (counter <= 0) counter = 1;
    adjusted = (potionPower / 25) + 8;

    switch (potionType) {
    case DM1_POTION_ROS_PC34:
        champion->statistic[DM1_CONSUMABLE_STAT_DEXTERITY] += (int16_t)adjusted;
        break;
    case DM1_POTION_KU_PC34:
        champion->statistic[DM1_CONSUMABLE_STAT_STRENGTH] += (int16_t)((potionPower / 35) + 5);
        break;
    case DM1_POTION_DANE_PC34:
        champion->statistic[DM1_CONSUMABLE_STAT_WISDOM] += (int16_t)adjusted;
        break;
    case DM1_POTION_NETA_PC34:
        champion->statistic[DM1_CONSUMABLE_STAT_VITALITY] += (int16_t)adjusted;
        break;
    case DM1_POTION_ANTIVENIN_PC34:
        champion->poisonDose = 0;
        break;
    case DM1_POTION_MON_PC34:
        champion->currentStamina += (int16_t)clamp_max_pc34(
            (int)champion->maximumStamina - (int)champion->currentStamina,
            (int)champion->maximumStamina / counter);
        break;
    case DM1_POTION_YA_PC34:
        adjusted += adjusted >> 1;
        if (champion->shieldDefense > 50) {
            adjusted >>= 2;
        }
        champion->shieldDefense += (int16_t)adjusted;
        if (outResult) {
            outResult->eventType = DM1_EVENT_CHAMPION_SHIELD_PC34;
            outResult->eventDelay = adjusted * adjusted;
        }
        break;
    case DM1_POTION_EE_PC34:
        mana = clamp_max_pc34(900,
                              (int)champion->currentMana + adjusted + (adjusted - 8));
        if (mana > champion->maximumMana) {
            mana -= (mana - max_int_pc34(champion->currentMana, champion->maximumMana)) >> 1;
        }
        champion->currentMana = (int16_t)mana;
        break;
    case DM1_POTION_VI_PC34:
        healIterations = max_int_pc34(1, potionPower / 42);
        champion->currentHealth += (int16_t)((int)champion->maximumHealth / counter);
        originalWounds = champion->wounds;
        if (originalWounds) {
            int tries = 10;
            do {
                int i;
                for (i = 0; i < healIterations; ++i) {
                    uint16_t mask = 0xFFFFu;
                    if (woundRandomMasks && randomIndex < woundRandomMaskCount) {
                        mask = woundRandomMasks[randomIndex++];
                    }
                    champion->wounds &= mask;
                }
                healIterations = 1;
            } while ((originalWounds == champion->wounds) && --tries);
        }
        break;
    case DM1_POTION_WATER_FLASK_PC34:
        champion->water = (int16_t)clamp_max_pc34((int)champion->water + 1600,
                                                   DM1_CONSUMABLE_FOOD_WATER_MAX_PC34);
        break;
    default:
        break;
    }

    if (champion->currentStamina > champion->maximumStamina) {
        champion->currentStamina = champion->maximumStamina;
    }
    if (champion->currentHealth > champion->maximumHealth) {
        champion->currentHealth = champion->maximumHealth;
    }

    if (outResult) {
        outResult->kind = DM1_CONSUMABLE_RESULT_POTION;
        outResult->consumed = 1;
        outResult->removeLeaderHandObject = 0;
        outResult->playSwallowSound = 1;
        outResult->amountApplied = adjusted;
        outResult->potionTypeAfter = DM1_CONSUMABLE_POTION_EMPTY_FLASK_PC34;
        outResult->potionPowerAfter = potionPower;
    }
    return 1;
}

int dm1_inventory_consumables_route_swallow_sound_pc34(const DM1ConsumableResultPc34* result,
                                                       DM1_SoundSystem* soundSystem,
                                                       int16_t partyMapX,
                                                       int16_t partyMapY)
{
    if (!result || !soundSystem || !result->consumed || !result->playSwallowSound) {
        return 0;
    }

    DM1_Sound_RequestPlay(soundSystem, DM1_SND_SWALLOW, partyMapX, partyMapY,
                          DM1_MODE_PLAY_IMMEDIATELY);
    return 1;
}

int dm1_inventory_consumables_mouth_animation_pc34(const DM1ConsumableResultPc34* result,
                                                   DM1ConsumableMouthAnimationFramePc34* frames,
                                                   int maxFrames)
{
    int counter;
    int frameIndex = 0;

    if (!result || !result->consumed || !result->removeLeaderHandObject) {
        return 0;
    }
    if (!frames) {
        return DM1_CONSUMABLE_MOUTH_ANIMATION_FRAME_COUNT_PC34;
    }
    if (maxFrames < DM1_CONSUMABLE_MOUTH_ANIMATION_FRAME_COUNT_PC34) {
        return 0;
    }

    /* ReDMCSB PANEL.C F0349 lines 1928-1938: for counter=5; --counter;
     * draw C205_ICON_MOUTH_OPEN + !(counter & 1), then delay 8. */
    for (counter = 5; --counter; ) {
        frames[frameIndex].iconIndex = DM1_CONSUMABLE_MOUTH_ICON_OPEN_PC34 + !(counter & 1);
        frames[frameIndex].delayTicks = DM1_CONSUMABLE_MOUTH_ANIMATION_DELAY_TICKS_PC34;
        ++frameIndex;
    }
    return frameIndex;
}
