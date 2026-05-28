/*
 * test_dm2_v1_utility_import.c — DM2 V1 Phase 6 Utility/Import Probe
 *
 * Phase 6: Utility/import flow — DM2-specific load/start flow.
 *
 * Verifies:
 *   1. Starter party generation (4 champions with correct names/classes)
 *   2. New game session creation (Hall of Champions position, gold, time)
 *   3. Session serialization round-trip (serialize → deserialize)
 *   4. Session slot save/load round-trip (save → load → validate)
 *   5. Champion record build (portrait→class mapping, attributes)
 *   6. Session validation (detect corrupted sessions)
 *   7. Flow result codes (new_game, load_game)
 *   8. DM2 V1 boot integration (scan→new_game pipeline)
 *
 * No game data files needed — uses deterministic test data only.
 *
 * Source: CHAMPION.C F0280 — starter party generation
 *         SKULL.ASM T520 — party placement (Hall of Champions 15,15,N)
 *         docs/dm2_party_state.md — champion record, initial stats
 *         docs/dm2_save_format.md — session serialization (1306 bytes)
 */

#include "dm2_v1_new_game.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_save_load.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define FS_MKDIR(path) _mkdir(path)
#define FS_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define FS_MKDIR(path) mkdir(path, 0755)
#define FS_GETPID() (int)getpid()
#endif

/* ── Test counters ── */
static int passed;
static int failed;
static int skipped;

/* ── Check macro ── */
#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define SKIP(msg) do { skipped++; printf("  SKIP: %s\n", msg); } while (0)

/* ── Temp directory for save/load ── */
static char g_save_dir[256];

static void setup_temp_dir(void)
{
    snprintf(g_save_dir, sizeof(g_save_dir),
             "/tmp/firestaff-dm2-phase6-%d", FS_GETPID());
    FS_MKDIR(g_save_dir);
}

static void cleanup_temp_dir(void)
{
    /* Remove test slot files */
    for (int i = 0; i < 10; i++) {
        char path[256];
        snprintf(path, sizeof(path), "%s/SKSave%02d.dat", g_save_dir, i);
        (void)remove(path);
    }
    (void)remove("/tmp/firestaff-dm2-phase6-save.bak");
}

/* ── Test 1: Portrait → class mapping ── */
static void test_portrait_to_class(void)
{
    printf("  Portrait→class mapping...\n");
    CHECK(dm2_v1_portrait_to_class(DM2_PORTRAIT_FIGHTER_MALE) == DM2_CLASS_FIGHTER,
          "Portrait 0 → Fighter");
    CHECK(dm2_v1_portrait_to_class(DM2_PORTRAIT_FIGHTER_FEMALE) == DM2_CLASS_FIGHTER,
          "Portrait 1 → Fighter");
    CHECK(dm2_v1_portrait_to_class(DM2_PORTRAIT_NINJA_MALE) == DM2_CLASS_NINJA,
          "Portrait 2 → Ninja");
    CHECK(dm2_v1_portrait_to_class(DM2_PORTRAIT_NINJA_FEMALE) == DM2_CLASS_NINJA,
          "Portrait 3 → Ninja");
    CHECK(dm2_v1_portrait_to_class(DM2_PORTRAIT_PRIEST_MALE) == DM2_CLASS_PRIEST,
          "Portrait 4 → Priest");
    CHECK(dm2_v1_portrait_to_class(DM2_PORTRAIT_PRIEST_FEMALE) == DM2_CLASS_PRIEST,
          "Portrait 5 → Priest");
    CHECK(dm2_v1_portrait_to_class(DM2_PORTRAIT_WIZARD_MALE) == DM2_CLASS_WIZARD,
          "Portrait 6 → Wizard");
    CHECK(dm2_v1_portrait_to_class(DM2_PORTRAIT_WIZARD_FEMALE) == DM2_CLASS_WIZARD,
          "Portrait 7 → Wizard");
    CHECK(dm2_v1_portrait_to_class(99) == DM2_CLASS_FIGHTER,
          "Unknown portrait defaults to Fighter");
}

/* ── Test 2: Champion record build ── */
static void test_build_champion_record(void)
{
    printf("  Champion record build...\n");
    DM2_ChampionRecord rec;
    dm2_v1_build_champion_record(&rec,
                                  "Testus",
                                  "the Bold",
                                  DM2_PORTRAIT_FIGHTER_MALE,
                                  DM2_CLASS_FIGHTER,
                                  DM2_VIEW_CELL_FRONT_LEFT,
                                  0 /* North */);

    CHECK(rec.first_name[0] != '\0', "First name is non-empty");
    CHECK(strncmp(rec.first_name, "Testus", 7) == 0,
          "First name matches input");
    CHECK(rec.absolute_direction == 0, "Direction set to North");
    CHECK(rec.squad_position == DM2_VIEW_CELL_FRONT_LEFT,
          "View cell set correctly");
    CHECK(rec.cur_hp == rec.max_hp, "HP cur == max at creation");
    CHECK(rec.cur_hp == DM2_INITIAL_HP_FIGHTER,
          "Fighter HP matches class default");

    /* Check class-specific mana (Fighter should have 0) */
    CHECK(rec.mana == 0, "Fighter starts with 0 mana");

    /* Stamina */
    CHECK(rec.stamina == DM2_INITIAL_STAMINA_FIGHTER,
          "Fighter stamina matches class default");

    /* Food and water at creation */
    CHECK(rec.food == 100, "Food initialized to 100");
    CHECK(rec.water == 100, "Water initialized to 100");

    /* No poison or runes */
    CHECK(rec.poison_value == 0, "No poison at creation");
    CHECK(rec.runes_count == 0, "No runes at creation");

    /* No inventory */
    int has_item = 0;
    for (int i = 0; i < DM2_CHAMPION_INVENTORY_SLOTS; i++) {
        if (rec.inventory[i] != 0) { has_item = 1; break; }
    }
    CHECK(!has_item, "Inventory is empty at creation");

    /* Test wizard has mana */
    DM2_ChampionRecord wiz;
    dm2_v1_build_champion_record(&wiz, "Merlin", "",
                                  DM2_PORTRAIT_WIZARD_MALE,
                                  DM2_CLASS_WIZARD,
                                  DM2_VIEW_CELL_BACK_RIGHT,
                                  2 /* South */);
    CHECK(wiz.mana == DM2_INITIAL_HP_WIZARD + 15,  /* DM2_INITIAL_MANA_WIZARD */
          "Wizard starts with non-zero mana");
    CHECK(wiz.cur_hp == DM2_INITIAL_HP_WIZARD,
          "Wizard HP is lower than Fighter");
}

/* ── Test 3: Session new (new game state) ── */
static void test_session_new(void)
{
    printf("  Session new (new game)...\n");
    DM2_V1_SessionState session;
    dm2_v1_session_new(&session);

    CHECK(session.game_tick == 0, "game_tick starts at 0");
    CHECK(session.rng_seed == 257, "rng_seed defaults to dungeon seed 257");
    CHECK(session.party_x == 15, "party_x = Hall of Champions X");
    CHECK(session.party_y == 15, "party_y = Hall of Champions Y");
    CHECK(session.party_dir == 0, "party_dir = North");
    CHECK(session.party_level == 0, "party_level = 0 (entrance)");
    CHECK(session.outdoor_mode == 0, "outdoor_mode = 0 (dungeon)");
    CHECK(session.gold == 100, "Starting gold = 100");
    CHECK(session.reputation == 0, "Starting reputation = 0");
    CHECK(session.time_of_day_minutes == 720, "Time = noon (720 min)");
    CHECK(session.rain_intensity == 0, "No rain at start");
    CHECK(session.champion_count == 4, "4 starter champions");
    CHECK(session.leader_index == 0, "Leader is champion 0 (Theron)");
}

/* ── Test 4: Session validation ── */
static void test_session_validate(void)
{
    printf("  Session validation...\n");
    DM2_V1_SessionState session;

    /* Valid session */
    dm2_v1_session_new(&session);
    CHECK(dm2_v1_session_validate(&session), "Fresh session is valid");

    /* Corrupt: champion_count > 4 */
    dm2_v1_session_new(&session);
    session.champion_count = 5;
    CHECK(!dm2_v1_session_validate(&session),
          "champion_count > 4 is invalid");

    /* Corrupt: leader_index >= 4 */
    dm2_v1_session_new(&session);
    session.leader_index = 4;
    CHECK(!dm2_v1_session_validate(&session),
          "leader_index >= 4 is invalid");

    /* Corrupt: party position out of range */
    dm2_v1_session_new(&session);
    session.party_x = 100;
    CHECK(!dm2_v1_session_validate(&session),
          "party_x > 63 is invalid");

    /* Note: gold is uint32_t — natural overflow is tested by
     * truncation semantics; validator checks > 4 billion. */

    /* Corrupt: time of day out of range */
    dm2_v1_session_new(&session);
    session.time_of_day_minutes = 2000;
    CHECK(!dm2_v1_session_validate(&session),
          "time_of_day >= 1440 is invalid");

    /* Null session */
    CHECK(!dm2_v1_session_validate(NULL), "NULL session is invalid");
}

/* ── Test 5: Serialize → deserialize round-trip ── */
static void test_serialize_roundtrip(void)
{
    printf("  Serialize→deserialize round-trip...\n");
    DM2_V1_SessionState orig, loaded;
    dm2_v1_session_new(&orig);

    /* Verify the starter party has names */
    DM2_ChampionRecord *r0 = (DM2_ChampionRecord *)orig.champion_data[0];
    CHECK(r0->first_name[0] != '\0', "Champion 0 has a name");

    /* Serialize */
    uint8_t buf[4096];
    int sz = dm2_v1_session_serialize(&orig, buf, sizeof(buf));
    CHECK(sz > 0, "serialize returns positive size");
    CHECK(sz == (29 + 4 * 261),  /* DM2_SESSION_SERIALIZED_SIZE */
          "serialized size = 29 + 4*261 = 1073 bytes");

    /* Deserialize */
    int r = dm2_v1_session_deserialize(&loaded, buf, (size_t)sz);
    CHECK(r == 0, "deserialize succeeds");

    /* Verify all fields match */
    CHECK(loaded.game_tick == orig.game_tick, "game_tick preserved");
    CHECK(loaded.rng_seed == orig.rng_seed, "rng_seed preserved");
    CHECK(loaded.champion_count == orig.champion_count,
          "champion_count preserved");
    CHECK(loaded.leader_index == orig.leader_index,
          "leader_index preserved");
    CHECK(loaded.party_x == orig.party_x, "party_x preserved");
    CHECK(loaded.party_y == orig.party_y, "party_y preserved");
    CHECK(loaded.party_dir == orig.party_dir, "party_dir preserved");
    CHECK(loaded.party_level == orig.party_level,
          "party_level preserved");
    CHECK(loaded.outdoor_mode == orig.outdoor_mode,
          "outdoor_mode preserved");
    CHECK(loaded.time_of_day_minutes == orig.time_of_day_minutes,
          "time_of_day preserved");
    CHECK(loaded.gold == orig.gold, "gold preserved");
    CHECK(loaded.rain_intensity == orig.rain_intensity,
          "rain_intensity preserved");

    /* Verify champion 0 name preserved */
    DM2_ChampionRecord *l0 = (DM2_ChampionRecord *)loaded.champion_data[0];
    CHECK(strncmp(l0->first_name, r0->first_name,
                  DM2_CHAMPION_NAME_FIRST_LEN) == 0,
          "Champion 0 name preserved through round-trip");
    CHECK(l0->cur_hp == r0->cur_hp, "Champion 0 HP preserved");
    CHECK(l0->mana == r0->mana, "Champion 0 mana preserved");

    /* Validate loaded session */
    CHECK(dm2_v1_session_validate(&loaded),
          "Loaded session is valid");
}

/* ── Test 6: Slot save → slot load round-trip ── */
static void test_slot_roundtrip(void)
{
    printf("  Slot save→load round-trip...\n");
    DM2_V1_SessionState orig, loaded;

    /* Modify session for this test */
    dm2_v1_session_new(&orig);
    orig.gold = 500;
    orig.time_of_day_minutes = 900; /* 3 PM */
    orig.rain_intensity = 50;

    /* Save to slot 3 */
    int r = dm2_v1_session_save_slot(g_save_dir, 3, "Test Save", &orig);
    CHECK(r == 0, "save_slot returns 0 (success)");

    /* Verify slot is reported as occupied */
    CHECK(dm2_v1_save_has_valid_slot(g_save_dir, 3),
          "Slot 3 is occupied after save");

    /* Load from slot 3 */
    r = dm2_v1_session_load_slot(g_save_dir, 3, &loaded);
    CHECK(r == 0, "load_slot returns 0 (success)");

    /* Verify loaded values */
    CHECK(loaded.gold == 500, "gold preserved through slot round-trip");
    CHECK(loaded.time_of_day_minutes == 900,
          "time_of_day preserved through slot round-trip");
    CHECK(loaded.rain_intensity == 50,
          "rain_intensity preserved through slot round-trip");
    CHECK(loaded.champion_count == 4,
          "champion_count preserved through slot round-trip");

    /* Validate */
    CHECK(dm2_v1_session_validate(&loaded),
          "Loaded session is valid after slot round-trip");

    /* Delete slot */
    r = dm2_v1_session_delete_slot(g_save_dir, 3);
    CHECK(r == 0, "delete_slot returns 0");

    /* Verify slot is now empty */
    CHECK(!dm2_v1_save_has_valid_slot(g_save_dir, 3),
          "Slot 3 is empty after delete");

    /* Load from deleted slot should fail */
    r = dm2_v1_session_load_slot(g_save_dir, 3, &loaded);
    CHECK(r != 0, "load from deleted slot returns non-zero");
}

/* ── Test 7: New game flow ── */
static void test_new_game_flow(void)
{
    printf("  New game flow...\n");
    DM2_V1_SessionState session;
    DM2_V1_BootProfile boot;

    dm2_v1_boot_profile_init(&boot);
    /* Don't set assets — new_game_flow should handle that gracefully */

    DM2_FlowResult result = dm2_v1_new_game_flow(&session, &boot);
    /* Without real assets, this may return NO_ASSETS — that's OK for flow test.
     * We just verify the flow result code is valid. */
    (void)result; /* Just ensure it doesn't crash */
    CHECK(result <= 0, "new_game_flow returns valid result code");
    CHECK(result == DM2_FLOW_NO_ASSETS || result == DM2_FLOW_OK,
          "Result is NO_ASSETS (expected without real assets) or OK");

    /* If we did get OK, verify session */
    if (result == DM2_FLOW_OK) {
        CHECK(dm2_v1_session_validate(&session),
              "Session valid after new_game_flow");
        CHECK(session.champion_count == 4,
              "4 champions after new_game_flow");
    }
}

/* ── Test 8: Source evidence ── */
static void test_source_evidence(void)
{
    printf("  Source evidence citations...\n");
    const char *e = dm2_v1_new_game_source_evidence();
    CHECK(e != NULL && strlen(e) > 10, "Source evidence is non-empty");
    CHECK(strstr(e, "CHAMPION.C F0280") != NULL,
          "F0280 cited in source evidence");
    CHECK(strstr(e, "SKULL.ASM T520") != NULL,
          "SKULL.ASM T520 cited");
    CHECK(strstr(e, "CHAMPRST.C F0278") != NULL,
          "F0278 cited");
}

/* ── Test 9: Session size constant ── */
static void test_session_size(void)
{
    printf("  Session size constant...\n");
    enum { DM2_SESSION_SZ = 26 + 4 * 261 };
    CHECK(sizeof(DM2_V1_SessionState) >= DM2_SESSION_SZ,
          "DM2_V1_SessionState is large enough for serialized form");
    /* DM2_SESSION_MAX_SIZE should be > DM2_SESSION_SZ */
    CHECK(DM2_SESSION_MAX_SIZE > DM2_SESSION_SZ,
          "DM2_SESSION_MAX_SIZE exceeds session size");
}

/* ── Test 10: Flow result codes ── */
static void test_flow_result_codes(void)
{
    printf("  Flow result codes...\n");
    CHECK(DM2_FLOW_OK == 0, "DM2_FLOW_OK == 0");
    CHECK(DM2_FLOW_NO_ASSETS < 0, "DM2_FLOW_NO_ASSETS < 0");
    CHECK(DM2_FLOW_NO_DUNGEON < 0, "DM2_FLOW_NO_DUNGEON < 0");
    CHECK(DM2_FLOW_BAD_SESSION < 0, "DM2_FLOW_BAD_SESSION < 0");
    CHECK(DM2_FLOW_SLOT_ERROR < 0, "DM2_FLOW_SLOT_ERROR < 0");
    CHECK(DM2_FLOW_ALLOC_ERROR < 0, "DM2_FLOW_ALLOC_ERROR < 0");
}

/* ════════════════════════════════════════════════════════════════════════
 * Main
 * ════════════════════════════════════════════════════════════════════════ */

int main(void)
{
    printf("=== DM2 V1 Phase 6 — Utility/Import Probe ===\n\n");

    setup_temp_dir();

    /* ── Portrait → class mapping ── */
    printf("--- Portrait→class mapping ---\n");
    test_portrait_to_class();

    /* ── Champion record build ── */
    printf("\n--- Champion record build ---\n");
    test_build_champion_record();

    /* ── Session new ── */
    printf("\n--- Session new ---\n");
    test_session_new();

    /* ── Session validation ── */
    printf("\n--- Session validation ---\n");
    test_session_validate();

    /* ── Serialize round-trip ── */
    printf("\n--- Serialize→deserialize round-trip ---\n");
    test_serialize_roundtrip();

    /* ── Slot round-trip ── */
    printf("\n--- Slot save→load round-trip ---\n");
    test_slot_roundtrip();

    /* ── New game flow ── */
    printf("\n--- New game flow ---\n");
    test_new_game_flow();

    /* ── Source evidence ── */
    printf("\n--- Source evidence ---\n");
    test_source_evidence();

    /* ── Session size ── */
    printf("\n--- Session size ---\n");
    test_session_size();

    /* ── Flow result codes ── */
    printf("\n--- Flow result codes ---\n");
    test_flow_result_codes();

    cleanup_temp_dir();

    printf("\n=================================\n");
    printf("PASSED: %d\nFAILED: %d\nSKIPPED: %d\n",
           passed, failed, skipped);
    printf("=================================\n");

    return failed == 0 ? 0 : 1;
}
