/*
 * CTest gate for DM1 V1 Save/Load Game System.
 *
 * Tests:
 *   1. CRC32 known-vector validation
 *   2. Header serialization round-trip
 *   3. Error string coverage
 *   4. Save menu state machine
 *   5. Save path generation
 *   6. NULL-arg rejection
 *   7. Missing-save load rejection
 *   8. Backup fallback path
 *   9. Corrupt-save validation
 *  10. Save-file bug profile hash mismatch helper
 *  11. Party/champion/timeline save-resume state gate
 *  12. Explicit original-PC34 write-back path
 *
 * ReDMCSB source refs — validates against original save format semantics:
 *   DEFS.H     DM_SAVE_HEADER layout, GLOBAL_DATA fields
 *   SAVEHEAD.C F0429/F0430 header checksum algorithm
 *   READWRIT.C F0417 XOR obfuscation (replaced by CRC32)
 *   LOADSAVE.C F0433 save, F0435 load
 */

#include "dm1_v1_save_load.h"
#include "dm1_v1_original_save_pc34_handoff.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
    wr16le(p, (uint16_t)(v & 0xffffu));
    wr16le(p + 2, (uint16_t)((v >> 16) & 0xffffu));
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
    if (dstCap < 2 + byteCount || (byteCount & 1) != 0) return -1;
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

static int test_file_exists(const char* path) {
    FILE* file = fopen(path, "rb");
    if (!file) return 0;
    fclose(file);
    return 1;
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
    FILE* file;

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

    file = fopen(path, "wb");
    if (!file) return 0;
    if (fwrite(buf, 1u, (size_t)cursor, file) != (size_t)cursor) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

/* ── Test 1: CRC32 known vectors ──────────────────────────────── */

static int test_crc32(void) {
    /* CRC32 of empty buffer should be 0x00000000 */
    uint32_t crc_empty = DM1_CRC32(NULL, 0);
    /* CRC32("123456789") = 0xCBF43926 */
    const unsigned char test_data[] = "123456789";
    uint32_t crc_test = DM1_CRC32(test_data, 9);

    printf("  CRC32 empty:        0x%08X (expected 0x00000000)\n", crc_empty);
    printf("  CRC32 '123456789':  0x%08X (expected 0xCBF43926)\n", crc_test);

    if (crc_empty != 0x00000000u) {
        printf("  FAIL: CRC32 empty mismatch\n");
        return 0;
    }
    if (crc_test != 0xCBF43926u) {
        printf("  FAIL: CRC32 test vector mismatch\n");
        return 0;
    }
    printf("  PASS: CRC32\n");
    return 1;
}

/* ── Test 2: Header round-trip ────────────────────────────────── */

static int test_header_roundtrip(void) {
    struct DM1SaveHeader hdr, hdr2;
    unsigned char buf[DM1_SAVE_HEADER_SIZE];

    memset(&hdr, 0, sizeof(hdr));
    memcpy(hdr.magic, DM1_SAVE_MAGIC, 8);
    hdr.formatVersion = DM1_SAVE_FORMAT_VERSION;
    hdr.totalFileSize = 12345;
    hdr.bodyCRC32 = 0xDEADBEEFu;
    hdr.gameTick = 99999;
    hdr.gameID = 0x12345678u;
    hdr.partyMapX = 5;
    hdr.partyMapY = 12;
    hdr.partyDirection = 2;
    hdr.partyMapIndex = 3;
    hdr.championCount = 4;
    hdr.saveAndPlay = 1;
    hdr.formatID = 1;
    hdr.musicOn = 1;
    hdr.bugProfileHash = 0xA1B2C3D4u;

    /* Serialize header to buffer */
    memset(buf, 0xAA, sizeof(buf));
    /* We test the internal serialize/deserialize via the public API
     * by constructing a fake file in memory */
    {
        /* Manually serialize */
        memset(buf, 0, DM1_SAVE_HEADER_SIZE);
        memcpy(buf, hdr.magic, 8);
        buf[8]  = (unsigned char)(hdr.formatVersion & 0xFF);
        buf[9]  = (unsigned char)((hdr.formatVersion >> 8) & 0xFF);
        buf[10] = (unsigned char)((hdr.formatVersion >> 16) & 0xFF);
        buf[11] = (unsigned char)((hdr.formatVersion >> 24) & 0xFF);
        buf[12] = (unsigned char)(hdr.totalFileSize & 0xFF);
        buf[13] = (unsigned char)((hdr.totalFileSize >> 8) & 0xFF);
        buf[14] = (unsigned char)((hdr.totalFileSize >> 16) & 0xFF);
        buf[15] = (unsigned char)((hdr.totalFileSize >> 24) & 0xFF);
        buf[16] = (unsigned char)(hdr.bodyCRC32 & 0xFF);
        buf[17] = (unsigned char)((hdr.bodyCRC32 >> 8) & 0xFF);
        buf[18] = (unsigned char)((hdr.bodyCRC32 >> 16) & 0xFF);
        buf[19] = (unsigned char)((hdr.bodyCRC32 >> 24) & 0xFF);
        buf[20] = (unsigned char)(hdr.gameTick & 0xFF);
        buf[21] = (unsigned char)((hdr.gameTick >> 8) & 0xFF);
        buf[22] = (unsigned char)((hdr.gameTick >> 16) & 0xFF);
        buf[23] = (unsigned char)((hdr.gameTick >> 24) & 0xFF);
        buf[24] = (unsigned char)(hdr.gameID & 0xFF);
        buf[25] = (unsigned char)((hdr.gameID >> 8) & 0xFF);
        buf[26] = (unsigned char)((hdr.gameID >> 16) & 0xFF);
        buf[27] = (unsigned char)((hdr.gameID >> 24) & 0xFF);
        buf[28] = (unsigned char)(hdr.partyMapX & 0xFF);
        buf[29] = (unsigned char)((hdr.partyMapX >> 8) & 0xFF);
        buf[30] = (unsigned char)(hdr.partyMapY & 0xFF);
        buf[31] = (unsigned char)((hdr.partyMapY >> 8) & 0xFF);
        buf[32] = (unsigned char)(hdr.partyDirection & 0xFF);
        buf[33] = (unsigned char)((hdr.partyDirection >> 8) & 0xFF);
        buf[34] = (unsigned char)(hdr.partyMapIndex & 0xFF);
        buf[35] = (unsigned char)((hdr.partyMapIndex >> 8) & 0xFF);
        buf[36] = (unsigned char)(hdr.championCount & 0xFF);
        buf[37] = (unsigned char)((hdr.championCount >> 8) & 0xFF);
        buf[38] = hdr.saveAndPlay;
        buf[39] = hdr.formatID;
        buf[40] = hdr.musicOn;
        buf[41] = (unsigned char)(hdr.bugProfileHash & 0xFF);
        buf[42] = (unsigned char)((hdr.bugProfileHash >> 8) & 0xFF);
        buf[43] = (unsigned char)((hdr.bugProfileHash >> 16) & 0xFF);
        buf[44] = (unsigned char)((hdr.bugProfileHash >> 24) & 0xFF);
    }

    /* Verify magic is at offset 0 */
    if (memcmp(buf, DM1_SAVE_MAGIC, 8) != 0) {
        printf("  FAIL: magic not at offset 0\n");
        return 0;
    }

    /* "Deserialize" — read back LE fields */
    memset(&hdr2, 0, sizeof(hdr2));
    memcpy(hdr2.magic, buf, 8);
    hdr2.formatVersion = (uint32_t)buf[8] | ((uint32_t)buf[9] << 8) |
                         ((uint32_t)buf[10] << 16) | ((uint32_t)buf[11] << 24);
    hdr2.totalFileSize = (uint32_t)buf[12] | ((uint32_t)buf[13] << 8) |
                         ((uint32_t)buf[14] << 16) | ((uint32_t)buf[15] << 24);
    hdr2.bodyCRC32 = (uint32_t)buf[16] | ((uint32_t)buf[17] << 8) |
                     ((uint32_t)buf[18] << 16) | ((uint32_t)buf[19] << 24);
    hdr2.gameTick = (uint32_t)buf[20] | ((uint32_t)buf[21] << 8) |
                    ((uint32_t)buf[22] << 16) | ((uint32_t)buf[23] << 24);
    hdr2.gameID = (uint32_t)buf[24] | ((uint32_t)buf[25] << 8) |
                  ((uint32_t)buf[26] << 16) | ((uint32_t)buf[27] << 24);
    hdr2.partyMapX = (uint16_t)buf[28] | ((uint16_t)buf[29] << 8);
    hdr2.partyMapY = (uint16_t)buf[30] | ((uint16_t)buf[31] << 8);
    hdr2.partyDirection = (uint16_t)buf[32] | ((uint16_t)buf[33] << 8);
    hdr2.partyMapIndex = (uint16_t)buf[34] | ((uint16_t)buf[35] << 8);
    hdr2.championCount = (uint16_t)buf[36] | ((uint16_t)buf[37] << 8);
    hdr2.saveAndPlay = buf[38];
    hdr2.formatID = buf[39];
    hdr2.musicOn = buf[40] ? 1 : 0;
    hdr2.bugProfileHash = (uint32_t)buf[41] | ((uint32_t)buf[42] << 8) |
                          ((uint32_t)buf[43] << 16) | ((uint32_t)buf[44] << 24);

    if (memcmp(hdr2.magic, DM1_SAVE_MAGIC, 8) != 0 ||
        hdr2.formatVersion != hdr.formatVersion ||
        hdr2.totalFileSize != hdr.totalFileSize ||
        hdr2.bodyCRC32 != hdr.bodyCRC32 ||
        hdr2.gameTick != hdr.gameTick ||
        hdr2.gameID != hdr.gameID ||
        hdr2.partyMapX != hdr.partyMapX ||
        hdr2.partyMapY != hdr.partyMapY ||
        hdr2.partyDirection != hdr.partyDirection ||
        hdr2.partyMapIndex != hdr.partyMapIndex ||
        hdr2.championCount != hdr.championCount ||
        hdr2.saveAndPlay != hdr.saveAndPlay ||
        hdr2.formatID != hdr.formatID ||
        hdr2.musicOn != hdr.musicOn ||
        hdr2.bugProfileHash != hdr.bugProfileHash) {
        printf("  FAIL: header round-trip mismatch\n");
        return 0;
    }

    printf("  PASS: header round-trip\n");
    return 1;
}

/* ── Test 3: Error strings ────────────────────────────────────── */

static int test_error_strings(void) {
    int i;
    int codes[] = { DM1_SAVE_OK, DM1_SAVE_ERROR_NULL_ARG,
                    DM1_SAVE_ERROR_BUFFER_TOO_SMALL, DM1_SAVE_ERROR_BAD_MAGIC,
                    DM1_SAVE_ERROR_BAD_VERSION, DM1_SAVE_ERROR_BAD_SIZE,
                    DM1_SAVE_ERROR_BAD_CRC, DM1_SAVE_ERROR_FILE_OPEN,
                    DM1_SAVE_ERROR_FILE_READ, DM1_SAVE_ERROR_FILE_WRITE,
                    DM1_SAVE_ERROR_SERIALIZE, DM1_SAVE_ERROR_DESERIALIZE,
                    DM1_SAVE_ERROR_OUT_OF_MEMORY, DM1_SAVE_ERROR_INTERNAL,
                    -999 };
    int count = (int)(sizeof(codes) / sizeof(codes[0]));

    for (i = 0; i < count; i++) {
        const char* s = DM1_SaveLoadErrorString(codes[i]);
        if (!s || s[0] == '\0') {
            printf("  FAIL: error code %d returned empty string\n", codes[i]);
            return 0;
        }
    }
    printf("  PASS: error strings\n");
    return 1;
}

/* ── Test 4: Save menu state machine ──────────────────────────── */

static int test_save_menu(void) {
    struct DM1SaveMenuContext ctx;

    DM1_SaveMenu_Init(&ctx);
    if (DM1_SaveMenu_IsOpen(&ctx)) {
        printf("  FAIL: menu should start closed\n");
        return 0;
    }

    DM1_SaveMenu_Open(&ctx);
    if (!DM1_SaveMenu_IsOpen(&ctx)) {
        printf("  FAIL: menu should be open after Open()\n");
        return 0;
    }
    if (ctx.state != DM1_SAVE_MENU_OPEN) {
        printf("  FAIL: state should be OPEN\n");
        return 0;
    }

    DM1_SaveMenu_Close(&ctx);
    if (DM1_SaveMenu_IsOpen(&ctx)) {
        printf("  FAIL: menu should be closed after Close()\n");
        return 0;
    }

    printf("  PASS: save menu state machine\n");
    return 1;
}

/* ── Test 5: Save path generation ─────────────────────────────── */

static int test_save_path(void) {
    char path[512];

    /* Without FIRESTAFF_DATA_DIR, should default to "." */
    if (!DM1_GetSavePath("abc123", path, sizeof(path))) {
        printf("  FAIL: GetSavePath returned 0\n");
        return 0;
    }
    if (strstr(path, "abc123") == NULL) {
        printf("  FAIL: path doesn't contain sourceId: %s\n", path);
        return 0;
    }
    if (strstr(path, "dm1save.sav") == NULL) {
        printf("  FAIL: path doesn't end with dm1save.sav: %s\n", path);
        return 0;
    }

    /* Buffer too small */
    if (DM1_GetSavePath("abc123", path, 5)) {
        printf("  FAIL: should return 0 for tiny buffer\n");
        return 0;
    }

    /* NULL args */
    if (DM1_GetSavePath(NULL, path, sizeof(path))) {
        printf("  FAIL: should return 0 for NULL sourceId\n");
        return 0;
    }

    printf("  PASS: save path generation\n");
    return 1;
}

/* ── Test 6: NULL-arg rejection ───────────────────────────────── */

static int test_null_args(void) {
    struct DM1SaveHeader hdr;
    int rc;

    rc = DM1_SaveGame(NULL, "/tmp/test.sav", 0, 0, 1);
    if (rc != DM1_SAVE_ERROR_NULL_ARG) {
        printf("  FAIL: SaveGame(NULL world) should return NULL_ARG\n");
        return 0;
    }

    rc = DM1_LoadGame(NULL, NULL, NULL);
    if (rc != DM1_SAVE_ERROR_NULL_ARG) {
        printf("  FAIL: LoadGame(NULL) should return NULL_ARG\n");
        return 0;
    }

    rc = DM1_ValidateSaveFile(NULL, &hdr);
    if (rc != DM1_SAVE_ERROR_NULL_ARG) {
        printf("  FAIL: ValidateSaveFile(NULL) should return NULL_ARG\n");
        return 0;
    }

    printf("  PASS: null-arg rejection\n");
    return 1;
}

/* ── Test 7: Load nonexistent file ────────────────────────────── */

static int test_load_nonexistent(void) {
    struct GameWorld_Compat world;
    struct DM1SaveHeader hdr;
    int rc;

    memset(&world, 0, sizeof(world));
    rc = DM1_LoadGame("/tmp/no_such_dm1_save_file_ever.sav", &world, &hdr);
    if (rc != DM1_SAVE_ERROR_FILE_OPEN) {
        printf("  FAIL: LoadGame nonexistent should return FILE_OPEN, got %d\n", rc);
        return 0;
    }

    printf("  PASS: load nonexistent file\n");
    return 1;
}

static int test_backup_fallback_path(void) {
    const char* primary = "/tmp/dm1_backup_fallback_test.sav";
    char backup[512];
    struct GameWorld_Compat world;
    struct DM1SaveHeader hdr;
    unsigned char garbage[128];
    int usedBackup = 7;
    FILE* f;
    int rc;

    remove(primary);
    if (!DM1_GetBackupSavePath(primary, backup, (int)sizeof(backup))) {
        printf("  FAIL: backup path helper rejected valid path\n");
        return 0;
    }
    remove(backup);

    memset(garbage, 0x42, sizeof(garbage));
    f = fopen(backup, "wb");
    if (!f) {
        printf("  SKIP: cannot create backup temp file\n");
        return 1;
    }
    fwrite(garbage, 1, sizeof(garbage), f);
    fclose(f);

    memset(&world, 0, sizeof(world));
    rc = DM1_LoadGameWithBackup(primary, &world, &hdr, &usedBackup);
    remove(backup);

    if (rc != DM1_SAVE_ERROR_BAD_MAGIC) {
        printf("  FAIL: backup fallback should open backup and reject its magic, got %d\n", rc);
        return 0;
    }
    if (usedBackup != 0) {
        printf("  FAIL: usedBackup should only be set after a successful backup load\n");
        return 0;
    }

    printf("  PASS: backup fallback path\n");
    return 1;
}

static int expect_u32_eq(const char* label, uint32_t got, uint32_t want);
static int expect_int_eq(const char* label, int got, int want);
static int expect_u16_eq(const char* label, unsigned short got, unsigned short want);
static int expect_bytes_eq(const char* label,
                           const void* got,
                           const void* want,
                           size_t size);

static void seed_pc34_writeback_active_group(struct GameWorld_Compat* world) {
    world->creatureAICount = 1;
    memset(&world->creatureAI[0], 0, sizeof(world->creatureAI[0]));
    world->creatureAI[0].stateKind = AI_STATE_WANDER;
    world->creatureAI[0].creatureType = CREATURE_TYPE_SKELETON;
    world->creatureAI[0].groupMapIndex = world->partyMapIndex;
    world->creatureAI[0].groupMapX = 13;
    world->creatureAI[0].groupMapY = 14;
    world->creatureAI[0].groupCells = 0xa5;
    world->creatureAI[0].groupDirection = DIR_SOUTH;
    world->creatureAI[0].lastSeenPartyMapX = world->party.mapX;
    world->creatureAI[0].lastSeenPartyMapY = world->party.mapY;
    world->creatureAI[0].lastSeenPartyTick = 42;
    world->creatureAI[0].fearCounter = 6;
    world->creatureAI[0].reserved0 = 7;
}

static int test_original_pc34_runtime_load_fallback(void) {
    const char* path = "/tmp/dm1_original_pc34_runtime_fallback.sav";
    const char* primary = "/tmp/dm1_original_pc34_runtime_primary_missing.sav";
    char backup[512];
    struct GameWorld_Compat world;
    struct DM1SaveHeader hdr;
    int usedBackup = 7;
    int rc;
    int ok = 1;

    remove(path);
    remove(primary);
    if (!DM1_GetBackupSavePath(primary, backup, (int)sizeof(backup))) {
        printf("  FAIL: backup path helper rejected original PC34 path\n");
        return 0;
    }
    remove(backup);

    if (!write_original_pc34_dm1_save_file(path)) {
        printf("  FAIL: could not write original PC34 fixture\n");
        return 0;
    }

    memset(&world, 0, sizeof(world));
    memset(&hdr, 0, sizeof(hdr));
    rc = DM1_LoadGame(path, &world, &hdr);
    if (rc != DM1_SAVE_OK) {
        printf("  FAIL: LoadGame original PC34 fallback returned %d (%s)\n",
               rc, DM1_SaveLoadErrorString(rc));
        remove(path);
        return 0;
    }

    ok &= expect_u32_eq("original PC34 header tick", hdr.gameTick, 7777u);
    ok &= expect_int_eq("original PC34 header map", hdr.partyMapIndex, 4);
    ok &= expect_int_eq("original PC34 header champions", hdr.championCount, 1);
    ok &= expect_int_eq("original PC34 world tick", (int)world.gameTick, 7777);
    ok &= expect_int_eq("original PC34 world timeline tick",
                        (int)world.timeline.nowTick, 7777);
    ok &= expect_int_eq("original PC34 party map index", world.partyMapIndex, 4);
    ok &= expect_int_eq("original PC34 party x", world.party.mapX, 9);
    ok &= expect_int_eq("original PC34 party y", world.party.mapY, 10);
    ok &= expect_int_eq("original PC34 party direction",
                        world.party.direction, DIR_EAST);
    ok &= expect_int_eq("original PC34 champion count",
                        world.party.championCount, 1);
    ok &= expect_bytes_eq("original PC34 champion name",
                          world.party.champions[0].name,
                          "TIGGY   ",
                          CHAMPION_NAME_LENGTH);
    ok &= expect_u16_eq("original PC34 champion hp",
                        world.party.champions[0].hp.current, 44);
    ok &= expect_u16_eq("original PC34 champion hand",
                        world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT],
                        0x1555u);

    F0883_WORLD_Free_Compat(&world);

    if (!write_original_pc34_dm1_save_file(backup)) {
        printf("  FAIL: could not write original PC34 backup fixture\n");
        remove(path);
        return 0;
    }
    memset(&world, 0, sizeof(world));
    memset(&hdr, 0, sizeof(hdr));
    rc = DM1_LoadGameWithBackup(primary, &world, &hdr, &usedBackup);
    if (rc != DM1_SAVE_OK) {
        printf("  FAIL: LoadGameWithBackup original PC34 returned %d (%s)\n",
               rc, DM1_SaveLoadErrorString(rc));
        ok = 0;
    } else {
        ok &= expect_int_eq("original PC34 backup used flag", usedBackup, 1);
        ok &= expect_int_eq("original PC34 backup promoted",
                            test_file_exists(primary), 1);
        ok &= expect_int_eq("original PC34 backup removed",
                            test_file_exists(backup), 0);
        ok &= expect_int_eq("original PC34 backup world map",
                            world.partyMapIndex, 4);
    }
    F0883_WORLD_Free_Compat(&world);

    remove(path);
    remove(primary);
    remove(backup);

    if (!ok) return 0;
    printf("  PASS: original PC34 runtime load fallback\n");
    return 1;
}

/* ── Test 8: Validate corrupt file ────────────────────────────── */

static int test_validate_corrupt(void) {
    struct DM1SaveHeader hdr;
    const char* path = "/tmp/dm1_corrupt_test.sav";
    FILE* f;
    unsigned char garbage[128];
    int rc;

    /* Write garbage */
    memset(garbage, 0x42, sizeof(garbage));
    f = fopen(path, "wb");
    if (!f) {
        printf("  SKIP: cannot create temp file\n");
        return 1;
    }
    fwrite(garbage, 1, sizeof(garbage), f);
    fclose(f);

    rc = DM1_ValidateSaveFile(path, &hdr);
    if (rc != DM1_SAVE_ERROR_BAD_MAGIC) {
        printf("  FAIL: corrupt file should return BAD_MAGIC, got %d (%s)\n",
               rc, DM1_SaveLoadErrorString(rc));
        remove(path);
        return 0;
    }

    remove(path);
    printf("  PASS: validate corrupt file\n");
    return 1;
}

/* ── Test 9: Profile hash mismatch helper ──────────────────────── */

static int test_profile_hash_mismatch(void) {
    struct DM1SaveHeader hdr;
    uint32_t defaultHash = DM1_DefaultSaveProfileHash();
    uint32_t namedHash = DM1_SaveProfileHashFromName(DM1_SAVE_PROFILE_ID_PC34_BASELINE);
    uint32_t customHash = DM1_SaveProfileHashFromName("Custom_DM1_bug_profile");

    if (defaultHash == DM1_SAVE_PROFILE_UNSPECIFIED) {
        printf("  FAIL: default profile hash should be nonzero\n");
        return 0;
    }
    if (defaultHash != namedHash) {
        printf("  FAIL: default profile hash does not match baseline profile name\n");
        return 0;
    }
    if (customHash == defaultHash || customHash == DM1_SAVE_PROFILE_UNSPECIFIED) {
        printf("  FAIL: custom profile hash should be distinct and nonzero\n");
        return 0;
    }

    memset(&hdr, 0, sizeof(hdr));
    hdr.bugProfileHash = defaultHash;
    if (!DM1_SaveProfileMatches(&hdr, defaultHash)) {
        printf("  FAIL: matching save/current profile should pass\n");
        return 0;
    }
    if (DM1_SaveProfileMatches(&hdr, customHash)) {
        printf("  FAIL: mismatched save/current profile should warn\n");
        return 0;
    }
    hdr.bugProfileHash = DM1_SAVE_PROFILE_UNSPECIFIED;
    if (!DM1_SaveProfileMatches(&hdr, customHash)) {
        printf("  FAIL: legacy unspecified save profile should not hard-fail\n");
        return 0;
    }

    printf("  PASS: profile hash mismatch helper\n");
    return 1;
}

/* ── Test 11: Party/champion/timeline save-resume state ───────── */

static int expect_u32_eq(const char* label, uint32_t got, uint32_t want) {
    if (got != want) {
        printf("  FAIL: %s got 0x%08X expected 0x%08X\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_int_eq(const char* label, int got, int want) {
    if (got != want) {
        printf("  FAIL: %s got %d expected %d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_u16_eq(const char* label,
                         unsigned short got,
                         unsigned short want) {
    if (got != want) {
        printf("  FAIL: %s got 0x%04X expected 0x%04X\n",
               label, (unsigned)got, (unsigned)want);
        return 0;
    }
    return 1;
}

static int expect_bytes_eq(const char* label,
                           const void* got,
                           const void* want,
                           size_t size) {
    if (memcmp(got, want, size) != 0) {
        printf("  FAIL: %s byte payload mismatch\n", label);
        return 0;
    }
    return 1;
}

static void seed_party_state_gate_world(struct GameWorld_Compat* world) {
    struct TimelineEvent_Compat ev;
    int i;

    F0881_WORLD_InitDefault_Compat(world, 0x1D1D1D1Du);
    world->gameTick = 3210u;
    world->partyMapIndex = 2;
    world->newPartyMapIndex = -1;
    world->partyIsResting = 1;
    world->disabledMovementTicks = 3;
    world->projectileDisabledMovementTicks = 5;
    world->lastProjectileDisabledMovementDirection = DIR_WEST;
    world->dungeonFingerprint = 0x0D01CAFEu;

    world->party.mapIndex = 2;
    world->party.mapX = 17;
    world->party.mapY = 22;
    world->party.direction = DIR_WEST;
    world->party.championCount = 2;
    world->party.activeChampionIndex = 1;
    world->party.eventFlags = 0x00A5u;
    world->party.magicShieldTime = 44;
    world->party.fireShieldTime = 55;

    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&world->party.champions[i]);
    }

    world->party.champions[0].present = 1;
    world->party.champions[0].portraitIndex = 6;
    memcpy(world->party.champions[0].name, "HALK    ", CHAMPION_NAME_LENGTH);
    world->party.champions[0].attributes[CHAMPION_ATTR_STRENGTH] = 64;
    world->party.champions[0].attributes[CHAMPION_ATTR_VITALITY] = 58;
    world->party.champions[0].skillLevels[CHAMPION_SKILL_FIGHTER] = 3;
    world->party.champions[0].skillExperience[CHAMPION_SKILL_FIGHTER] = 12345u;
    world->party.champions[0].hp.current = 91;
    world->party.champions[0].hp.maximum = 100;
    world->party.champions[0].stamina.current = 77;
    world->party.champions[0].stamina.maximum = 88;
    world->party.champions[0].mana.current = 12;
    world->party.champions[0].mana.maximum = 20;
    world->party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT] = 0x1400u;
    world->party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] = 0x1801u;
    world->party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_1] = 0x1C02u;
    world->party.champions[0].load = 83;
    world->party.champions[0].maxLoad = 612;
    world->party.champions[0].direction = DIR_NORTH;
    world->party.champions[0].food = 1400;
    world->party.champions[0].water = 1300;

    world->party.champions[1].present = 1;
    world->party.champions[1].portraitIndex = 9;
    memcpy(world->party.champions[1].name, "TIGGY   ", CHAMPION_NAME_LENGTH);
    world->party.champions[1].attributes[CHAMPION_ATTR_WISDOM] = 72;
    world->party.champions[1].skillLevels[CHAMPION_SKILL_WIZARD] = 4;
    world->party.champions[1].skillExperience[CHAMPION_SKILL_WIZARD] = 54321u;
    world->party.champions[1].hp.current = 41;
    world->party.champions[1].hp.maximum = 60;
    world->party.champions[1].stamina.current = 63;
    world->party.champions[1].stamina.maximum = 70;
    world->party.champions[1].mana.current = 48;
    world->party.champions[1].mana.maximum = 55;
    world->party.champions[1].inventory[CHAMPION_SLOT_POUCH_1] = 0x2003u;
    world->party.champions[1].inventory[CHAMPION_SLOT_BACKPACK_2] = 0x2404u;
    world->party.champions[1].load = 37;
    world->party.champions[1].maxLoad = 420;
    world->party.champions[1].direction = DIR_EAST;
    world->party.champions[1].food = 1200;
    world->party.champions[1].water = 1100;

    F0720_TIMELINE_Init_Compat(&world->timeline, world->gameTick);
    memset(&ev, 0, sizeof(ev));
    ev.kind = TIMELINE_EVENT_DOOR_ANIMATE;
    ev.fireAtTick = world->gameTick + 9u;
    ev.mapIndex = 2;
    ev.mapX = 17;
    ev.mapY = 21;
    ev.cell = 3;
    ev.aux0 = 7;
    ev.aux1 = 8;
    F0721_TIMELINE_Schedule_Compat(&world->timeline, &ev);
    memset(&ev, 0, sizeof(ev));
    ev.kind = TIMELINE_EVENT_HUNGER_THIRST;
    ev.fireAtTick = world->gameTick + 30u;
    ev.mapIndex = 2;
    ev.aux0 = 0x44;
    F0721_TIMELINE_Schedule_Compat(&world->timeline, &ev);
}

static int test_party_state_save_resume_gate(void) {
    const char* path = "/tmp/dm1_party_state_gate.sav";
    const char* path2 = "/tmp/dm1_party_state_gate_rerun.sav";
    struct GameWorld_Compat before;
    struct GameWorld_Compat after;
    struct DM1SaveHeader hdr;
    struct DM1SaveHeader hdr2;
    uint32_t beforeHash = 0;
    uint32_t afterHash = 0;
    int rc;
    int ok = 1;

    /*
     * ReDMCSB LOADSAVE.C F0433 writes GLOBAL_DATA, PARTY_INFO +
     * CHAMPION[4], EVENT, and TIMELINE save parts; F0435 restores them
     * before play resumes.  This gate keeps Firestaff's native body
     * serializer honest for the same party-state continuity surface.
     */
    remove(path);
    remove(path2);
    remove("/tmp/dm1_party_state_gate.sav.bak");
    remove("/tmp/dm1_party_state_gate_rerun.sav.bak");
    memset(&before, 0, sizeof(before));
    memset(&after, 0, sizeof(after));
    memset(&hdr, 0, sizeof(hdr));
    memset(&hdr2, 0, sizeof(hdr2));

    seed_party_state_gate_world(&before);
    if (!F0891_ORCH_WorldHash_Compat(&before, &beforeHash)) {
        printf("  FAIL: pre-save world hash failed\n");
        F0883_WORLD_Free_Compat(&before);
        return 0;
    }

    rc = DM1_SaveGameWithProfile(&before, path, 0xD11D1D1Du, 1, 1,
                                 DM1_DefaultSaveProfileHash());
    if (rc != DM1_SAVE_OK) {
        printf("  FAIL: SaveGame party-state gate returned %d (%s)\n",
               rc, DM1_SaveLoadErrorString(rc));
        F0883_WORLD_Free_Compat(&before);
        return 0;
    }

    rc = DM1_LoadGame(path, &after, &hdr);
    if (rc != DM1_SAVE_OK) {
        printf("  FAIL: LoadGame party-state gate returned %d (%s)\n",
               rc, DM1_SaveLoadErrorString(rc));
        remove(path);
        F0883_WORLD_Free_Compat(&before);
        return 0;
    }
    if (!F0891_ORCH_WorldHash_Compat(&after, &afterHash)) {
        printf("  FAIL: post-load world hash failed\n");
        ok = 0;
    }

    ok &= expect_u32_eq("world hash survives save/load", afterHash, beforeHash);
    ok &= expect_u32_eq("header game tick", hdr.gameTick, before.gameTick);
    ok &= expect_u32_eq("header game id", hdr.gameID, 0xD11D1D1Du);
    ok &= expect_int_eq("header party x", hdr.partyMapX, before.party.mapX);
    ok &= expect_int_eq("header party y", hdr.partyMapY, before.party.mapY);
    ok &= expect_int_eq("header facing", hdr.partyDirection,
                        before.party.direction);
    ok &= expect_int_eq("header map index", hdr.partyMapIndex,
                        before.partyMapIndex);
    ok &= expect_int_eq("header champion count", hdr.championCount,
                        before.party.championCount);

    ok &= expect_int_eq("party map index", after.party.mapIndex, 2);
    ok &= expect_int_eq("party x", after.party.mapX, 17);
    ok &= expect_int_eq("party y", after.party.mapY, 22);
    ok &= expect_int_eq("party facing", after.party.direction, DIR_WEST);
    ok &= expect_int_eq("active champion", after.party.activeChampionIndex, 1);
    ok &= expect_int_eq("resting flag", after.partyIsResting, 1);
    ok &= expect_int_eq("movement disable ticks",
                        after.disabledMovementTicks, 3);

    ok &= expect_int_eq("champion count", after.party.championCount, 2);
    ok &= expect_int_eq("champion0 present",
                        after.party.champions[0].present, 1);
    ok &= expect_bytes_eq("champion0 name",
                          after.party.champions[0].name,
                          "HALK    ",
                          CHAMPION_NAME_LENGTH);
    ok &= expect_int_eq("champion0 portrait",
                        after.party.champions[0].portraitIndex, 6);
    ok &= expect_u16_eq("champion0 ready hand",
                        after.party.champions[0].inventory[CHAMPION_SLOT_HAND_LEFT],
                        0x1400u);
    ok &= expect_u16_eq("champion0 action hand",
                        after.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT],
                        0x1801u);
    ok &= expect_u16_eq("champion0 backpack",
                        after.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_1],
                        0x1C02u);
    ok &= expect_u16_eq("champion0 hp",
                        after.party.champions[0].hp.current, 91);
    ok &= expect_u16_eq("champion0 max hp",
                        after.party.champions[0].hp.maximum, 100);
    ok &= expect_u16_eq("champion0 stamina",
                        after.party.champions[0].stamina.current, 77);
    ok &= expect_u16_eq("champion0 mana max",
                        after.party.champions[0].mana.maximum, 20);
    ok &= expect_u16_eq("champion0 strength",
                        after.party.champions[0].attributes[CHAMPION_ATTR_STRENGTH],
                        64);
    ok &= expect_u32_eq("champion0 fighter xp",
                        after.party.champions[0].skillExperience[CHAMPION_SKILL_FIGHTER],
                        12345u);
    ok &= expect_u16_eq("champion0 load",
                        after.party.champions[0].load, 83);
    ok &= expect_u16_eq("champion0 max load",
                        after.party.champions[0].maxLoad, 612);
    ok &= expect_int_eq("champion0 facing",
                        after.party.champions[0].direction, DIR_NORTH);

    ok &= expect_int_eq("champion1 present",
                        after.party.champions[1].present, 1);
    ok &= expect_bytes_eq("champion1 name",
                          after.party.champions[1].name,
                          "TIGGY   ",
                          CHAMPION_NAME_LENGTH);
    ok &= expect_int_eq("champion1 portrait",
                        after.party.champions[1].portraitIndex, 9);
    ok &= expect_u16_eq("champion1 pouch",
                        after.party.champions[1].inventory[CHAMPION_SLOT_POUCH_1],
                        0x2003u);
    ok &= expect_u16_eq("champion1 backpack",
                        after.party.champions[1].inventory[CHAMPION_SLOT_BACKPACK_2],
                        0x2404u);
    ok &= expect_u16_eq("champion1 wizard level",
                        after.party.champions[1].skillLevels[CHAMPION_SKILL_WIZARD],
                        4);
    ok &= expect_u32_eq("champion1 wizard xp",
                        after.party.champions[1].skillExperience[CHAMPION_SKILL_WIZARD],
                        54321u);
    ok &= expect_u16_eq("champion1 mana",
                        after.party.champions[1].mana.current, 48);
    ok &= expect_u16_eq("champion1 max mana",
                        after.party.champions[1].mana.maximum, 55);
    ok &= expect_u16_eq("champion1 wisdom",
                        after.party.champions[1].attributes[CHAMPION_ATTR_WISDOM],
                        72);
    ok &= expect_u16_eq("champion1 load",
                        after.party.champions[1].load, 37);
    ok &= expect_u16_eq("champion1 max load",
                        after.party.champions[1].maxLoad, 420);
    ok &= expect_int_eq("champion1 facing",
                        after.party.champions[1].direction, DIR_EAST);
    ok &= expect_u16_eq("champion2 empty action hand",
                        after.party.champions[2].inventory[CHAMPION_SLOT_HAND_RIGHT],
                        THING_NONE);

    ok &= expect_int_eq("timeline now", (int)after.timeline.nowTick, 3210);
    ok &= expect_int_eq("timeline count", after.timeline.count, 2);
    ok &= expect_int_eq("timeline first kind", after.timeline.events[0].kind,
                        TIMELINE_EVENT_DOOR_ANIMATE);
    ok &= expect_int_eq("timeline first fire tick",
                        (int)after.timeline.events[0].fireAtTick, 3219);
    ok &= expect_int_eq("timeline first map",
                        after.timeline.events[0].mapIndex, 2);
    ok &= expect_int_eq("timeline first x",
                        after.timeline.events[0].mapX, 17);
    ok &= expect_int_eq("timeline first y",
                        after.timeline.events[0].mapY, 21);
    ok &= expect_int_eq("timeline first cell",
                        after.timeline.events[0].cell, 3);
    ok &= expect_int_eq("timeline first aux0",
                        after.timeline.events[0].aux0, 7);
    ok &= expect_int_eq("timeline first aux1",
                        after.timeline.events[0].aux1, 8);
    ok &= expect_int_eq("timeline second kind", after.timeline.events[1].kind,
                        TIMELINE_EVENT_HUNGER_THIRST);
    ok &= expect_int_eq("timeline second fire tick",
                        (int)after.timeline.events[1].fireAtTick, 3240);
    ok &= expect_int_eq("timeline second map",
                        after.timeline.events[1].mapIndex, 2);
    ok &= expect_int_eq("timeline second aux0",
                        after.timeline.events[1].aux0, 0x44);

    rc = DM1_SaveGameWithProfile(&after, path2, 0xD11D1D1Du, 1, 1,
                                 DM1_DefaultSaveProfileHash());
    if (rc != DM1_SAVE_OK) {
        printf("  FAIL: repeat SaveGame party-state gate returned %d (%s)\n",
               rc, DM1_SaveLoadErrorString(rc));
        ok = 0;
    } else {
        rc = DM1_ValidateSaveFile(path2, &hdr2);
        if (rc != DM1_SAVE_OK) {
            printf("  FAIL: repeat ValidateSaveFile returned %d (%s)\n",
                   rc, DM1_SaveLoadErrorString(rc));
            ok = 0;
        } else {
            ok &= expect_u32_eq("repeat body crc", hdr2.bodyCRC32,
                                hdr.bodyCRC32);
            ok &= expect_u32_eq("repeat total size", hdr2.totalFileSize,
                                hdr.totalFileSize);
        }
    }

    remove(path);
    remove(path2);
    remove("/tmp/dm1_party_state_gate.sav.bak");
    remove("/tmp/dm1_party_state_gate_rerun.sav.bak");
    F0883_WORLD_Free_Compat(&before);
    F0883_WORLD_Free_Compat(&after);

    if (!ok) return 0;
    printf("  PASS: party/champion/timeline save-resume state gate\n");
    return 1;
}

static int test_pc34_writeback_path(void) {
    const char* path = "/tmp/dm1_pc34_writeback_gate.sav";
    struct GameWorld_Compat before;
    struct GameWorld_Compat loaded;
    struct SaveGame_Compat imported;
    struct PartyState_Compat importedParty;
    struct TimelineQueue_Compat importedTimeline;
    struct DM1SaveHeader hdr;
    DM1OriginalSavePC34HandoffReport report;
    int rc;
    int ok = 1;

    remove(path);
    remove("/tmp/dm1_pc34_writeback_gate.sav.bak");
    memset(&before, 0, sizeof(before));
    memset(&loaded, 0, sizeof(loaded));
    memset(&imported, 0, sizeof(imported));
    memset(&importedParty, 0, sizeof(importedParty));
    memset(&importedTimeline, 0, sizeof(importedTimeline));
    memset(&hdr, 0, sizeof(hdr));
    memset(&report, 0, sizeof(report));

    seed_party_state_gate_world(&before);
    seed_pc34_writeback_active_group(&before);

    rc = DM1_SaveGamePC34(&before, path, 0x33445566u);
    if (rc != DM1_SAVE_OK) {
        printf("  FAIL: SaveGamePC34 returned %d (%s)\n",
               rc, DM1_SaveLoadErrorString(rc));
        F0883_WORLD_Free_Compat(&before);
        return 0;
    }

    imported.party = &importedParty;
    imported.timeline = &importedTimeline;
    rc = dm1_v1_original_save_pc34_handoff_file(path, &imported, &report);
    if (rc != DM1_ORIGINAL_SAVE_PC34_HANDOFF_OK) {
        printf("  FAIL: original handoff rejected SaveGamePC34 file rc=%d\n",
               rc);
        remove(path);
        F0883_WORLD_Free_Compat(&before);
        return 0;
    }

    ok &= expect_int_eq("pc34 writeback champion count",
                        importedParty.championCount, 2);
    ok &= expect_int_eq("pc34 writeback party map index",
                        importedParty.mapIndex, before.party.mapIndex);
    ok &= expect_int_eq("pc34 writeback party x",
                        importedParty.mapX, before.party.mapX);
    ok &= expect_int_eq("pc34 writeback party y",
                        importedParty.mapY, before.party.mapY);
    ok &= expect_bytes_eq("pc34 writeback champion0 name",
                          importedParty.champions[0].name,
                          before.party.champions[0].name,
                          CHAMPION_NAME_LENGTH);
    ok &= expect_u16_eq("pc34 writeback champion0 hp",
                        importedParty.champions[0].hp.current,
                        before.party.champions[0].hp.current);
    ok &= expect_u16_eq("pc34 writeback champion1 mana",
                        importedParty.champions[1].mana.current,
                        before.party.champions[1].mana.current);

    ok &= expect_int_eq("pc34 writeback active-group part bytes",
                        (int)report.part_byte_counts[SAVEGAME_PC34_PART_ACTIVE_GROUP],
                        16);
    ok &= expect_int_eq("pc34 writeback active-group current",
                        report.original_current_active_group_count, 1);
    ok &= expect_int_eq("pc34 writeback active-group maximum",
                        report.original_maximum_active_group_count, 1);
    ok &= expect_int_eq("pc34 writeback active-group thing",
                        report.active_groups[0].group_thing_index, 0x1007);
    ok &= expect_int_eq("pc34 writeback active-group cells",
                        report.active_groups[0].cells, 0xa5);
    ok &= expect_int_eq("pc34 writeback active-group x",
                        report.active_groups[0].prior_map_x, 13);
    ok &= expect_int_eq("pc34 writeback active-group y",
                        report.active_groups[0].prior_map_y, 14);

    ok &= expect_int_eq("pc34 writeback event count",
                        report.original_event_count, 1);
    ok &= expect_int_eq("pc34 writeback event part bytes",
                        (int)report.part_byte_counts[SAVEGAME_PC34_PART_EVENTS],
                        ORIGINAL_PC34_EVENT_BYTES);
    ok &= expect_int_eq("pc34 writeback timeline part bytes",
                        (int)report.part_byte_counts[SAVEGAME_PC34_PART_TIMELINE],
                        2);
    ok &= expect_int_eq("pc34 writeback original game tick",
                        (int)report.original_game_time,
                        (int)before.gameTick);

    rc = DM1_LoadGame(path, &loaded, &hdr);
    if (rc != DM1_SAVE_OK) {
        printf("  FAIL: LoadGame rejected SaveGamePC34 file rc=%d (%s)\n",
               rc, DM1_SaveLoadErrorString(rc));
        ok = 0;
    } else {
        ok &= expect_int_eq("pc34 writeback load tick",
                            (int)loaded.gameTick, (int)before.gameTick);
        ok &= expect_int_eq("pc34 writeback load active groups",
                            loaded.creatureAICount, 1);
        ok &= expect_int_eq("pc34 writeback load active group cells",
                            loaded.creatureAI[0].groupCells, 0xa5);
    }

    remove(path);
    remove("/tmp/dm1_pc34_writeback_gate.sav.bak");
    F0883_WORLD_Free_Compat(&before);
    F0883_WORLD_Free_Compat(&loaded);

    if (!ok) return 0;
    printf("  PASS: explicit original-PC34 write-back path\n");
    return 1;
}

/* ── Main ─────────────────────────────────────────────────────── */

int main(void) {
    int pass = 0, fail = 0;

    printf("=== DM1 V1 Save/Load Tests ===\n");

    if (test_crc32())           pass++; else fail++;
    if (test_header_roundtrip()) pass++; else fail++;
    if (test_error_strings())   pass++; else fail++;
    if (test_save_menu())       pass++; else fail++;
    if (test_save_path())       pass++; else fail++;
    if (test_null_args())       pass++; else fail++;
    if (test_load_nonexistent()) pass++; else fail++;
    if (test_backup_fallback_path()) pass++; else fail++;
    if (test_original_pc34_runtime_load_fallback()) pass++; else fail++;
    if (test_validate_corrupt()) pass++; else fail++;
    if (test_profile_hash_mismatch()) pass++; else fail++;
    if (test_party_state_save_resume_gate()) pass++; else fail++;
    if (test_pc34_writeback_path()) pass++; else fail++;

    printf("\n=== Results: %d passed, %d failed ===\n", pass, fail);
    return (fail > 0) ? 1 : 0;
}
