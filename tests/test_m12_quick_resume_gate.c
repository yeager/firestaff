#include "menu_startup_m12.h"
#include "csbwin_resume_fixture.h"
#include "config_m12.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "dm1_v1_save_load.h"
#include "dm1_v1_original_save_pc34_handoff.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


static int m12_test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static int expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

static void force_dm1_available(M12_StartupMenuState* state) {
    state->entries[0].title = "DUNGEON MASTER";
    state->entries[0].gameId = "dm1";
    state->entries[0].kind = M12_MENU_ENTRY_GAME;
    state->entries[0].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[0].available = 1;
    state->assetStatus.versions[0][0].gameId = "dm1";
    state->assetStatus.versions[0][0].versionId = "pc34";
    state->assetStatus.versions[0][0].label = "PC 3.4";
    state->assetStatus.versions[0][0].shortLabel = "PC34";
    state->assetStatus.versions[0][0].matched = 1;
    state->assetStatus.dm1Available = 1;
    state->gameOptions[0].versionIndex = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->view = M12_MENU_VIEW_MAIN;
}

static void force_csb_available(M12_StartupMenuState* state) {
    state->entries[1].title = "CHAOS STRIKES BACK";
    state->entries[1].gameId = "csb";
    state->entries[1].kind = M12_MENU_ENTRY_GAME;
    state->entries[1].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[1].available = 1;
    state->assetStatus.originalFileCandidateFound = 1;
    state->assetStatus.csbAvailable = 1;
    state->assetStatus.versions[1][0].gameId = "csb";
    state->assetStatus.versions[1][0].versionId = "pc34-en";
    state->assetStatus.versions[1][0].label = "PC 3.4 English";
    state->assetStatus.versions[1][0].shortLabel = "PC34 EN";
    state->assetStatus.versions[1][0].matched = 1;
    state->assetStatus.requiredFileCounts[1] = 2;
    state->assetStatus.requiredFiles[1][0].gameId = "csb";
    state->assetStatus.requiredFiles[1][0].roleId = "graphics";
    state->assetStatus.requiredFiles[1][0].label = "GRAPHICS.DAT";
    state->assetStatus.requiredFiles[1][0].required = 1;
    state->assetStatus.requiredFiles[1][0].matched = 1;
    state->assetStatus.requiredFiles[1][1].gameId = "csb";
    state->assetStatus.requiredFiles[1][1].roleId = "dungeon";
    state->assetStatus.requiredFiles[1][1].label = "DUNGEON.DAT";
    state->assetStatus.requiredFiles[1][1].required = 1;
    state->assetStatus.requiredFiles[1][1].matched = 1;
    state->gameOptions[1].versionIndex = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->view = M12_MENU_VIEW_MAIN;
    state->activatedIndex = -1;
    state->launchRequested = 0;
    state->quickResumeLaunchRequested = 0;
    state->messageLine1 = "";
    state->messageLine2 = "";
    state->messageLine3 = "";
}

static int write_fake_quicksave(const char* path) {
    static const unsigned char hdr[16] = {
        'F','S','M','1','1','Q','S','1',
        4,0,0,0,
        0,0,0,0
    };
    static const unsigned char blob[4] = { 0, 0, 0, 0 };
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr) ||
        fwrite(blob, 1U, sizeof(blob), fp) != sizeof(blob) ||
        fclose(fp) != 0) {
        return 0;
    }
    return 1;
}

static int write_serialized_csb_quicksave(const char* path) {
    CSB_V1_RuntimeProfile runtime;
    int rc;

    csb_v1_runtime_init(&runtime, NULL);
    runtime.variant_id = CSB_V1_VARIANT_PC34_EN;
    runtime.party_x = CSB_V1_START_PARTY_X + 3;
    runtime.party_y = CSB_V1_START_PARTY_Y + 2;
    runtime.party_dir = CSB_V1_DIR_EAST;
    runtime.magic_caster_index = 1;
    runtime.game_time = 42U;
    runtime.tick_count = 42U;
    runtime.total_play_ms = 42ULL * (uint64_t)CSB_V1_TICK_MS_NOMINAL;
    runtime.party_state.PartyMapX = runtime.party_x;
    runtime.party_state.PartyMapY = runtime.party_y;
    runtime.party_state.PartyDirection = (uint8_t)runtime.party_dir;
    runtime.party_state.MagicCasterIndex = runtime.magic_caster_index;

    rc = csb_v1_runtime_save_game_to_path(&runtime, path);
    csb_v1_runtime_cleanup(&runtime);
    return rc == CSB_V1_SAVE_OK;
}

static void fill_raw_csbgame_champion(CSB_V1_Champion* champ,
                                      const char* name,
                                      int hp,
                                      int cell) {
    int i;
    if (!champ) return;
    memset(champ, 0, sizeof(*champ));
    snprintf(champ->Name, sizeof(champ->Name), "%s", name);
    champ->CurrentHealth = (int16_t)hp;
    champ->MaximumHealth = (int16_t)hp;
    champ->CurrentStamina = (int16_t)(hp + 10);
    champ->MaximumStamina = (int16_t)(hp + 10);
    champ->CurrentMana = (int16_t)(hp / 2);
    champ->MaximumMana = (int16_t)(hp / 2);
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        champ->Statistics[i][0] = (uint16_t)(20 + i);
        champ->Statistics[i][1] = (uint16_t)(30 + i);
        champ->Statistics[i][2] = (uint16_t)(40 + i);
    }
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        champ->Skills[i] = (uint8_t)(i + 1);
    }
    champ->Cell = (uint8_t)cell;
    champ->Direction = CSB_V1_DIR_EAST;
}

static int write_raw_csbgame_roster_quicksave(const char* path) {
    CSB_V1_PartyState party;
    unsigned char buf[CSB_SAVE_HEADER_SIZE + CSB_SAVE_CHAMP_SIZE * 2];
    long len;
    FILE* fp;
    int ok;

    csb_v1_character_init_default(&party);
    party.ChampionCount = 2;
    party.LeaderIndex = 0;
    party.MagicCasterIndex = 0;
    party.PartyMapX = CSB_V1_START_PARTY_X + 4;
    party.PartyMapY = CSB_V1_START_PARTY_Y + 5;
    party.PartyDirection = CSB_V1_DIR_EAST;
    fill_raw_csbgame_champion(&party.Champions[0], "ROSTERA", 96,
                              CSB_V1_CELL_FRONT_LEFT);
    fill_raw_csbgame_champion(&party.Champions[1], "ROSTERB", 88,
                              CSB_V1_CELL_RIGHT);
    len = csb_v1_build_csb_save_buffer(&party, CSB_SAVE_VERSION_V21,
                                       buf, (long)sizeof(buf));
    if (len <= 0) {
        return 0;
    }

    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    ok = fwrite(buf, 1u, (size_t)len, fp) == (size_t)len &&
         fclose(fp) == 0;
    return ok;
}

static void wr32le(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
    p[2] = (unsigned char)((v >> 16) & 0xffu);
    p[3] = (unsigned char)((v >> 24) & 0xffu);
}

static void fill_test_portrait(struct ChampionState_Compat* champ,
                               unsigned char seed) {
    int i;
    if (!champ) return;
    for (i = 0; i < CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT; ++i) {
        champ->portraitBitmap[i] =
            (unsigned char)((seed + (unsigned char)(i * 17)) & 0xffu);
    }
    champ->portraitBitmapValid = 1;
}

static int write_serialized_dm1_quicksave(const char* path) {
    struct GameWorld_Compat world;
    unsigned char* blob = NULL;
    int blobSize;
    int written = 0;
    uint32_t hash = 0;
    unsigned char hdr[16];
    FILE* fp;
    int ok = 0;
    int i;

    memset(&world, 0, sizeof(world));
    F0881_WORLD_InitDefault_Compat(&world, 0x12345678u);
    world.gameTick = 6789u;
    world.partyMapIndex = 5;
    world.party.mapIndex = 5;
    world.party.mapX = 14;
    world.party.mapY = 16;
    world.party.direction = DIR_WEST;
    world.party.championCount = 1;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&world.party.champions[i]);
    }
    world.party.champions[0].present = 1;
    memcpy(world.party.champions[0].name, "HALK    ", CHAMPION_NAME_LENGTH);
    world.party.champions[0].hp.current = 88;
    world.party.champions[0].hp.maximum = 99;
    fill_test_portrait(&world.party.champions[0], 0x21u);

    world.creatureAICount = 1;
    memset(&world.creatureAI[0], 0, sizeof(world.creatureAI[0]));
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = CREATURE_TYPE_SKELETON;
    world.creatureAI[0].groupMapIndex = world.partyMapIndex;
    world.creatureAI[0].groupMapX = 7;
    world.creatureAI[0].groupMapY = 8;
    world.creatureAI[0].groupCells = 0x55;
    world.creatureAI[0].groupDirection = DIR_EAST;
    world.creatureAI[0].reserved0 = 3;

    blobSize = F0899_WORLD_SerializedSize_Compat(&world);
    blob = (unsigned char*)malloc((size_t)blobSize);
    if (!blob) {
        F0883_WORLD_Free_Compat(&world);
        return 0;
    }
    if (!F0891_ORCH_WorldHash_Compat(&world, &hash) ||
        !F0897_WORLD_Serialize_Compat(&world, blob, blobSize, &written) ||
        written != blobSize) {
        goto done;
    }

    memcpy(hdr, "FSM11QS1", 8u);
    wr32le(hdr + 8, (uint32_t)blobSize);
    wr32le(hdr + 12, hash);
    fp = fopen(path, "wb");
    if (!fp) {
        goto done;
    }
    ok = fwrite(hdr, 1u, sizeof(hdr), fp) == sizeof(hdr) &&
         fwrite(blob, 1u, (size_t)blobSize, fp) == (size_t)blobSize &&
         fclose(fp) == 0;

done:
    free(blob);
    F0883_WORLD_Free_Compat(&world);
    return ok;
}

static int write_native_dm1_save(const char* path) {
    struct GameWorld_Compat world;
    int i;
    int rc;

    memset(&world, 0, sizeof(world));
    F0881_WORLD_InitDefault_Compat(&world, 0x42525331u);
    world.gameTick = 2468u;
    world.partyMapIndex = 2;
    world.party.mapIndex = 2;
    world.party.mapX = 9;
    world.party.mapY = 11;
    world.party.direction = DIR_NORTH;
    world.party.championCount = 1;
    world.party.activeChampionIndex = 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&world.party.champions[i]);
    }
    world.party.champions[0].present = 1;
    memcpy(world.party.champions[0].name, "TIGGY   ", CHAMPION_NAME_LENGTH);
    world.party.champions[0].hp.current = 44;
    world.party.champions[0].hp.maximum = 55;
    fill_test_portrait(&world.party.champions[0], 0x42u);
    world.creatureAICount = 1;
    world.creatureAI[0].stateKind = AI_STATE_WANDER;
    world.creatureAI[0].creatureType = CREATURE_TYPE_SKELETON;
    world.creatureAI[0].groupMapIndex = 2;
    world.creatureAI[0].groupMapX = 6;
    world.creatureAI[0].groupMapY = 7;
    world.creatureAI[0].groupCells = 0x33;
    world.creatureAI[0].groupDirection = DIR_WEST;

    rc = DM1_SaveGameWithProfile(&world, path, 0x42525331u, 1, 1,
                                 DM1_DefaultSaveProfileHash());
    F0883_WORLD_Free_Compat(&world);
    return rc == DM1_SAVE_OK;
}

static int select_save_entry(M12_StartupMenuState* state,
                             const char* filename) {
    int i;
    if (!state || !filename) {
        return 0;
    }
    for (i = 0; i < state->saveBrowser.entryCount; ++i) {
        if (strcmp(state->saveBrowser.entries[i].filename, filename) == 0) {
            state->saveBrowser.selectedIndex = i;
            return 1;
        }
    }
    return 0;
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-m12-qr-XXXXXX";
    char savePath[512];
    char csbSavePath[512];
    char nativeSavePath[512];
    M12_StartupMenuState state;
    M12_LaunchIntent intent;
    char pc34Path[512];

    if (!mkdtemp(tmpTemplate)) {
        perror("mkdtemp");
        return 1;
    }
    m12_test_setenv("HOME", tmpTemplate);

    M12_Config_SetLastSavePath("");
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&state);
    if (!expect(state.quickResumeAvailable == 0, "no-save must disable quick Resume")) return 1;

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav", tmpTemplate);
    if (!expect(write_fake_quicksave(savePath), "should write fake quicksave")) return 1;
    M12_Config_SetLastSavePath(savePath);

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&state);
    if (!expect(state.quickResumeAvailable == 1, "valid DM1 quicksave must enable quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "dm1") == 0, "quick Resume should identify dm1")) return 1;
    if (!expect(strcmp(state.quickResumeSavePath, savePath) == 0, "quick Resume should retain save path")) return 1;
    if (!expect(state.selectedIndex == 0,
                "valid quick Resume must not steal default Enter from DM1 new-game launch")) return 1;
    if (!expect(M12_StartupMenu_ExportQuickResumeDM1PC34(&state, pc34Path,
                                                         (int)sizeof(pc34Path)) == -1,
                "fake quick Resume save should not export as PC34")) return 1;

    if (!expect(write_serialized_dm1_quicksave(savePath),
                "should replace fake quicksave with serialized DM1 quicksave")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&state);
    if (!expect(M12_StartupMenu_ExportQuickResumeDM1PC34(&state, pc34Path,
                                                         (int)sizeof(pc34Path)) == 0,
                "serialized quick Resume should export as DM1 PC34")) return 1;
    if (!expect(strstr(pc34Path, "firestaff-dm1-quicksave-pc34.sav") != NULL,
                "PC34 export path should use quicksave suffix")) return 1;
    {
        struct SaveGame_Compat imported;
        struct PartyState_Compat importedParty;
        struct TimelineQueue_Compat importedTimeline;
        DM1OriginalSavePC34HandoffReport report;
        memset(&imported, 0, sizeof(imported));
        memset(&importedParty, 0, sizeof(importedParty));
        memset(&importedTimeline, 0, sizeof(importedTimeline));
        memset(&report, 0, sizeof(report));
        imported.party = &importedParty;
        imported.timeline = &importedTimeline;
        if (!expect(dm1_v1_original_save_pc34_handoff_file(pc34Path,
                                                           &imported,
                                                           &report) ==
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
                    "M12 quick Resume PC34 export should pass original handoff")) return 1;
        if (!expect(importedParty.mapIndex == 5 &&
                    importedParty.mapX == 14 &&
                    importedParty.mapY == 16,
                    "M12 quick Resume PC34 export should preserve party pose")) return 1;
        if (!expect(memcmp(importedParty.champions[0].name, "HALK    ",
                           CHAMPION_NAME_LENGTH) == 0,
                    "M12 quick Resume PC34 export should preserve champion name")) return 1;
        {
            struct ChampionState_Compat expectedChampion;
            F0600_CHAMPION_InitEmpty_Compat(&expectedChampion);
            fill_test_portrait(&expectedChampion, 0x21u);
            if (!expect(importedParty.champions[0].portraitBitmapValid == 1 &&
                        memcmp(importedParty.champions[0].portraitBitmap,
                               expectedChampion.portraitBitmap,
                               CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT) == 0,
                        "M12 quick Resume PC34 export should preserve external portrait bytes")) {
                return 1;
            }
        }
        if (!expect(report.original_game_time == 6789u,
                    "M12 quick Resume PC34 export should preserve game tick")) return 1;
        if (!expect(report.original_current_active_group_count == 1 &&
                    report.active_groups[0].cells == 0x55,
                    "M12 quick Resume PC34 export should preserve active group")) return 1;
    }
    if (!expect(M12_StartupMenu_ExportQuickResumeDM1PC34(&state, pc34Path,
                                                         (int)sizeof(pc34Path)) == -1,
                "duplicate quick Resume PC34 export should preserve existing file")) return 1;

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 0, "default Enter on DM1 opens game options, not quick Resume")) return 1;
    if (!expect(state.quickResumeLaunchRequested == 0,
                "default Enter on DM1 must not arm quick Resume")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS,
                "default Enter on DM1 should enter the normal launch path")) return 1;

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&state);
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1, "Resume accept should request launch")) return 1;
    if (!expect(state.activatedIndex == 0, "Resume accept should activate DM1 slot")) return 1;

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1, "Resume launch intent should be valid for matched DM1")) return 1;
    if (!expect(intent.savePath && strcmp(intent.savePath, savePath) == 0,
                "Resume launch intent must carry exact save path")) return 1;

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&state);
    state.selectedIndex = 0;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 0, "normal DM1 accept opens game options first")) return 1;
    if (!expect(state.quickResumeLaunchRequested == 0,
                "normal DM1 accept must not arm quick Resume")) return 1;
    state.launchRequested = 1;
    state.activatedIndex = 0;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1, "normal DM1 launch intent should still be valid")) return 1;
    if (!expect(intent.savePath == NULL,
                "normal DM1 launch must not inherit quick Resume save path")) return 1;

    snprintf(csbSavePath, sizeof(csbSavePath),
             "%s/firestaff-csb-quicksave.sav", tmpTemplate);
    if (!expect(write_fake_quicksave(csbSavePath),
                "should write fake CSB quicksave envelope")) return 1;
    M12_Config_SetLastSavePath(csbSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);
    if (!expect(state.quickResumeAvailable == 0,
                "CSB quick Resume must reject DM1-shaped quicksave bytes")) return 1;

    if (!expect(write_raw_csbgame_roster_quicksave(csbSavePath),
                "should write raw CSBGAME roster quicksave")) return 1;
    M12_Config_SetLastSavePath(csbSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "raw CSBGAME roster save must enable CSB quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "csb") == 0,
                "raw CSBGAME quick Resume should identify csb")) return 1;
    if (!expect(strcmp(state.quickResumeSavePath, csbSavePath) == 0,
                "raw CSBGAME quick Resume should retain save path")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1,
                "raw CSBGAME quick Resume accept should request launch")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0,
                "raw CSBGAME quick Resume launch intent should identify CSB")) return 1;
    if (!expect(intent.savePath && strcmp(intent.savePath, csbSavePath) == 0,
                "raw CSBGAME quick Resume launch intent must carry exact save path")) return 1;

    if (!expect(firestaff_test_write_csbwin_resume_fixture(csbSavePath, 0),
                "should write CSBWin verified-body quicksave")) return 1;
    M12_Config_SetLastSavePath(csbSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "CSBWin verified-body save must enable CSB quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "csb") == 0,
                "CSBWin quick Resume should identify csb")) return 1;
    if (!expect(strcmp(state.quickResumeSavePath, csbSavePath) == 0,
                "CSBWin quick Resume should retain save path")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1,
                "CSBWin quick Resume accept should request launch")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0,
                "CSBWin quick Resume launch intent should identify CSB")) return 1;
    if (!expect(intent.savePath && strcmp(intent.savePath, csbSavePath) == 0,
                "CSBWin quick Resume launch intent must carry exact save path")) return 1;

    if (!expect(firestaff_test_write_csbwin_resume_fixture(csbSavePath, 1),
                "should write corrupt CSBWin quicksave")) return 1;
    M12_Config_SetLastSavePath(csbSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);
    if (!expect(state.quickResumeAvailable == 0,
                "corrupt CSBWin save must not enable CSB quick Resume")) return 1;

    if (!expect(write_serialized_csb_quicksave(csbSavePath),
                "should write serialized CSB runtime quicksave")) return 1;
    M12_Config_SetLastSavePath(csbSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "valid CSB runtime save must enable quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "csb") == 0,
                "quick Resume should identify csb")) return 1;
    if (!expect(strcmp(state.quickResumeSavePath, csbSavePath) == 0,
                "CSB quick Resume should retain save path")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1,
                "CSB quick Resume accept should request launch")) return 1;
    if (!expect(state.activatedIndex == 1,
                "CSB quick Resume accept should activate CSB slot")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1,
                "CSB quick Resume launch intent should be valid")) return 1;
    if (!expect(intent.gameId && strcmp(intent.gameId, "csb") == 0,
                "CSB quick Resume launch intent should identify CSB")) return 1;
    if (!expect(intent.savePath && strcmp(intent.savePath, csbSavePath) == 0,
                "CSB quick Resume launch intent must carry exact save path")) return 1;

    snprintf(nativeSavePath, sizeof(nativeSavePath),
             "%s/firestaff-dm1-browser.sav", tmpTemplate);
    if (!expect(write_native_dm1_save(nativeSavePath),
                "should write native DM1 browser save")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_dm1_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser when saves exist")) return 1;
    if (!expect(state.view == M12_MENU_VIEW_SAVE_BROWSER,
                "open save browser should enter save browser view")) return 1;
    if (!expect(select_save_entry(&state, "firestaff-dm1-browser.sav"),
                "save browser should list native DM1 save")) return 1;
    if (!expect(M12_StartupMenu_ExportSelectedSaveBrowserDM1PC34(
                    &state, pc34Path, (int)sizeof(pc34Path)) == 0,
                "save browser should export selected native DM1 save as PC34")) return 1;
    if (!expect(strstr(pc34Path, "firestaff-dm1-browser-pc34.sav") != NULL,
                "save browser PC34 export should use selected save suffix")) return 1;
    {
        struct SaveGame_Compat imported;
        struct PartyState_Compat importedParty;
        DM1OriginalSavePC34HandoffReport report;
        memset(&imported, 0, sizeof(imported));
        memset(&importedParty, 0, sizeof(importedParty));
        memset(&report, 0, sizeof(report));
        imported.party = &importedParty;
        if (!expect(dm1_v1_original_save_pc34_handoff_file(pc34Path,
                                                           &imported,
                                                           &report) ==
                    DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
                    "save browser PC34 export should pass original handoff")) return 1;
        if (!expect(importedParty.mapIndex == 2 &&
                    importedParty.mapX == 9 &&
                    importedParty.mapY == 11,
                    "save browser PC34 export should preserve party pose")) return 1;
        if (!expect(memcmp(importedParty.champions[0].name, "TIGGY   ",
                           CHAMPION_NAME_LENGTH) == 0,
                    "save browser PC34 export should preserve selected champion")) return 1;
        {
            struct ChampionState_Compat expectedChampion;
            F0600_CHAMPION_InitEmpty_Compat(&expectedChampion);
            fill_test_portrait(&expectedChampion, 0x42u);
            if (!expect(importedParty.champions[0].portraitBitmapValid == 1 &&
                        memcmp(importedParty.champions[0].portraitBitmap,
                               expectedChampion.portraitBitmap,
                               CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT) == 0,
                        "save browser PC34 export should preserve external portrait bytes")) {
                return 1;
            }
        }
        if (!expect(report.original_current_active_group_count == 1 &&
                    report.active_groups[0].cells == 0x33,
                    "save browser PC34 export should preserve active group cells")) return 1;
    }
    if (!expect(M12_StartupMenu_ExportSelectedSaveBrowserDM1PC34(
                    &state, pc34Path, (int)sizeof(pc34Path)) == -1,
                "save browser PC34 export should refuse overwrite")) return 1;

    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should reopen save browser after export")) return 1;
    if (!expect(select_save_entry(&state, "firestaff-dm1-browser.sav"),
                "save browser should keep original save selectable")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1,
                "save browser accept should request launch")) return 1;
    if (!expect(state.quickResumeLaunchRequested == 1,
                "save browser accept should route selected save as savePath")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1,
                "save browser launch intent should be valid")) return 1;
    if (!expect(intent.savePath && strcmp(intent.savePath, nativeSavePath) == 0,
                "save browser launch intent should carry selected save path")) return 1;

    puts("ok: quick Resume only carries save path for explicit Continue, never normal DM1 launch");
    return 0;
}
