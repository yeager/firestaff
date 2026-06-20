#include "firestaff/dm1/v1/bitmap_arrow_pointer_pc34_compat.h"

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

static void test_plane_pointers(void)
{
    const unsigned char *p0 = dm1_v1_bitmap_arrow_pointer_plane_pc34(0);
    const unsigned char *p1 = dm1_v1_bitmap_arrow_pointer_plane_pc34(1);
    CHECK(p0 != 0);
    CHECK(p1 != 0);
    CHECK(p0[0] == 0xFF);
    CHECK(p0[65] == 0x00);
    CHECK(p1[0] == 0xFF);
    CHECK(p1[65] == 0x00);
    CHECK(dm1_v1_bitmap_arrow_pointer_plane_bytes_pc34() == 66);
    CHECK(dm1_v1_bitmap_arrow_pointer_plane_pc34(-1) == 0);
    CHECK(dm1_v1_bitmap_arrow_pointer_plane_pc34(2) == 0);
}

static void test_lookup_function(void)
{
    int i;
    for (i = 0; i < 66; ++i) {
        CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(0, i) >= 0);
        CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(1, i) >= 0);
    }
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(2, 0) == -1);
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(0, 66) == -1);
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(0, 0) == 0xFF);
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(0, 65) == 0x00);
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(1, 0) == 0xFF);
    CHECK(dm1_v1_bitmap_arrow_pointer_get_pc34(1, 65) == 0x00);
}

static void test_run_accepted(void)
{
    DM1_V1_BitmapArrowPointerResultPc34 r;
    int ok = dm1_v1_bitmap_arrow_pointer_run_pc34(&r);
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 9);
    CHECK(r.planeSizeBytes == 66);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.plane0FirstByte0xFF == 1);
    CHECK(r.plane0LastByte0x00 == 1);
    CHECK(r.plane1FirstByte0xFF == 1);
    CHECK(r.plane1LastByte0x00 == 1);
    CHECK(r.planesByteIdentical == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
}

int main(void)
{
    test_plane_pointers();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_bitmap_arrow_pointer: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}