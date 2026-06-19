#include "firestaff/dm1/v1/icon_graphic_first_icon_index_pc34_compat.h"

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
    /* DATA.C:253-260 G0026 init:
     *   { 0, 32, 64, 96, 128, 160, 192 }
     * The comment in DATA.C says "First icon index in graphic #42..#48".
     */
    const int *t = dm1_v1_icon_graphic_first_icon_index_table_pc34();
    int n = dm1_v1_icon_graphic_first_icon_index_size_pc34();
    CHECK(t != 0);
    CHECK(n == 7);
    CHECK(t[0] == 0);    /* graphic #42, first icon */
    CHECK(t[1] == 32);   /* graphic #43, first icon */
    CHECK(t[2] == 64);   /* graphic #44, first icon */
    CHECK(t[3] == 96);   /* graphic #45, first icon */
    CHECK(t[4] == 128);  /* graphic #46, first icon */
    CHECK(t[5] == 160);  /* graphic #47, first icon */
    CHECK(t[6] == 192);  /* graphic #48, first icon */
}

static void test_block_size(void)
{
    /* OBJECT.C:312-319 + OBJECT.C:455-467 — the loop walks G0026 with
     * a 32-icon stride (graphic block size). */
    CHECK(dm1_v1_icon_graphic_first_icon_index_block_size_pc34() == 32);
    CHECK(dm1_v1_icon_graphic_first_icon_index_first_graph_pc34() == 0);
    CHECK(dm1_v1_icon_graphic_first_icon_index_graph_count_pc34() == 7);
}

static void test_lookup_function(void)
{
    /* Replicates the OBJECT.C read site: G0026[c]. */
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(0) == 0);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(1) == 32);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(2) == 64);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(3) == 96);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(4) == 128);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(5) == 160);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(6) == 192);
    /* Out of range returns -1 (sentinel). */
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(-1) == -1);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(7) == -1);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(223) == -1);
    CHECK(dm1_v1_icon_graphic_first_icon_index_pc34(9999) == -1);
}

static void test_resolve_function(void)
{
    /* Replicates OBJECT.C:312-319 walk:
     *   for (c = 0; c < 7; ++c)
     *       if (G0026[c] > IconIndex) break;
     *   IconGraphicIndex = --c;
     *   IconIndex -= G0026[IconGraphicIndex];
     *
     * For each canonical index, assert (graph, within_block) is the
     * one the original C loop would compute.
     */
    int g;
    int w;

    /* Index 0 -> graph 0, within-block 0 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(0, &g, &w) == 1);
    CHECK(g == 0);
    CHECK(w == 0);

    /* Index 31 -> graph 0, within-block 31 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(31, &g, &w) == 1);
    CHECK(g == 0);
    CHECK(w == 31);

    /* Index 32 -> graph 1, within-block 0 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(32, &g, &w) == 1);
    CHECK(g == 1);
    CHECK(w == 0);

    /* Index 63 -> graph 1, within-block 31 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(63, &g, &w) == 1);
    CHECK(g == 1);
    CHECK(w == 31);

    /* Index 64 -> graph 2, within-block 0 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(64, &g, &w) == 1);
    CHECK(g == 2);
    CHECK(w == 0);

    /* Index 96 -> graph 3, within-block 0 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(96, &g, &w) == 1);
    CHECK(g == 3);
    CHECK(w == 0);

    /* Index 128 -> graph 4, within-block 0 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(128, &g, &w) == 1);
    CHECK(g == 4);
    CHECK(w == 0);

    /* Index 160 -> graph 5, within-block 0 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(160, &g, &w) == 1);
    CHECK(g == 5);
    CHECK(w == 0);

    /* Index 192 -> graph 6, within-block 0 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(192, &g, &w) == 1);
    CHECK(g == 6);
    CHECK(w == 0);

    /* Index 223 (last valid) -> graph 6, within-block 31 */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(223, &g, &w) == 1);
    CHECK(g == 6);
    CHECK(w == 31);

    /* Index 224 (just past last) -> reject */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(224, &g, &w) == 0);
    CHECK(g == -1);
    CHECK(w == -1);

    /* Negative index -> reject */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(-1, &g, &w) == 0);
    CHECK(g == -1);
    CHECK(w == -1);
}

static void test_resolve_null_safety(void)
{
    /* Out-param NULL is allowed and must not crash. */
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(64, 0, 0) == 1);
    CHECK(dm1_v1_icon_graphic_first_icon_index_resolve_pc34(-1, 0, 0) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_IconGraphicFirstIconIndexResultPc34 r;
    int ok = dm1_v1_icon_graphic_first_icon_index_run_pc34(&r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 14);
    CHECK(r.tableSize == 7);
    CHECK(r.tableEntries[0] == 0);
    CHECK(r.tableEntries[1] == 32);
    CHECK(r.tableEntries[2] == 64);
    CHECK(r.tableEntries[3] == 96);
    CHECK(r.tableEntries[4] == 128);
    CHECK(r.tableEntries[5] == 160);
    CHECK(r.tableEntries[6] == 192);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstBlockStartZero == 1);
    CHECK(r.lastBlockStart192 == 1);
    CHECK(r.monotonicIncreasing32 == 1);
    CHECK(r.allWithinIconRange == 1);
    CHECK(r.lookupGraph42_Index0 == 1);
    CHECK(r.lookupGraph43_Index32 == 1);
    CHECK(r.lookupGraph44_Index64 == 1);
    CHECK(r.lookupGraph45_Index96 == 1);
    CHECK(r.lookupGraph46_Index128 == 1);
    CHECK(r.lookupGraph47_Index160 == 1);
    CHECK(r.lookupGraph48_Index192 == 1);
    CHECK(r.outOfRangeReturnsMinusOne == 1);
    CHECK(r.declarationMatchesInit == 1);
}

int main(void)
{
    test_table_values();
    test_block_size();
    test_lookup_function();
    test_resolve_function();
    test_resolve_null_safety();
    test_run_accepted();
    printf("dm1_v1_icon_graphic_first_icon_index: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
