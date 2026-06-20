#include "firestaff/dm1/v1/box_champion_icons_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *file, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void test_table_values(void)
{
    const int *t = dm1_v1_box_champion_icons_table_pc34();
    int n = dm1_v1_box_champion_icons_size_pc34();
    CHECK(t != 0);
    CHECK(n == 16);
    CHECK(dm1_v1_box_champion_icons_box_count_pc34() == 4);
    /* Box 0 */
    CHECK(t[0] == 281);
    CHECK(t[1] == 299);
    CHECK(t[2] == 0);
    CHECK(t[3] == 13);
    /* Box 1 */
    CHECK(t[4] == 301);
    CHECK(t[5] == 319);
    CHECK(t[6] == 0);
    CHECK(t[7] == 13);
    /* Box 2 */
    CHECK(t[8]  == 301);
    CHECK(t[9]  == 319);
    CHECK(t[10] == 15);
    CHECK(t[11] == 28);
    /* Box 3 */
    CHECK(t[12] == 281);
    CHECK(t[13] == 299);
    CHECK(t[14] == 15);
    CHECK(t[15] == 28);
}

static void test_lookup_function(void)
{
    int b, v;
    for (b = 0; b < 4; ++b) {
        for (v = 0; v < 4; ++v) {
            CHECK(dm1_v1_box_champion_icons_get_pc34(b, v) >= 0);
        }
    }
    CHECK(dm1_v1_box_champion_icons_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_box_champion_icons_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_box_champion_icons_get_pc34(4, 0) == -1);
    CHECK(dm1_v1_box_champion_icons_get_pc34(0, 4) == -1);
    CHECK(dm1_v1_box_champion_icons_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_box_champion_icons_get_pc34(0, 0) == 281);
    CHECK(dm1_v1_box_champion_icons_get_pc34(3, 3) == 28);
}

static void test_run_accepted(void)
{
    DM1_V1_BoxChampionIconsResultPc34 r;
    int ok = dm1_v1_box_champion_icons_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 16);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.box0Left281Right299 == 1);
    CHECK(r.box1Left301Right319 == 1);
    CHECK(r.box2Left301Right319Top15 == 1);
    CHECK(r.box3Left281Right299Top15 == 1);
    CHECK(r.allXInByteRange == 1);
    CHECK(r.allYInByteRange == 1);
    CHECK(r.allWidthsInRange == 1);
    CHECK(r.allHeightsInRange == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 16; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_box_champion_icons_table_pc34()[i]);
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_box_champion_icons: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}