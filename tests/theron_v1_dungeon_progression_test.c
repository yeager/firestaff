/*
 * test_theron_v1_dungeon_progression.c — Theron V1 Phase 6 verification
 *
 * Tests the 7-dungeon sequence, per-dungeon item reset semantics,
 * between-dungeon save, and seven-quest-item retrieval goal.
 *
 * Phase 6 source-lock (2026-05-27)
 * Run: cd build && ./test_theron_v1_dungeon_progression
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "theron_v1_dungeon_progression.h"
#include "theron_v1_save_load.h"

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

/* ── Test: dungeon metadata ─────────────────────────────────────── */

static int test_dungeon_meta(void) {
    TEST("Dungeon metadata lookup");

    for (int id = 1; id <= THERON_DUNGEON_COUNT; id++) {
        const Theron_DungeonMeta *meta = theron_v1_dungeon_meta((Theron_DungeonID)id);
        ASSERT(meta != NULL, "meta is NULL");
        ASSERT(meta->id == id, "id mismatch");
        ASSERT(meta->quest_item_count == 1, "quest_item_count not 1");
        ASSERT(meta->quest_item_bit == (1 << (id - 1)), "quest_item_bit wrong");
        ASSERT(meta->champion_reset == 1, "champion_reset not 1");
    }

    /* Invalid IDs return NULL */
    ASSERT(theron_v1_dungeon_meta(THERON_DUNGEON_INVALID) == NULL, "INVALID returns non-NULL");
    ASSERT(theron_v1_dungeon_meta((Theron_DungeonID)0) == NULL, "0 returns non-NULL");
    ASSERT(theron_v1_dungeon_meta((Theron_DungeonID)8) == NULL, "8 returns non-NULL");

    PASS();
    return 1;
}

/* ── Test: progression init ──────────────────────────────────────── */

static int test_init(void) {
    TEST("Progression init — dungeon 1 available, rest locked");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    ASSERT(prog.current_dungeon == THERON_DUNGEON_1_HALL_OF_RECORDS,
           "current_dungeon != 1");
    ASSERT(prog.quest_items_collected == 0, "quest_items != 0");
    ASSERT(prog.quest_complete == 0, "quest_complete != 0");
    ASSERT(prog.champion_stats_persist == 1, "stats_persist != 1");
    ASSERT(prog.champion_inv_persist == 0, "inv_persist != 0");
    ASSERT(prog.item_reset_applied == 0, "reset_applied != 0");

    ASSERT(prog.dungeon_states[0] == THERON_DUNGEON_STATE_AVAILABLE,
           "dungeon 1 not AVAILABLE");
    for (int i = 1; i < THERON_DUNGEON_COUNT; i++) {
        ASSERT(prog.dungeon_states[i] == THERON_DUNGEON_STATE_LOCKED,
               "dungeon state not LOCKED");
    }

    PASS();
    return 1;
}

/* ── Test: dungeon advance sequence ──────────────────────────────── */

static int test_dungeon_advance(void) {
    TEST("Dungeon advance — 7-dungeon sequence");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    /* Simulate completing each dungeon in sequence */
    for (int d = 1; d <= THERON_DUNGEON_COUNT; d++) {
        /* Simulate dungeon completion */
        Theron_DungeonID current = prog.current_dungeon;
        prog.dungeon_states[current - 1] = THERON_DUNGEON_STATE_COMPLETE;

        Theron_DungeonID next = theron_v1_dungeon_advance(&prog);

        if (d < THERON_DUNGEON_COUNT) {
            ASSERT(next == (Theron_DungeonID)(d + 1),
                   "next dungeon wrong");
            ASSERT(prog.current_dungeon == (Theron_DungeonID)(d + 1),
                   "current_dungeon not advanced");
            ASSERT(prog.dungeon_states[d] == THERON_DUNGEON_STATE_AVAILABLE,
                   "next dungeon not AVAILABLE after advance");
        } else {
            ASSERT(next == THERON_DUNGEON_INVALID, "final next != INVALID");
            ASSERT(prog.quest_complete == 1, "quest_complete not set after dungeon 7");
        }
    }

    PASS();
    return 1;
}

/* ── Test: quest item collection ─────────────────────────────────── */

static int test_quest_item_collect(void) {
    TEST("Quest item collection — bitmask tracking");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    /* Collect items in sequence */
    for (int i = 0; i < THERON_QUEST_ITEM_COUNT; i++) {
        Theron_QuestItem item = (Theron_QuestItem)(1 << i);

        /* Set current dungeon to match the item */
        prog.current_dungeon = (Theron_DungeonID)(i + 1);

        int result = theron_v1_quest_item_collect(&prog, item);
        ASSERT(result == 1, "item collect failed");

        uint8_t expected_mask = (uint8_t)((1 << (i + 1)) - 1);
        ASSERT(prog.quest_items_collected == expected_mask,
               "bitmask mismatch after collection");
    }

    /* All 7 collected → quest complete */
    ASSERT(prog.quest_items_collected == THERON_QUEST_ALL_ITEMS,
           "all items not collected");
    ASSERT(prog.quest_complete == 1, "quest_complete not set");

    PASS();
    return 1;
}

/* ── Test: item reset semantics ───────────────────────────────────── */

static int test_item_reset(void) {
    TEST("Per-dungeon item reset — applied flag logic");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    /* Dungeon 1 — reset required on first entry */
    ASSERT(theron_v1_item_reset_required(&prog, THERON_DUNGEON_1_HALL_OF_RECORDS) == 1,
           "reset not required for dungeon 1");

    /* Apply reset */
    theron_v1_item_reset_mark_applied(&prog);
    ASSERT(theron_v1_item_reset_required(&prog, THERON_DUNGEON_1_HALL_OF_RECORDS) == 0,
           "reset still required after mark_applied");

    /* Enter dungeon 1 */
    int enter_result = theron_v1_dungeon_enter(&prog, THERON_DUNGEON_1_HALL_OF_RECORDS);
    ASSERT(enter_result == 0, "dungeon enter failed");

    /* After completing and advancing to dungeon 2, reset needed again */
    prog.dungeon_states[0] = THERON_DUNGEON_STATE_COMPLETE;
    theron_v1_dungeon_advance(&prog);

    ASSERT(theron_v1_item_reset_required(&prog, THERON_DUNGEON_2_CRYPT_OF_SHADOWS) == 1,
           "reset not required for dungeon 2");

    /* Attempt to enter completed dungeon → fails */
    prog.dungeon_states[0] = THERON_DUNGEON_STATE_COMPLETE;
    prog.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    int retry_enter = theron_v1_dungeon_enter(&prog, THERON_DUNGEON_1_HALL_OF_RECORDS);
    ASSERT(retry_enter == -3, "re-entering complete dungeon should fail");

    PASS();
    return 1;
}

/* ── Test: dungeon exit requires complete ─────────────────────────── */

static int test_dungeon_exit(void) {
    TEST("Dungeon exit only allowed when COMPLETE");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    /* Attempt to exit before completion → INVALID */
    prog.dungeon_states[0] = THERON_DUNGEON_STATE_IN_PROGRESS;
    prog.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;

    Theron_DungeonID exit_result = theron_v1_dungeon_exit(&prog);
    ASSERT(exit_result == THERON_DUNGEON_INVALID,
           "exit succeeded before COMPLETE");

    /* After completion, exit succeeds */
    prog.dungeon_states[0] = THERON_DUNGEON_STATE_COMPLETE;
    exit_result = theron_v1_dungeon_exit(&prog);
    ASSERT(exit_result == THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
           "exit returned wrong next dungeon");

    PASS();
    return 1;
}

/* ── Test: save/restore ──────────────────────────────────────────── */

static int test_save_restore(void) {
    TEST("Save/restore — quest bitmask + dungeon state reconstruction");

    Theron_DungeonProgression original;
    theron_v1_dungeon_progression_init(&original);

    /* Collect items 0, 1, 2 (dungeons 1, 2, 3) */
    for (int i = 0; i < 3; i++) {
        original.current_dungeon = (Theron_DungeonID)(i + 1);
        theron_v1_quest_item_collect(&original, (Theron_QuestItem)(1 << i));
        original.dungeon_states[i] = THERON_DUNGEON_STATE_COMPLETE;
    }

    original.dungeon_states[3] = THERON_DUNGEON_STATE_AVAILABLE;
    original.current_dungeon = THERON_DUNGEON_4_TOMB_OF_WOE;

    /* Serialize only what fits in a between-dungeon save */
    uint8_t quest_items = theron_v1_quest_item_bitmask(&original);
    Theron_DungeonID current = original.current_dungeon;
    uint32_t seeds[THERON_DUNGEON_COUNT];
    for (int i = 0; i < THERON_DUNGEON_COUNT; i++) seeds[i] = original.dungeon_seeds[i];

    /* Restore to new instance */
    Theron_DungeonProgression restored;
    theron_v1_dungeon_progression_restore(&restored, quest_items, current, seeds);

    ASSERT(restored.quest_items_collected == quest_items,
           "quest_items mismatch after restore");
    ASSERT(restored.current_dungeon == current,
           "current_dungeon mismatch after restore");
    ASSERT(restored.quest_complete == 0,
           "quest_complete set before all items collected");

    /* Dungeon 4 should be AVAILABLE after restore */
    ASSERT(restored.dungeon_states[3] == THERON_DUNGEON_STATE_AVAILABLE,
           "dungeon 4 state wrong after restore");

    /* Dungeons 1-3 should be COMPLETE */
    for (int i = 0; i < 3; i++) {
        ASSERT(restored.dungeon_states[i] == THERON_DUNGEON_STATE_COMPLETE,
               "dungeon state wrong after restore");
    }

    /* Dungeon 5+ should be LOCKED */
    for (int i = 4; i < THERON_DUNGEON_COUNT; i++) {
        ASSERT(restored.dungeon_states[i] == THERON_DUNGEON_STATE_LOCKED,
               "dungeon state wrong after restore");
    }

    PASS();
    return 1;
}

/* ── Test: quest complete detection ───────────────────────────────── */

static int test_quest_complete_detection(void) {
    TEST("Quest complete — all 7 items collected");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    ASSERT(!theron_v1_quest_complete(&prog), "quest complete at start");

    /* Collect all 7 items */
    for (int i = 0; i < THERON_QUEST_ITEM_COUNT; i++) {
        prog.current_dungeon = (Theron_DungeonID)(i + 1);
        theron_v1_quest_item_collect(&prog, (Theron_QuestItem)(1 << i));
    }

    ASSERT(theron_v1_quest_complete(&prog), "quest not complete after 7 items");

    /* Duplicate collection doesn't affect completion */
    prog.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    int r = theron_v1_quest_item_collect(&prog, THERON_QUEST_ITEM_1_SACRED_AMPLIFIER);
    ASSERT(r == 0, "duplicate collection should be no-op");
    ASSERT(theron_v1_quest_complete(&prog), "quest complete broken by dup collection");

    PASS();
    return 1;
}

/* ── Test: dungeon name lookup ────────────────────────────────────── */

static int test_dungeon_names(void) {
    TEST("Dungeon name lookup — all 7 dungeons");

    static const char *const expected_names[THERON_DUNGEON_COUNT] = {
        "Hall of Records",
        "Crypt of Shadows",
        "Abyss of Flames",
        "Tomb of Woe",
        "Vault of Secrets",
        "Castle of Fate",
        "Tower of Epilogue",
    };

    for (int i = 1; i <= THERON_DUNGEON_COUNT; i++) {
        const char *name = theron_v1_dungeon_name((Theron_DungeonID)i);
        ASSERT(name != NULL, "name NULL");
        ASSERT(strcmp(name, expected_names[i - 1]) == 0, "name mismatch");
    }

    ASSERT(strcmp(theron_v1_dungeon_name(THERON_DUNGEON_INVALID), "(invalid)") == 0,
           "invalid dungeon name wrong");

    PASS();
    return 1;
}

/* ── Test: dungeon next ───────────────────────────────────────────── */

static int test_dungeon_next(void) {
    TEST("Dungeon next — correct sequence wrapping");

    ASSERT(theron_v1_dungeon_next(THERON_DUNGEON_1_HALL_OF_RECORDS) ==
           THERON_DUNGEON_2_CRYPT_OF_SHADOWS, "next(1) != 2");
    ASSERT(theron_v1_dungeon_next(THERON_DUNGEON_6_CASTLE_OF_FATE) ==
           THERON_DUNGEON_7_TOWER_OF_EPILOGUE, "next(6) != 7");
    ASSERT(theron_v1_dungeon_next(THERON_DUNGEON_7_TOWER_OF_EPILOGUE) ==
           THERON_DUNGEON_INVALID, "next(7) != INVALID");
    ASSERT(theron_v1_dungeon_next(THERON_DUNGEON_INVALID) ==
           THERON_DUNGEON_INVALID, "next(INVALID) != INVALID");

    PASS();
    return 1;
}

/* ── Test: item reset mode per dungeon ──────────────────────────── */

static int test_item_reset_mode(void) {
    TEST("Item reset mode — all dungeons use CHAMPION reset");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    for (int d = 1; d <= THERON_DUNGEON_COUNT; d++) {
        prog.current_dungeon = (Theron_DungeonID)d;
        ASSERT(prog.item_reset_mode == THERON_ITEM_RESET_MODE_CHAMPION,
               "reset mode not CHAMPION");
    }

    PASS();
    return 1;
}

/* ── Test: print function ─────────────────────────────────────────── */

static int test_print(void) {
    TEST("Dungeon progression print — no crash");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);
    theron_v1_dungeon_progression_print(&prog);

    /* Also test NULL */
    theron_v1_dungeon_progression_print(NULL);

    PASS();
    return 1;
}

/* ── Test: state names ───────────────────────────────────────────── */

static int test_state_names(void) {
    TEST("Dungeon state name lookup");

    ASSERT(strcmp(theron_v1_dungeon_state_name(THERON_DUNGEON_STATE_LOCKED),
                  "LOCKED") == 0, "LOCKED name wrong");
    ASSERT(strcmp(theron_v1_dungeon_state_name(THERON_DUNGEON_STATE_AVAILABLE),
                  "AVAILABLE") == 0, "AVAILABLE name wrong");
    ASSERT(strcmp(theron_v1_dungeon_state_name(THERON_DUNGEON_STATE_IN_PROGRESS),
                  "IN_PROGRESS") == 0, "IN_PROGRESS name wrong");
    ASSERT(strcmp(theron_v1_dungeon_state_name(THERON_DUNGEON_STATE_COMPLETE),
                  "COMPLETE") == 0, "COMPLETE name wrong");
    ASSERT(strcmp(theron_v1_dungeon_state_name(THERON_DUNGEON_STATE_COUNT),
                  "(unknown)") == 0, "COUNT name wrong");

    PASS();
    return 1;
}

/* ── Test: wrong-dungeon item rejection ───────────────────────────── */

static int test_wrong_dungeon_item_rejection(void) {
    TEST("Quest item rejection — item doesn't match current dungeon");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    /* We're in dungeon 3, try to collect dungeon 1's item */
    prog.current_dungeon = THERON_DUNGEON_3_ABYSS_OF_FLAMES;
    int result = theron_v1_quest_item_collect(&prog, THERON_QUEST_ITEM_1_SACRED_AMPLIFIER);
    ASSERT(result == -2, "wrong-dungeon item should be rejected with -2");

    /* Dungeon 3's correct item should be accepted */
    result = theron_v1_quest_item_collect(&prog, THERON_QUEST_ITEM_3_FLAME_ORBS);
    ASSERT(result == 1, "correct dungeon item should be collected");

    PASS();
    return 1;
}

/* ── Test: full sequence simulation ──────────────────────────────── */

static int test_full_sequence(void) {
    TEST("Full 7-dungeon sequence simulation");

    Theron_DungeonProgression prog;
    theron_v1_dungeon_progression_init(&prog);

    /* Simulate completing all 7 dungeons */
    for (int d = 1; d <= THERON_DUNGEON_COUNT; d++) {
        prog.current_dungeon = (Theron_DungeonID)d;

        /* Mark available */
        prog.dungeon_states[d - 1] = THERON_DUNGEON_STATE_AVAILABLE;

        /* Enter dungeon */
        int enter = theron_v1_dungeon_enter(&prog, (Theron_DungeonID)d);
        ASSERT(enter == 0, "dungeon enter failed");

        /* Apply item reset */
        ASSERT(theron_v1_item_reset_required(&prog, (Theron_DungeonID)d) == 1,
               "reset not required");
        theron_v1_item_reset_mark_applied(&prog);

        /* Collect quest item */
        Theron_QuestItem item = (Theron_QuestItem)(1 << (d - 1));
        int coll = theron_v1_quest_item_collect(&prog, item);
        ASSERT(coll == 1, "quest item collect failed");

        /* Mark complete and exit */
        prog.dungeon_states[d - 1] = THERON_DUNGEON_STATE_COMPLETE;
        Theron_DungeonID next = theron_v1_dungeon_exit(&prog);

        if (d < THERON_DUNGEON_COUNT) {
            ASSERT(next == (Theron_DungeonID)(d + 1), "wrong next dungeon");
        } else {
            ASSERT(next == THERON_DUNGEON_INVALID, "last dungeon exit != INVALID");
            ASSERT(prog.quest_complete == 1, "quest not complete");
        }
    }

    /* Verify all items collected */
    ASSERT(prog.quest_items_collected == THERON_QUEST_ALL_ITEMS,
           "not all items collected");

    PASS();
    return 1;
}

/* ── Test: source evidence ───────────────────────────────────────── */

static int test_source_evidence(void) {
    TEST("Source evidence string — non-empty");

    const char *ev = theron_v1_dungeon_progression_source_evidence();
    ASSERT(ev != NULL && strlen(ev) > 50, "source evidence too short");

    const char *ev2 = theron_v1_save_source_evidence();
    ASSERT(ev2 != NULL && strlen(ev2) > 20, "save source evidence too short");

    PASS();
    return 1;
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void) {
    printf("\n=== Theron V1 Phase 6 — Dungeon Progression Tests ===\n\n");

    int (*tests[])(void) = {
        test_dungeon_meta,
        test_init,
        test_dungeon_advance,
        test_quest_item_collect,
        test_item_reset,
        test_dungeon_exit,
        test_save_restore,
        test_quest_complete_detection,
        test_dungeon_names,
        test_dungeon_next,
        test_item_reset_mode,
        test_print,
        test_state_names,
        test_wrong_dungeon_item_rejection,
        test_full_sequence,
        test_source_evidence,
    };
    int n = (int)(sizeof(tests) / sizeof(tests[0]));

    for (int i = 0; i < n; i++) {
        if (!tests[i]()) {
            printf("*** TEST FAILED ***\n");
        }
    }

    printf("\n=====================================================\n");
    printf("Results: %d/%d passed\n", g_tests_passed, g_tests_run);
    printf("=====================================================\n\n");

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}