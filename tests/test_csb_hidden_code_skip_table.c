/*
 * test_csb_hidden_code_skip_table.c
 *
 * Unit tests for the CSB / DM GRAPHICS.DAT hidden-code skip
 * table. See csb_hidden_code_skip_table.h for the spec.
 *
 * Each test exercises one of:
 *   - the in-source SelfTest
 *   - the table size / shape
 *   - positive cases (items that MUST skip)
 *   - negative cases (items that MUST NOT skip)
 *   - the diagnostic Why() helper
 *   - the PLATFORM_NONE wildcard (lookup-by-game-only)
 *
 * Run via:
 *   ctest --test-dir build -R csb_hidden_code_skip_table
 *
 * No game data required. Pure logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csb_hidden_code_skip_table.h"

#define ASSERT_TRUE(cond) do {                                            \
    if (!(cond)) {                                                        \
        fprintf(stderr, "ASSERTION FAILED at %s:%d: %s\n",               \
                __FILE__, __LINE__, #cond);                               \
        return 0;                                                          \
    }                                                                      \
} while (0)

/* ── shape ─────────────────────────────────────────────────────── */

static int test_table_size_is_six(void) {
    size_t count = 0;
    const FirestaffHiddenCodeEntry* t =
        FirestaffHiddenCodeSkipTable(&count);
    ASSERT_TRUE(t != NULL);
    /* 4 Atari/Amiga executable rows + 2 kid dungeon rows = 6 */
    ASSERT_TRUE(count == 6);
    return 1;
}

static int test_table_rows_have_valid_invariants(void) {
    size_t count = 0;
    const FirestaffHiddenCodeEntry* t =
        FirestaffHiddenCodeSkipTable(&count);
    for (size_t i = 0; i < count; ++i) {
        ASSERT_TRUE(t[i].first_index <= t[i].last_index);
        ASSERT_TRUE(t[i].note != NULL);
        ASSERT_TRUE(t[i].note[0] != '\0');
    }
    return 1;
}

/* ── positive cases ────────────────────────────────────────────── */

static int test_dm_atari_st_558_skips(void) {
    ASSERT_TRUE(FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 558));
    return 1;
}

static int test_dm_atari_st_562_skips(void) {
    ASSERT_TRUE(FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 562));
    return 1;
}

static int test_csb_atari_st_full_range_skips(void) {
    for (uint16_t i = 558; i <= 562; ++i) {
        ASSERT_TRUE(FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_CSB,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, i));
    }
    return 1;
}

static int test_dm_amiga_full_range_skips(void) {
    for (uint16_t i = 558; i <= 562; ++i) {
        ASSERT_TRUE(FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA, i));
    }
    return 1;
}

static int test_csb_amiga_full_range_skips(void) {
    for (uint16_t i = 558; i <= 562; ++i) {
        ASSERT_TRUE(FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_CSB,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA, i));
    }
    return 1;
}

static int test_amiga_kid_dungeon_range_skips(void) {
    /* DM1 Amiga v2.2 had a German retail variant where items
       135-138 were string keys, not graphics. */
    for (uint16_t i = 135; i <= 138; ++i) {
        ASSERT_TRUE(FirestaffHiddenCodeShouldSkip(
            FIRESTAFF_HIDDEN_CODE_GAME_DM,
            FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA, i));
    }
    return 1;
}

/* ── negative cases ────────────────────────────────────────────── */

static int test_normal_graphics_items_do_not_skip(void) {
    /* Items 100, 250, 670 are typical graphics items in DM/CSB
       GRAPHICS.DAT. They MUST NOT be flagged as hidden. */
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 100));
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 250));
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA, 670));
    return 1;
}

static int test_out_of_range_indices_do_not_skip(void) {
    /* 557 is just below the hidden range; 563 is just above. */
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 557));
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 563));
    /* 134 is just below the kid dungeon range; 139 is just above. */
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA, 134));
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_AMIGA, 139));
    return 1;
}

static int test_cross_game_isolation(void) {
    /* Atari ST range for DM1 must not skip on CSB if and only if
       the same range applies (it does for 558-562 -- we want the
       test to assert this for clarity). */
    /* CSB Atari ST 558 IS hidden. */
    ASSERT_TRUE(FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_CSB,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 558));
    /* PC 3.4 is NOT in our table; PLATFORM_NONE acts as a wildcard,
       so the lookup should still find the Atari ST / Amiga rows. */
    ASSERT_TRUE(FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_NONE, 560));
    return 1;
}

/* ── Why() ─────────────────────────────────────────────────────── */

static int test_why_returns_note_for_hidden(void) {
    const char* why = FirestaffHiddenCodeWhy(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 560);
    ASSERT_TRUE(why != NULL);
    /* The note must contain the word "executable" or "code" so
       log readers can spot it. */
    ASSERT_TRUE(strstr(why, "executable") != NULL ||
                strstr(why, "code") != NULL);
    return 1;
}

static int test_why_returns_null_for_normal(void) {
    ASSERT_TRUE(FirestaffHiddenCodeWhy(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 100) == NULL);
    return 1;
}

/* ── SelfTest wiring ───────────────────────────────────────────── */

static int test_self_test_runs(void) {
    ASSERT_TRUE(FirestaffHiddenCodeSkipTableSelfTest() == 0);
    return 1;
}

/* ── boundary / overflow safety ────────────────────────────────── */

static int test_extreme_indices_are_safe(void) {
    /* 0xFFFF should never be flagged. */
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 0xFFFFu));
    /* 0 should never be flagged. */
    ASSERT_TRUE(!FirestaffHiddenCodeShouldSkip(
        FIRESTAFF_HIDDEN_CODE_GAME_DM,
        FIRESTAFF_HIDDEN_CODE_PLATFORM_ATARI_ST, 0));
    return 1;
}

int main(void) {
    int passed = 0, total = 0;

    #define RUN(name) do {                                                 \
        total++;                                                           \
        if (name()) {                                                      \
            passed++;                                                      \
        } else {                                                           \
            fprintf(stderr, "test failed: %s\n", #name);                  \
        }                                                                   \
    } while (0)

    /* shape */
    RUN(test_table_size_is_six);
    RUN(test_table_rows_have_valid_invariants);

    /* positive */
    RUN(test_dm_atari_st_558_skips);
    RUN(test_dm_atari_st_562_skips);
    RUN(test_csb_atari_st_full_range_skips);
    RUN(test_dm_amiga_full_range_skips);
    RUN(test_csb_amiga_full_range_skips);
    RUN(test_amiga_kid_dungeon_range_skips);

    /* negative */
    RUN(test_normal_graphics_items_do_not_skip);
    RUN(test_out_of_range_indices_do_not_skip);
    RUN(test_cross_game_isolation);

    /* why */
    RUN(test_why_returns_note_for_hidden);
    RUN(test_why_returns_null_for_normal);

    /* self-test */
    RUN(test_self_test_runs);

    /* boundary */
    RUN(test_extreme_indices_are_safe);

    printf("test_csb_hidden_code_skip_table: %d/%d passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}
