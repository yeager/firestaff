#include "csb_v1_viewport_f0115_projectile_metadata_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const int k_ordinals[] = {
    CSB_V1_F0115_PROJECTILE_ORDINAL_M715,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M716,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M717,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M718,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M719,
    CSB_V1_F0115_PROJECTILE_ORDINAL_M720
};

static int g_assertions = 0;
static int g_failures = 0;

static int check_int(const char *label, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
        return 0;
    }
    printf("CHECK %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int check_true(const char *label, int condition, const char *anchor)
{
    return check_int(label, condition ? 1 : 0, 1, anchor);
}

static int check_contains(const char *label, const char *haystack,
                          const char *needle, const char *anchor)
{
    return check_true(label, haystack && needle && strstr(haystack, needle) != NULL,
                      anchor);
}

static int test_fixture_shape(void)
{
    int ok = 1;

    ok &= check_int("fixture.table_count",
                    (int)csb_v1_viewport_f0115_projectile_metadata_table_count(),
                    12,
                    "ReDMCSB DUNVIEW.C:1511-1516 G0210 first six projectile aspects");
    ok &= check_int("fixture.min_bitmap",
                    csb_v1_viewport_f0115_projectile_metadata_min_bitmap_pc34(),
                    454,
                    "ReDMCSB DEFS.H:2388 M613_GRAPHIC_FIRST_PROJECTILE");
    ok &= check_int("fixture.max_bitmap",
                    csb_v1_viewport_f0115_projectile_metadata_max_bitmap_pc34(),
                    670,
                    "ReDMCSB DEFS.H:2393 M719_GRAPHIC_FIRST_SOUND");
    ok &= check_true("fixture.first_entry",
                     csb_v1_viewport_f0115_projectile_metadata_table_entry(0) != NULL,
                     "ReDMCSB DUNVIEW.C:5691-5694 projectile aspect table lookup");
    ok &= check_true("fixture.out_of_range_entry",
                     csb_v1_viewport_f0115_projectile_metadata_table_entry(12) == NULL,
                     "fixture has exactly 12 ordinal/side records");

    return ok;
}

static int test_every_ordinal_side_tuple(void)
{
    int ok = 1;

    for (size_t i = 0; i < sizeof(k_ordinals) / sizeof(k_ordinals[0]); ++i) {
        const CSB_V1_ViewportF0115ProjectileMetadataPc34 *left =
            csb_v1_viewport_f0115_projectile_metadata_lookup(
                k_ordinals[i], CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
                CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR);
        const CSB_V1_ViewportF0115ProjectileMetadataPc34 *right =
            csb_v1_viewport_f0115_projectile_metadata_lookup(
                k_ordinals[i], CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
                CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR);

        ok &= check_true("tuple.left.non_null", left != NULL,
                         "ReDMCSB DUNVIEW.C:5691-5694 ordinal to G0210 row");
        ok &= check_true("tuple.right.non_null", right != NULL,
                         "ReDMCSB DUNVIEW.C:5807 bitmap delta for facing side");
        ok &= check_int("tuple.left.ordinal", left ? left->ordinal : -1, k_ordinals[i],
                        "DUNGEON.C:1164-1222 projectile aspect ordinal");
        ok &= check_int("tuple.right.ordinal", right ? right->ordinal : -1, k_ordinals[i],
                        "DUNGEON.C:1164-1222 projectile aspect ordinal");
        ok &= check_int("tuple.left.transparent", left ? left->transparentFlag : 0, 1,
                        "ReDMCSB DUNVIEW.C:5881-5882 F0791 C10");
        ok &= check_int("tuple.right.transparent", right ? right->transparentFlag : 0, 1,
                        "ReDMCSB DUNVIEW.C:5881-5882 F0791 C10");
        ok &= check_int("tuple.left.double_width", left ? left->doubleWidthFlag : 0, 1,
                        "ReDMCSB DEFS.H:2037-2044 PROJECTIL_ASPECT metadata");
        ok &= check_int("tuple.right.double_width", right ? right->doubleWidthFlag : 0, 1,
                        "ReDMCSB DEFS.H:2046-2055 projectile GraphicInfo bits");
    }

    return ok;
}

static int test_lr_bitmap_pairs(void)
{
    int ok = 1;

    for (size_t i = 0; i < sizeof(k_ordinals) / sizeof(k_ordinals[0]); ++i) {
        const CSB_V1_ViewportF0115ProjectileMetadataPc34 *left =
            csb_v1_viewport_f0115_projectile_metadata_lookup(
                k_ordinals[i], CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
                CSB_V1_F0115_PROJECTILE_COORDINATE_SET_MIDDLE);
        const CSB_V1_ViewportF0115ProjectileMetadataPc34 *right =
            csb_v1_viewport_f0115_projectile_metadata_lookup(
                k_ordinals[i], CSB_V1_F0115_PROJECTILE_SIDE_RIGHT,
                CSB_V1_F0115_PROJECTILE_COORDINATE_SET_MIDDLE);

        ok &= check_int("lr.bitmap_delta",
                        (left && right) ? right->bitmapIndex - left->bitmapIndex : -99,
                        1,
                        "ReDMCSB DUNVIEW.C:5807 AL0127_i_NativeBitmapIndex += delta");
        ok &= check_int("lr.zorder_delta",
                        (left && right) ? right->zOrder - left->zOrder : -99,
                        1,
                        "ReDMCSB DUNVIEW.C:5683 C2900_ZONE_ + row*4+ViewCell");
    }

    return ok;
}

static int test_coordinate_sets_and_zorder(void)
{
    int ok = 1;

    for (size_t i = 0; i < sizeof(k_ordinals) / sizeof(k_ordinals[0]); ++i) {
        const CSB_V1_ViewportF0115ProjectileMetadataPc34 *near_meta =
            csb_v1_viewport_f0115_projectile_metadata_lookup(
                k_ordinals[i], CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
                CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR);
        const CSB_V1_ViewportF0115ProjectileMetadataPc34 *middle_meta =
            csb_v1_viewport_f0115_projectile_metadata_lookup(
                k_ordinals[i], CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
                CSB_V1_F0115_PROJECTILE_COORDINATE_SET_MIDDLE);
        const CSB_V1_ViewportF0115ProjectileMetadataPc34 *far_meta =
            csb_v1_viewport_f0115_projectile_metadata_lookup(
                k_ordinals[i], CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
                CSB_V1_F0115_PROJECTILE_COORDINATE_SET_FAR);

        ok &= check_true("coordinate.distinct.near_middle",
                         near_meta && middle_meta &&
                             near_meta->bitmapIndex != middle_meta->bitmapIndex,
                         "ReDMCSB DUNVIEW.C:5710-5722 coordinate scale index");
        ok &= check_true("coordinate.distinct.middle_far",
                         middle_meta && far_meta &&
                             middle_meta->bitmapIndex != far_meta->bitmapIndex,
                         "ReDMCSB DUNVIEW.C:5816/5865 derived bitmap metadata");
        ok &= check_true("zorder.near_before_middle",
                         near_meta && middle_meta && near_meta->zOrder < middle_meta->zOrder,
                         "ReDMCSB DUNVIEW.C:5683 row*4+ViewCell ordering");
        ok &= check_true("zorder.middle_before_far",
                         middle_meta && far_meta && middle_meta->zOrder < far_meta->zOrder,
                         "CSB lineage Viewport.cpp:1903-1915 F1 room-object order");
    }

    return ok;
}

static int test_bitmap_range(void)
{
    int ok = 1;
    const int min_bitmap = csb_v1_viewport_f0115_projectile_metadata_min_bitmap_pc34();
    const int max_bitmap = csb_v1_viewport_f0115_projectile_metadata_max_bitmap_pc34();

    for (size_t i = 0; i < sizeof(k_ordinals) / sizeof(k_ordinals[0]); ++i) {
        for (int side = CSB_V1_F0115_PROJECTILE_SIDE_LEFT;
             side <= CSB_V1_F0115_PROJECTILE_SIDE_RIGHT; ++side) {
            for (int coordinateSet = CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR;
                 coordinateSet <= CSB_V1_F0115_PROJECTILE_COORDINATE_SET_FAR;
                 ++coordinateSet) {
                const CSB_V1_ViewportF0115ProjectileMetadataPc34 *meta =
                    csb_v1_viewport_f0115_projectile_metadata_lookup(
                        k_ordinals[i], side, coordinateSet);
                ok &= check_true("bitmap.in_projectile_range",
                                 meta && meta->bitmapIndex >= min_bitmap &&
                                     meta->bitmapIndex <= max_bitmap,
                                 "ReDMCSB DEFS.H:2388/2393 M613..M719");
            }
        }
    }

    return ok;
}

static int test_null_unknown_and_evidence(void)
{
    int ok = 1;
    const char *evidence =
        csb_v1_viewport_f0115_projectile_metadata_source_evidence_pc34();

    ok &= check_true("null.unknown_ordinal",
                     csb_v1_viewport_f0115_projectile_metadata_lookup(
                         999, CSB_V1_F0115_PROJECTILE_SIDE_LEFT,
                         CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR) == NULL,
                     "unknown projectile ordinal returns no metadata");
    ok &= check_true("null.bad_side",
                     csb_v1_viewport_f0115_projectile_metadata_lookup(
                         CSB_V1_F0115_PROJECTILE_ORDINAL_M715, 99,
                         CSB_V1_F0115_PROJECTILE_COORDINATE_SET_NEAR) == NULL,
                     "F0115 has only left/right projectile side metadata");
    ok &= check_true("null.bad_coordinate_set",
                     csb_v1_viewport_f0115_projectile_metadata_lookup(
                         CSB_V1_F0115_PROJECTILE_ORDINAL_M715,
                         CSB_V1_F0115_PROJECTILE_SIDE_LEFT, 3) == NULL,
                     "coordinateSet fixture accepts only 0/1/2");
    ok &= check_contains("evidence.f0115_table", evidence, "F0115:5691-5694",
                         "ReDMCSB DUNVIEW.C projectile ordinal table read");
    ok &= check_contains("evidence.f0116_dispatch", evidence, "F0116:6361-6480",
                         "ReDMCSB DUNVIEW.C D3 dispatch follow-up");
    ok &= check_contains("evidence.defs_projectil", evidence, "PROJECTIL_ASPECT",
                         "ReDMCSB DEFS.H:2037-2044");
    ok &= check_contains("evidence.csb_lineage", evidence, "Viewport.cpp:1903-1915",
                         "CSB lineage projectile/room-object dispatch");

    return ok;
}

int main(void)
{
    int ok = 1;

    ok &= test_fixture_shape();
    ok &= test_every_ordinal_side_tuple();
    ok &= test_lr_bitmap_pairs();
    ok &= test_coordinate_sets_and_zorder();
    ok &= test_bitmap_range();
    ok &= test_null_unknown_and_evidence();

    if (g_failures == 0 && ok) {
        printf("PASS test_csb_v1_viewport_f0115_projectile_metadata_pc34_compat "
               "assertions=%d failures=0\n",
               g_assertions);
        return 0;
    }

    printf("FAIL test_csb_v1_viewport_f0115_projectile_metadata_pc34_compat "
           "assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return 1;
}
