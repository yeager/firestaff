#include "firestaff_game_data_fingerprint.h"
#include <stdio.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

static void test_classify_known_hashes(void) {
    FirestaffGameDataClassifyResult r;

    r = firestaff_game_data_classify_hex("FA6B1AA29E191418713BF2CDA93D962E");
    ASSERT(r.valid, "DM PC 3.4 EN found");
    ASSERT(r.entry->game == FIRESTAFF_GAME_DM1, "game is DM1");
    ASSERT(r.entry->platform == FIRESTAFF_PLATFORM_PC, "platform is PC");
    ASSERT(r.entry->language == FIRESTAFF_LANG_ENGLISH, "lang is EN");
    ASSERT(r.entry->file_type == FIRESTAFF_FILE_GRAPHICS_DAT, "file is GRAPHICS.DAT");
    ASSERT(r.entry->version && strcmp(r.entry->version, "3.4") == 0, "version 3.4");

    r = firestaff_game_data_classify_hex("fa6b1aa29e191418713bf2cda93d962e");
    ASSERT(r.valid, "lowercase hex works");

    r = firestaff_game_data_classify_hex("405B757038EEA3C263E60F240854D6DE");
    ASSERT(r.valid, "CSB FM-Towns EN found");
    ASSERT(r.entry->game == FIRESTAFF_GAME_CSB, "game is CSB");
    ASSERT(r.entry->platform == FIRESTAFF_PLATFORM_FM_TOWNS, "platform is FM Towns");

    r = firestaff_game_data_classify_hex("25247EDE4DABB6A71E5DABDFBCD5907D");
    ASSERT(r.valid, "DMII PC EN found");
    ASSERT(r.entry->game == FIRESTAFF_GAME_DM2, "game is DM2");

    r = firestaff_game_data_classify_hex("C20E5B8F756E360A631595CC9260F62D");
    ASSERT(r.valid, "DM PC SONG.DAT found");
    ASSERT(r.entry->file_type == FIRESTAFF_FILE_SONG_DAT, "file is SONG.DAT");

    r = firestaff_game_data_classify_hex("DBCED13A38D3036F42B9797175B7EC88");
    ASSERT(r.valid, "DMII Sega CD EN found");
    ASSERT(r.entry->platform == FIRESTAFF_PLATFORM_SEGA_CD, "platform is Sega CD");

    r = firestaff_game_data_classify_hex("BD2D316EB77C6D6D217BFB76BD0D7E41");
    ASSERT(r.valid, "DMII PC Demo 19950112 found");
    ASSERT(r.entry->game == FIRESTAFF_GAME_DM2, "DMII Demo game is DM2");

    r = firestaff_game_data_classify_hex("61F51D7FFBE0A8CCD6A49C2FEC3295FC");
    ASSERT(r.valid, "CSB PC-98 ENTER.SNG found");

    r = firestaff_game_data_classify_hex("CEFADDFDF5651DF2C91F61B5611A8362");
    ASSERT(r.valid, "CSB Amiga 3.5 ML found");
    ASSERT(r.entry->language == FIRESTAFF_LANG_MULTILANGUAGE, "CSB 3.5 ML lang");

    r = firestaff_game_data_classify_hex("4174D6DE5384323072B185640ED31723");
    ASSERT(r.valid && r.entry->game == FIRESTAFF_GAME_CSB &&
           r.entry->file_type == FIRESTAFF_FILE_ANIMATE_SCR,
           "CSB Atari ANIMATE.SCR source hash found");

    r = firestaff_game_data_classify_hex("67007E7943F9EF6F0B12FF4BD1BEF3D1");
    ASSERT(r.valid && r.entry->file_type == FIRESTAFF_FILE_HINT_FTL,
           "CSB Atari Hint Oracle HINT.FTL source hash found");

    r = firestaff_game_data_classify_hex("B1FC60F2C0D8F8A89E5D4F295E93AE42");
    ASSERT(r.valid && r.entry->file_type == FIRESTAFF_FILE_SWITCH_DAT,
           "CSB Atari Utility Disk SWITCH.DAT source hash found");

    r = firestaff_game_data_classify_hex("18ABDF771F37E8953BF95BA2F462469D");
    ASSERT(r.valid && r.entry->file_type == FIRESTAFF_FILE_CSB_FTL_MODULE,
           "CSB Atari FTLCODE runtime module source hash found");

    r = firestaff_game_data_classify_hex("531EA104A2FBC2011EA73D11F274C57D");
    ASSERT(r.valid && r.entry->file_type == FIRESTAFF_FILE_MINI_DAT,
           "CSB Atari campaign MINI.DAT source hash found");
}

static void test_classify_unknown(void) {
    FirestaffGameDataClassifyResult r;

    r = firestaff_game_data_classify_hex("00000000000000000000000000000000");
    ASSERT(!r.valid, "unknown hash not found");

    r = firestaff_game_data_classify_hex(NULL);
    ASSERT(!r.valid, "NULL hex not found");

    r = firestaff_game_data_classify_md5(NULL);
    ASSERT(!r.valid, "NULL md5 not found");
}

static void test_name_functions(void) {
    ASSERT(strcmp(firestaff_game_name(FIRESTAFF_GAME_DM1), "Dungeon Master") == 0, "DM1 name");
    ASSERT(strcmp(firestaff_game_name(FIRESTAFF_GAME_CSB), "Chaos Strikes Back") == 0, "CSB name");
    ASSERT(strcmp(firestaff_game_name(FIRESTAFF_GAME_DM2), "Dungeon Master II") == 0, "DM2 name");
    ASSERT(strcmp(firestaff_platform_name(FIRESTAFF_PLATFORM_FM_TOWNS), "FM Towns") == 0, "FMT name");
    ASSERT(strcmp(firestaff_language_name(FIRESTAFF_LANG_JAPANESE), "Japanese") == 0, "JP name");
}

static void test_table_integrity(void) {
    int i;
    for (i = 0; i < FIRESTAFF_FINGERPRINT_COUNT; i++) {
        const FirestaffGameDataFingerprint *e = &firestaff_fingerprint_table[i];
        ASSERT(e->game != FIRESTAFF_GAME_UNKNOWN, "entry has game");
        ASSERT(e->platform != FIRESTAFF_PLATFORM_UNKNOWN, "entry has platform");
        ASSERT(e->description != NULL && strlen(e->description) > 0, "entry has description");
    }
}

static void test_binary_classify(void) {
    uint8_t md5[16] = {0xFA,0x6B,0x1A,0xA2,0x9E,0x19,0x14,0x18,
                       0x71,0x3B,0xF2,0xCD,0xA9,0x3D,0x96,0x2E};
    FirestaffGameDataClassifyResult r = firestaff_game_data_classify_md5(md5);
    ASSERT(r.valid, "binary MD5 lookup works");
    ASSERT(r.entry->game == FIRESTAFF_GAME_DM1, "binary lookup game correct");
}

int main(void) {
    test_classify_known_hashes();
    test_classify_unknown();
    test_name_functions();
    test_table_integrity();
    test_binary_classify();
    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
