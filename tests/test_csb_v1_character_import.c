/*
 * CTest gate for CSB V1 Phase 6 — utility disk / champion import flow.
 *
 * Tests:
 *   1. csb_v1_character_init_default — party starts empty
 *   2. 1-champion DM1 save buffer round-trips with correct vital/stats
 *   3. Dead champion (health=0) gets DEAD attribute
 *   4. Stats below 30 are clamped to min=30
 *   5. csb_v1_character_import_dm1_save — 0-champ file rejected
 *   6. csb_v1_character_import_dm1_save — non-existent path returns -1
 *   7. csb_v1_character_import_dm1_save — 4-champ synthetic save round-trips
 *   8. csb_v1_character_import_dm1_buffer — same as above, in-memory
 *   9. utility disk check on non-existent path returns -1
 *  10. source_evidence() cites CEDTINC7.C, CEDTDATA.C, CHAMPION.C
 *
 * Source references:
 *   ReDMCSB CEDTINC7.C  — utility disk prompt flow
 *   ReDMCSB CEDTDATA.C — G3921 PLEASE_INSERT_UTILITY_DISK
 *   ReDMCSB CHAMPION.C — F0284-F0330 champion core
 *   CSBWin/SaveGame.cpp — DM1 import (2953 lines)
 *   CSBWin/Character.cpp — champion management (5528 lines)
 */

#include "csb_v1_character_pc34_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, label, fmt) do { \
    if ((got) == (want)) { \
        passed++; printf("  PASS: %s == %" fmt "\n", label, (want)); \
    } else { \
        failed++; printf("  FAIL: %s got=%" fmt " want=%" fmt "\n", label, (got), (want)); \
    } \
} while (0)

/* ── Test 1: default party ───────────────────────────────────────── */
static void test_party_defaults(void)
{
    CSB_V1_PartyState party;
    int i;

    csb_v1_character_init_default(&party);

    CHECK(party.ChampionCount == 0, "party starts with 0 champions");
    CHECK(party.ImportedFromDM1 == 0, "ImportedFromDM1 starts 0");
    CHECK(party.ImportSource == 0, "ImportSource starts 0");
    CHECK(party.PartyDirection == 0, "PartyDirection starts 0");
    CHECK(party.LeaderIndex == -1, "LeaderIndex starts -1");

    for (i = 0; i < CSB_V1_MAX_CHAMPIONS; i++) {
        CHECK(party.Champions[i].CurrentHealth == 0,
              "champion slot i starts with 0 health");
    }
}

/* ── Test 2–4: DM1 record parsing via import_buffer ─────────────── */
static void test_import_one_living_champion(void)
{
    uint8_t buf[1024];
    CSB_V1_PartyState party;
    CSB_V1_Champion *c;
    int i;

    memset(buf, 0, sizeof(buf));
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 1;

    /* Champion record: name ALPHA */
    memcpy((char *)buf + CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_NAME,
           "ALPHA   ", 8);
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_HEALTH]     = 80;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH] = 100;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STAMINA]    = 60;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA] = 100;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MANA]       = 30;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_MANA]  = 50;
    /* Stats at offsets 20-23 (before SKILLS at 24) */
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STR] = 55;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_DEX] = 66;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_WIS] = 77;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_VIT] = 88;
    /* Skills all 0 (already 0 from memset) */
    /* Equipment slots: 30 × 2 bytes LE = 0xFFFF (empty thing) */
    {
        int s;
        size_t equip_off = CSB_V1_DM1_HDR_CHAMPION_START
                         + CSB_V1_DM1_CHAMP_OFF_EQUIP;
        for (s = 0; s < CSB_V1_SLOT_COUNT; s++) {
            buf[equip_off + s * 2]     = 0xFF;
            buf[equip_off + s * 2 + 1] = 0xFF;
        }
    }

    csb_v1_character_init_default(&party);
    int n = csb_v1_character_import_dm1_buffer(&party, buf, (int)sizeof(buf));

    CHECK_EQ(n, 1, "1-champ import returns 1", "d");
    CHECK_EQ(party.ChampionCount, 1, "ChampionCount is 1", "d");

    c = &party.Champions[0];

    /* Name */
    CHECK(strncmp(c->Name, "ALPHA", 5) == 0, "name is ALPHA");
    CHECK(c->Name[5] == '\0', "name is null-terminated");

    /* Vitals */
    CHECK_EQ(c->CurrentHealth,   80, "CurrentHealth 80", "d");
    CHECK_EQ(c->MaximumHealth, 100, "MaximumHealth 100", "d");
    CHECK_EQ(c->CurrentStamina,  60, "CurrentStamina 60", "d");
    CHECK_EQ(c->MaximumStamina, 100, "MaximumStamina 100", "d");
    CHECK_EQ(c->CurrentMana,     30, "CurrentMana 30", "d");
    CHECK_EQ(c->MaximumMana,     50, "MaximumMana 50", "d");

    /* Stats: min=30, raw 55>30 so stays 55 */
    CHECK_EQ(c->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR], 55, "STR cur 55", "d");
    CHECK_EQ(c->Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX], 55, "STR max 55", "d");
    CHECK_EQ(c->Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR], 66, "DEX cur 66", "d");
    CHECK_EQ(c->Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_CUR], 77, "WIS cur 77", "d");
    CHECK_EQ(c->Statistics[CSB_V1_STAT_VIT][CSB_V1_STAT_CUR], 88, "VIT cur 88", "d");

    /* All stats min should be 30 */
    for (i = 0; i < CSB_V1_STAT_COUNT; i++) {
        CHECK_EQ(c->Statistics[i][CSB_V1_STAT_MIN], 30,
                 "stat min is 30", "d");
    }

    /* Skills */
    CHECK(c->Skills[0] == 0 && c->Skills[15] == 0, "skills zero-initialized");

    /* Empty slots = 0xFFFF */
    CHECK(c->Slots[0] == 0xFFFFu, "empty slot 0 is 0xFFFF");
    CHECK(c->Slots[29] == 0xFFFFu, "empty slot 29 is 0xFFFF");
}

static void test_import_dead_champion(void)
{
    uint8_t buf[1024];
    CSB_V1_PartyState party;

    memset(buf, 0, sizeof(buf));
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 1;

    memcpy((char *)buf + CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_NAME,
           "GHOST  ", 8);
    /* All vitals 0 = dead */
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_HEALTH]     = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH] = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STAMINA]    = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA] = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MANA]       = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_MANA]   = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STR] = 40;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_DEX] = 40;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_WIS] = 40;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_VIT] = 40;

    csb_v1_character_init_default(&party);
    csb_v1_character_import_dm1_buffer(&party, buf, (int)sizeof(buf));

    CHECK(party.Champions[0].Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD,
          "dead champion (health=0) has DEAD attribute");
}

static void test_import_stat_min_clamp(void)
{
    uint8_t buf[1024];
    CSB_V1_PartyState party;

    memset(buf, 0, sizeof(buf));
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 1;

    memcpy((char *)buf + CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_NAME,
           "WEAKLIN ", 8);
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_HEALTH]     = 10;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH] = 10;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STAMINA]    = 5;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA] = 5;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MANA]       = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_MAX_MANA]   = 0;
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_STR] = 10; /* below min 30 */
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_DEX] = 20; /* below min 30 */
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_WIS] = 30; /* exactly min 30 */
    buf[CSB_V1_DM1_HDR_CHAMPION_START + CSB_V1_DM1_CHAMP_OFF_VIT] = 99;

    csb_v1_character_init_default(&party);
    csb_v1_character_import_dm1_buffer(&party, buf, (int)sizeof(buf));

    CHECK_EQ(party.Champions[0].Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR],
             30, "STR clamped to min 30", "d");
    CHECK_EQ(party.Champions[0].Statistics[CSB_V1_STAT_DEX][CSB_V1_STAT_CUR],
             30, "DEX clamped to min 30", "d");
    CHECK_EQ(party.Champions[0].Statistics[CSB_V1_STAT_WIS][CSB_V1_STAT_CUR],
             30, "WIS kept at 30", "d");
}

/* ── Test 5–7: csb_v1_character_import_dm1_save ─────────────────── */
static void test_import_file(void)
{
    /* 5: zero champ count rejected */
    {
        uint8_t buf[256];
        memset(buf, 0, sizeof(buf));
        buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 0;
        const char *path = "firestaff-csb-test-empty.sav";
        FILE *f = fopen(path, "wb");
        CHECK(f != NULL, "tmp file created for zero-champ test");
        if (f) { fwrite(buf, 1, sizeof(buf), f); fclose(f); }

        CSB_V1_PartyState party;
        csb_v1_character_init_default(&party);
        int r = csb_v1_character_import_dm1_save(&party, path);
        CHECK_EQ(r, -1, "0-champ file rejected", "d");
        remove(path);
    }

    /* 6: non-existent path */
    {
        const char *path = "firestaff-csb-missing-input.sav";
        CSB_V1_PartyState party;
        remove(path);
        csb_v1_character_init_default(&party);
        int r = csb_v1_character_import_dm1_save(&party, path);
        CHECK_EQ(r, -1, "non-existent path returns -1", "d");
    }

    /* 7: 4 champions file import round-trips */
    {
        uint8_t buf[1024];
        CSB_V1_PartyState party;
        int i;

        memset(buf, 0, sizeof(buf));
        buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 4;

        /* game time at offset 10 */
        buf[10] = 0x80; buf[11] = 0x5A; buf[12] = 0; buf[13] = 0;

        for (i = 0; i < 4; i++) {
            size_t off = (size_t)CSB_V1_DM1_HDR_CHAMPION_START
                       + (size_t)i * CSB_V1_DM1_CHAMP_SIZE;
            memcpy((char *)buf + off + CSB_V1_DM1_CHAMP_OFF_NAME, "WINNER ", 8);

            if (i == 3) {
                /* Champion 4 (index 3) is dead */
                buf[off + CSB_V1_DM1_CHAMP_OFF_HEALTH]     = 0;
                buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH] = 0;
                buf[off + CSB_V1_DM1_CHAMP_OFF_STAMINA]    = 0;
                buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA] = 0;
                buf[off + CSB_V1_DM1_CHAMP_OFF_MANA]       = 0;
                buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_MANA]   = 0;
            } else {
                buf[off + CSB_V1_DM1_CHAMP_OFF_HEALTH]     = 80;
                buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH]  = 100;
                buf[off + CSB_V1_DM1_CHAMP_OFF_STAMINA]     = 60;
                buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA] = 100;
                buf[off + CSB_V1_DM1_CHAMP_OFF_MANA]        = 30;
                buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_MANA]    = 50;
            }
            buf[off + CSB_V1_DM1_CHAMP_OFF_STR] = (uint8_t)(50 + i);
            buf[off + CSB_V1_DM1_CHAMP_OFF_DEX] = (uint8_t)(60 + i);
            buf[off + CSB_V1_DM1_CHAMP_OFF_WIS] = (uint8_t)(70 + i);
            buf[off + CSB_V1_DM1_CHAMP_OFF_VIT] = (uint8_t)(80 + i);
            /* Skills all 0 */
            /* Equipment: 30 slots of 0xFFFF (empty) */
            {
                int s;
                for (s = 0; s < CSB_V1_SLOT_COUNT; s++) {
                    buf[off + CSB_V1_DM1_CHAMP_OFF_EQUIP + s * 2]     = 0xFF;
                    buf[off + CSB_V1_DM1_CHAMP_OFF_EQUIP + s * 2 + 1] = 0xFF;
                }
            }
        }

        size_t file_size = (size_t)CSB_V1_DM1_HDR_CHAMPION_START
                         + 4 * CSB_V1_DM1_CHAMP_SIZE;
        const char *path = "firestaff-csb-4champ.sav";
        FILE *f = fopen(path, "wb");
        CHECK(f != NULL, "4-champ tmp file created");
        if (f) { fwrite(buf, 1, file_size, f); fclose(f); }

        csb_v1_character_init_default(&party);
        int r = csb_v1_character_import_dm1_save(&party, path);
        CHECK_EQ(r, 4, "4-champ file returns 4", "d");
        CHECK_EQ(party.ChampionCount, 4, "ChampionCount is 4", "d");
        CHECK(party.ImportedFromDM1 == 1, "ImportedFromDM1 set after save import");
        CHECK(party.ImportSource == 2, "ImportSource is 2 (dm1_save)");
        CHECK(party.LeaderIndex == 0, "LeaderIndex is 0 (first living champion)");
        CHECK_EQ(party.Champions[0].CurrentHealth, 80, "champ 0 health 80", "d");
        CHECK(party.Champions[3].Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD,
              "champ 3 (index 3) is dead");
        CHECK(!(party.Champions[0].Attributes & CSB_V1_CHAMPION_ATTRIBUTE_DEAD),
              "champ 0 is alive");

        remove(path);
    }
}

/* ── Test 8: in-memory import ───────────────────────────────────── */
static void test_import_buffer(void)
{
    uint8_t buf[1024];
    CSB_V1_PartyState party;
    int i;

    memset(buf, 0, sizeof(buf));
    buf[CSB_V1_DM1_HDR_CHAMP_COUNT] = 4;

    for (i = 0; i < 4; i++) {
        size_t off = (size_t)CSB_V1_DM1_HDR_CHAMPION_START
                   + (size_t)i * CSB_V1_DM1_CHAMP_SIZE;
        memcpy((char *)buf + off + CSB_V1_DM1_CHAMP_OFF_NAME, "TEST  ", 8);
        buf[off + CSB_V1_DM1_CHAMP_OFF_HEALTH]     = (uint8_t)(50 + i * 10);
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_HEALTH]  = 100;
        buf[off + CSB_V1_DM1_CHAMP_OFF_STAMINA]     = 80;
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_STAMINA]  = 100;
        buf[off + CSB_V1_DM1_CHAMP_OFF_MANA]        = 20;
        buf[off + CSB_V1_DM1_CHAMP_OFF_MAX_MANA]    = 40;
        buf[off + CSB_V1_DM1_CHAMP_OFF_STR] = 55;
        buf[off + CSB_V1_DM1_CHAMP_OFF_DEX] = 65;
        buf[off + CSB_V1_DM1_CHAMP_OFF_WIS] = 75;
        buf[off + CSB_V1_DM1_CHAMP_OFF_VIT] = 85;
    }

    csb_v1_character_init_default(&party);
    int r = csb_v1_character_import_dm1_buffer(&party, buf, (int)sizeof(buf));
    CHECK_EQ(r, 4, "buffer import returns 4", "d");
    CHECK_EQ(party.ChampionCount, 4, "ChampionCount is 4", "d");
    CHECK(party.ImportedFromDM1 == 1, "ImportedFromDM1 set after buffer import");
    CHECK_EQ(party.Champions[2].CurrentHealth, 70,
             "champ 2 health is 70", "d");
}

/* ── Test 9: utility disk check ──────────────────────────────────── */
static void test_utility_disk(void)
{
    /* Non-existent path → -1 (error) */
    const char *path = "firestaff-csb-missing-utility-disk";
    remove(path);
    int r = csb_v1_util_check_disk(path);
    CHECK_EQ(r, -1, "util_check_disk on nonexistent returns -1", "d");
}

/* ── Test 10: source evidence ───────────────────────────────────── */
static void test_source_evidence(void)
{
    const char *e = csb_v1_character_source_evidence();
    CHECK(e != NULL, "source evidence is non-null");
    CHECK(strstr(e, "CEDTINC7.C") != NULL,
          "source evidence cites CEDTINC7.C");
    CHECK(strstr(e, "CEDTDATA.C") != NULL,
          "source evidence cites CEDTDATA.C");
    CHECK(strstr(e, "CHAMPION.C") != NULL,
          "source evidence cites CHAMPION.C");
    CHECK(strstr(e, "SaveGame.cpp") != NULL,
          "source evidence cites SaveGame.cpp");
}

/* ── Main ────────────────────────────────────────────────────────── */
int main(void)
{
    printf("=== CSB V1 Phase 6 — Utility / Import Flow Tests ===\n\n");

    test_party_defaults();
    printf("\n");
    test_import_one_living_champion();
    printf("\n");
    test_import_dead_champion();
    printf("\n");
    test_import_stat_min_clamp();
    printf("\n");
    test_import_file();
    printf("\n");
    test_import_buffer();
    printf("\n");
    test_utility_disk();
    printf("\n");
    test_source_evidence();

    printf("\n========================================\n");
    printf("PASSED: %d\n", passed);
    printf("FAILED: %d\n", failed);
    return failed == 0 ? 0 : 1;
}
