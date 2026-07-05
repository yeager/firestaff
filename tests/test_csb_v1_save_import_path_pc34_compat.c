/*
 * test_csb_v1_save_import_path_pc34_compat.c
 *
 * CSB V1 Champion Transfer/Import (Champions GAP 3, HoC delta).
 * Source-locked per ReDMCSB CHARACTER.C / CHAMPION.C
 * ReadingChampion()/WritingChampion(), DEFS.H:1289,
 * CEDT006.C:101-118, Character.cpp:14.
 *
 * Covers:
 *   - detect each variant (DM1 / CSB v2.0 / CSB v2.1) from a
 *     synthetic 256-byte header
 *   - round-trip a 4-champion party (build -> import)
 *   - reincarnation penalty applies on import
 *   - version mismatch + bad magic + truncation rejected
 *   - import stamp prevents re-import semantics
 */
#include "csb_v1_save_import_path_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "csbwin_resume_fixture.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

static void make_champion(CSB_V1_Champion* c, const char* name,
                          int hp, int dead) {
    int i;
    csb_v1_champion_init(c);
    memset(c->Name, 0, sizeof(c->Name));
    strncpy(c->Name, name, CSB_V1_MAX_NAME_LEN);
    c->CurrentHealth = (int16_t)(dead ? 0 : hp);
    c->MaximumHealth = (int16_t)(dead ? 0 : hp);
    c->CurrentStamina = 80; c->MaximumStamina = 80;
    c->CurrentMana = 40;    c->MaximumMana = 40;
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        c->Statistics[i][CSB_V1_STAT_MIN] = 30;
        c->Statistics[i][CSB_V1_STAT_CUR] = 80;
        c->Statistics[i][CSB_V1_STAT_MAX] = 80;
    }
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) c->Skills[i] = (uint8_t)i;
    for (i = 0; i < CSB_V1_SLOT_COUNT; ++i) c->Slots[i] = 0xFFFF;
    if (dead) c->Attributes |= CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
}

int main(void) {
    printf("=== CSB V1 Save Import (Champions GAP 3) ===\n");

    CHECK(csb_v1_save_import_path_implemented() == 1,
          "Champions GAP 3 now implemented (returns 1)");

    /* ── Detect each variant from a 256-byte header ── */
    {
        unsigned char hdr[CSB_SAVE_HEADER_SIZE];
        memset(hdr, 0, sizeof(hdr));
        memcpy(hdr, "RDMCSB15", 8);
        CHECK(csb_v1_detect_save_variant(hdr, (int)sizeof(hdr))
                  == CSB_V1_SAVE_VARIANT_DM1_PC34,
              "256-byte DM1 header detected");
    }
    {
        unsigned char hdr[CSB_SAVE_HEADER_SIZE];
        memset(hdr, 0, sizeof(hdr));
        memcpy(hdr, "CSBGAME\0", 8);
        hdr[CSB_SAVE_HDR_OFF_VERSION] = 0x00;
        hdr[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x02; /* 0x200 */
        CHECK(csb_v1_detect_save_variant(hdr, (int)sizeof(hdr))
                  == CSB_V1_SAVE_VARIANT_CSB_V20,
              "256-byte CSB v2.0 header detected");
    }
    {
        unsigned char hdr[CSB_SAVE_HEADER_SIZE];
        memset(hdr, 0, sizeof(hdr));
        memcpy(hdr, "CSBGAME\0", 8);
        hdr[CSB_SAVE_HDR_OFF_VERSION] = 0x01;
        hdr[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x02; /* 0x201 */
        CHECK(csb_v1_detect_save_variant(hdr, (int)sizeof(hdr))
                  == CSB_V1_SAVE_VARIANT_CSB_V21,
              "256-byte CSB v2.1 header detected");
    }
    {
        unsigned char hdr[CSB_SAVE_HEADER_SIZE];
        memset(hdr, 0, sizeof(hdr));
        memcpy(hdr, "CSBGAME\0", 8);
        hdr[CSB_SAVE_HDR_OFF_VERSION] = 0x99; /* bogus version */
        CHECK(csb_v1_detect_save_variant(hdr, (int)sizeof(hdr))
                  == CSB_V1_SAVE_VARIANT_UNKNOWN,
              "unknown CSB version -> UNKNOWN");
    }

    /* ── Round-trip a 4-champion party (build -> import) ── */
    {
        CSB_V1_PartyState src, dst;
        unsigned char buf[CSB_SAVE_HEADER_SIZE + 4 * CSB_SAVE_CHAMP_SIZE + 16];
        long blen;
        int n, i;

        csb_v1_character_init_default(&src);
        make_champion(&src.Champions[0], "ALPHA", 100, 0);
        make_champion(&src.Champions[1], "BETA",  90, 0);
        make_champion(&src.Champions[2], "GAMMA", 110, 0);
        make_champion(&src.Champions[3], "DELTA", 0, 1); /* dead */
        src.ChampionCount = 4;

        blen = csb_v1_build_csb_save_buffer(&src, CSB_SAVE_VERSION_V20,
                                            buf, (long)sizeof(buf));
        CHECK(blen > 0, "build 4-champion CSB v2.0 save buffer");
        CHECK(csb_v1_detect_save_variant(buf, (int)blen)
                  == CSB_V1_SAVE_VARIANT_CSB_V20,
              "built buffer detects as CSB v2.0");

        n = csb_v1_import_csb_save_buffer(&dst, buf, blen);
        CHECK(n == 4, "import returns 4 champions");
        CHECK(dst.ChampionCount == 4, "ChampionCount is 4");
        CHECK(dst.ImportSource == CSB_SAVE_IMPORT_SOURCE,
              "party stamped with CSB import source");
        CHECK(dst.Reserved[0] == 0x20, "variant stamp recorded (v2.0)");
        CHECK(dst.ImportedFromDM1 == 0, "not flagged as DM1 import");

        CHECK(strncmp(dst.Champions[0].Name, "ALPHA", 5) == 0,
              "champion 0 name round-trips (ALPHA)");
        CHECK(dst.Champions[2].MaximumHealth == 110,
              "champion 2 max HP round-trips (110)");
        CHECK(dst.Champions[0].Skills[5] == 5,
              "champion 0 skills round-trip");
        CHECK(dst.Champions[3].Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD,
              "champion 3 imported dead");
        CHECK(dst.LeaderIndex == 0,
              "leader is first living champion (index 0)");
        for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
            CHECK(dst.Champions[1].Statistics[i][CSB_V1_STAT_CUR] == 80,
                  "champion 1 non-reincarnated stat preserved (80)");
            break; /* spot-check one to keep output short */
        }
    }

    /* ── Reincarnation penalty applies on import ── */
    {
        CSB_V1_PartyState src, dst;
        unsigned char buf[CSB_SAVE_HEADER_SIZE + CSB_SAVE_CHAMP_SIZE + 16];
        long blen;
        int n;

        csb_v1_character_init_default(&src);
        make_champion(&src.Champions[0], "PHOENIX", 100, 0);
        src.ChampionCount = 1;

        blen = csb_v1_build_csb_save_buffer(&src, CSB_SAVE_VERSION_V21,
                                            buf, (long)sizeof(buf));
        CHECK(blen > 0, "build 1-champion CSB v2.1 save buffer");

        /* Flip the reincarnated flag in the record. */
        buf[CSB_SAVE_HEADER_SIZE + CSB_SAVE_CH_OFF_REINCARNATED] = 1;

        n = csb_v1_import_csb_save_buffer(&dst, buf, blen);
        CHECK(n == 1, "reincarnated import returns 1 champion");
        CHECK(dst.Reserved[0] == 0x21, "variant stamp recorded (v2.1)");
        /* HP halved (100 -> 50). */
        CHECK(dst.Champions[0].MaximumHealth == 50,
              "reincarnation halves max HP (100 -> 50)");
        /* Stat 80 reduced by 1/8 (10) -> 70. */
        CHECK(dst.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] == 70,
              "reincarnation reduces STR by 1/8 (80 -> 70)");
        /* Luck (index 6) exempt: still 80. */
        CHECK(dst.Champions[0].Statistics[CSB_V1_STAT_LUCK][CSB_V1_STAT_CUR] == 80,
              "reincarnation leaves Luck untouched");
        CHECK(dst.Champions[0].Attributes & CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME,
              "reincarnated champion flagged NEEDS_RENAME");
    }

    /* ── Version mismatch / bad magic / truncation rejected ── */
    {
        CSB_V1_PartyState dst;
        unsigned char buf[CSB_SAVE_HEADER_SIZE + CSB_SAVE_CHAMP_SIZE];
        memset(buf, 0, sizeof(buf));
        memcpy(buf, "CSBGAME\0", 8);
        buf[CSB_SAVE_HDR_OFF_VERSION] = 0x55; /* unsupported version */
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = 1;
        CHECK(csb_v1_import_csb_save_buffer(&dst, buf, (long)sizeof(buf))
                  == CSB_SAVE_IMPORT_ERR_VERSION,
              "unsupported version -> ERR_VERSION");
    }
    {
        CSB_V1_PartyState dst;
        unsigned char buf[CSB_SAVE_HEADER_SIZE + CSB_SAVE_CHAMP_SIZE];
        memset(buf, 0, sizeof(buf));
        memcpy(buf, "DMSAVE\0\0", 8); /* wrong magic */
        CHECK(csb_v1_import_csb_save_buffer(&dst, buf, (long)sizeof(buf))
                  == CSB_SAVE_IMPORT_ERR_BAD_MAGIC,
              "non-CSB magic -> ERR_BAD_MAGIC");
    }
    {
        CSB_V1_PartyState dst;
        unsigned char buf[CSB_SAVE_HEADER_SIZE]; /* header only, no records */
        memset(buf, 0, sizeof(buf));
        memcpy(buf, "CSBGAME\0", 8);
        buf[CSB_SAVE_HDR_OFF_VERSION] = 0x00;
        buf[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x02;
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = 4; /* claims 4, no data */
        CHECK(csb_v1_import_csb_save_buffer(&dst, buf, (long)sizeof(buf))
                  == CSB_SAVE_IMPORT_ERR_TRUNCATED,
              "claimed champions but no records -> ERR_TRUNCATED");
    }
    {
        CSB_V1_PartyState dst;
        unsigned char buf[CSB_SAVE_HEADER_SIZE + CSB_SAVE_CHAMP_SIZE];
        memset(buf, 0, sizeof(buf));
        memcpy(buf, "CSBGAME\0", 8);
        buf[CSB_SAVE_HDR_OFF_VERSION] = 0x00;
        buf[CSB_SAVE_HDR_OFF_VERSION + 1] = 0x02;
        buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = 0; /* zero champions */
        CHECK(csb_v1_import_csb_save_buffer(&dst, buf, (long)sizeof(buf))
                  == CSB_SAVE_IMPORT_ERR_NO_CHAMPIONS,
              "zero champions -> ERR_NO_CHAMPIONS");
    }

    /* ── File path round-trip ── */
    {
        CSB_V1_PartyState src, dst;
        unsigned char buf[CSB_SAVE_HEADER_SIZE + 2 * CSB_SAVE_CHAMP_SIZE + 16];
        long blen;
        const char* path = "firestaff-csb-save-import.csb";
        FILE* f;
        int n;

        csb_v1_character_init_default(&src);
        make_champion(&src.Champions[0], "DISKA", 75, 0);
        make_champion(&src.Champions[1], "DISKB", 85, 0);
        src.ChampionCount = 2;
        blen = csb_v1_build_csb_save_buffer(&src, CSB_SAVE_VERSION_V20,
                                            buf, (long)sizeof(buf));
        f = fopen(path, "wb");
        CHECK(f != NULL, "temp CSB save file created");
        if (f) { fwrite(buf, 1, (size_t)blen, f); fclose(f); }

        n = csb_v1_import_csb_save_file(&dst, path);
        CHECK(n == 2, "file import returns 2 champions");
        CHECK(strncmp(dst.Champions[1].Name, "DISKB", 5) == 0,
              "file import maps second champion name");
        remove(path);

        /* Nonexistent path -> IO error (legacy entry point). */
        CHECK(csb_v1_import_csb_save("firestaff-no-such.csb")
                  == CSB_SAVE_IMPORT_ERR_IO,
              "nonexistent path -> ERR_IO");
    }

    /* ── Runtime handoff from raw CSBGAME roster ── */
    {
        CSB_V1_PartyState src;
        CSB_V1_RuntimeProfile runtime;
        unsigned char buf[CSB_SAVE_HEADER_SIZE + 3 * CSB_SAVE_CHAMP_SIZE + 16];
        long blen;
        const char* path = "firestaff-csb-runtime-import.csb";
        FILE* f;

        csb_v1_character_init_default(&src);
        make_champion(&src.Champions[0], "RUNEA", 70, 0);
        make_champion(&src.Champions[1], "RUNEB", 80, 0);
        make_champion(&src.Champions[2], "RUNEC", 90, 0);
        src.ChampionCount = 3;

        blen = csb_v1_build_csb_save_buffer(&src, CSB_SAVE_VERSION_V21,
                                            buf, (long)sizeof(buf));
        CHECK(blen > 0, "build 3-champion CSBGAME runtime import buffer");
        f = fopen(path, "wb");
        CHECK(f != NULL, "temp CSBGAME runtime import file created");
        if (f) { fwrite(buf, 1, (size_t)blen, f); fclose(f); }

        csb_v1_runtime_init(&runtime, NULL);
        runtime.party_x = 11;
        runtime.party_y = 7;
        runtime.party_z = 0;
        runtime.party_dir = 2;
        runtime.current_level = 4;
        csb_v1_dungeon_set_current_level(1);
        runtime.game_time = 1234;
        CHECK(csb_v1_runtime_import_csbgame_roster_from_path(&runtime, path)
                  == CSB_V1_LOAD_OK,
              "runtime imports raw CSBGAME roster file");
        CHECK(runtime.champion_count == 3,
              "runtime CSBGAME import sets champion count");
        CHECK(strncmp(runtime.party_state.Champions[2].Name, "RUNEC", 5) == 0,
              "runtime CSBGAME import maps champion names");
        CHECK(runtime.party_x == 11 && runtime.party_y == 7 &&
              runtime.party_dir == 2 && runtime.current_level == 4,
              "runtime CSBGAME import preserves booted dungeon pose");
        CHECK(csb_v1_dungeon_get_current_level() == 4,
              "runtime CSBGAME import keeps dungeon singleton on booted level");
        CHECK(runtime.party_state.PartyMapX == 11 &&
              runtime.party_state.PartyMapY == 7 &&
              runtime.party_state.PartyDirection == 2,
              "runtime CSBGAME import reanchors party snapshot to runtime pose");
        CHECK(runtime.difficulty == CSB_V1_DIFFICULTY_HARD,
              "runtime CSBGAME import recalculates difficulty for 3 champions");

        csb_v1_runtime_init(&runtime, NULL);
        runtime.party_x = 6;
        runtime.party_y = 6;
        runtime.party_dir = 1;
        CHECK(csb_v1_runtime_load_game_from_path(&runtime, path)
                  == CSB_V1_LOAD_OK,
              "runtime load falls back to raw CSBGAME roster import");
        CHECK(runtime.champion_count == 3,
              "runtime load fallback sets champion count");
        CHECK(runtime.party_x == 6 && runtime.party_y == 6 &&
              runtime.party_dir == 1,
              "runtime load fallback preserves booted pose");
        remove(path);
    }

    /* ── Runtime handoff from verified CSBWin 512-byte save ── */
    {
        CSB_V1_RuntimeProfile runtime;
        const char* path = "firestaff-csbwin-runtime-import.csb";

        CHECK(firestaff_test_write_csbwin_resume_fixture(path, 0),
              "write verified CSBWin runtime import fixture");
        csb_v1_runtime_init(&runtime, NULL);
        runtime.party_x = 1;
        runtime.party_y = 1;
        runtime.party_dir = 0;
        runtime.current_level = 0;
        CHECK(csb_v1_runtime_load_game_from_path(&runtime, path)
                  == CSB_V1_LOAD_OK,
              "runtime load accepts verified CSBWin 512-byte save body");
        CHECK(runtime.game_time == 0x01020304u,
              "runtime CSBWin load imports GAMEBLOCK2 game time");
        CHECK(runtime.party_x == 12 && runtime.party_y == 7 &&
              runtime.party_dir == 3 && runtime.current_level == 4,
              "runtime CSBWin load imports GAMEBLOCK2 party pose");
        CHECK(runtime.party_state_valid == 1 &&
              runtime.party_state.ChampionCount == 2 &&
              strcmp(runtime.party_state.Champions[0].Name, "TIGGY") == 0 &&
              strcmp(runtime.party_state.Champions[1].Name, "BORIS") == 0,
              "runtime CSBWin load imports champion summaries");
        CHECK(runtime.party_state.LeaderHandThing == 0x4321u,
              "runtime CSBWin load imports object-in-hand");
        CHECK(runtime.csbwin_runtime_item16_count == 2u,
              "runtime CSBWin load materializes ITEM16 summaries");
        CHECK(runtime.timeline_queue.eventCount == 3,
              "runtime CSBWin load materializes timer queue");
        remove(path);
    }

    /* NULL safety. */
    {
        unsigned char buf[CSB_SAVE_HEADER_SIZE];
        memset(buf, 0, sizeof(buf));
        CHECK(csb_v1_import_csb_save_buffer(NULL, buf, sizeof(buf))
                  == CSB_SAVE_IMPORT_ERR_NULL, "NULL party -> ERR_NULL");
        CHECK(csb_v1_import_csb_save_buffer((CSB_V1_PartyState*)buf, NULL, 0)
                  == CSB_SAVE_IMPORT_ERR_NULL, "NULL buf -> ERR_NULL");
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
