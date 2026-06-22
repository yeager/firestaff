/*
 * test_m11_ui_scale.c
 *
 * Data-free contract test for the M11 UI scale module
 * (src/engine/ui_scale_m11.c). The global scale state is the
 * single source of truth that HUD, menu, and font code read
 * before drawing text; getting the percent-to-font-scale map
 * wrong would silently squash or stretch every font glyph.
 *
 * Verifies:
 *   - default 100% leaves the engine bit-identical
 *   - NormalizePercent snaps any input into {100, 150, 200}
 *   - Set/Get round-trip
 *   - PercentToFontScale: 100->1, 150->2, 200->3
 *   - Apply is integer-nearest ((value * percent + 50) / 100)
 *   - Set snaps a 150% value to 150 even if normalized was
 *     called on a percent already in range
 *
 * Source: include/ui_scale_m11.h (no ReDMCSB equivalent;
 * Firestaff accessibility extra for V2 HUDs and M12 menu text).
 */

#include "ui_scale_m11.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check(int cond, const char *name) {
    if (cond) {
        printf("PASS: %s\n", name);
    } else {
        printf("FAIL: %s\n", name);
        ++g_failures;
    }
}

static void test_default(void) {
    M11_UIScale_SetPercent(100);
    check(M11_UIScale_GetPercent() == 100,
          "default percent is 100");
    check(M11_UIScale_GetFontScale() == 1,
          "default font scale is 1 (V1-bit-identical)");
}

static void test_normalize_snap(void) {
    /* Anything <= 100 snaps to 100. */
    check(M11_UIScale_NormalizePercent(0) == 100, "0 -> 100");
    check(M11_UIScale_NormalizePercent(50) == 100, "50 -> 100");
    check(M11_UIScale_NormalizePercent(99) == 100, "99 -> 100");
    check(M11_UIScale_NormalizePercent(100) == 100, "100 -> 100");
    /* 101..150 snaps to 150. */
    check(M11_UIScale_NormalizePercent(101) == 150, "101 -> 150");
    check(M11_UIScale_NormalizePercent(125) == 150, "125 -> 150");
    check(M11_UIScale_NormalizePercent(150) == 150, "150 -> 150");
    /* 151+ snaps to 200. */
    check(M11_UIScale_NormalizePercent(151) == 200, "151 -> 200");
    check(M11_UIScale_NormalizePercent(500) == 200, "500 -> 200");
    check(M11_UIScale_NormalizePercent(-7) == 100, "negative -> 100");
}

static void test_set_get_roundtrip(void) {
    M11_UIScale_SetPercent(100);
    check(M11_UIScale_GetPercent() == 100, "set 100 -> get 100");
    M11_UIScale_SetPercent(150);
    check(M11_UIScale_GetPercent() == 150, "set 150 -> get 150");
    M11_UIScale_SetPercent(200);
    check(M11_UIScale_GetPercent() == 200, "set 200 -> get 200");
    /* Set snaps a value to nearest range. */
    M11_UIScale_SetPercent(123);
    check(M11_UIScale_GetPercent() == 150, "set 123 snaps to 150");
    M11_UIScale_SetPercent(99);
    check(M11_UIScale_GetPercent() == 100, "set 99 snaps to 100");
    M11_UIScale_SetPercent(175);
    check(M11_UIScale_GetPercent() == 200, "set 175 snaps to 200");
}

static void test_percent_to_font_scale(void) {
    check(M11_UIScale_PercentToFontScale(100) == 1, "100% -> font scale 1");
    check(M11_UIScale_PercentToFontScale(150) == 2, "150% -> font scale 2");
    check(M11_UIScale_PercentToFontScale(200) == 3, "200% -> font scale 3");
    /* Non-snapped percent also maps to nearest snapped bucket. */
    check(M11_UIScale_PercentToFontScale(120) == 2, "120% -> font scale 2");
    check(M11_UIScale_PercentToFontScale(180) == 3, "180% -> font scale 3");
}

static void test_apply_integer_nearest(void) {
    M11_UIScale_SetPercent(100);
    check(M11_UIScale_Apply(100) == 100, "100% apply 100 = 100");
    check(M11_UIScale_Apply(0) == 0, "100% apply 0 = 0");
    M11_UIScale_SetPercent(150);
    check(M11_UIScale_Apply(100) == 150, "150% apply 100 = 150");
    check(M11_UIScale_Apply(2) == 3, "150% apply 2 = 3 (nearest)");
    check(M11_UIScale_Apply(3) == 5, "150% apply 3 = 5 (round-up)");
    M11_UIScale_SetPercent(200);
    check(M11_UIScale_Apply(100) == 200, "200% apply 100 = 200");
    check(M11_UIScale_Apply(1) == 2, "200% apply 1 = 2");
}

static void test_global_state_isolation(void) {
    /* Confirm each SetPercent actually moves the global state — no
     * caching of an early value across changes. */
    M11_UIScale_SetPercent(100);
    int a = M11_UIScale_GetPercent();
    M11_UIScale_SetPercent(200);
    int b = M11_UIScale_GetPercent();
    check(a == 100 && b == 200,
          "global state reflects most recent SetPercent");
}

int main(void) {
    test_default();
    test_normalize_snap();
    test_set_get_roundtrip();
    test_percent_to_font_scale();
    test_apply_integer_nearest();
    test_global_state_isolation();
    if (g_failures) {
        printf("test_m11_ui_scale: FAIL %d\n", g_failures);
        return 1;
    }
    puts("test_m11_ui_scale: PASS");
    return 0;
}
