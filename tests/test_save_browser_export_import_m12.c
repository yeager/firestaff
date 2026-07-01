#include "save_browser_m12.h"
#include "dm1_v1_original_save_pc34_handoff.h"
#include "dm1_v1_save_load.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef _WIN32
#include <direct.h>
#endif

#define ORIGINAL_PC34_CHAMPION_BYTES 319
#define ORIGINAL_PC34_PARTY_INFO_BYTES 128
#define ORIGINAL_PC34_PARTY_BYTES \
    ((ORIGINAL_PC34_CHAMPION_BYTES * CHAMPION_MAX_PARTY) + \
     ORIGINAL_PC34_PARTY_INFO_BYTES)
#define ORIGINAL_PC34_EVENT_BYTES 10
#define ORIGINAL_PC34_ADDITIONAL_DATA_META_OFFSET 122

static int failures = 0;

static void check(int ok, const char* name) {
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

static int mkdir_one(const char* path) {
#ifdef _WIN32
    if (_mkdir(path) == 0 || errno == EEXIST) return 1;
#else
    if (mkdir(path, 0755) == 0 || errno == EEXIST) return 1;
#endif
    return 0;
}

static int write_bytes(const char* path, const char* bytes) {
    FILE* fp = fopen(path, "wb");
    size_t len = strlen(bytes);
    if (!fp) return 0;
    if (fwrite(bytes, 1, len, fp) != len) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static int write_blob(const char* path, const unsigned char* bytes, int len) {
    FILE* fp;
    if (!path || !bytes || len <= 0) return 0;
    fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(bytes, 1, (size_t)len, fp) != (size_t)len) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

static void wr16le(unsigned char* p, uint16_t v) {
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
}

static void wr32le(unsigned char* p, uint32_t v) {
    wr16le(p, (uint16_t)(v & 0xffffu));
    wr16le(p + 2, (uint16_t)((v >> 16) & 0xffffu));
}

static uint16_t rd16le(const unsigned char* p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint16_t checksum_original_first_half(const unsigned char* header) {
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

static uint16_t checksum_original_second_half_plain(const unsigned char* header) {
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
    if (dstCap < 2 + byteCount || (byteCount & 1) != 0) return -1;
    wr16le(dst, (uint16_t)byteCount);
    if (byteCount > 0) {
        memcpy(dst + 2, plain, (size_t)byteCount);
    }
    *outChecksum = checksum_and_xor_original_words(
        dst + 2, (size_t)byteCount / 2u, key);
    return 2 + byteCount;
}

static void write_original_pc34_champion(unsigned char* dst,
                                         const char* name,
                                         const char* title) {
    memset(dst, 0, ORIGINAL_PC34_CHAMPION_BYTES);
    memset(dst + 0, ' ', 8u);
    memset(dst + 8, ' ', 20u);
    memcpy(dst + 0, name, strlen(name));
    memcpy(dst + 8, title, strlen(title));
    dst[28] = 1u;
    wr16le(dst + 52, 44u);
    wr16le(dst + 54, 55u);
    wr16le(dst + 56, 66u);
    wr16le(dst + 58, 77u);
    wr16le(dst + 60, 8u);
    wr16le(dst + 62, 9u);
    wr16le(dst + 66, 1500u);
    wr16le(dst + 68, 1200u);
    wr32le(dst + 91 + 2, 1000u);
    wr16le(dst + 271, 345u);
}

static int write_original_pc34_dm1_save(const char* path) {
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

    memset(buf, 0, sizeof(buf));
    memset(header, 0, sizeof(header));
    memset(global, 0, sizeof(global));
    memset(party, 0, sizeof(party));
    memset(event, 0, sizeof(event));
    memset(timeline, 0, sizeof(timeline));
    memset(checksums, 0, sizeof(checksums));

    for (i = 0; i < 127; ++i) {
        wr16le(header + (size_t)i * 2u,
               (uint16_t)(0x3141u + (uint16_t)(i * 13u)));
    }
    wr16le(header + 10u * 2u, 0x1357u);
    header[298] = 1u;
    header[299] = SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC;
    wr32le(header + 306u, 0x4f524731u);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = (uint16_t)(0x4100u + (uint16_t)(i * 0x31u));
    }

    wr32le(global + 0u, 1000u);
    wr16le(global + 10u, 1u);
    wr16le(global + 12u, 4u);
    wr16le(global + 14u, 5u);
    wr16le(global + 16u, 1u);
    wr16le(global + 18u, 8u);
    wr16le(global + 20u, 0u);
    wr16le(global + 24u, 0u);
    wr16le(global + 26u, 0u);
    wr16le(global + 28u, 1u);
    wr16le(global + 30u, 0u);
    wr16le(global + 46u, 0u);
    write_original_pc34_champion(party, "TIGGY", "APPRENTICE");

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
        uint16_t secondSum = checksum_original_second_half_plain(header);
        uint16_t firstBeforeLast = checksum_original_first_half(header);
        uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                   firstBeforeLast ^
                                   secondSum);
        wr16le(header + 254u, last);
    }
    xor_original_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(buf, header, sizeof(header));

    return write_blob(path, buf, cursor);
}

static int write_pc34_native_dm1_save(const char* path) {
    struct SaveGame_Compat state;
    struct PartyState_Compat party;
    unsigned char buf[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int rc;

    memset(&state, 0, sizeof(state));
    memset(&party, 0, sizeof(party));
    party.championCount = 1;
    party.mapIndex = 4;
    party.mapX = 7;
    party.mapY = 9;
    party.direction = 2;
    party.activeChampionIndex = 0;
    party.champions[0].present = 1;
    memcpy(party.champions[0].name, "TEST    ", 8);
    state.party = &party;

    rc = F0795_SAVEGAME_ExportPC34_Compat(
        &state, 0x44534D31u, buf, (int)sizeof(buf), &written);
    return rc == SAVEGAME_PC34_OK && write_blob(path, buf, written);
}

static int strip_pc34_manifest(unsigned char* buf, int len) {
    unsigned char metaHalf[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE / 2];
    uint16_t key;
    int i;

    if (!buf || len < SAVEGAME_PC34_DM_SAVE_HEADER_SIZE) return 0;
    key = (uint16_t)((unsigned)buf
                       [SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2]
                     | ((unsigned)buf
                       [SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2 + 1]
                        << 8));
    memcpy(metaHalf, buf + SAVEGAME_PC34_DM_SAVE_HEADER_SIZE / 2,
           sizeof(metaHalf));
    for (i = 0; i < SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS; ++i) {
        unsigned char* word = metaHalf + (size_t)i * 2u;
        uint16_t rollingKey =
            (uint16_t)(key + (uint16_t)(i * SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS));
        wr16le(word, (uint16_t)(rd16le(word) ^ rollingKey));
    }
    memset(metaHalf + ORIGINAL_PC34_ADDITIONAL_DATA_META_OFFSET, 0,
           SAVEGAME_PC34_MANIFEST_SIZE);
    for (i = 0; i < SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS; ++i) {
        unsigned char* word = metaHalf + (size_t)i * 2u;
        uint16_t rollingKey =
            (uint16_t)(key + (uint16_t)(i * SAVEGAME_PC34_DM_SAVE_HEADER_HALF_WORDS));
        wr16le(word, (uint16_t)(rd16le(word) ^ rollingKey));
    }
    memcpy(buf + SAVEGAME_PC34_DM_SAVE_HEADER_SIZE / 2, metaHalf,
           sizeof(metaHalf));
    return 1;
}

static int write_pc34_vanilla_dm1_save(const char* path) {
    struct SaveGame_Compat state;
    struct PartyState_Compat party;
    unsigned char buf[SAVEGAME_PC34_MAX_FILE_SIZE];
    int written = 0;
    int rc;

    memset(&state, 0, sizeof(state));
    memset(&party, 0, sizeof(party));
    party.championCount = 1;
    party.mapIndex = 6;
    party.mapX = 11;
    party.mapY = 13;
    party.direction = 1;
    party.activeChampionIndex = 0;
    party.champions[0].present = 1;
    memcpy(party.champions[0].name, "LEGACY  ", 8);
    state.party = &party;

    rc = F0795_SAVEGAME_ExportPC34_Compat(
        &state, 0x56414E31u, buf, (int)sizeof(buf), &written);
    return rc == SAVEGAME_PC34_OK &&
           strip_pc34_manifest(buf, written) &&
           write_blob(path, buf, written);
}

static int read_bytes(const char* path, char* out, size_t outBytes) {
    FILE* fp = fopen(path, "rb");
    size_t n;
    if (!fp || outBytes == 0) return 0;
    n = fread(out, 1, outBytes - 1u, fp);
    out[n] = '\0';
    fclose(fp);
    return 1;
}

static void seed_firestaff_native_dm1_world(struct GameWorld_Compat* world) {
    int i;
    F0881_WORLD_InitDefault_Compat(world, 0x53425631u);
    world->gameTick = 4567u;
    world->partyMapIndex = 3;
    world->party.mapIndex = 3;
    world->party.mapX = 18;
    world->party.mapY = 20;
    world->party.direction = 1;
    world->party.championCount = 1;
    world->party.activeChampionIndex = 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&world->party.champions[i]);
    }
    world->party.champions[0].present = 1;
    memcpy(world->party.champions[0].name, "HALK    ", CHAMPION_NAME_LENGTH);
    world->party.champions[0].hp.current = 91;
    world->party.champions[0].hp.maximum = 100;
    world->party.champions[0].mana.current = 12;
    world->party.champions[0].mana.maximum = 20;

    world->creatureAICount = 1;
    memset(&world->creatureAI[0], 0, sizeof(world->creatureAI[0]));
    world->creatureAI[0].stateKind = AI_STATE_WANDER;
    world->creatureAI[0].creatureType = CREATURE_TYPE_SKELETON;
    world->creatureAI[0].groupMapIndex = world->partyMapIndex;
    world->creatureAI[0].groupMapX = 12;
    world->creatureAI[0].groupMapY = 13;
    world->creatureAI[0].groupCells = 0x9a;
    world->creatureAI[0].groupDirection = DIR_SOUTH;
    world->creatureAI[0].reserved0 = 5;
}

static int write_firestaff_native_dm1_save(const char* path) {
    struct GameWorld_Compat world;
    int rc;

    memset(&world, 0, sizeof(world));
    seed_firestaff_native_dm1_world(&world);
    rc = DM1_SaveGameWithProfile(&world, path, 0x53425631u, 1, 1,
                                 DM1_DefaultSaveProfileHash());
    F0883_WORLD_Free_Compat(&world);
    return rc == DM1_SAVE_OK;
}

static void cleanup(const char* root) {
    char path[512];
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1-slot.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1-quicksave.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1-original.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-dm1-native.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data/firestaff-csb.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/backup/firestaff-dm1-slot.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/backup/firestaff-dm1-native-pc34.sav", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/backup/not-a-save.dat", root);
    unlink(path);
    snprintf(path, sizeof(path), "%s/data", root);
    rmdir(path);
    snprintf(path, sizeof(path), "%s/backup", root);
    rmdir(path);
    rmdir(root);
}

static const M12_SaveBrowserEntry* find_entry(
    const M12_SaveBrowserState* state, const char* filename) {
    int i;
    for (i = 0; i < state->entryCount; ++i) {
        if (strcmp(state->entries[i].filename, filename) == 0) {
            return &state->entries[i];
        }
    }
    return NULL;
}

int main(void) {
    char root[256];
    char dataDir[512];
    char backupDir[512];
    char savePath[512];
    char badPath[512];
    char outPath[512];
    char bytes[64];
    M12_SaveBrowserState state;
    const char* tmpRoot = getenv("FIRESTAFF_TEST_TMPDIR");

    if (!tmpRoot || !*tmpRoot) tmpRoot = "/tmp";
    snprintf(root, sizeof(root), "%s/firestaff_save_browser_export_import_%ld",
             tmpRoot, (long)getpid());
    cleanup(root);
    snprintf(dataDir, sizeof(dataDir), "%s/data", root);
    snprintf(backupDir, sizeof(backupDir), "%s/backup", root);
    check(mkdir_one(root), "created root");
    check(mkdir_one(dataDir), "created data dir");
    check(mkdir_one(backupDir), "created backup dir");

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-slot.sav", dataDir);
    check(write_bytes(savePath, "SAVE-BYTES-01"), "wrote source save bytes");
    check(M12_SaveBrowser_Scan(&state, dataDir) == 1, "scan finds source save");
    check(strcmp(state.entries[0].filename, "firestaff-dm1-slot.sav") == 0,
          "scan records save basename");

    check(M12_SaveBrowser_ExportSelected(&state, backupDir, outPath, (int)sizeof(outPath)) == 0,
          "export selected save succeeds");
    check(strstr(outPath, "/backup/firestaff-dm1-slot.sav") != NULL,
          "export reports backup path");
    check(read_bytes(outPath, bytes, sizeof(bytes)) && strcmp(bytes, "SAVE-BYTES-01") == 0,
          "export preserves file bytes");

    check(unlink(savePath) == 0, "removed original before import");
    check(M12_SaveBrowser_ImportFile(dataDir, outPath, savePath, (int)sizeof(savePath)) == 0,
          "import copied save back to data dir");
    check(read_bytes(savePath, bytes, sizeof(bytes)) && strcmp(bytes, "SAVE-BYTES-01") == 0,
          "import preserves file bytes");
    check(M12_SaveBrowser_ImportFile(dataDir, outPath, NULL, 0) == -1,
          "duplicate import preserves existing destination");

    snprintf(badPath, sizeof(badPath), "%s/not-a-save.dat", backupDir);
    check(write_bytes(badPath, "NOPE"), "wrote bad import fixture");
    check(M12_SaveBrowser_ImportFile(dataDir, badPath, outPath, (int)sizeof(outPath)) == -1,
          "non firestaff save filename rejected");
    check(M12_SaveBrowser_ExportSelected(NULL, backupDir, outPath, (int)sizeof(outPath)) == -1,
          "NULL export state rejected");

    check(unlink(savePath) == 0, "removed restored save before manifest scan");
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1.sav", dataDir);
    check(write_pc34_native_dm1_save(savePath), "wrote DM1 PC34 native manifest fixture");
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav", dataDir);
    check(write_pc34_vanilla_dm1_save(savePath), "wrote DM1 PC34 vanilla fixture");
    snprintf(savePath, sizeof(savePath), "%s/firestaff-csb.sav", dataDir);
    check(write_pc34_native_dm1_save(savePath), "wrote wrong-game PC34 native manifest fixture");
    check(M12_SaveBrowser_Scan(&state, dataDir) == 3,
          "scan finds native manifest fixtures");
    {
        const M12_SaveBrowserEntry* dm1 =
            find_entry(&state, "firestaff-dm1.sav");
        const M12_SaveBrowserEntry* vanilla =
            find_entry(&state, "firestaff-dm1-quicksave.sav");
        const M12_SaveBrowserEntry* csb =
            find_entry(&state, "firestaff-csb.sav");
        check(dm1 != NULL, "DM1 native manifest entry present");
        check(vanilla != NULL, "DM1 vanilla PC34 entry present");
        check(csb != NULL, "CSB wrong-game manifest entry present");
        if (dm1) {
            check(dm1->expectedGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "DM1 filename maps to DM1 game code");
            check(dm1->manifestGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "DM1 native manifest reports DM1 game code");
            check(dm1->manifestStatus == SAVE_BROWSER_MANIFEST_MATCH,
                  "DM1 native manifest matches selected game");
            check(dm1->valid == 1, "DM1 native manifest is load-browser valid");
            check(strstr(dm1->label, "PC34 DM1 save") != NULL,
                  "DM1 native manifest label is specific");
            state.selectedIndex = (int)(dm1 - state.entries);
            check(M12_SaveBrowser_HandleInput(&state, 5) == 1,
                  "DM1 native manifest can request load handoff");
        }
        if (vanilla) {
            check(vanilla->expectedGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "DM1 vanilla filename maps to DM1 game code");
            check(vanilla->manifestGameCode == 0,
                  "DM1 vanilla PC34 save has no manifest game code");
            check(vanilla->manifestStatus == SAVE_BROWSER_MANIFEST_NOT_PRESENT,
                  "DM1 vanilla PC34 save reports manifest-not-present");
            check(vanilla->valid == 1,
                  "DM1 vanilla PC34 save remains load-browser valid");
            check(vanilla->mapLevel == 6,
                  "DM1 vanilla PC34 save imports map level via native importer");
            check(strstr(vanilla->label, "vanilla PC34 save") != NULL,
                  "DM1 vanilla PC34 label names legacy interop path");
            state.selectedIndex = (int)(vanilla - state.entries);
            check(M12_SaveBrowser_HandleInput(&state, 5) == 1,
                  "DM1 vanilla PC34 save can request load handoff");
        }
        if (csb) {
            check(csb->expectedGameCode == SAVEGAME_PC34_GAME_CODE_CSB,
                  "CSB filename maps to CSB game code");
            check(csb->manifestGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "CSB wrong-game fixture still reports DM1 manifest");
            check(csb->manifestStatus == SAVE_BROWSER_MANIFEST_WRONG_GAME,
                  "CSB wrong-game manifest is rejected before launch");
            check(csb->valid == 0, "CSB wrong-game manifest is not valid");
            check(strstr(csb->label, "wrong-game save is DM1") != NULL,
                  "CSB wrong-game manifest label names actual game");
            state.selectedIndex = (int)(csb - state.entries);
            check(M12_SaveBrowser_HandleInput(&state, 5) == 0,
                  "CSB wrong-game manifest cannot request load handoff");
        }
    }

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav", dataDir);
    check(unlink(savePath) == 0, "removed vanilla save before original PC34 scan");
    check(write_original_pc34_dm1_save(savePath), "wrote DM1 original PC34 fixture");
    check(M12_SaveBrowser_Scan(&state, dataDir) == 3,
          "scan finds original PC34 fixture");
    {
        const M12_SaveBrowserEntry* original =
            find_entry(&state, "firestaff-dm1-quicksave.sav");
        check(original != NULL, "DM1 original PC34 entry present");
        if (original) {
            check(original->expectedGameCode == SAVEGAME_PC34_GAME_CODE_DM1,
                  "DM1 original filename maps to DM1 game code");
            check(original->manifestGameCode == 0,
                  "DM1 original PC34 save has no manifest game code");
            check(original->manifestStatus == SAVE_BROWSER_MANIFEST_NOT_PRESENT,
                  "DM1 original PC34 save reports manifest-not-present");
            check(original->valid == 1,
                  "DM1 original PC34 save is load-browser valid");
            check(original->mapLevel == 8,
                  "DM1 original PC34 save imports map level via original handoff");
            check(original->championCount == 1,
                  "DM1 original PC34 save imports champion count");
            check(strstr(original->champions, "TIGGY") != NULL,
                  "DM1 original PC34 save imports champion name");
            check(strstr(original->label, "original PC34 save") != NULL,
                  "DM1 original PC34 label names original handoff path");
            state.selectedIndex = (int)(original - state.entries);
            check(M12_SaveBrowser_HandleInput(&state, 5) == 1,
                  "DM1 original PC34 save can request load handoff");
        }
    }

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-native.sav", dataDir);
    check(write_firestaff_native_dm1_save(savePath),
          "wrote Firestaff-native DM1 save for PC34 export");
    check(M12_SaveBrowser_Scan(&state, dataDir) == 4,
          "scan finds Firestaff-native DM1 save before PC34 export");
    {
        const M12_SaveBrowserEntry* native =
            find_entry(&state, "firestaff-dm1-native.sav");
        check(native != NULL, "Firestaff-native DM1 entry present");
        if (native) {
            struct SaveGame_Compat imported;
            struct PartyState_Compat importedParty;
            struct TimelineQueue_Compat importedTimeline;
            DM1OriginalSavePC34HandoffReport report;
            int rc;

            check(native->valid == 1,
                  "Firestaff-native DM1 save is browser-valid");
            state.selectedIndex = (int)(native - state.entries);
            check(M12_SaveBrowser_ExportSelectedAsDM1PC34(
                      &state, backupDir, outPath, (int)sizeof(outPath)) == 0,
                  "export Firestaff-native DM1 save as PC34 succeeds");
            check(strstr(outPath, "/backup/firestaff-dm1-native-pc34.sav") != NULL,
                  "PC34 export reports suffixed backup path");

            memset(&imported, 0, sizeof(imported));
            memset(&importedParty, 0, sizeof(importedParty));
            memset(&importedTimeline, 0, sizeof(importedTimeline));
            memset(&report, 0, sizeof(report));
            imported.party = &importedParty;
            imported.timeline = &importedTimeline;
            rc = dm1_v1_original_save_pc34_handoff_file(outPath, &imported,
                                                        &report);
            check(rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK,
                  "PC34 export is accepted by original handoff");
            if (rc == DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
                check(importedParty.championCount == 1,
                      "PC34 export preserves champion count");
                check(importedParty.mapIndex == 3,
                      "PC34 export preserves party map");
                check(importedParty.mapX == 18 && importedParty.mapY == 20,
                      "PC34 export preserves party coordinates");
                check(memcmp(importedParty.champions[0].name, "HALK    ",
                             CHAMPION_NAME_LENGTH) == 0,
                      "PC34 export preserves champion name");
                check(importedParty.champions[0].hp.current == 91,
                      "PC34 export preserves champion HP");
                check(report.original_game_time == 4567u,
                      "PC34 export preserves game tick");
                check(report.original_current_active_group_count == 1,
                      "PC34 export preserves active-group count");
                check(report.active_groups[0].cells == 0x9a,
                      "PC34 export preserves active-group cells");
            }
            check(M12_SaveBrowser_ExportSelectedAsDM1PC34(
                      &state, backupDir, outPath, (int)sizeof(outPath)) == -1,
                  "duplicate PC34 export preserves existing destination");
        }
    }

    cleanup(root);
    if (failures) {
        printf("test_save_browser_export_import_m12: FAIL %d\n", failures);
        return 1;
    }
    puts("test_save_browser_export_import_m12: PASS");
    return 0;
}
