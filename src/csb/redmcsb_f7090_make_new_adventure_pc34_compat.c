#include "redmcsb_f7090_make_new_adventure_pc34_compat.h"

#include <string.h>

uint8_t redmcsb_f7089_get_first_cell_with_no_champion_pc34(
    const CSB_V1_PartyState *party, uint16_t loaded_champion_count)
{
    uint8_t available[REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT];
    uint16_t i;

    if (party == NULL ||
        loaded_champion_count > REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT) {
        return REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT;
    }
    for (i = 0U; i < REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT; ++i) {
        available[i] = 1U;
    }
    for (i = 0U; i < loaded_champion_count; ++i) {
        uint8_t cell = party->Champions[i].Cell;
        if (cell < REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT) {
            available[cell] = 0U;
        }
    }
    for (i = 0U; i < REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT; ++i) {
        if (available[i] != 0U) {
            return (uint8_t)i;
        }
    }
    return REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT;
}

static void normalize_champion(CSB_V1_Champion *champion, uint8_t rotation,
                               uint16_t champion_index,
                               RedmcsbF7090RandomBytePc34 random_byte,
                               RedmcsbF7090RemoveObjectModifierPc34 remove,
                               void *context)
{
    uint16_t stat;
    uint16_t slot;

    if (rotation != 0U) {
        champion->Cell = (uint8_t)((champion->Cell + rotation) & 3U);
        champion->Direction = (uint8_t)((champion->Direction + rotation) & 3U);
    }
    champion->ActionIndex = CSB_V1_ACTION_NONE;
    champion->SymbolStep = 0U;
    champion->Incantation[0] = 0;
    champion->DirectionMaximumDamageReceived = 0U;
    champion->MaximumDamageReceived = 0U;
    champion->PoisonEventCount = 0U;
    champion->EnableActionEventIndex = -1;
    champion->HideDamageReceivedEventIndex = -1;
    champion->Attributes &= CSB_V1_CHAMPION_ATTRIBUTE_MALE;
    champion->Wounds = 0U;
    champion->CurrentHealth = champion->MaximumHealth;
    champion->CurrentStamina = champion->MaximumStamina;
    champion->CurrentMana = champion->MaximumMana;
    champion->ActionDefense = 0U;
    champion->Food = (int16_t)((int)random_byte(context) + 1500);
    champion->Water = (int16_t)((int)random_byte(context) + 1500);
    for (stat = 0U; stat < CSB_V1_STAT_COUNT; ++stat) {
        champion->Statistics[stat][CSB_V1_STAT_MIN] = 30U;
        champion->Statistics[stat][CSB_V1_STAT_CUR] =
            champion->Statistics[stat][CSB_V1_STAT_MAX];
    }
    champion->Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MIN] = 10U;
    for (stat = 0U; stat < CSB_V1_FULL_SKILL_COUNT; ++stat) {
        champion->SkillTemporaryExperience[stat] = 0;
    }
    for (slot = 0U; slot < CSB_V1_SLOT_COUNT; ++slot) {
        remove(context, champion_index, slot, champion->Slots[slot]);
        champion->Slots[slot] = REDMCSB_F7090_PC34_THING_NONE;
    }
    champion->Load = 0U;
    champion->ShieldStrength = 0U;
}

int redmcsb_f7090_make_new_adventure_pc34(
    const CSB_V1_SaveHeader *source_header, uint16_t source_platform,
    CSB_V1_SaveHeader *destination_header, uint16_t *destination_platform,
    uint16_t *destination_useless, const CSB_V1_PartyState *source_party,
    CSB_V1_PartyState *destination_party, uint16_t loaded_champion_count,
    RedmcsbF7090RandomBytePc34 random_byte,
    RedmcsbF7090RemoveObjectModifierPc34 remove_object_modifier,
    void *context)
{
    int rotation;
    uint16_t champion;

    if (source_header == NULL || destination_header == NULL ||
        destination_platform == NULL || destination_useless == NULL ||
        source_party == NULL || destination_party == NULL ||
        random_byte == NULL || remove_object_modifier == NULL ||
        loaded_champion_count > REDMCSB_F7090_PC34_IMPORTED_PARTY_LIMIT) {
        return 0;
    }

    memcpy((uint8_t *)destination_header + REDMCSB_F7090_PC34_HEADER_TAIL_OFFSET,
           (const uint8_t *)source_header + REDMCSB_F7090_PC34_HEADER_TAIL_OFFSET,
           REDMCSB_F7090_PC34_HEADER_TAIL_SIZE);
    destination_header->GameID = (uint16_t)(source_header->GameID + 1U);
    *destination_platform = source_platform;
    *destination_useless = 2U;
    destination_party->ChampionCount = (int)loaded_champion_count;

    rotation = destination_party->PartyDirection - source_party->PartyDirection;
    if (rotation != 0) {
        rotation += 4;
    }
    for (champion = 0U; champion < loaded_champion_count; ++champion) {
        normalize_champion(&destination_party->Champions[champion],
                            (uint8_t)(rotation & 3), champion, random_byte,
                            remove_object_modifier, context);
    }
    for (champion = 0U; champion < loaded_champion_count; ++champion) {
        uint16_t other;
        uint8_t cell = destination_party->Champions[champion].Cell;

        for (other = 0U; other < loaded_champion_count; ++other) {
            if (other != champion &&
                destination_party->Champions[other].Cell == cell) {
                destination_party->Champions[other].Cell =
                    redmcsb_f7089_get_first_cell_with_no_champion_pc34(
                        destination_party, loaded_champion_count);
            }
        }
    }
    return 1;
}

const char *redmcsb_f7090_make_new_adventure_pc34_source_evidence(void)
{
    return "ReDMCSB CEDT008.C F7089_GetFirstCellWithNoChampion; "
           "CEDTINCI.C F7090_MakeNewAdventure; "
           "CEDTINCR.C F7088_CopyChampionPortraits; "
           "F7020_RemoveObjectModifiersFromStatistics";
}
