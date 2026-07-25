#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0745_set_file_names_according_to_language_pc34_compat.h"

typedef struct {
    char dungeon[64];
    char expansion[64];
    char bonus[64];
    char save[64];
    char backup[64];
    char graphics[64];
    redmcsb_f0745_file_names_pc34_compat file_names;
} redmcsb_f0745_fixture_pc34_compat;

static void redmcsb_f0745_fixture_initialize(
    redmcsb_f0745_fixture_pc34_compat *fixture)
{
    (void)strcpy(fixture->dungeon, "DungeonMaster:Dungeon~.dat");
    (void)strcpy(fixture->expansion, "DungeonMaster:Dungeon~.FTL");
    (void)strcpy(fixture->bonus, "DungeonMaster:DungeonB~.dat");
    (void)strcpy(fixture->save, "DF0:DMGAME~~.DAT");
    (void)strcpy(fixture->backup, "DF0:DMGAME~.BAK");
    (void)strcpy(fixture->graphics, "DungeonMaster:graphics.dat");
    fixture->file_names.dungeon_file_name = fixture->dungeon;
    fixture->file_names.expansion_set_dungeon_file_name = fixture->expansion;
    fixture->file_names.bonus_dungeon_file_name = fixture->bonus;
    fixture->file_names.saved_game_file_name = fixture->save;
    fixture->file_names.saved_game_backup_file_name = fixture->backup;
}

static void redmcsb_f0745_assert_file_names(
    const redmcsb_f0745_fixture_pc34_compat *fixture,
    const char *dungeon,
    const char *expansion,
    const char *bonus,
    const char *save,
    const char *backup)
{
    (void)backup;
    (void)save;
    (void)bonus;
    (void)expansion;
    (void)dungeon;
    (void)fixture;
    assert(strcmp(fixture->dungeon, dungeon) == 0);
    assert(strcmp(fixture->expansion, expansion) == 0);
    assert(strcmp(fixture->bonus, bonus) == 0);
    assert(strcmp(fixture->save, save) == 0);
    assert(strcmp(fixture->backup, backup) == 0);
    assert(strcmp(fixture->graphics, "DungeonMaster:graphics.dat") == 0);
}

int main(void)
{
    redmcsb_f0745_fixture_pc34_compat fixture;

    redmcsb_f0745_fixture_initialize(&fixture);
    redmcsb_f0745_set_file_names_according_to_language_pc34_compat(
        &fixture.file_names, REDMCSB_F0745_LANGUAGE_ENGLISH_PC34_COMPAT);
    redmcsb_f0745_assert_file_names(&fixture,
                                    "DungeonMaster:Dungeon.dat",
                                    "DungeonMaster:Dungeon.FTL",
                                    "DungeonMaster:DungeonB.dat",
                                    "DF0:DMGAME.DAT",
                                    "DF0:DMGAME.BAK");

    redmcsb_f0745_fixture_initialize(&fixture);
    redmcsb_f0745_set_file_names_according_to_language_pc34_compat(
        &fixture.file_names, REDMCSB_F0745_LANGUAGE_FRENCH_PC34_COMPAT);
    redmcsb_f0745_assert_file_names(&fixture,
                                    "DungeonMaster:DungeonF.dat",
                                    "DungeonMaster:DungeonF.FTL",
                                    "DungeonMaster:DungeonBF.dat",
                                    "DF0:DMGAMEFF.DAT",
                                    "DF0:DMGAMEF.BAK");

    redmcsb_f0745_set_file_names_according_to_language_pc34_compat(
        &fixture.file_names, REDMCSB_F0745_LANGUAGE_GERMAN_PC34_COMPAT);
    redmcsb_f0745_assert_file_names(&fixture,
                                    "DungeonMaster:DungeonF.dat",
                                    "DungeonMaster:DungeonF.FTL",
                                    "DungeonMaster:DungeonBF.dat",
                                    "DF0:DMGAMEFF.DAT",
                                    "DF0:DMGAMEF.BAK");

    redmcsb_f0745_fixture_initialize(&fixture);
    redmcsb_f0745_set_file_names_according_to_language_pc34_compat(
        &fixture.file_names, REDMCSB_F0745_LANGUAGE_GERMAN_PC34_COMPAT);
    redmcsb_f0745_assert_file_names(&fixture,
                                    "DungeonMaster:DungeonG.dat",
                                    "DungeonMaster:DungeonG.FTL",
                                    "DungeonMaster:DungeonBG.dat",
                                    "DF0:DMGAMEGG.DAT",
                                    "DF0:DMGAMEG.BAK");

    assert(strstr(
               redmcsb_f0745_set_file_names_according_to_language_source_evidence_pc34(),
               "FILENAME.C:84-105") != NULL);

    puts("ok: ReDMCSB F0745 PC 3.4 language filename mutation");
    return 0;
}
