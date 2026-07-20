/*
 * test_m12_menu_row_hit_height_audit — launcher menu-row hit-height
 * audit at fontScale 1..3, the launcher half of the remaining "UI
 * scaling and touch-target audit across launcher and game views" TODO
 * (sibling to the shipped m12_touch_layout_audit preset audit).
 *
 * Audits every launcher menu-row surface through the shared
 * menu_row_metrics_m12.h contract (single source of truth consumed by
 * both draw paths):
 *
 *   legacy 320x200 launcher (menu_startup_m12.c):
 *     - settings view (classic)  — scale-aware pitch, overlapping rows
 *     - settings view (dense)    — fixed 34 px pitch
 *     - save browser             — scale-aware pitch
 *   modern 1080p launcher (menu_startup_render_modern_m12.c):
 *     - settings rows 50/70, tab strip 34 (fontScale-independent)
 *
 * Checks per surface and fontScale 1..3:
 *   1. containment — the presented label (conservative 11-row Unicode
 *      glyph bound at the m12_effective_text_scale-resolved scale)
 *      always fits inside the effective hit height (row pitch);
 *   2. classification — the presented hit height at presentation
 *      scales 1x..4x is classified against the 24 px floor / 44 px
 *      recommendation, cross-checked through the fs_gesture zone audit
 *      (the same contract m12_touch_layout_audit uses);
 *   3. per-row decisions — legacy rows are source-space small by V1
 *      parity design: at 1x they sit below the floor, at >=2x
 *      presentation scale every row must clear the 24 px floor.
 *
 * Data-free: pure arithmetic over shared constants, no assets, no SDL.
 */

#include "menu_row_metrics_m12.h"
#include "fs_gesture_navigation_gate.h"

#include <stdio.h>

static int g_failures = 0;

#define CHECK(cond_, msg_) do {                                            \
    if (!(cond_)) {                                                        \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, msg_);          \
        g_failures++;                                                      \
    }                                                                      \
} while (0)

/* ── 1. constants sanity + cross-checks ─────────────────────────────── */
static void test_constants(void) {
    printf("[constants]\n");
    CHECK(M12_MENU_ROW_MIN_TARGET_PX == FS_GG_PLATFORM_MIN_TARGET_PX,
          "menu-row floor must mirror the fs_gesture platform floor");
    CHECK(M12_MENU_ROW_RECOMMENDED_TARGET_PX == FS_GG_PLATFORM_RECOMMENDED_PX,
          "menu-row recommendation must mirror the fs_gesture recommendation");
    CHECK(M12_MENU_ROW_SETTINGS_CLASSIC_BASE_PITCH == 18,
          "classic settings base pitch is the shipped 18 px");
    CHECK(M12_MENU_ROW_SETTINGS_CLASSIC_FRAME >= M12_MENU_ROW_SETTINGS_CLASSIC_BASE_PITCH,
          "classic settings frame must cover the base pitch");
    CHECK(M12_MENU_ROW_SETTINGS_DENSE_PITCH == 34,
          "dense settings pitch is the shipped 34 px");
    CHECK(M12_MENU_ROW_SAVE_BROWSER_BASE_PITCH == 22,
          "save-browser base pitch is the shipped 22 px");
    CHECK(M12_MENU_ROW_MODERN_SETTINGS_HEIGHT == 50 &&
          M12_MENU_ROW_MODERN_SETTINGS_PITCH == 70 &&
          M12_MENU_ROW_MODERN_TAB_HEIGHT == 34,
          "modern settings/tab geometry is the shipped 50/70/34");
    CHECK(M12_MENU_ASCII_GLYPH_ROWS == 7 && M12_MENU_UNICODE_GLYPH_ROWS == 11,
          "glyph bounds are the shipped 7-row ASCII / 11-row Unicode tables");
}

/* ── 2. effective-text-scale rule mirrors the draw path ─────────────── */
static void test_effective_text_scale_rule(void) {
    static const int expected[4][3] = {
        /* baseScale 1..4 x fontScale 1..3 (clamped 4) */
        {1, 2, 3},
        {2, 3, 4},
        {3, 4, 4},
        {4, 4, 4},
    };
    int base;
    int fs;
    printf("[effective-text-scale]\n");
    for (base = 1; base <= 4; ++base) {
        for (fs = 1; fs <= 3; ++fs) {
            CHECK(m12_menu_row_effective_text_scale(base, fs) == expected[base - 1][fs - 1],
                  "effective text scale must follow base + fontScale - 1 (clamp 4)");
        }
    }
    /* Out-of-range fontScale input clamps to [1, 3]. */
    CHECK(m12_menu_row_effective_text_scale(1, 0) == 1,
          "fontScale 0 clamps to 1");
    CHECK(m12_menu_row_effective_text_scale(1, 9) == 3,
          "fontScale 9 clamps to 3");
    CHECK(m12_menu_row_effective_text_scale(0, 2) == 1,
          "non-positive base scale resolves to 1 like the draw path");
}

/* ── 3. containment at every fontScale ──────────────────────────────── */
static void test_containment(void) {
    int fs;
    printf("[containment]\n");
    for (fs = 1; fs <= 3; ++fs) {
        int labelH = m12_menu_row_label_height_px(fs);
        int classicPitch = m12_menu_row_settings_classic_pitch(fs);
        int savePitch = m12_menu_row_save_browser_pitch(fs);
        printf("  fontScale=%d label=%dpx classic=%dpx dense=%dpx save=%dpx\n",
               fs, labelH, classicPitch, M12_MENU_ROW_SETTINGS_DENSE_PITCH,
               savePitch);
        /* Classic settings: label + pads must fit the pitch. */
        CHECK(M12_MENU_ROW_SETTINGS_CLASSIC_TEXT_TOP + labelH
                  + M12_MENU_ROW_SETTINGS_CLASSIC_TEXT_BOTTOM <= classicPitch,
              "classic settings pitch must contain the presented label");
        /* Dense settings: fixed pitch must contain the label. */
        CHECK(labelH <= M12_MENU_ROW_SETTINGS_DENSE_PITCH,
              "dense settings pitch must contain the presented label");
        /* Save browser: label + pads must fit the pitch. */
        CHECK(M12_MENU_ROW_SAVE_BROWSER_TEXT_TOP + labelH
                  + M12_MENU_ROW_SAVE_BROWSER_TEXT_BOTTOM <= savePitch,
              "save-browser pitch must contain the presented label");
        /* Visible-row count stays within the shipped bounds. */
        CHECK(m12_menu_row_settings_classic_visible_rows(fs) >= 1 &&
                  m12_menu_row_settings_classic_visible_rows(fs)
                      <= M12_MENU_ROW_SETTINGS_CLASSIC_MAX_VISIBLE,
              "classic settings visible rows within [1, max]");
        /* The pitch never shrinks below the shipped base (fontScale 1
         * stays bit-identical to the pre-audit layout). */
        CHECK(classicPitch >= M12_MENU_ROW_SETTINGS_CLASSIC_BASE_PITCH,
              "classic pitch never below the shipped base");
        CHECK(savePitch >= M12_MENU_ROW_SAVE_BROWSER_BASE_PITCH,
              "save-browser pitch never below the shipped base");
    }
    /* fontScale 1 pins the exact pre-audit layout. */
    CHECK(m12_menu_row_settings_classic_pitch(1) == 18 &&
              m12_menu_row_settings_classic_visible_rows(1) == 6,
          "fontScale 1 classic layout is bit-identical (18 px pitch, 6 rows)");
    CHECK(m12_menu_row_save_browser_pitch(1) == 22,
          "fontScale 1 save-browser layout is bit-identical (22 px pitch)");
}

/* ── 4. presented-height classification + fs_gesture cross-check ────── */
static void audit_presented(const char* name, int sourceHeight, int presentScale) {
    FsGestureZone zone;
    FsGestureZoneAuditReport report;
    int presented = sourceHeight * presentScale;
    M12_MenuRowFit fit = m12_menu_row_classify(presented);

    zone.x = 0;
    zone.y = 0;
    zone.w = 200;
    zone.h = presented;
    zone.groupName = name;
    (void)fs_gesture_audit_zones(&zone, 1,
                                 FS_GG_PLATFORM_MIN_TARGET_PX,
                                 FS_GG_PLATFORM_RECOMMENDED_PX,
                                 &report);
    CHECK(report.totalZones == 1, "one zone audited");
    /* The metric classification and the gesture-audit verdict must
     * agree on the 24 px floor. */
    CHECK((fit != M12_MENU_ROW_FIT_BELOW_MINIMUM)
              == (report.zonesBelowMinimum == 0),
          "menu-row classification and fs_gesture floor verdict disagree");
    CHECK((fit == M12_MENU_ROW_FIT_RECOMMENDED)
              == (report.zonesBelowRecommended == 0),
          "menu-row classification and fs_gesture recommendation disagree");
}

static void test_presented_classification(void) {
    int fs;
    int scale;
    printf("[presented-classification]\n");
    for (fs = 1; fs <= 3; ++fs) {
        for (scale = 1; scale <= 4; ++scale) {
            audit_presented("settings.classic",
                            m12_menu_row_settings_classic_pitch(fs), scale);
            audit_presented("settings.dense",
                            M12_MENU_ROW_SETTINGS_DENSE_PITCH, scale);
            audit_presented("save_browser",
                            m12_menu_row_save_browser_pitch(fs), scale);
        }
    }
    /* Modern surface is native-resolution and fontScale-independent. */
    audit_presented("modern.settings", M12_MENU_ROW_MODERN_SETTINGS_HEIGHT, 1);
    audit_presented("modern.tab", M12_MENU_ROW_MODERN_TAB_HEIGHT, 1);

    /* ── Per-row decisions (the TODO's "decide per-zone" contract) ──
     * Legacy rows are V1-parity source-space small: at 1x presentation
     * they sit below the 24 px floor by design; the launcher presents
     * the 320x200 framebuffer at >= 2x on every shipped window mode,
     * so every row must clear the floor at >= 2x. */
    for (fs = 1; fs <= 3; ++fs) {
        CHECK(m12_menu_row_classify(m12_menu_row_settings_classic_pitch(fs) * 2)
                  != M12_MENU_ROW_FIT_BELOW_MINIMUM,
              "classic settings row clears the floor at 2x presentation");
        CHECK(m12_menu_row_classify(M12_MENU_ROW_SETTINGS_DENSE_PITCH * 2)
                  != M12_MENU_ROW_FIT_BELOW_MINIMUM,
              "dense settings row clears the floor at 2x presentation");
        CHECK(m12_menu_row_classify(m12_menu_row_save_browser_pitch(fs) * 2)
                  != M12_MENU_ROW_FIT_BELOW_MINIMUM,
              "save-browser row clears the floor at 2x presentation");
        /* Decision record: at 1x the classic row is below the floor. */
        CHECK(m12_menu_row_classify(m12_menu_row_settings_classic_pitch(1) * 1)
                  == M12_MENU_ROW_FIT_BELOW_MINIMUM,
              "1x classic row is documented below-floor (V1 parity)");
    }
    /* Modern decisions: settings rows meet the 44 px recommendation
     * natively; the tab strip meets the 24 px floor but stays below
     * the recommendation (accepted: secondary navigation, pointer
     * targets span the full tab width). */
    CHECK(m12_menu_row_classify(M12_MENU_ROW_MODERN_SETTINGS_HEIGHT)
              == M12_MENU_ROW_FIT_RECOMMENDED,
          "modern settings rows meet the recommendation natively");
    CHECK(m12_menu_row_classify(M12_MENU_ROW_MODERN_TAB_HEIGHT)
              == M12_MENU_ROW_FIT_MINIMUM,
          "modern tab strip meets the floor (accepted below recommendation)");
}

int main(void) {
    test_constants();
    test_effective_text_scale_rule();
    test_containment();
    test_presented_classification();

    printf("\nResult: %s (%d failure%s)\n",
           g_failures == 0 ? "PASS" : "FAIL",
           g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
