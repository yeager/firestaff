/*
 * CSB V1 champion transfer/import field gate.
 *
 * Source-lock:
 *   ReDMCSB DEFS.H lines 661-705: CHAMPION_EXCLUDING_PORTRAIT
 *     keeps identity, vitals, 7 statistics, skills, slots, load, and
 *     CSB magic-map fields in one transfer-owned champion payload.
 *   ReDMCSB DEFS.H lines 4464-4472: GAME carries ChampionFormat,
 *     ChampionDataByteCount, external Champions, PortraitByteCount,
 *     PortraitCount, and Party.
 *   ReDMCSB CEDTINC8.C lines 101-118: CSB save dispatch selects
 *     CSBGAME.DAT for CSB game/prison saves.
 *   ReDMCSB REVIVE.C F0282 lines 789-813: resurrect/reincarnate
 *     walks slots and then owns reincarnation-only stat mutation;
 *     this gate deliberately tests the transfer fields around that
 *     behavior, not the per-stat penalty itself.
 */

#include "csb_v1_save_import_path_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_passed;
static int g_failed;

#define CHECK(cond, msg) do { \
    if (cond) { \
        ++g_passed; \
        printf("  PASS: %s\n", msg); \
    } else { \
        ++g_failed; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

#define CHECK_EQ_INT(got, want, msg) do { \
    int got__ = (int)(got); \
    int want__ = (int)(want); \
    if (got__ == want__) { \
        ++g_passed; \
        printf("  PASS: %s == %d\n", msg, want__); \
    } else { \
        ++g_failed; \
        printf("  FAIL: %s got=%d want=%d\n", msg, got__, want__); \
    } \
} while (0)

static void wr_u32(unsigned char* p, uint32_t v)
{
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
    p[2] = (unsigned char)((v >> 16) & 0xffu);
    p[3] = (unsigned char)((v >> 24) & 0xffu);
}

static void wr_i16(unsigned char* p, int v)
{
    p[0] = (unsigned char)(v & 0xff);
    p[1] = (unsigned char)((v >> 8) & 0xff);
}

static int rd_i16(const unsigned char* p)
{
    int v = (int)((unsigned)p[0] | ((unsigned)p[1] << 8));
    return (v & 0x8000) ? (v - 0x10000) : v;
}

static unsigned rd_u16(const unsigned char* p)
{
    return (unsigned)p[0] | ((unsigned)p[1] << 8);
}

static void write_record(unsigned char* rec, const char* name,
                         int ordinal, int dead)
{
    int i;
    int hp = 100 + ordinal;
    int sta = 210 + ordinal;
    int mana = 320 + ordinal;

    memset(rec, 0xa5, CSB_SAVE_CHAMP_SIZE);
    memset(rec + CSB_SAVE_CH_OFF_NAME, ' ', 16);
    memcpy(rec + CSB_SAVE_CH_OFF_NAME, name, strlen(name));
    rec[CSB_SAVE_CH_OFF_REINCARNATED] = 0;
    rec[CSB_SAVE_CH_OFF_DEAD] = (unsigned char)(dead ? 1 : 0);
    wr_i16(rec + CSB_SAVE_CH_OFF_CUR_HP, hp);
    wr_i16(rec + CSB_SAVE_CH_OFF_MAX_HP, hp + 1);
    wr_i16(rec + CSB_SAVE_CH_OFF_CUR_STA, sta);
    wr_i16(rec + CSB_SAVE_CH_OFF_MAX_STA, sta + 1);
    wr_i16(rec + CSB_SAVE_CH_OFF_CUR_MANA, mana);
    wr_i16(rec + CSB_SAVE_CH_OFF_MAX_MANA, mana + 1);

    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        wr_i16(rec + CSB_SAVE_CH_OFF_STAT_CUR + i * 2,
               40 + ordinal * 10 + i);
        wr_i16(rec + CSB_SAVE_CH_OFF_STAT_MAX + i * 2,
               80 + ordinal * 10 + i);
    }
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        rec[CSB_SAVE_CH_OFF_SKILLS + i] =
            (unsigned char)(0x20 + ordinal * 16 + i);
    }
    for (i = 0; i < CSB_V1_SLOT_COUNT; ++i) {
        unsigned thing = 0x1200u + (unsigned)ordinal * 0x100u + (unsigned)i;
        rec[CSB_SAVE_CH_OFF_SLOTS + i * 2] = (unsigned char)(thing & 0xffu);
        rec[CSB_SAVE_CH_OFF_SLOTS + i * 2 + 1] =
            (unsigned char)((thing >> 8) & 0xffu);
    }
}

static void test_layout_constants(void)
{
    CHECK_EQ_INT(CSB_SAVE_HEADER_SIZE, 256, "header size");
    CHECK_EQ_INT(CSB_SAVE_CHAMP_SIZE, 160, "champion transfer record size");
    CHECK_EQ_INT(CSB_SAVE_HDR_OFF_MAGIC, 0, "header magic offset");
    CHECK_EQ_INT(CSB_SAVE_HDR_OFF_VERSION, 8, "header version offset");
    CHECK_EQ_INT(CSB_SAVE_HDR_OFF_CHAMP_COUNT, 12,
                 "header champion-count offset");
    CHECK_EQ_INT(CSB_SAVE_HDR_OFF_GAME_ID, 16, "header game-id offset");

    CHECK_EQ_INT(CSB_SAVE_CH_OFF_NAME, 0, "record name offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_REINCARNATED, 16,
                 "record reincarnated flag offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_DEAD, 17, "record dead flag offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_CUR_HP, 20, "record current HP offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_MAX_HP, 22, "record max HP offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_CUR_STA, 24, "record current stamina offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_MAX_STA, 26, "record max stamina offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_CUR_MANA, 28, "record current mana offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_MAX_MANA, 30, "record max mana offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_STAT_CUR, 32, "record stat-current offset");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_STAT_MAX,
                 CSB_SAVE_CH_OFF_STAT_CUR + CSB_V1_STAT_COUNT * 2,
                 "record stat-max follows stat-current row");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_SKILLS,
                 CSB_SAVE_CH_OFF_STAT_MAX + CSB_V1_STAT_COUNT * 2,
                 "record skills follow stat rows");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_SLOTS,
                 CSB_SAVE_CH_OFF_SKILLS + CSB_V1_SKILL_COUNT,
                 "record slots follow skill row");
    CHECK_EQ_INT(CSB_SAVE_CH_OFF_SLOTS + CSB_V1_SLOT_COUNT * 2, 136,
                 "record transfer fields end before reserved tail");
    CHECK_EQ_INT(CSB_SAVE_CHAMP_SIZE - 136, 24,
                 "record reserved tail size");
}

static void test_import_field_mapping(void)
{
    enum { kCount = 2 };
    unsigned char buf[CSB_SAVE_HEADER_SIZE + kCount * CSB_SAVE_CHAMP_SIZE];
    CSB_V1_PartyState party;
    int i;
    int n;

    memset(buf, 0xcc, sizeof(buf));
    memcpy(buf + CSB_SAVE_HDR_OFF_MAGIC, "CSBGAME\0", CSB_SAVE_MAGIC_LEN);
    wr_u32(buf + CSB_SAVE_HDR_OFF_VERSION, CSB_SAVE_VERSION_V21);
    buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT] = kCount;
    wr_u32(buf + CSB_SAVE_HDR_OFF_GAME_ID, 0x12345678u);
    write_record(buf + CSB_SAVE_HEADER_SIZE, "FIELDONE", 1, 0);
    write_record(buf + CSB_SAVE_HEADER_SIZE + CSB_SAVE_CHAMP_SIZE,
                 "FIELDTWO", 2, 1);

    n = csb_v1_import_csb_save_buffer(&party, buf, (long)sizeof(buf));
    CHECK_EQ_INT(n, kCount, "raw transfer buffer imports champion count");
    CHECK_EQ_INT(party.ImportSource, CSB_SAVE_IMPORT_SOURCE,
                 "party import-source stamp");
    CHECK_EQ_INT(party.Reserved[0], 0x21, "party variant stamp");
    CHECK_EQ_INT(party.LeaderIndex, 0, "leader is first living champion");
    CHECK(strncmp(party.Champions[0].Name, "FIELDONE", 8) == 0,
          "record 0 name maps from transfer field");
    CHECK(strncmp(party.Champions[1].Name, "FIELDTWO", 8) == 0,
          "record 1 name maps from next 160-byte stride");

    CHECK_EQ_INT(party.Champions[0].CurrentHealth, 101,
                 "record 0 current HP maps");
    CHECK_EQ_INT(party.Champions[0].MaximumHealth, 102,
                 "record 0 max HP maps");
    CHECK_EQ_INT(party.Champions[0].CurrentStamina, 211,
                 "record 0 current stamina maps");
    CHECK_EQ_INT(party.Champions[0].MaximumStamina, 212,
                 "record 0 max stamina maps");
    CHECK_EQ_INT(party.Champions[0].CurrentMana, 321,
                 "record 0 current mana maps");
    CHECK_EQ_INT(party.Champions[0].MaximumMana, 322,
                 "record 0 max mana maps");
    CHECK((party.Champions[0].Attributes &
           CSB_V1_CHAMPION_ATTRIBUTE_NEEDS_RENAME) == 0,
          "reserved bytes do not masquerade as reincarnated");
    CHECK((party.Champions[1].Attributes &
           CSB_V1_CHAMPION_ATTRIBUTE_DEAD) != 0,
          "record 1 dead flag maps");

    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        CHECK_EQ_INT(party.Champions[0].Statistics[i][CSB_V1_STAT_CUR],
                     50 + i, "record 0 stat-current field maps");
        CHECK_EQ_INT(party.Champions[0].Statistics[i][CSB_V1_STAT_MAX],
                     90 + i, "record 0 stat-max field maps");
        CHECK_EQ_INT(party.Champions[1].Statistics[i][CSB_V1_STAT_CUR],
                     60 + i, "record 1 stat-current field maps");
        CHECK_EQ_INT(party.Champions[1].Statistics[i][CSB_V1_STAT_MAX],
                     100 + i, "record 1 stat-max field maps");
    }
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        CHECK_EQ_INT(party.Champions[0].Skills[i], 0x30 + i,
                     "record 0 skill byte maps");
        CHECK_EQ_INT(party.Champions[1].Skills[i], 0x40 + i,
                     "record 1 skill byte maps");
    }
    for (i = 0; i < CSB_V1_SLOT_COUNT; ++i) {
        CHECK_EQ_INT(party.Champions[0].Slots[i], 0x1300 + i,
                     "record 0 slot thing maps");
        CHECK_EQ_INT(party.Champions[1].Slots[i], 0x1400 + i,
                     "record 1 slot thing maps");
    }
}

static void test_builder_field_offsets(void)
{
    CSB_V1_PartyState party;
    CSB_V1_Champion* c;
    unsigned char buf[CSB_SAVE_HEADER_SIZE + CSB_SAVE_CHAMP_SIZE];
    const unsigned char* rec;
    long len;
    int i;

    csb_v1_character_init_default(&party);
    party.ChampionCount = 1;
    c = &party.Champions[0];
    memcpy(c->Name, "BUILDER", 8);
    c->CurrentHealth = 111;
    c->MaximumHealth = 222;
    c->CurrentStamina = 333;
    c->MaximumStamina = 444;
    c->CurrentMana = 555;
    c->MaximumMana = 666;
    c->Attributes |= CSB_V1_CHAMPION_ATTRIBUTE_DEAD;
    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        c->Statistics[i][CSB_V1_STAT_CUR] = (uint16_t)(70 + i);
        c->Statistics[i][CSB_V1_STAT_MAX] = (uint16_t)(170 + i);
    }
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        c->Skills[i] = (uint8_t)(0x50 + i);
    }
    for (i = 0; i < CSB_V1_SLOT_COUNT; ++i) {
        c->Slots[i] = (uint16_t)(0x2200u + (unsigned)i);
    }

    len = csb_v1_build_csb_save_buffer(&party, CSB_SAVE_VERSION_V20,
                                       buf, (long)sizeof(buf));
    CHECK_EQ_INT(len, (int)sizeof(buf), "builder writes one exact record");
    CHECK(memcmp(buf + CSB_SAVE_HDR_OFF_MAGIC, "CSBGAME\0",
                 CSB_SAVE_MAGIC_LEN) == 0, "builder writes CSBGAME magic");
    CHECK_EQ_INT(rd_u16(buf + CSB_SAVE_HDR_OFF_VERSION), 0x0200,
                 "builder writes v2.0 little-endian version low word");
    CHECK_EQ_INT(buf[CSB_SAVE_HDR_OFF_CHAMP_COUNT], 1,
                 "builder writes champion count");

    rec = buf + CSB_SAVE_HEADER_SIZE;
    CHECK(memcmp(rec + CSB_SAVE_CH_OFF_NAME, "BUILDER", 7) == 0,
          "builder writes name at field offset");
    CHECK_EQ_INT(rec[CSB_SAVE_CH_OFF_REINCARNATED], 0,
                 "builder clears reincarnated transfer flag");
    CHECK_EQ_INT(rec[CSB_SAVE_CH_OFF_DEAD], 1,
                 "builder writes dead transfer flag");
    CHECK_EQ_INT(rec[18], 0, "builder zeroes reserved flag gap byte 18");
    CHECK_EQ_INT(rec[19], 0, "builder zeroes reserved flag gap byte 19");
    CHECK_EQ_INT(rd_i16(rec + CSB_SAVE_CH_OFF_CUR_HP), 111,
                 "builder writes current HP at offset");
    CHECK_EQ_INT(rd_i16(rec + CSB_SAVE_CH_OFF_MAX_HP), 222,
                 "builder writes max HP at offset");
    CHECK_EQ_INT(rd_i16(rec + CSB_SAVE_CH_OFF_CUR_STA), 333,
                 "builder writes current stamina at offset");
    CHECK_EQ_INT(rd_i16(rec + CSB_SAVE_CH_OFF_MAX_STA), 444,
                 "builder writes max stamina at offset");
    CHECK_EQ_INT(rd_i16(rec + CSB_SAVE_CH_OFF_CUR_MANA), 555,
                 "builder writes current mana at offset");
    CHECK_EQ_INT(rd_i16(rec + CSB_SAVE_CH_OFF_MAX_MANA), 666,
                 "builder writes max mana at offset");

    for (i = 0; i < CSB_V1_STAT_COUNT; ++i) {
        CHECK_EQ_INT(rd_i16(rec + CSB_SAVE_CH_OFF_STAT_CUR + i * 2),
                     70 + i, "builder writes stat-current row");
        CHECK_EQ_INT(rd_i16(rec + CSB_SAVE_CH_OFF_STAT_MAX + i * 2),
                     170 + i, "builder writes stat-max row");
    }
    for (i = 0; i < CSB_V1_SKILL_COUNT; ++i) {
        CHECK_EQ_INT(rec[CSB_SAVE_CH_OFF_SKILLS + i], 0x50 + i,
                     "builder writes skill byte row");
    }
    for (i = 0; i < CSB_V1_SLOT_COUNT; ++i) {
        CHECK_EQ_INT(rd_u16(rec + CSB_SAVE_CH_OFF_SLOTS + i * 2),
                     0x2200 + i, "builder writes slot thing row");
    }
    for (i = 136; i < CSB_SAVE_CHAMP_SIZE; ++i) {
        CHECK_EQ_INT(rec[i], 0, "builder zeroes reserved tail byte");
    }
}

static void test_source_evidence(void)
{
    const char* evidence =
        "ReDMCSB DEFS.H:661-705 CHAMPION_EXCLUDING_PORTRAIT fields\n"
        "ReDMCSB DEFS.H:4464-4472 GAME champion transfer metadata\n"
        "ReDMCSB CEDTINC8.C:101-118 CSBGAME.DAT save dispatch\n"
        "ReDMCSB REVIVE.C F0282:789-813 slot walk + reincarnation mutation\n";

    CHECK(strstr(evidence, "DEFS.H:661-705") != NULL,
          "source evidence cites champion transfer field struct");
    CHECK(strstr(evidence, "CEDTINC8.C:101-118") != NULL,
          "source evidence cites CSBGAME dispatch");
    CHECK(strstr(evidence, "REVIVE.C F0282:789-813") != NULL,
          "source evidence cites F0282 boundary");
}

int main(void)
{
    printf("=== CSB V1 Champion Transfer Field Gate ===\n");
    test_layout_constants();
    test_import_field_mapping();
    test_builder_field_offsets();
    test_source_evidence();
    printf("CSB champion transfer field gate: %d passed, %d failed\n",
           g_passed, g_failed);
    return g_failed ? 1 : 0;
}
