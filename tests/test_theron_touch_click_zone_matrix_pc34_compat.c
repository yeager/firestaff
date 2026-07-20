/*
 * test_theron_touch_click_zone_matrix — Theron per-view zone inventory
 * sanity + hit-zone touch-target audit, sibling to
 * test_dm2_touch_click_zone_matrix_pc34_compat (DM2 lane).
 *
 * Consumes the LIVE Theron zone inventory from
 * src/theron/theron_touch_click_zone_matrix_pc34_compat.c.  Provenance
 * is deliberately honest: the inventory is the IMPLEMENTED Firestaff
 * chrome geometry (V1 320x240 extended canvas + V2 256x224 native
 * overlay), not an extracted original table — ReDMCSB has zero Theron
 * coverage and only the IPL/stage2 boot loaders are disassembled
 * locally, so no original THQUEST.BIN zone tables exist to extract.
 *
 * Sections:
 *   1. inventory sanity — pinned total/per-view counts, positive
 *      extents, per-view bounds (320x240 V1 / 256x224 V2), names +
 *      source evidence on every zone;
 *   2. source-disjoint sets — the slot grids the renderer lays out
 *      (champion slots, rune slots, champion bars, action icons);
 *   3. hit-test probes — per-view dispatch incl. the nested coarse V1
 *      panels and view isolation;
 *   4. hit-zone audit — per-zone classification cross-checked through
 *      fs_gesture_audit_zones at UI 100/150/200 x 1x..4x, pinned
 *      aggregate decisions and floor counts, UI-scale finding.
 *
 * Data-free: pure arithmetic over the live zone table, no assets.
 */

#include "theron_touch_click_zone_matrix_pc34_compat.h"
#include "hit_zone_audit_m11.h"
#include "fs_gesture_navigation_gate.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond_, msg_) do {                                            \
    if (!(cond_)) {                                                        \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, msg_);          \
        g_failures++;                                                      \
    }                                                                      \
} while (0)

static int zone_min_side(const TheronTouchClickZonePc34Compat* z) {
    return z->w < z->h ? z->w : z->h;
}

/* ── 1. inventory sanity ────────────────────────────────────────────── */
static int test_inventory(void) {
    unsigned int count = THERON_TOUCHCLICK_Compat_GetZoneCount();
    unsigned int i;
    printf("[inventory] zones=%u\n", count);
    CHECK(count == 26, "Theron implemented-geometry inventory is the pinned 26 zones");
    CHECK(THERON_TOUCHCLICK_Compat_GetViewZoneCount(
              THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT) == 9,
          "V1 chrome view carries 9 zones");
    CHECK(THERON_TOUCHCLICK_Compat_GetViewZoneCount(
              THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT) == 17,
          "V2 HUD overlay view carries 17 zones");
    for (i = 0; i < count; ++i) {
        TheronTouchClickZonePc34Compat z;
        CHECK(THERON_TOUCHCLICK_Compat_GetZone(i, &z),
              "every ordinal resolvable");
        if (!THERON_TOUCHCLICK_Compat_GetZone(i, &z)) continue;
        CHECK(z.w > 0 && z.h > 0, "every zone has positive extent");
        CHECK(z.coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
              "every Theron zone is screen-relative in its view space");
        if (z.view == THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT) {
            CHECK(z.x >= 0 && z.y >= 0 && z.x + z.w <= 320 && z.y + z.h <= 240,
                  "every V1 zone inside the 320x240 extended canvas");
        } else {
            CHECK(z.x >= 0 && z.y >= 0 && z.x + z.w <= 256 && z.y + z.h <= 224,
                  "every V2 zone inside the 256x224 native framebuffer");
        }
        CHECK(z.groupName && z.groupName[0], "every zone is named");
        CHECK(z.sourceEvidence && z.sourceEvidence[0],
              "every zone carries source evidence");
    }
    /* Per-view ordinals must round-trip through the flat table. */
    {
        unsigned int v;
        unsigned int total = 0;
        for (v = THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT;
             v <= THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT; ++v) {
            unsigned int vc = THERON_TOUCHCLICK_Compat_GetViewZoneCount(
                (TheronTouchClickViewPc34Compat)v);
            unsigned int k;
            total += vc;
            for (k = 0; k < vc; ++k) {
                TheronTouchClickZonePc34Compat z;
                CHECK(THERON_TOUCHCLICK_Compat_GetViewZone(
                          (TheronTouchClickViewPc34Compat)v, k, &z),
                      "per-view ordinal resolvable");
                CHECK(z.view == (TheronTouchClickViewPc34Compat)v,
                      "per-view zone reports its own view");
            }
        }
        CHECK(total == count, "per-view counts sum to the flat count");
    }
    return (int)count;
}

/* ── 2. source-disjoint slot grids (per view) ───────────────────────── */
static int rects_overlap(const TheronTouchClickZonePc34Compat* a,
                         const TheronTouchClickZonePc34Compat* b) {
    return a->x < b->x + b->w && b->x < a->x + a->w &&
           a->y < b->y + b->h && b->y < a->y + a->h;
}

static void check_family_disjoint(const char* prefix, int expected,
                                  int zoneCount) {
    TheronTouchClickZonePc34Compat zones[8];
    int n = 0;
    int i;
    int j;
    for (i = 0; i < zoneCount && n < 8; ++i) {
        TheronTouchClickZonePc34Compat z;
        if (!THERON_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        if (strncmp(z.groupName, prefix, strlen(prefix)) != 0) continue;
        for (j = 0; j < n; ++j) {
            CHECK(!rects_overlap(&zones[j], &z),
                  "renderer slot grid must be pairwise disjoint");
        }
        zones[n++] = z;
    }
    CHECK(n == expected, "family carries the pinned member count");
}

static void test_disjoint_families(int zoneCount) {
    printf("[disjoint-families]\n");
    check_family_disjoint("champion.slot_", 4, zoneCount);
    check_family_disjoint("hud.rune_slot_", 4, zoneCount);
    check_family_disjoint("hud.champion_bar_", 4, zoneCount);
    check_family_disjoint("action.", 5, zoneCount);
    /* The V1 coarse panels intentionally nest (message_bar and the
     * champion slots sit inside bottom_panel) — that is the documented
     * coarse-panel layout of the implemented chrome, not an overlap
     * defect, and is therefore not asserted disjoint. */
}

/* ── 3. hit-test probes ─────────────────────────────────────────────── */
static void probe(TheronTouchClickViewPc34Compat view, int x, int y,
                  unsigned int button, unsigned int expectCommand,
                  const char* msg) {
    TheronTouchClickZonePc34Compat z;
    int hit = THERON_TOUCHCLICK_Compat_HitTestInView(view, x, y, button, &z);
    CHECK(hit, msg);
    if (hit) {
        CHECK(z.commandId == expectCommand, msg);
    }
}

static void test_hit_test_probes(void) {
    TheronTouchClickZonePc34Compat z;
    printf("[hit-test-probes]\n");
    /* V1 chrome (320x240 extended canvas) */
    probe(THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT, 160, 10,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 1u,
          "click in the top bar dispatches the TR_UI_TOPBAR zone id");
    probe(THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0u,
          "click in the viewport hits the no-command viewport zone");
    probe(THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT, 270, 100,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 2u,
          "click in the right panel dispatches TR_UI_RIGHT_PANEL");
    probe(THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT, 160, 200,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 4u,
          "click in the bottom panel hits the coarse panel first (source order)");
    probe(THERON_TOUCH_CLICK_VIEW_V1_CHROME_PC34_COMPAT, 10, 186,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 4u,
          "click over the message strip still hits bottom_panel first (source order)");
    /* V2 HUD overlay (256x224 native framebuffer) */
    probe(THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 16, 12,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0u,
          "click on the compass hits the presentation-only compass zone");
    probe(THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 38, 10,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0u,
          "click on rune slot 0 hits the 4x4 indicator cell");
    probe(THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 70, 6,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0u,
          "click on the quest-items text hits its glyph box");
    probe(THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 20, 188,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0u,
          "click on champion bar 0 hits the mini-bar zone");
    probe(THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 20, 215,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 1u,
          "click on the ATK icon dispatches the 1-based ATTACK id");
    probe(THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 150, 215,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 5u,
          "click on the MOV icon dispatches the 1-based MOVE id");
    /* Misses: outside every zone in a view. */
    CHECK(!THERON_TOUCHCLICK_Compat_HitTestInView(
              THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 100, 100,
              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &z),
          "V2 overlay misses in the uncovered viewport middle");
    CHECK(!THERON_TOUCHCLICK_Compat_HitTestInView(
              THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 100, 100,
              TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &z),
          "right-button has no Theron zone mapping");
    /* View isolation: a V1 viewport point is a miss in the V2 view. */
    CHECK(!THERON_TOUCHCLICK_Compat_HitTestInView(
              THERON_TOUCH_CLICK_VIEW_V2_HUD_OVERLAY_PC34_COMPAT, 100, 100,
              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &z),
          "per-view hit-test isolation holds");
}

/* ── 4. hit-zone audit ──────────────────────────────────────────────── */
static void test_classification_crosscheck(int zoneCount) {
    static const int percents[3] = {100, 150, 200};
    int pi;
    int i;
    printf("[classification-crosscheck]\n");
    for (pi = 0; pi < 3; ++pi) {
        int scale;
        for (scale = M11_HIT_ZONE_PRESENT_SCALE_MIN;
             scale <= M11_HIT_ZONE_PRESENT_SCALE_MAX; ++scale) {
            for (i = 0; i < zoneCount; ++i) {
                TheronTouchClickZonePc34Compat z;
                FsGestureZone gz;
                FsGestureZoneAuditReport report;
                M11HitZoneFit fit;
                int pw;
                int ph;
                int pmin;
                if (!THERON_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z))
                    continue;
                pw = m11_hit_zone_presented_px(z.w, percents[pi], scale);
                ph = m11_hit_zone_presented_px(z.h, percents[pi], scale);
                pmin = pw < ph ? pw : ph;
                fit = m11_hit_zone_classify(pmin);
                gz.x = 0;
                gz.y = 0;
                gz.w = pw;
                gz.h = ph;
                gz.groupName = z.groupName;
                (void)fs_gesture_audit_zones(&gz, 1,
                                             FS_GG_PLATFORM_MIN_TARGET_PX,
                                             FS_GG_PLATFORM_RECOMMENDED_PX,
                                             &report);
                CHECK(report.totalZones == 1, "one zone audited");
                CHECK((fit != M11_HIT_ZONE_FIT_BELOW_MINIMUM)
                          == (report.zonesBelowMinimum == 0),
                      "metric classification and fs_gesture floor verdict disagree");
                CHECK((fit == M11_HIT_ZONE_FIT_RECOMMENDED)
                          == (report.zonesBelowRecommended == 0),
                      "metric classification and fs_gesture recommendation disagree");
            }
        }
    }
}

static void test_decisions_and_counts(int zoneCount) {
    int expectedBelowMin[M11_HIT_ZONE_PRESENT_SCALE_MAX + 1] = {0, 0, 0, 0, 0};
    int expectedBelowRec[M11_HIT_ZONE_PRESENT_SCALE_MAX + 1] = {0, 0, 0, 0, 0};
    int decisionCount[5] = {0, 0, 0, 0, 0}; /* index = min-lifting scale */
    int scale;
    int i;

    printf("[decisions]\n");
    for (i = 0; i < zoneCount; ++i) {
        TheronTouchClickZonePc34Compat z;
        int minSide;
        int lift;
        if (!THERON_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        minSide = zone_min_side(&z);
        lift = m11_hit_zone_min_lifting_scale(minSide, 100);
        decisionCount[lift]++;
        for (scale = 1; scale <= M11_HIT_ZONE_PRESENT_SCALE_MAX; ++scale) {
            int presented = m11_hit_zone_presented_px(minSide, 100, scale);
            if (m11_hit_zone_classify(presented)
                    == M11_HIT_ZONE_FIT_BELOW_MINIMUM) {
                expectedBelowMin[scale]++;
            }
            if (m11_hit_zone_classify(presented)
                    != M11_HIT_ZONE_FIT_RECOMMENDED) {
                expectedBelowRec[scale]++;
            }
        }
        if (lift > 0) {
            CHECK(m11_hit_zone_classify(
                      m11_hit_zone_presented_px(minSide, 100, lift))
                      != M11_HIT_ZONE_FIT_BELOW_MINIMUM,
                  "zone clears the floor at its decided lifting scale");
            if (lift > 1) {
                CHECK(m11_hit_zone_classify(
                          m11_hit_zone_presented_px(minSide, 100, lift - 1))
                          == M11_HIT_ZONE_FIT_BELOW_MINIMUM,
                      "zone is below the floor one scale under its decision");
            }
        }
    }
    printf("  decisions: 1x=%d 2x=%d 3x=%d 4x=%d exempt=%d\n",
           decisionCount[1], decisionCount[2], decisionCount[3],
           decisionCount[4], decisionCount[0]);
    for (scale = 1; scale <= M11_HIT_ZONE_PRESENT_SCALE_MAX; ++scale) {
        printf("  present=%dx below-min=%d below-recommended=%d\n",
               scale, expectedBelowMin[scale], expectedBelowRec[scale]);
    }

    /* ── Pinned implemented-geometry contract (Theron chrome) ────── */
    CHECK(decisionCount[0] == 7,
          "exactly seven never-lifts zones (4x4 rune slots + 5px text glyph boxes — presentation-only indicators, consistent with the gesture runtime binding no commands there)");
    CHECK(decisionCount[1] == 9, "floor-at-1x count pinned");
    CHECK(decisionCount[2] == 6,
          "needs-2x count (16px message bar, 24px top bar/compass, 14px action strip cells, 8px champion bars, ...)");
    CHECK(decisionCount[3] == 4, "needs-3x count pinned");
    CHECK(decisionCount[4] == 0, "no zone needs 4x");
    CHECK(expectedBelowMin[1] == 17, "1x below-minimum count pinned");
    CHECK(expectedBelowMin[2] == 11, "2x below-minimum count pinned");
    CHECK(expectedBelowMin[3] == 7,  "3x below-minimum count pinned");
    CHECK(expectedBelowMin[4] == 7,  "4x below-minimum count pinned");
    CHECK(expectedBelowRec[1] == 19, "1x below-recommended count pinned");
    CHECK(expectedBelowRec[2] == 17, "2x below-recommended count pinned");
    CHECK(expectedBelowRec[3] == 16, "3x below-recommended count pinned");
    CHECK(expectedBelowRec[4] == 11, "4x below-recommended count pinned");
}

static void test_ui_scale_finding(int zoneCount) {
    int hypotheticalBelowMinAt2x[3];
    static const int percents[3] = {100, 150, 200};
    int hypotheticalLiftedAt200 = 0;
    int pi;
    int i;
    printf("[ui-scale-finding]\n");
    CHECK(m11_hit_zone_apply_ui_scale(11, 100) == 11 &&
              m11_hit_zone_apply_ui_scale(7, 100) == 7 &&
              m11_hit_zone_apply_ui_scale(2, 100) == 2,
          "percent 100 is the identity (implemented percent-independent geometry)");
    for (pi = 0; pi < 3; ++pi) {
        int below = 0;
        for (i = 0; i < zoneCount; ++i) {
            TheronTouchClickZonePc34Compat z;
            if (!THERON_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z))
                continue;
            if (m11_hit_zone_classify(
                    m11_hit_zone_presented_px(zone_min_side(&z),
                                              percents[pi], 2))
                    == M11_HIT_ZONE_FIT_BELOW_MINIMUM) {
                below++;
            }
        }
        hypotheticalBelowMinAt2x[pi] = below;
    }
    printf("  hypothetical 2x below-min at UI 100/150/200: %d/%d/%d\n",
           hypotheticalBelowMinAt2x[0], hypotheticalBelowMinAt2x[1],
           hypotheticalBelowMinAt2x[2]);
    CHECK(hypotheticalBelowMinAt2x[0] == 11,
          "hypothetical at percent 100 matches the implemented 2x count");
    CHECK(hypotheticalBelowMinAt2x[1] == 7,
          "hypothetical UI-150 leaves only the never-lifts indicator cells sub-floor at 2x");
    CHECK(hypotheticalBelowMinAt2x[2] == 7,
          "hypothetical UI-200 leaves the same seven indicator cells sub-floor at 2x");

    for (i = 0; i < zoneCount; ++i) {
        TheronTouchClickZonePc34Compat z;
        if (!THERON_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        if (m11_hit_zone_min_lifting_scale(zone_min_side(&z), 200) == 2) {
            hypotheticalLiftedAt200++;
        }
    }
    printf("  hypothetical UI-200 zones lifting at 2x: %d\n",
           hypotheticalLiftedAt200);
    CHECK(hypotheticalLiftedAt200 == 4,
          "UI-200 hypothetical lift count pinned");
}

int main(void) {
    int zoneCount = test_inventory();
    test_disjoint_families(zoneCount);
    test_hit_test_probes();
    test_classification_crosscheck(zoneCount);
    test_decisions_and_counts(zoneCount);
    test_ui_scale_finding(zoneCount);

    printf("\nResult: %s (%d failure%s)\n",
           g_failures == 0 ? "PASS" : "FAIL",
           g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
