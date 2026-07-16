#include "dm1_v1_dungeon_data_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass;
static int g_fail;

static void expect_int(const char *label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        ++g_fail;
    } else {
        ++g_pass;
    }
}

static void expect_contains(const char *label, const char *text, const char *needle)
{
    if (!text || !needle || !strstr(text, needle)) {
        fprintf(stderr, "FAIL %s missing '%s'\n", label, needle ? needle : "(null)");
        ++g_fail;
    } else {
        ++g_pass;
    }
}

static void seed_loaded_dungeon(DM1_V1_DungeonDataPc34 *dd)
{
    DM1_V1_DungeonData_InitPc34Compat(dd);
    dd->loaded = true;
    dd->dungeon.header.level_count = 3;
    dd->dungeon.header.levels[0].width = 4;
    dd->dungeon.header.levels[0].height = 5;
    dd->dungeon.header.levels[1].width = 11;
    dd->dungeon.header.levels[1].height = 12;
    dd->dungeon.header.levels[2].width = 20;
    dd->dungeon.header.levels[2].height = 21;
    dd->currentMapIndex = 0;
    dd->currentMapWidth = 4;
    dd->currentMapHeight = 5;
    dd->party.mapIndex = 0;
    dd->party.posX = 7;
    dd->party.posY = 8;
    dd->party.facing = 2;
}

static void test_f0173_updates_current_map_only(void)
{
    DM1_V1_DungeonDataPc34 dd;
    DM1_V1_DungeonDataF0173SetCurrentMapReceiptPc34 receipt;

    seed_loaded_dungeon(&dd);
    expect_int("f0173_accept",
               DM1_V1_DungeonData_F0173SetCurrentMapPc34Compat(&dd, 1, &receipt),
               1);
    expect_int("f0173_receipt_accepted", receipt.accepted, 1);
    expect_int("f0173_previous_current", receipt.previousCurrentMapIndex, 0);
    expect_int("f0173_current_map", dd.currentMapIndex, 1);
    expect_int("f0173_width", dd.currentMapWidth, 11);
    expect_int("f0173_height", dd.currentMapHeight, 12);
    expect_int("f0173_party_map_unchanged", dd.party.mapIndex, 0);
    expect_int("f0173_party_x_unchanged", dd.party.posX, 7);
    expect_int("f0173_party_y_unchanged", dd.party.posY, 8);
    expect_int("f0173_party_dir_unchanged", dd.party.facing, 2);
    expect_contains("f0173_source", receipt.sourceEvidence,
                    "F0173_DUNGEON_SetCurrentMap");
}

static void test_f0174_updates_current_and_party_map(void)
{
    DM1_V1_DungeonDataPc34 dd;
    DM1_V1_DungeonDataF0174SetCurrentAndPartyMapReceiptPc34 receipt;

    seed_loaded_dungeon(&dd);
    expect_int("f0174_accept",
               DM1_V1_DungeonData_F0174SetCurrentMapAndPartyMapPc34Compat(&dd, 2, &receipt),
               1);
    expect_int("f0174_receipt_accepted", receipt.accepted, 1);
    expect_int("f0174_previous_current", receipt.previousCurrentMapIndex, 0);
    expect_int("f0174_previous_party", receipt.previousPartyMapIndex, 0);
    expect_int("f0174_current_map", dd.currentMapIndex, 2);
    expect_int("f0174_party_map", dd.party.mapIndex, 2);
    expect_int("f0174_width", dd.currentMapWidth, 20);
    expect_int("f0174_height", dd.currentMapHeight, 21);
    expect_int("f0174_party_x_unchanged", dd.party.posX, 7);
    expect_int("f0174_party_y_unchanged", dd.party.posY, 8);
    expect_int("f0174_party_dir_unchanged", dd.party.facing, 2);
    expect_contains("f0174_source", receipt.sourceEvidence,
                    "F0174_DUNGEON_SetCurrentMapAndPartyMap");
}

static void test_invalid_map_rejects_without_mutation(void)
{
    DM1_V1_DungeonDataPc34 dd;
    DM1_V1_DungeonDataF0174SetCurrentAndPartyMapReceiptPc34 receipt;

    seed_loaded_dungeon(&dd);
    expect_int("invalid_reject",
               DM1_V1_DungeonData_F0174SetCurrentMapAndPartyMapPc34Compat(&dd, 9, &receipt),
               0);
    expect_int("invalid_receipt_reject", receipt.accepted, 0);
    expect_int("invalid_current_preserved", dd.currentMapIndex, 0);
    expect_int("invalid_party_preserved", dd.party.mapIndex, 0);
    expect_int("invalid_width_preserved", dd.currentMapWidth, 4);
    expect_int("invalid_height_preserved", dd.currentMapHeight, 5);
}

int main(void)
{
    test_f0173_updates_current_map_only();
    test_f0174_updates_current_and_party_map();
    test_invalid_map_rejects_without_mutation();

    if (g_fail) {
        fprintf(stderr, "FAIL dm1_v1_f0173_f0174_current_map_pc34_compat pass=%d fail=%d\n",
                g_pass, g_fail);
        return 1;
    }
    printf("PASS dm1_v1_f0173_f0174_current_map_pc34_compat checks=%d\n", g_pass);
    return 0;
}
