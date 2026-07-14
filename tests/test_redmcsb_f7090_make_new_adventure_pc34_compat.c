#include "redmcsb_f7090_make_new_adventure_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
#define CHECK(label, condition) do { if (!(condition)) { ++failures; fprintf(stderr, "FAIL: %s\n", label); } } while (0)

typedef struct {
    uint8_t random[8];
    unsigned random_index;
    unsigned modifier_calls;
} Fixture;

static uint8_t next_random(void *context)
{
    Fixture *fixture = (Fixture *)context;
    return fixture->random[fixture->random_index++];
}

static void remove_modifier(void *context, uint16_t champion, uint16_t slot,
                            uint16_t thing)
{
    Fixture *fixture = (Fixture *)context;
    (void)champion;
    (void)slot;
    (void)thing;
    ++fixture->modifier_calls;
}

static void populate(CSB_V1_PartyState *party)
{
    unsigned champion;
    unsigned stat;
    unsigned slot;
    memset(party, 0, sizeof(*party));
    party->PartyDirection = 3;
    for (champion = 0U; champion < 2U; ++champion) {
        CSB_V1_Champion *c = &party->Champions[champion];
        c->Cell = 0U;
        c->Direction = 3U;
        c->ActionIndex = CSB_V1_ACTION_ATTACK;
        c->SymbolStep = 3U;
        c->Incantation[0] = 12;
        c->Attributes = CSB_V1_CHAMPION_ATTRIBUTE_MALE |
                        CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
        c->MaximumHealth = 100 + (int)champion;
        c->MaximumStamina = 200 + (int)champion;
        c->MaximumMana = 30 + (int)champion;
        c->CurrentHealth = 1;
        c->CurrentStamina = 2;
        c->CurrentMana = 3;
        c->DirectionMaximumDamageReceived = 4U;
        c->MaximumDamageReceived = 5U;
        c->PoisonEventCount = 6U;
        c->EnableActionEventIndex = 7;
        c->HideDamageReceivedEventIndex = 8;
        c->Wounds = 9U;
        c->ActionDefense = 10U;
        c->Load = 11U;
        c->ShieldStrength = 12U;
        for (stat = 0U; stat < CSB_V1_STAT_COUNT; ++stat) {
            c->Statistics[stat][CSB_V1_STAT_MAX] = (uint16_t)(50U + stat);
            c->Statistics[stat][CSB_V1_STAT_CUR] = 1U;
        }
        for (stat = 0U; stat < CSB_V1_FULL_SKILL_COUNT; ++stat) {
            c->SkillTemporaryExperience[stat] = 9;
        }
        for (slot = 0U; slot < CSB_V1_SLOT_COUNT; ++slot) {
            c->Slots[slot] = (uint16_t)(0x100U + slot);
        }
    }
}

int main(void)
{
    CSB_V1_SaveHeader source_header;
    CSB_V1_SaveHeader destination_header;
    CSB_V1_PartyState source_party;
    CSB_V1_PartyState destination_party;
    Fixture fixture = { { 1U, 2U, 3U, 4U }, 0U, 0U };
    uint16_t platform = 0U;
    uint16_t useless = 0U;
    unsigned slot;

    memset(&source_header, 0xA5, sizeof(source_header));
    memset(&destination_header, 0x5A, sizeof(destination_header));
    source_header.GameID = 0x4242U;
    populate(&source_party);
    populate(&destination_party);
    source_party.PartyDirection = 2;

    CHECK("F7089 scan retains first unused cell",
          redmcsb_f7089_get_first_cell_with_no_champion_pc34(&destination_party, 2U) == 1U);
    CHECK("F7090 accepts complete live dependencies",
          redmcsb_f7090_make_new_adventure_pc34(
              &source_header, 7U, &destination_header, &platform, &useless,
              &source_party, &destination_party, 2U, next_random,
              remove_modifier, &fixture) == 1);
    CHECK("header tail [256,296) is source-owned",
          memcmp((const uint8_t *)&source_header + 256U,
                 (const uint8_t *)&destination_header + 256U, 40U) == 0);
    CHECK("header increments game id and carries platform",
          destination_header.GameID == 0x4243U && platform == 7U && useless == 2U);
    CHECK("all original object modifiers were consulted",
          fixture.modifier_calls == 2U * CSB_V1_SLOT_COUNT);
    CHECK("imported party keeps loaded count", destination_party.ChampionCount == 2);
    CHECK("collision uses F7089 first free cell",
          destination_party.Champions[0].Cell == 1U &&
          destination_party.Champions[1].Cell == 0U);
    CHECK("rotation applies to direction",
          destination_party.Champions[0].Direction == 0U);
    CHECK("status and resources are reset from source maxima/random",
          destination_party.Champions[0].ActionIndex == CSB_V1_ACTION_NONE &&
          destination_party.Champions[0].SymbolStep == 0U &&
          destination_party.Champions[0].Incantation[0] == 0 &&
          destination_party.Champions[0].CurrentHealth == 100 &&
          destination_party.Champions[0].CurrentStamina == 200 &&
          destination_party.Champions[0].CurrentMana == 30 &&
          destination_party.Champions[0].Food == 1501 &&
          destination_party.Champions[0].Water == 1502 &&
          destination_party.Champions[1].Food == 1503 &&
          destination_party.Champions[1].Water == 1504);
    CHECK("only source male bit survives",
          destination_party.Champions[0].Attributes == CSB_V1_CHAMPION_ATTRIBUTE_MALE);
    for (slot = 0U; slot < CSB_V1_SLOT_COUNT; ++slot) {
        CHECK("slots clear after modifier removal",
              destination_party.Champions[0].Slots[slot] == REDMCSB_F7090_PC34_THING_NONE);
    }
    CHECK("stat and temporary experience reset",
          destination_party.Champions[0].Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_MIN] == 10U &&
          destination_party.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] == 50U &&
          destination_party.Champions[0].SkillTemporaryExperience[19] == 0);

    CHECK("source evidence is available",
          strstr(redmcsb_f7090_make_new_adventure_pc34_source_evidence(), "F7090") != NULL);
    CHECK("missing live dependencies fail closed",
          redmcsb_f7090_make_new_adventure_pc34(
              &source_header, 7U, &destination_header, &platform, &useless,
              &source_party, &destination_party, 2U, NULL,
              remove_modifier, &fixture) == 0 &&
          redmcsb_f7090_make_new_adventure_pc34(
              &source_header, 7U, &destination_header, &platform, &useless,
              &source_party, &destination_party, 5U, next_random,
              remove_modifier, &fixture) == 0);

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("PASSED: ReDMCSB F7089/F7090 new-adventure normalization");
    return 0;
}
