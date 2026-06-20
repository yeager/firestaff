#include "firestaff/dm1/v1/mandatory_graphic_indices_pc34_compat.h"

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
    const int *t = dm1_v1_mandatory_graphic_indices_table_pc34();
    int n = dm1_v1_mandatory_graphic_indices_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 70);
    CHECK(t[0] == 7);     /* C007_GRAPHIC_STATUS_BOX */
    CHECK(t[19] == 42);   /* C042_GRAPHIC_OBJECT_ICONS_000_TO_031 */
    CHECK(t[39] == 62);   /* M767_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C */
    CHECK(t[46] == 69);   /* C069_GRAPHIC_FIELD_MASK_D3R */
    CHECK(t[49] == 72);   /* C072_GRAPHIC_FIELD_MASK_D0R */
    CHECK(t[62] == 28);   /* C028_GRAPHIC_CHAMPION_ICONS */
    CHECK(t[63] == 29);   /* C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE */
    CHECK(t[69] == 39);   /* C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD (last) */
    for (i = 0; i < n; ++i) {
        CHECK(t[i] >= 0);
    }
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 70; ++i) {
        CHECK(dm1_v1_mandatory_graphic_indices_get_pc34(i) >= 0);
    }
    CHECK(dm1_v1_mandatory_graphic_indices_get_pc34(-1) == -1);
    CHECK(dm1_v1_mandatory_graphic_indices_get_pc34(70) == -1);
    CHECK(dm1_v1_mandatory_graphic_indices_get_pc34(999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_mandatory_graphic_indices_first_pc34() == 7);
    CHECK(dm1_v1_mandatory_graphic_indices_last_pc34() == 39);
}

static void test_run_accepted(void)
{
    DM1_V1_MandatoryGraphicIndicesResultPc34 r;
    int ok = dm1_v1_mandatory_graphic_indices_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableSize == 70);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.firstEntryStatusBox == 1);
    CHECK(r.lastEntryBorderPartySpellshield == 1);
    CHECK(r.allValuesNonNegative == 1);
    CHECK(r.allValuesDistinct == 1);
    CHECK(r.firstIconBase42 == 1);
    CHECK(r.lastFloorPitInvisibleD0C == 1);
    CHECK(r.fieldMaskStride == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 70; ++i) {
        CHECK(r.tableEntries[i] == dm1_v1_mandatory_graphic_indices_get_pc34(i));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_mandatory_graphic_indices: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}