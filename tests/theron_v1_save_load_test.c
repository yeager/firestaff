/*
 * test_theron_v1_save_load.c — Theron V1 Phase 7: Save/Load Tests
 *
 * Tests:
 *   - Slot path construction
 *   - Save to slot / load from slot round-trip
 *   - Slot enumeration
 *   - Slot deletion
 *   - Save slot verification
 *   - Default root path
 *
 * Phase 7 source-lock (2026-05-27)
 * Run: cd build_test_phase7 && ./test_theron_v1_save_load
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "theron_v1_save_load.h"
#include "theron_v1_dungeon_progression.h"

/* ── Temp dir for tests ─────────────────────────────────────────── */

static char g_test_dir[256];

static void test_dir_setup(void) {
    snprintf(g_test_dir, sizeof(g_test_dir), "%s/test_theron_save",
             getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
    /* Remove any old test dir */
    system("rm -rf /tmp/test_theron_save");
    mkdir(g_test_dir, 0755);
}

static void test_dir_teardown(void) {
    system("rm -rf /tmp/test_theron_save");
}

/* ── Test helpers ─────────────────────────────────────────────────── */

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST(name) do { \
    printf("  %-55s ", name); \
    fflush(stdout); \
    g_tests_run++; \
} while (0)

#define PASS() do { \
    printf("PASS\n"); \
    g_tests_passed++; \
} while (0)

#define FAIL(msg) do { \
    printf("FAIL: %s\n", msg); \
} while (0)

#define ASSERT(cond, msg) do { \
    if (!(cond)) { FAIL(msg); return 0; } \
} while (0)

/* ── Test: slot path construction ────────────────────────────────── */

static int test_slot_path(void) {
    TEST("Slot path — format saves/theron/slotN.tqsv");

    char path[256];
    theron_v1_save_slot_path(g_test_dir, 0, path, sizeof(path));
    ASSERT(strstr(path, "slot0.tqsv") != NULL, "slot 0 path wrong");

    theron_v1_save_slot_path(g_test_dir, 7, path, sizeof(path));
    ASSERT(strstr(path, "slot7.tqsv") != NULL, "slot 7 path wrong");

    /* Invalid slot → empty path */
    path[0] = '\0';
    theron_v1_save_slot_path(g_test_dir, -1, path, sizeof(path));
    ASSERT(path[0] == '\0', "negative slot should give empty path");
    theron_v1_save_slot_path(g_test_dir, 8, path, sizeof(path));
    ASSERT(path[0] == '\0', "slot 8 should give empty path");

    PASS();
    return 1;
}

/* ── Test: default root ───────────────────────────────────────────── */

static int test_default_root(void) {
    TEST("Default root — contains .firestaff/saves/theron");

    char buf[256];
    theron_v1_save_default_root(buf, sizeof(buf));
    ASSERT(strstr(buf, ".firestaff") != NULL, "default root missing .firestaff");
    ASSERT(strstr(buf, "theron") != NULL, "default root missing theron");

    PASS();
    return 1;
}

/* ── Test: save / load round-trip ─────────────────────────────────── */

static int test_save_load_roundtrip(void) {
    TEST("Save/load round-trip — champion + progression");

    /* Build a dummy champion block (4 × 128 bytes) */
    uint8_t champ_data[THERON_SAVE_CHAMPION_COUNT * THERON_SAVE_CHAMPION_BLOCK_SIZE];
    memset(champ_data, 0, sizeof(champ_data));
    champ_data[0] = 0x42; champ_data[128] = 0x43; champ_data[256] = 0x44;

    /* Build a dummy dungeon progression */
    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);
    prog.quest_items_collected = 0x0F; /* items 0-3 collected */
    prog.current_dungeon = THERON_DUNGEON_4_TOMB_OF_WOE;
    prog.dungeon_states[0] = THERON_DUNGEON_STATE_COMPLETE;
    prog.dungeon_states[1] = THERON_DUNGEON_STATE_COMPLETE;
    prog.dungeon_states[2] = THERON_DUNGEON_STATE_COMPLETE;
    prog.dungeon_states[3] = THERON_DUNGEON_STATE_AVAILABLE;

    /* Save to slot 3 */
    int r = theron_v1_save_to_slot(g_test_dir, 3, champ_data,
                                    sizeof(champ_data), &prog, "Test dungeon 4");
    ASSERT(r == 0, "save to slot failed");

    /* Load into fresh buffers */
    uint8_t champ_read[sizeof(champ_data)];
    memset(champ_read, 0, sizeof(champ_read));
    Theron_DungeonProgression prog_read;
    memset(&prog_read, 0, sizeof(prog_read));
    Theron_SaveSlot slot_info;
    memset(&slot_info, 0, sizeof(slot_info));

    r = theron_v1_save_load_from_slot(g_test_dir, 3,
                                       champ_read, sizeof(champ_read),
                                       &prog_read, sizeof(prog_read),
                                       &slot_info);
    ASSERT(r == 0, "load from slot failed");
    ASSERT(slot_info.valid == 1, "loaded slot not valid");
    ASSERT(slot_info.quest_items == 0x0F, "quest_items mismatch");
    ASSERT(slot_info.current_dungeon == THERON_DUNGEON_4_TOMB_OF_WOE,
           "current_dungeon mismatch");

    /* Verify champion data survived */
    ASSERT(champ_read[0] == 0x42, "champion data[0] mismatch");
    ASSERT(champ_read[128] == 0x43, "champion data[128] mismatch");
    ASSERT(champ_read[256] == 0x44, "champion data[256] mismatch");

    /* Verify progression */
    ASSERT(prog_read.quest_items_collected == 0x0F, "restored quest_items mismatch");
    ASSERT(prog_read.current_dungeon == THERON_DUNGEON_4_TOMB_OF_WOE,
           "restored current_dungeon mismatch");

    PASS();
    return 1;
}

/* ── Test: slot enumeration ───────────────────────────────────────── */

static int test_slot_enumeration(void) {
    TEST("Slot enumeration — 8 slots, slot 5 populated");

    /* Save to slot 5 */
    uint8_t champ_data[THERON_SAVE_CHAMPION_COUNT * THERON_SAVE_CHAMPION_BLOCK_SIZE];
    memset(champ_data, 0, sizeof(champ_data));
    champ_data[0] = 0x99;

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);
    prog.quest_items_collected = 0x03;
    prog.current_dungeon = THERON_DUNGEON_2_CRYPT_OF_SHADOWS;

    int r = theron_v1_save_to_slot(g_test_dir, 5, champ_data,
                                    sizeof(champ_data), &prog, "After dungeon 2");
    ASSERT(r == 0, "save to slot 5 failed");

    /* Enumerate slots */
    Theron_SaveSlot slots[THERON_SAVE_SLOT_COUNT];
    memset(slots, 0, sizeof(slots));
    int count = theron_v1_save_enum_slots(g_test_dir, slots, THERON_SAVE_SLOT_COUNT);
    ASSERT(count > 0, "no slots enumerated");

    /* Find slot 5 in the enumeration */
    Theron_SaveSlot *slot5 = NULL;
    for (int i = 0; i < count; i++) {
        if (slots[i].slot_index == 5) {
            slot5 = &slots[i];
            break;
        }
    }
    ASSERT(slot5 != NULL, "slot 5 not found in enumeration");
    ASSERT(slot5->valid == 1, "slot 5 not valid");
    ASSERT(slot5->quest_items == 0x03, "slot 5 quest_items wrong");
    ASSERT(slot5->current_dungeon == THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
           "slot 5 current_dungeon wrong");

    PASS();
    return 1;
}

/* ── Test: slot deletion ───────────────────────────────────────────── */

static int test_slot_deletion(void) {
    TEST("Slot deletion — slot removed, enum updated");

    uint8_t champ_data[THERON_SAVE_CHAMPION_COUNT * THERON_SAVE_CHAMPION_BLOCK_SIZE];
    memset(champ_data, 0, sizeof(champ_data));

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    int r = theron_v1_save_to_slot(g_test_dir, 2, champ_data,
                                    sizeof(champ_data), &prog, "To delete");
    ASSERT(r == 0, "save to slot 2 failed");

    /* Verify slot 2 is occupied */
    Theron_SaveSlot slots[THERON_SAVE_SLOT_COUNT];
    int count_before = theron_v1_save_enum_slots(g_test_dir, slots, THERON_SAVE_SLOT_COUNT);
    ASSERT(count_before >= 1, "no slots before delete");

    /* Delete slot 2 */
    r = theron_v1_save_delete_slot(g_test_dir, 2);
    ASSERT(r == 0, "delete failed");

    /* Verify slot 2 is gone */
    r = theron_v1_save_verify_slot(g_test_dir, 2);
    ASSERT(r == 0, "deleted slot still verifies");

    PASS();
    return 1;
}

/* ── Test: slot verification ──────────────────────────────────────── */

static int test_slot_verification(void) {
    TEST("Slot verification — valid and invalid slots");

    uint8_t champ_data[THERON_SAVE_CHAMPION_COUNT * THERON_SAVE_CHAMPION_BLOCK_SIZE];
    memset(champ_data, 0, sizeof(champ_data));
    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    /* Valid slot */
    int r = theron_v1_save_to_slot(g_test_dir, 1, champ_data,
                                    sizeof(champ_data), &prog, "Verify me");
    ASSERT(r == 0, "save failed");
    r = theron_v1_save_verify_slot(g_test_dir, 1);
    ASSERT(r == 1, "valid slot fails verification");

    /* Empty slot */
    r = theron_v1_save_verify_slot(g_test_dir, 7);
    ASSERT(r == 0, "empty slot should not verify");

    /* Invalid slot index */
    r = theron_v1_save_verify_slot(g_test_dir, -1);
    ASSERT(r == 0, "negative slot should not verify");
    r = theron_v1_save_verify_slot(g_test_dir, 99);
    ASSERT(r == 0, "large slot should not verify");

    PASS();
    return 1;
}

/* ── Test: multi-slot isolation ───────────────────────────────────── */

static int test_multi_slot_isolation(void) {
    TEST("Multi-slot isolation — 4 slots, independent data");

    Theron_DungeonProgression prog;
    for (int slot = 0; slot < 4; slot++) {
        theron_v1_dungeon_progression_init(&prog);
        prog.quest_items_collected = (uint8_t)(1 << slot);
        prog.current_dungeon = (Theron_DungeonID)(slot + 1);

        uint8_t champ_data[THERON_SAVE_CHAMPION_COUNT * THERON_SAVE_CHAMPION_BLOCK_SIZE];
        memset(champ_data, 0, sizeof(champ_data));
        champ_data[0] = (uint8_t)(0x10 + slot);

        int r = theron_v1_save_to_slot(g_test_dir, slot,
                                       champ_data, sizeof(champ_data), &prog, "Test");
        ASSERT(r == 0, "save failed");
    }

    /* Load each slot and verify independence */
    for (int slot = 0; slot < 4; slot++) {
        uint8_t champ_read[THERON_SAVE_CHAMPION_COUNT * THERON_SAVE_CHAMPION_BLOCK_SIZE];
        memset(champ_read, 0, sizeof(champ_read));
        Theron_DungeonProgression prog_read;
        memset(&prog_read, 0, sizeof(prog_read));
        Theron_SaveSlot slot_info;
        memset(&slot_info, 0, sizeof(slot_info));

        int r = theron_v1_save_load_from_slot(g_test_dir, slot,
                                               champ_read, sizeof(champ_read),
                                               &prog_read, sizeof(prog_read),
                                               &slot_info);
        ASSERT(r == 0, "load failed");
        ASSERT(slot_info.quest_items == (1 << slot), "quest_items mismatch");
        ASSERT(prog_read.current_dungeon == (Theron_DungeonID)(slot + 1),
               "current_dungeon mismatch");
        ASSERT(champ_read[0] == (uint8_t)(0x10 + slot), "champion data mismatch");
    }

    PASS();
    return 1;
}

/* ── Test: source evidence ───────────────────────────────────────── */

static int test_source_evidence(void) {
    TEST("Source evidence string — non-empty");

    const char *ev = theron_v1_save_source_evidence();
    ASSERT(ev != NULL && strlen(ev) > 10, "source evidence too short");
    ASSERT(strstr(ev, "THQUEST") != NULL, "source evidence missing THQUEST");

    PASS();
    return 1;
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void) {
    printf("\n=== Theron V1 Phase 7 — Save/Load Tests ===\n\n");

    test_dir_setup();

    int (*tests[])(void) = {
        test_slot_path,
        test_default_root,
        test_save_load_roundtrip,
        test_slot_enumeration,
        test_slot_deletion,
        test_slot_verification,
        test_multi_slot_isolation,
        test_source_evidence,
    };
    int n = (int)(sizeof(tests) / sizeof(tests[0]));

    for (int i = 0; i < n; i++) {
        if (!tests[i]()) {
            printf("*** TEST FAILED ***\n");
        }
    }

    test_dir_teardown();

    printf("\n=====================================================\n");
    printf("Results: %d/%d passed\n", g_tests_passed, g_tests_run);
    printf("=====================================================\n\n");

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}