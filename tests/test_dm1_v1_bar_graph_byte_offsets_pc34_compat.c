#include "firestaff/dm1/v1/bar_graph_byte_offsets_pc34_compat.h"

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
    const unsigned int *t = dm1_v1_bar_graph_byte_offsets_table_pc34();
    int n = dm1_v1_bar_graph_byte_offsets_size_pc34();
    CHECK(t != 0);
    CHECK(n == 12);
    /* Champion 0 health/mana/stamina = 16/24/24. */
    CHECK(t[0] == 16);
    CHECK(t[1] == 24);
    CHECK(t[2] == 24);
    /* Champion 3 health/mana/stamina = 120/128/128. */
    CHECK(t[9]  == 120);
    CHECK(t[10] == 128);
    CHECK(t[11] == 128);
}

static void test_lookup_function(void)
{
    int c, g;
    for (c = 0; c < 4; ++c) {
        for (g = 0; g < 3; ++g) {
            CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(c, g) >= 0);
            CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(c, g) <= 0xFFFF);
        }
    }
    CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(4, 0) == -1);
    CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(0, 3) == -1);
    CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(0, 0) == 16);
    CHECK(dm1_v1_bar_graph_byte_offsets_get_pc34(3, 2) == 128);
}

static void test_run_accepted(void)
{
    DM1_V1_BarGraphByteOffsetsResultPc34 r;
    int ok = dm1_v1_bar_graph_byte_offsets_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableSize == 12);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.champion0HealthOffset16 == 1);
    CHECK(r.champion1HealthOffset56 == 1);
    CHECK(r.champion2HealthOffset88 == 1);
    CHECK(r.champion3HealthOffset120 == 1);
    CHECK(r.allOffsetsNonNegative == 1);
    CHECK(r.allOffsetsInByteRange == 1);
    CHECK(r.monotonicPerChampion == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 12; ++i) {
        CHECK(r.tableEntries[i] == (int)dm1_v1_bar_graph_byte_offsets_table_pc34()[i]);
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_bar_graph_byte_offsets: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}