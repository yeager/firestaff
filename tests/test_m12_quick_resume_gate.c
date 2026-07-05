#include "menu_startup_m12.h"
#include "csbwin_resume_fixture.h"
#include "config_m12.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "dm1_v1_save_load.h"
#include "dm1_v1_original_save_pc34_handoff.h"
#include "dm2_v1_new_game.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_save.h"
#include "nexus_v1_world.h"
#include "theron_v1_dungeon_progression.h"
#include "theron_v1_save_load.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef FIRESTAFF_HAS_ZLIB
#define FIRESTAFF_HAS_ZLIB 0
#endif

static const unsigned char g_valid_gzip_srm[] = {
    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff,
    0x73, 0x0b, 0x0e, 0x09, 0x0c, 0x08, 0x72, 0x37, 0x64, 0x64,
    0x66, 0x66, 0xd4, 0x61, 0x64, 0x60, 0x60, 0x14, 0x60, 0x60,
    0x60, 0x02, 0x62, 0x66, 0x20, 0x66, 0x01, 0x62, 0x56, 0x20,
    0x66, 0x03, 0x62, 0x76, 0x20, 0x06, 0x00, 0x50, 0x8a, 0x0c,
    0xc3, 0x2c, 0x00, 0x00, 0x00
};

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

static void force_dm2_available(M12_StartupMenuState* state) {
    state->entries[2].title = "DUNGEON MASTER II";
    state->entries[2].gameId = "dm2";
    state->entries[2].kind = M12_MENU_ENTRY_GAME;
    state->entries[2].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[2].available = 1;
    state->assetStatus.dm2Available = 1;
    state->assetStatus.versions[2][0].gameId = "dm2";
    state->assetStatus.versions[2][0].versionId = "pc-en";
    state->assetStatus.versions[2][0].label = "PC English";
    state->assetStatus.versions[2][0].shortLabel = "PC EN";
    state->assetStatus.versions[2][0].matched = 1;
    state->assetStatus.requiredFileCounts[2] = 2;
    state->assetStatus.requiredFiles[2][0].gameId = "dm2";
    state->assetStatus.requiredFiles[2][0].roleId = "graphics";
    state->assetStatus.requiredFiles[2][0].label = "GRAPHICS.DAT";
    state->assetStatus.requiredFiles[2][0].required = 1;
    state->assetStatus.requiredFiles[2][0].matched = 1;
    state->assetStatus.requiredFiles[2][1].gameId = "dm2";
    state->assetStatus.requiredFiles[2][1].roleId = "dungeon";
    state->assetStatus.requiredFiles[2][1].label = "DUNGEON.DAT";
    state->assetStatus.requiredFiles[2][1].required = 1;
    state->assetStatus.requiredFiles[2][1].matched = 1;
    state->gameOptions[2].versionIndex = 0;
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

static void force_nexus_available(M12_StartupMenuState* state) {
    state->entries[3].title = "DUNGEON MASTER NEXUS";
    state->entries[3].gameId = "nexus";
    state->entries[3].kind = M12_MENU_ENTRY_GAME;
    state->entries[3].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[3].available = 1;
    state->assetStatus.nexusAvailable = 1;
    state->assetStatus.versions[3][0].gameId = "nexus";
    state->assetStatus.versions[3][0].versionId = "saturn";
    state->assetStatus.versions[3][0].label = "Saturn";
    state->assetStatus.versions[3][0].shortLabel = "SAT";
    state->assetStatus.versions[3][0].matched = 1;
    state->gameOptions[3].versionIndex = 0;
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

static void force_theron_available(M12_StartupMenuState* state) {
    state->entries[4].title = "THERON'S QUEST";
    state->entries[4].gameId = "theron";
    state->entries[4].kind = M12_MENU_ENTRY_GAME;
    state->entries[4].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[4].available = 1;
    state->assetStatus.theronAvailable = 1;
    state->assetStatus.versions[4][0].gameId = "theron";
    state->assetStatus.versions[4][0].versionId = "pce-us";
    state->assetStatus.versions[4][0].label = "PC Engine US";
    state->assetStatus.versions[4][0].shortLabel = "PCE US";
    state->assetStatus.versions[4][0].matched = 1;
    state->gameOptions[4].versionIndex = 0;
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

static int write_dm2_slot_save(const char* root,
                               unsigned char slot,
                               char* outPath,
                               size_t outPathSize) {
    DM2_V1_SessionState session;

    if (!root || !outPath || outPathSize == 0u) {
        return 0;
    }
    dm2_v1_session_new(&session);
    session.game_tick = 77u;
    session.party_x = 22u;
    session.party_y = 13u;
    session.party_dir = 1u;
    session.party_level = 3u;
    session.outdoor_mode = 1u;
    session.time_of_day_minutes = 615u;
    session.rain_intensity = 40u;
    if (dm2_v1_session_save_slot(root, slot, "M12 DM2 Slot",
                                 &session) != 0) {
        return 0;
    }
    snprintf(outPath, outPathSize, "%s/SKSave%02u.dat",
             root, (unsigned)slot);
    return 1;
}

static int write_dm2_last_session_save(const char* root,
                                       char* outPath,
                                       size_t outPathSize) {
    DM2_V1_SessionState session;

    if (!root || !outPath || outPathSize == 0u) {
        return 0;
    }
    dm2_v1_session_new(&session);
    session.game_tick = 91u;
    session.party_x = 17u;
    session.party_y = 8u;
    session.party_dir = 2u;
    session.party_level = 6u;
    session.outdoor_mode = 0u;
    if (dm2_v1_session_save_last_session(root, "M12 DM2 Last",
                                         &session) != 0) {
        return 0;
    }
    snprintf(outPath, outPathSize, "%s/SKSave.dat", root);
    return 1;
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

#define ORIGINAL_PC34_CHAMPION_BYTES 319
#define ORIGINAL_PC34_PARTY_INFO_BYTES 128
#define ORIGINAL_PC34_PARTY_BYTES \
    ((ORIGINAL_PC34_CHAMPION_BYTES * CHAMPION_MAX_PARTY) + \
     ORIGINAL_PC34_PARTY_INFO_BYTES)
#define ORIGINAL_PC34_EVENT_BYTES 10

static void wr16le(unsigned char* p, uint16_t v) {
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
}

static void wr32le(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
    p[2] = (unsigned char)((v >> 16) & 0xffu);
    p[3] = (unsigned char)((v >> 24) & 0xffu);
}

static uint16_t rd16le(const unsigned char* p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint16_t original_first_half_checksum(const unsigned char* header) {
    uint16_t acc = 0;
    size_t i;
    for (i = 0; i < 32u; ++i) {
        acc = (uint16_t)(acc + rd16le(header + (i * 8u) + 0u));
        acc = (uint16_t)(acc ^ rd16le(header + (i * 8u) + 2u));
        acc = (uint16_t)(acc - rd16le(header + (i * 8u) + 4u));
        acc = (uint16_t)(acc ^ rd16le(header + (i * 8u) + 6u));
    }
    return acc;
}

static uint16_t original_second_half_plain_checksum(const unsigned char* header) {
    uint16_t sum = 0;
    size_t i;
    for (i = 128u; i < 256u; ++i) {
        sum = (uint16_t)(sum + rd16le(header + (i * 2u)));
    }
    return sum;
}

static void xor_original_second_half(unsigned char* header, uint16_t key) {
    uint16_t rollingKey = key;
    size_t i;
    for (i = 128u; i < 256u; ++i) {
        unsigned char* word = header + (i * 2u);
        wr16le(word, (uint16_t)(rd16le(word) ^ rollingKey));
        rollingKey = (uint16_t)(rollingKey + 128u);
    }
}

static uint16_t checksum_and_xor_original_words(unsigned char* bytes,
                                                size_t wordCount,
                                                uint16_t key) {
    uint16_t rollingKey = key;
    uint16_t checksum = key;
    size_t i;
    for (i = 0u; i < wordCount; ++i) {
        unsigned char* word = bytes + i * 2u;
        uint16_t v = rd16le(word);
        checksum = (uint16_t)(checksum + v);
        v = (uint16_t)(v ^ rollingKey);
        wr16le(word, v);
        checksum = (uint16_t)(checksum + v);
        rollingKey = (uint16_t)(rollingKey + (uint16_t)wordCount);
    }
    return checksum;
}

static int write_original_part(unsigned char* dst,
                               int dstCap,
                               const unsigned char* plain,
                               int byteCount,
                               uint16_t key,
                               uint16_t* outChecksum) {
    if (dstCap < 2 + byteCount || (byteCount & 1) != 0) {
        return -1;
    }
    wr16le(dst, (uint16_t)byteCount);
    if (byteCount > 0 && plain) {
        memcpy(dst + 2, plain, (size_t)byteCount);
    }
    *outChecksum = checksum_and_xor_original_words(
        dst + 2, (size_t)byteCount / 2u, key);
    return 2 + byteCount;
}

static void write_original_champion(unsigned char* dst) {
    memset(dst, 0, ORIGINAL_PC34_CHAMPION_BYTES);
    memset(dst + 0, ' ', 8u);
    memset(dst + 8, ' ', 20u);
    memcpy(dst + 0, "TIGGY", 5u);
    memcpy(dst + 8, "APPRENTICE", 10u);
    dst[28] = DIR_EAST;
    wr16le(dst + 52, 44u);
    wr16le(dst + 54, 55u);
    wr16le(dst + 56, 66u);
    wr16le(dst + 58, 77u);
    wr16le(dst + 60, 8u);
    wr16le(dst + 62, 9u);
    wr16le(dst + 66, 1500u);
    wr16le(dst + 68, 1200u);
    wr16le(dst + 70 + 3u, 33u);
    wr32le(dst + 91 + 2u, 1000u);
    wr16le(dst + 211 + (size_t)CHAMPION_SLOT_HAND_RIGHT * 2u, 0x1555u);
    wr16le(dst + 271, 345u);
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

static int write_original_pc34_dm1_save_file(const char* path) {
    unsigned char buf[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    unsigned char global[SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT];
    unsigned char party[ORIGINAL_PC34_PARTY_BYTES];
    unsigned char event[ORIGINAL_PC34_EVENT_BYTES];
    unsigned char timeline[2];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    int cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int n;
    int i;
    FILE* fp;

    memset(buf, 0, sizeof(buf));
    memset(header, 0, sizeof(header));
    memset(global, 0, sizeof(global));
    memset(party, 0, sizeof(party));
    memset(event, 0, sizeof(event));
    memset(timeline, 0, sizeof(timeline));
    memset(checksums, 0, sizeof(checksums));

    for (i = 0; i < 127; ++i) {
        wr16le(header + (size_t)i * 2u,
               (uint16_t)(0x5151u + (uint16_t)(i * 11u)));
    }
    wr16le(header + 10u * 2u, 0x2468u);
    header[298] = 1u;
    header[299] = SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC;
    wr32le(header + 306u, 0x50433334u);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = (uint16_t)(0x3000u + (uint16_t)(i * 0x77u));
    }

    wr32le(global + 0u, 7777u);
    wr16le(global + 10u, 1u);
    wr16le(global + 12u, 9u);
    wr16le(global + 14u, 10u);
    wr16le(global + 16u, DIR_EAST);
    wr16le(global + 18u, 4u);
    wr16le(global + 20u, 0u);
    wr16le(global + 24u, 0u);
    wr16le(global + 26u, 0u);
    wr16le(global + 28u, 1u);
    wr16le(global + 30u, 0u);
    wr16le(global + 46u, 0u);
    write_original_champion(party);

    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            global, (int)sizeof(global),
                            keys[SAVEGAME_PC34_PART_GLOBAL_DATA],
                            &checksums[SAVEGAME_PC34_PART_GLOBAL_DATA]);
    if (n < 0) return 0;
    cursor += n;
    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            NULL, 0,
                            keys[SAVEGAME_PC34_PART_ACTIVE_GROUP],
                            &checksums[SAVEGAME_PC34_PART_ACTIVE_GROUP]);
    if (n < 0) return 0;
    cursor += n;
    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            party, (int)sizeof(party),
                            keys[SAVEGAME_PC34_PART_PARTY],
                            &checksums[SAVEGAME_PC34_PART_PARTY]);
    if (n < 0) return 0;
    cursor += n;
    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            event, (int)sizeof(event),
                            keys[SAVEGAME_PC34_PART_EVENTS],
                            &checksums[SAVEGAME_PC34_PART_EVENTS]);
    if (n < 0) return 0;
    cursor += n;
    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            timeline, (int)sizeof(timeline),
                            keys[SAVEGAME_PC34_PART_TIMELINE],
                            &checksums[SAVEGAME_PC34_PART_TIMELINE]);
    if (n < 0) return 0;
    cursor += n;

    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        wr16le(header + 310u + (size_t)i * 2u, keys[i]);
        wr16le(header + 342u + (size_t)i * 2u, checksums[i]);
    }
    wr16le(header + 374u, SAVEGAME_PC34_PLATFORM_PC);
    wr16le(header + 376u, SAVEGAME_PC34_DUNGEON_ID_DM);
    {
        uint16_t secondSum = original_second_half_plain_checksum(header);
        uint16_t firstBeforeLast = original_first_half_checksum(header);
        uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                   firstBeforeLast ^
                                   secondSum);
        wr16le(header + 254u, last);
    }
    xor_original_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(buf, header, sizeof(header));

    fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    if (fwrite(buf, 1u, (size_t)cursor, fp) != (size_t)cursor) {
        fclose(fp);
        return 0;
    }
    return fclose(fp) == 0;
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

static int write_nexus_fnxs_save(const char* path) {
    Nexus_V1_ChampionPool pool;
    Nexus_V1_World world;
    Nexus_SaveResult result;

    if (!path) {
        return 0;
    }
    nexus_v1_champions_init(&pool);
    pool.party_count = 0;
    pool.leader_index = -1;
    nexus_v1_world_init(&world);
    nexus_v1_party_place(&world, 2, 18, 12, 3);
    nexus_v1_world_tick(&world);

    result = nexus_v1_save_full_to_path(path,
                                        world.party_level,
                                        world.party_x,
                                        world.party_y,
                                        world.party_dir,
                                        (uint32_t)world.world_tick,
                                        world.state_hash,
                                        &pool,
                                        &world);
    return result == NEXUS_SAVE_OK;
}

static int write_theron_tqsv_save(const char* root,
                                  int slot,
                                  char* outPath,
                                  size_t outPathSize) {
    unsigned char championData[
        THERON_SAVE_CHAMPION_COUNT * THERON_SAVE_CHAMPION_BLOCK_SIZE];
    Theron_DungeonProgression progression;
    if (!root || !outPath || outPathSize == 0u) {
        return 0;
    }
    memset(championData, 0x31, sizeof(championData));
    theron_v1_dungeon_progression_init(&progression);
    progression.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
    progression.current_level = 1;
    progression.dungeon_playtime_seconds = 4321u;
    progression.quest_items_collected = 0x03u;
    if (theron_v1_save_to_slot(root,
                               slot,
                               championData,
                               sizeof(championData),
                               &progression,
                               "M12 Theron") != 0) {
        return 0;
    }
    theron_v1_save_slot_path(root, slot, outPath, outPathSize);
    return 1;
}

static int write_theron_srm_save(const char* root,
                                 int slot,
                                 char* outPath,
                                 size_t outPathSize) {
    int rc;
    FILE* fp;
    if (!root || !outPath || outPathSize == 0u || slot < 0 || slot > 9) {
        return 0;
    }
    rc = snprintf(outPath, outPathSize, "%s/slot%d.srm", root, slot);
    if (rc <= 0 || (size_t)rc >= outPathSize) {
        return 0;
    }
    fp = fopen(outPath, "wb");
    if (!fp) {
        return 0;
    }
    rc = fwrite(g_valid_gzip_srm, 1u, sizeof(g_valid_gzip_srm), fp) ==
         sizeof(g_valid_gzip_srm);
    fclose(fp);
    return rc;
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
    char csbBrowserSavePath[512];
    char csbWinBrowserSavePath[512];
    char importedCsbWinBrowserSavePath[512];
    char importedCsbWinQuickResumePath[512];
    char wrongKnownGameQuickResumePath[512];
    char originalCsbGameBrowserSavePath[512];
    char originalDmSaveBrowserSavePath[512];
    char originalDm1SavePath[512];
    char dm2SlotSavePath[512];
    char dm2LastSessionSavePath[512];
    char nexusSavePath[512];
    char nexusBrowserSavePath[512];
    char nexusSlotSavePath[512];
    char theronSaveRoot[512];
    char theronSlotSavePath[512];
    char theronSrmSavePath[512];
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

    snprintf(originalDm1SavePath, sizeof(originalDm1SavePath),
             "%s/firestaff-dm1-original-pc34.sav", tmpTemplate);
    if (!expect(write_original_pc34_dm1_save_file(originalDm1SavePath),
                "should write original PC34 DM1 save fixture")) return 1;
    M12_Config_SetLastSavePath(originalDm1SavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "original PC34 DM1 save must enable quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "dm1") == 0,
                "original PC34 DM1 quick Resume should identify dm1")) return 1;
    if (!expect(strcmp(state.quickResumeSavePath, originalDm1SavePath) == 0,
                "original PC34 DM1 quick Resume should retain save path")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1,
                "original PC34 DM1 Resume accept should request launch")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1,
                "original PC34 DM1 Resume launch intent should be valid")) return 1;
    if (!expect(intent.savePath &&
                strcmp(intent.savePath, originalDm1SavePath) == 0,
                "original PC34 DM1 Resume launch intent must carry exact path")) return 1;

    snprintf(originalDm1SavePath, sizeof(originalDm1SavePath),
             "%s/DMSAVE.DAT", tmpTemplate);
    if (!expect(write_original_pc34_dm1_save_file(originalDm1SavePath),
                "should write original PC34 DM1 DMSAVE.DAT fixture")) return 1;
    M12_Config_SetLastSavePath(originalDm1SavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_dm1_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "original DM1 DMSAVE.DAT must enable quick Resume by content")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "dm1") == 0,
                "original DM1 DMSAVE.DAT quick Resume should identify dm1")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.savePath &&
                strcmp(intent.savePath, originalDm1SavePath) == 0,
                "original DM1 DMSAVE.DAT Resume must carry exact path")) return 1;

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

    snprintf(originalCsbGameBrowserSavePath, sizeof(originalCsbGameBrowserSavePath),
             "%s/CSBGAME.DAT", tmpTemplate);
    if (!expect(write_raw_csbgame_roster_quicksave(originalCsbGameBrowserSavePath),
                "should write original-name CSBGAME.DAT quick Resume fixture")) return 1;
    M12_Config_SetLastSavePath(originalCsbGameBrowserSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "CSBGAME.DAT must enable CSB quick Resume by content")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "csb") == 0,
                "CSBGAME.DAT quick Resume should identify csb")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0 &&
                intent.savePath &&
                strcmp(intent.savePath, originalCsbGameBrowserSavePath) == 0,
                "CSBGAME.DAT quick Resume launch intent should carry exact CSB path")) return 1;

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

    snprintf(importedCsbWinQuickResumePath,
             sizeof(importedCsbWinQuickResumePath),
             "%s/firestaff-imported-csbwin.sav", tmpTemplate);
    if (!expect(firestaff_test_write_csbwin_resume_fixture(
                    importedCsbWinQuickResumePath, 0),
                "should write imported-name CSBWin quicksave")) return 1;
    M12_Config_SetLastSavePath(importedCsbWinQuickResumePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "unknown firestaff save name should enable CSB quick Resume by content")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "csb") == 0,
                "unknown firestaff save name should classify CSB by content")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0 &&
                intent.savePath &&
                strcmp(intent.savePath, importedCsbWinQuickResumePath) == 0,
                "unknown firestaff CSB quick Resume should carry exact path")) return 1;

    snprintf(wrongKnownGameQuickResumePath,
             sizeof(wrongKnownGameQuickResumePath),
             "%s/firestaff-dm2-imported-csbwin.sav", tmpTemplate);
    if (!expect(firestaff_test_write_csbwin_resume_fixture(
                    wrongKnownGameQuickResumePath, 0),
                "should write known-other-game CSBWin quicksave")) return 1;
    M12_Config_SetLastSavePath(wrongKnownGameQuickResumePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_csb_available(&state);
    if (!expect(state.quickResumeAvailable == 0,
                "known DM2 firestaff name must not be reclassified as CSB")) return 1;

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

    snprintf(csbBrowserSavePath, sizeof(csbBrowserSavePath),
             "%s/firestaff-csb-browser.sav", tmpTemplate);
    if (!expect(write_raw_csbgame_roster_quicksave(csbBrowserSavePath),
                "should write raw CSBGAME browser save")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_csb_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for CSB saves")) return 1;
    if (!expect(select_save_entry(&state, "firestaff-csb-browser.sav"),
                "save browser should list raw CSBGAME CSB save")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark CSB save loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "csb") == 0,
                "save browser should classify CSB save as csb")) return 1;
    if (!expect(strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "ROSTERA") != NULL &&
                strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "ROSTERB") != NULL,
                "save browser should expose CSB champion names")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1,
                "save browser accept should request CSB launch")) return 1;
    if (!expect(state.quickResumeLaunchRequested == 1,
                "save browser accept should route selected CSB save as savePath")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0,
                "save browser CSB launch intent should identify CSB")) return 1;
    if (!expect(intent.savePath && strcmp(intent.savePath, csbBrowserSavePath) == 0,
                "save browser CSB launch intent should carry selected save path")) return 1;

    snprintf(csbWinBrowserSavePath, sizeof(csbWinBrowserSavePath),
             "%s/firestaff-csb-csbwin-browser.sav", tmpTemplate);
    if (!expect(firestaff_test_write_csbwin_resume_fixture(csbWinBrowserSavePath, 0),
                "should write CSBWin verified-body browser save")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_csb_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for CSBWin saves")) return 1;
    if (!expect(select_save_entry(&state, "firestaff-csb-csbwin-browser.sav"),
                "save browser should list CSBWin verified-body CSB save")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark CSBWin save loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "csb") == 0,
                "save browser should classify CSBWin save as csb")) return 1;
    if (!expect(strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "TIGGY") != NULL &&
                strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "BORIS") != NULL,
                "save browser should expose CSBWin champion names")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1,
                "save browser accept should request CSBWin CSB launch")) return 1;
    if (!expect(state.quickResumeLaunchRequested == 1,
                "save browser accept should route selected CSBWin save as savePath")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0,
                "save browser CSBWin launch intent should identify CSB")) return 1;
    if (!expect(intent.savePath && strcmp(intent.savePath, csbWinBrowserSavePath) == 0,
                "save browser CSBWin launch intent should carry selected save path")) return 1;

    snprintf(importedCsbWinBrowserSavePath, sizeof(importedCsbWinBrowserSavePath),
             "%s/firestaff-imported-csbwin-browser.sav", tmpTemplate);
    if (!expect(firestaff_test_write_csbwin_resume_fixture(
                    importedCsbWinBrowserSavePath, 0),
                "should write imported CSBWin browser save")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_csb_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for imported CSBWin saves")) return 1;
    if (!expect(select_save_entry(&state, "firestaff-imported-csbwin-browser.sav"),
                "save browser should list imported CSBWin save")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark imported CSBWin save loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "csb") == 0,
                "save browser should classify imported CSBWin save as csb by content")) return 1;
    if (!expect(strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "TIGGY") != NULL &&
                strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "BORIS") != NULL,
                "save browser should expose imported CSBWin champion names")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(state.launchRequested == 1 &&
                state.quickResumeLaunchRequested == 1,
                "save browser imported CSBWin accept should request save-path launch")) return 1;
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0,
                "save browser imported CSBWin launch intent should identify CSB")) return 1;
    if (!expect(intent.savePath &&
                strcmp(intent.savePath, importedCsbWinBrowserSavePath) == 0,
                "save browser imported CSBWin launch intent should carry exact path")) return 1;

    snprintf(originalCsbGameBrowserSavePath,
             sizeof(originalCsbGameBrowserSavePath),
             "%s/CSBGAME.DAT", tmpTemplate);
    if (!expect(write_raw_csbgame_roster_quicksave(
                    originalCsbGameBrowserSavePath),
                "should write original-name CSBGAME browser save")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_csb_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for original CSBGAME name")) return 1;
    if (!expect(select_save_entry(&state, "CSBGAME.DAT"),
                "save browser should list CSBGAME.DAT")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark CSBGAME.DAT loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "csb") == 0,
                "save browser should classify CSBGAME.DAT as csb")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0,
                "save browser CSBGAME.DAT launch intent should identify CSB")) return 1;
    if (!expect(intent.savePath &&
                strcmp(intent.savePath, originalCsbGameBrowserSavePath) == 0,
                "save browser CSBGAME.DAT launch intent should carry exact path")) return 1;

    snprintf(originalDmSaveBrowserSavePath,
             sizeof(originalDmSaveBrowserSavePath),
             "%s/DMSAVE.DAT", tmpTemplate);
    if (!expect(firestaff_test_write_csbwin_resume_fixture(
                    originalDmSaveBrowserSavePath, 0),
                "should write original-name CSBWin DMSAVE browser save")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_csb_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for original DMSAVE name")) return 1;
    if (!expect(select_save_entry(&state, "DMSAVE.DAT"),
                "save browser should list DMSAVE.DAT")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark DMSAVE.DAT loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "csb") == 0,
                "save browser should classify DMSAVE.DAT as csb")) return 1;
    if (!expect(strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "TIGGY") != NULL &&
                strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "BORIS") != NULL,
                "save browser should expose DMSAVE.DAT champion names")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0,
                "save browser DMSAVE.DAT launch intent should identify CSB")) return 1;
    if (!expect(intent.savePath &&
                strcmp(intent.savePath, originalDmSaveBrowserSavePath) == 0,
                "save browser DMSAVE.DAT launch intent should carry exact path")) return 1;

    if (!expect(write_dm2_slot_save(tmpTemplate, 4u,
                                    dm2SlotSavePath,
                                    sizeof(dm2SlotSavePath)),
                "should write DM2 SKSave browser slot")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_dm2_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for DM2 SKSave slots")) return 1;
    if (!expect(select_save_entry(&state, "SKSave04.dat"),
                "save browser should list DM2 SKSave04.dat")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark DM2 SKSave slot loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "dm2") == 0,
                "save browser should classify SKSave slot as dm2")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].mapLevel == 3,
                "save browser should expose DM2 saved level")) return 1;
    if (!expect(strstr(state.saveBrowser.entries[state.saveBrowser.selectedIndex].champions,
                       "Theron") != NULL,
                "save browser should expose DM2 champion names")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(state.launchRequested == 1 &&
                state.quickResumeLaunchRequested == 1,
                "save browser DM2 accept should request save-path launch")) return 1;
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "dm2") == 0,
                "save browser DM2 launch intent should identify DM2")) return 1;
    if (!expect(intent.savePath &&
                strcmp(intent.savePath, dm2SlotSavePath) == 0,
                "save browser DM2 launch intent should carry exact SKSave path")) return 1;

    if (!expect(write_dm2_last_session_save(tmpTemplate,
                                            dm2LastSessionSavePath,
                                            sizeof(dm2LastSessionSavePath)),
                "should write DM2 SKSave.dat browser last-session")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_dm2_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for DM2 SKSave.dat")) return 1;
    if (!expect(select_save_entry(&state, "SKSave.dat"),
                "save browser should list DM2 SKSave.dat")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark DM2 SKSave.dat loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "dm2") == 0,
                "save browser should classify SKSave.dat as dm2")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].mapLevel == 6,
                "save browser should expose DM2 last-session level")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "dm2") == 0,
                "save browser DM2 SKSave.dat launch intent should identify DM2")) return 1;
    if (!expect(intent.savePath &&
                strcmp(intent.savePath, dm2LastSessionSavePath) == 0,
                "save browser DM2 SKSave.dat launch intent should carry exact path")) return 1;

    snprintf(nexusSavePath, sizeof(nexusSavePath),
             "%s/firestaff-nexus-quicksave.sav", tmpTemplate);
    if (!expect(write_nexus_fnxs_save(nexusSavePath),
                "should write Nexus FNXS quick Resume save")) return 1;
    M12_Config_SetLastSavePath(nexusSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_nexus_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "Nexus FNXS save must enable quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "nexus") == 0,
                "Nexus FNXS quick Resume should identify nexus")) return 1;
    if (!expect(strcmp(state.quickResumeSavePath, nexusSavePath) == 0,
                "Nexus FNXS quick Resume should retain save path")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(state.launchRequested == 1 &&
                state.quickResumeLaunchRequested == 1,
                "Nexus quick Resume accept should request save-path launch")) return 1;
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "nexus") == 0,
                "Nexus quick Resume launch intent should identify Nexus")) return 1;
    if (!expect(intent.savePath &&
                strcmp(intent.savePath, nexusSavePath) == 0,
                "Nexus quick Resume launch intent should carry exact FNXS path")) return 1;

    snprintf(nexusBrowserSavePath, sizeof(nexusBrowserSavePath),
             "%s/firestaff-nexus-browser.sav", tmpTemplate);
    if (!expect(write_nexus_fnxs_save(nexusBrowserSavePath),
                "should write Nexus FNXS browser save")) return 1;
    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_nexus_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for Nexus FNXS saves")) return 1;
    if (!expect(select_save_entry(&state, "firestaff-nexus-browser.sav"),
                "save browser should list Nexus FNXS save")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark Nexus FNXS save loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "nexus") == 0,
                "save browser should classify FNXS save as nexus")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].mapLevel == 2,
                "save browser should expose Nexus saved level")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(state.launchRequested == 1 &&
                state.quickResumeLaunchRequested == 1,
                "save browser Nexus accept should request save-path launch")) return 1;
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "nexus") == 0,
                "save browser Nexus launch intent should identify Nexus")) return 1;
    if (!expect(intent.savePath &&
                strcmp(intent.savePath, nexusBrowserSavePath) == 0,
                "save browser Nexus launch intent should carry exact FNXS path")) return 1;

    snprintf(nexusSlotSavePath, sizeof(nexusSlotSavePath),
             "%s/nexus_save_03.dat", tmpTemplate);
    if (!expect(write_nexus_fnxs_save(nexusSlotSavePath),
                "should write Nexus manager slot save")) return 1;
    M12_Config_SetLastSavePath(nexusSlotSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_nexus_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "Nexus manager slot save must enable quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "nexus") == 0,
                "Nexus manager slot quick Resume should identify nexus")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "nexus") == 0 &&
                intent.savePath &&
                strcmp(intent.savePath, nexusSlotSavePath) == 0,
                "Nexus manager slot quick Resume should carry exact path")) return 1;

    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_nexus_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for Nexus manager slots")) return 1;
    if (!expect(select_save_entry(&state, "nexus_save_03.dat"),
                "save browser should list Nexus manager slot save")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark Nexus manager slot loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "nexus") == 0,
                "save browser should classify Nexus manager slot as nexus")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "nexus") == 0 &&
                intent.savePath &&
                strcmp(intent.savePath, nexusSlotSavePath) == 0,
                "save browser Nexus manager slot should carry exact path")) return 1;

    snprintf(theronSaveRoot, sizeof(theronSaveRoot),
             "%s/saves/theron", tmpTemplate);
    if (!expect(write_theron_tqsv_save(theronSaveRoot,
                                       4,
                                       theronSlotSavePath,
                                       sizeof(theronSlotSavePath)),
                "should write Theron .tqsv browser slot")) return 1;
    M12_Config_SetLastSavePath(theronSlotSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_theron_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "Theron .tqsv slot must enable quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "theron") == 0,
                "Theron .tqsv quick Resume should identify theron")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "theron") == 0 &&
                intent.savePath &&
                strcmp(intent.savePath, theronSlotSavePath) == 0,
                "Theron quick Resume should carry exact .tqsv path")) return 1;

    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_theron_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for Theron .tqsv slots")) return 1;
    if (!expect(select_save_entry(&state, "slot4.tqsv"),
                "save browser should list Theron slot4.tqsv")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark Theron .tqsv loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "theron") == 0,
                "save browser should classify Theron .tqsv as theron")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].mapLevel == 3,
                "save browser should expose Theron saved dungeon")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "theron") == 0 &&
                intent.savePath &&
                strcmp(intent.savePath, theronSlotSavePath) == 0,
                "save browser Theron launch intent should carry exact .tqsv path")) return 1;

#if FIRESTAFF_HAS_ZLIB
    if (!expect(write_theron_srm_save(theronSaveRoot,
                                      2,
                                      theronSrmSavePath,
                                      sizeof(theronSrmSavePath)),
                "should write Theron .srm browser slot")) return 1;
    M12_Config_SetLastSavePath(theronSrmSavePath);
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets", NULL);
    force_theron_available(&state);
    if (!expect(state.quickResumeAvailable == 1,
                "Theron .srm slot must enable quick Resume when decoded")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "theron") == 0,
                "Theron .srm quick Resume should identify theron")) return 1;
    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "theron") == 0 &&
                intent.savePath &&
                strcmp(intent.savePath, theronSrmSavePath) == 0,
                "Theron quick Resume should carry exact .srm path")) return 1;

    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_theron_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for Theron .srm slots")) return 1;
    if (!expect(select_save_entry(&state, "slot2.srm"),
                "save browser should list Theron slot2.srm")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1,
                "save browser should mark Theron .srm loadable")) return 1;
    if (!expect(strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "theron") == 0,
                "save browser should classify Theron .srm as theron")) return 1;
    if (!expect(state.saveBrowser.entries[state.saveBrowser.selectedIndex].mapLevel == 3,
                "save browser should expose Theron .srm saved dungeon")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "theron") == 0 &&
                intent.savePath &&
                strcmp(intent.savePath, theronSrmSavePath) == 0,
                "save browser Theron launch intent should carry exact .srm path")) return 1;
#endif

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

    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_dm1_available(&state);
    force_csb_available(&state);
    state.selectedIndex = 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACTION);
    if (!expect(state.view == M12_MENU_VIEW_SAVE_BROWSER,
                "main CSB action should open utility save browser")) return 1;
    if (!expect(state.saveBrowserReturnView == M12_MENU_VIEW_MAIN,
                "main CSB utility save browser should remember main return view")) return 1;
    if (!expect(state.saveBrowser.entryCount > 0 &&
                state.saveBrowser.selectedIndex >= 0 &&
                state.saveBrowser.entries[state.saveBrowser.selectedIndex].valid == 1 &&
                strcmp(state.saveBrowser.entries[state.saveBrowser.selectedIndex].gameId,
                       "dm1") == 0,
                "main CSB action should preselect a DM1 import candidate")) return 1;
    if (!expect(state.launchRequested == 0 &&
                state.csbImportDm1LaunchRequested == 0,
                "main CSB action should not launch before import confirmation")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_MAIN,
                "main CSB utility save browser Back should return to main")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.view == M12_MENU_VIEW_GAME_OPTIONS &&
                state.activatedIndex == 1,
                "main CSB accept should still open normal CSB options")) return 1;

    M12_StartupMenu_InitWithDataDir(&state, tmpTemplate, NULL);
    force_dm1_available(&state);
    force_csb_available(&state);
    if (!expect(M12_StartupMenu_OpenSaveBrowser(&state) == 0,
                "startup should open save browser for CSB DM1 import")) return 1;
    if (!expect(select_save_entry(&state, "firestaff-dm1-browser.sav"),
                "save browser should select DM1 save for CSB import")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACTION);
    if (!expect(state.view == M12_MENU_VIEW_MESSAGE &&
                state.csbImportDm1ConfirmActive == 1,
                "save browser action on DM1 save should open CSB import confirmation")) return 1;
    if (!expect(state.launchRequested == 0 &&
                state.csbImportDm1LaunchRequested == 0,
                "CSB DM1 import confirmation should not launch before confirm")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    if (!expect(state.view == M12_MENU_VIEW_SAVE_BROWSER &&
                state.csbImportDm1ConfirmActive == 0 &&
                state.launchRequested == 0,
                "CSB DM1 import confirmation Back should cancel and return to save browser")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACTION);
    if (!expect(state.csbImportDm1ConfirmActive == 1,
                "CSB DM1 import confirmation should re-arm after cancel")) return 1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1,
                "confirmed save browser action on DM1 save should request CSB import launch")) return 1;
    if (!expect(state.quickResumeLaunchRequested == 0,
                "CSB DM1 import launch should not mark quick Resume requested")) return 1;
    if (!expect(state.csbImportDm1LaunchRequested == 1,
                "CSB DM1 import launch should mark import requested")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 &&
                intent.gameId &&
                strcmp(intent.gameId, "csb") == 0,
                "CSB DM1 import launch intent should target CSB")) return 1;
    if (!expect(intent.savePath == NULL,
                "CSB DM1 import launch intent must not treat DM1 save as Resume")) return 1;
    if (!expect(intent.csbImportDm1SavePath &&
                strcmp(intent.csbImportDm1SavePath, nativeSavePath) == 0,
                "CSB DM1 import launch intent should carry selected DM1 save path")) return 1;

    puts("ok: quick Resume only carries save path for explicit Continue, never normal DM1 launch");
    return 0;
}
