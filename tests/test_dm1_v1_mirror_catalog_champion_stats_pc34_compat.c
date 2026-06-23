#include "memory_champion_state_pc34_compat.h"
#include "memory_champion_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void expect_int(const char* label, int got, int want)
{
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d\n", label, got, want);
    } else {
        printf("PASS %s=%d\n", label, got);
    }
}

static void expect_nonzero(const char* label, int value)
{
    if (!value) {
        ++g_failures;
        printf("FAIL %s got=0\n", label);
    } else {
        printf("PASS %s=%d\n", label, value);
    }
}

int main(void)
{
    struct ChampionState_Compat parsed;
    struct ChampionMirrorCatalog_Compat catalog;
    struct PartyState_Compat party;
    struct ChampionState_Compat* recruited;
    char name[CHAMPION_NAME_TEXT_CAPACITY];
    char title[CHAMPION_TITLE_TEXT_CAPACITY];

    printf("probe=dm1_v1_mirror_catalog_champion_stats_pc34_compat\n");
    printf("sourceEvidence=REVIVE.C F0280:227-245 decodes HP/stamina/mana/statistics from mirror text; memory_champion_state F0606/F0673 copies decoded record into party\n");

    F0600_CHAMPION_InitEmpty_Compat(&parsed);
    expect_nonzero(
        "parse mirror identity",
        F0606_CHAMPION_ParseMirrorTextIdentity_Compat(
            "HALK|THE BRAVE||M|AAGEAAHIAABJAAAA|AABOCACCCECGCIAAAA|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
            &parsed));

    memset(&catalog, 0, sizeof(catalog));
    catalog.count = 1;
    catalog.records[0].textStringIndex = 7;
    catalog.records[0].mirrorOrdinal = 3;
    catalog.records[0].champion = parsed;
    (void)F0628_CHAMPION_UnpackName_Compat(
        &parsed, catalog.records[0].nameText, sizeof(catalog.records[0].nameText));
    (void)F0629_CHAMPION_UnpackTitle_Compat(
        &parsed, catalog.records[0].titleText, sizeof(catalog.records[0].titleText));

    memset(&party, 0, sizeof(party));
    party.activeChampionIndex = -1;
    party.direction = DIR_EAST;
    F0600_CHAMPION_InitEmpty_Compat(&party.champions[0]);
    F0600_CHAMPION_InitEmpty_Compat(&party.champions[1]);
    F0600_CHAMPION_InitEmpty_Compat(&party.champions[2]);
    F0600_CHAMPION_InitEmpty_Compat(&party.champions[3]);

    expect_nonzero("catalog record valid",
                   F0670_CHAMPION_MirrorCatalogAllRecordsValid_Compat(&catalog));
    expect_nonzero("catalog recruit ordinal",
                   F0673_CHAMPION_MirrorCatalogRecruitOrdinalIfAbsent_Compat(
                       &catalog, 3, &party));

    recruited = &party.champions[0];
    expect_int("party count", party.championCount, 1);
    expect_int("active champion slot", party.activeChampionIndex, 0);
    expect_int("present", recruited->present, 1);
    expect_int("portrait text index", recruited->portraitIndex, 7);
    expect_int("party direction copied", recruited->direction, DIR_EAST);
    expect_int("decoded health", recruited->hp.maximum, 100);
    expect_int("decoded stamina", recruited->stamina.maximum, 120);
    expect_int("decoded mana", recruited->mana.maximum, 25);
    expect_int("decoded strength", recruited->attributes[CHAMPION_ATTR_STRENGTH], 30);
    expect_int("decoded dexterity", recruited->attributes[CHAMPION_ATTR_DEXTERITY], 32);
    expect_int("decoded wisdom", recruited->attributes[CHAMPION_ATTR_WISDOM], 34);
    expect_int("decoded vitality", recruited->attributes[CHAMPION_ATTR_VITALITY], 36);
    expect_int("decoded antimagic", recruited->attributes[CHAMPION_ATTR_ANTIMAGIC], 38);
    expect_int("decoded antifire", recruited->attributes[CHAMPION_ATTR_ANTIFIRE], 40);
    expect_int("strength maximum copied",
               recruited->attributeMaximums[CHAMPION_ATTR_STRENGTH], 30);
    expect_int("stamina is not flat default", recruited->stamina.maximum != 100, 1);
    expect_int("mana is not flat default", recruited->mana.maximum != 50, 1);
    expect_int("decoded source maximum load", recruited->maxLoad, 340);
    expect_int("empty resurrected movement ticks",
               F0841_LIFECYCLE_ComputeMoveTicks_Compat(
                   recruited->load,
                   recruited->maxLoad,
                   recruited->wounds,
                   LIFECYCLE_ICON_NONE),
               2);

    name[0] = '\0';
    title[0] = '\0';
    (void)F0660_CHAMPION_MirrorCatalogGetName_Compat(&catalog, 3, name, sizeof(name));
    (void)F0661_CHAMPION_MirrorCatalogGetTitle_Compat(&catalog, 3, title, sizeof(title));
    expect_int("name lookup", strcmp(name, "HALK") == 0, 1);
    expect_int("title lookup", strcmp(title, "THE BRAVE") == 0, 1);

    if (g_failures) {
        printf("FAIL assertions failed=%d\n", g_failures);
        return 1;
    }
    printf("SUMMARY failures=0\n");
    return 0;
}
