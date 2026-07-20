/*
 * test_csb_touch_click_zone_matrix — CSB dungeon-view per-view zone
 * inventory sanity + hit-zone touch-target audit, sibling to
 * test_m11_ingame_hit_zone_audit (DM1 lane).
 *
 * Consumes the LIVE CSB zone inventory from
 * src/csb/csb_touch_click_zone_matrix_pc34_compat.c (source-locked
 * against ReDMCSB COMMAND.C PC-media route tables G0447/G0448/G0452/
 * G0453/G0454/G0455 + the shared I34E layout zone space) — no zone
 * geometry is duplicated here, so the audit can never drift from the
 * shipped inventory.
 *
 * Sections:
 *   1. inventory sanity — pinned total/per-view counts, positive
 *      extents, 320x200 bounds, names + source evidence on every zone;
 *   2. source-disjoint sets — zone families the source lays out as
 *      grids must not overlap (movement arrows, status boxes, bar
 *      toggles, names, hands, spell symbols, action rows, action
 *      icons, champion icons);
 *   3. hit-test probes — source-ordered per-view dispatch incl. button
 *      masking;
 *   4. hit-zone audit — per-zone classification cross-checked through
 *      fs_gesture_audit_zones at UI 100/150/200 x 1x..4x, pinned
 *      aggregate decisions and floor counts (same contract as the M11
 *      audit), UI-scale independence finding.
 *
 * Data-free: pure arithmetic over the live zone table, no assets.
 */

#include "csb_touch_click_zone_matrix_pc34_compat.h"
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

static int zone_min_side(const CsbTouchClickZonePc34Compat* z) {
    return z->w < z->h ? z->w : z->h;
}

/* ── 1. inventory sanity ────────────────────────────────────────────── */
static int test_inventory(void) {
    unsigned int count = CSB_TOUCHCLICK_Compat_GetZoneCount();
    unsigned int i;
    printf("[inventory] zones=%u\n", count);
    CHECK(count == 56, "CSB dungeon-view inventory is the pinned 56 zones");
    CHECK(CSB_TOUCHCLICK_Compat_GetViewZoneCount(
              CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT) == 19,
          "G0447 interface view carries 19 routes");
    CHECK(CSB_TOUCHCLICK_Compat_GetViewZoneCount(
              CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT) == 8,
          "G0448 movement view carries 8 routes");
    CHECK(CSB_TOUCHCLICK_Compat_GetViewZoneCount(
              CSB_TOUCH_CLICK_VIEW_ACTION_AREA_NAMES_PC34_COMPAT) == 4,
          "G0452 action-area names view carries 4 routes");
    CHECK(CSB_TOUCHCLICK_Compat_GetViewZoneCount(
              CSB_TOUCH_CLICK_VIEW_ACTION_AREA_ICONS_PC34_COMPAT) == 4,
          "G0453 action-area icons view carries 4 routes");
    CHECK(CSB_TOUCHCLICK_Compat_GetViewZoneCount(
              CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT) == 9,
          "G0454 spell-area view carries 9 routes");
    CHECK(CSB_TOUCHCLICK_Compat_GetViewZoneCount(
              CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT) == 12,
          "G0455 champion names/hands view carries 12 routes");
    for (i = 0; i < count; ++i) {
        CsbTouchClickZonePc34Compat z;
        CHECK(CSB_TOUCHCLICK_Compat_GetZone(i, &z),
              "every ordinal resolvable");
        if (!CSB_TOUCHCLICK_Compat_GetZone(i, &z)) continue;
        CHECK(z.w > 0 && z.h > 0, "every zone has positive extent");
        CHECK(z.x >= 0 && z.y >= 0 && z.x + z.w <= 320 && z.y + z.h <= 200,
              "every dungeon-view zone inside the 320x200 source screen");
        CHECK(z.groupName && z.groupName[0], "every zone is named");
        CHECK(z.sourceEvidence && z.sourceEvidence[0],
              "every zone carries source evidence");
        CHECK(z.coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
              "dungeon-view routes are all CM1 screen-relative");
    }
    /* Per-view ordinals must round-trip through the flat table. */
    {
        unsigned int v;
        unsigned int total = 0;
        for (v = CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT;
             v <= CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT; ++v) {
            unsigned int vc = CSB_TOUCHCLICK_Compat_GetViewZoneCount(
                (CsbTouchClickViewPc34Compat)v);
            unsigned int k;
            total += vc;
            for (k = 0; k < vc; ++k) {
                CsbTouchClickZonePc34Compat z;
                CHECK(CSB_TOUCHCLICK_Compat_GetViewZone(
                          (CsbTouchClickViewPc34Compat)v, k, &z),
                      "per-view ordinal resolvable");
                CHECK(z.view == (CsbTouchClickViewPc34Compat)v,
                      "per-view zone reports its own view");
            }
        }
        CHECK(total == count, "per-view counts sum to the flat count");
    }
    return (int)count;
}

/* ── 2. source-disjoint zone families ───────────────────────────────── */
static int rects_overlap(const CsbTouchClickZonePc34Compat* a,
                         const CsbTouchClickZonePc34Compat* b) {
    return a->x < b->x + b->w && b->x < a->x + a->w &&
           a->y < b->y + b->h && b->y < a->y + a->h;
}

static void check_family_disjoint(const char* prefix, int expected,
                                  int zoneCount) {
    CsbTouchClickZonePc34Compat zones[16];
    int n = 0;
    int i;
    int j;
    for (i = 0; i < zoneCount && n < 16; ++i) {
        CsbTouchClickZonePc34Compat z;
        if (!CSB_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        if (strncmp(z.groupName, prefix, strlen(prefix)) != 0) continue;
        /* Button-variant siblings (same rect, different mask) belong to
         * different prefixes already; within one prefix the source
         * lays out a disjoint grid. */
        for (j = 0; j < n; ++j) {
            CHECK(!rects_overlap(&zones[j], &z),
                  "source grid family must be pairwise disjoint");
        }
        zones[n++] = z;
    }
    CHECK(n == expected, "family carries the pinned member count");
}

static void test_disjoint_families(int zoneCount) {
    printf("[disjoint-families]\n");
    check_family_disjoint("movement.turn_", 2, zoneCount);
    check_family_disjoint("champion0.name", 1, zoneCount);
    check_family_disjoint("champion1.name", 1, zoneCount);
    check_family_disjoint("champion2.name", 1, zoneCount);
    check_family_disjoint("champion3.name", 1, zoneCount);
    check_family_disjoint("champion0.ready_hand", 1, zoneCount);
    check_family_disjoint("spell.symbol", 6, zoneCount);
    check_family_disjoint("action.icon", 4, zoneCount);
    check_family_disjoint("action.row", 3, zoneCount);
    check_family_disjoint("champion.icon_", 4, zoneCount);
    check_family_disjoint("champion0.bar_graphs_toggle", 1, zoneCount);
    /* The six movement arrows as one family (mixed prefixes). */
    {
        CsbTouchClickZonePc34Compat arrows[6];
        int n = 0;
        int i;
        int j;
        for (i = 0; i < zoneCount; ++i) {
            CsbTouchClickZonePc34Compat z;
            if (!CSB_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
            if (strncmp(z.groupName, "movement.", 9) != 0) continue;
            for (j = 0; j < n; ++j) {
                CHECK(!rects_overlap(&arrows[j], &z),
                      "movement arrow grid is pairwise disjoint");
            }
            if (n < 6) arrows[n] = z;
            ++n;
        }
        CHECK(n == 6, "six movement arrow zones audited");
    }
    /* The four champion status boxes (left-button routes) are disjoint;
     * the right-button toggle boxes intentionally coincide with them
     * (same rect, different button) — that is source nesting, not an
     * overlap defect. */
    {
        CsbTouchClickZonePc34Compat boxes[4];
        int n = 0;
        int i;
        int j;
        for (i = 0; i < zoneCount; ++i) {
            CsbTouchClickZonePc34Compat z;
            if (!CSB_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
            if (strstr(z.groupName, ".status_box") == NULL) continue;
            for (j = 0; j < n; ++j) {
                CHECK(!rects_overlap(&boxes[j], &z),
                      "champion status boxes are pairwise disjoint");
            }
            if (n < 4) boxes[n] = z;
            ++n;
        }
        CHECK(n == 4, "four champion status boxes audited");
    }
}

/* ── 3. hit-test probes ─────────────────────────────────────────────── */
static void probe(CsbTouchClickViewPc34Compat view, int x, int y,
                  unsigned int button, unsigned int expectCommand,
                  const char* msg) {
    CsbTouchClickZonePc34Compat z;
    int hit = CSB_TOUCHCLICK_Compat_HitTestInView(view, x, y, button, &z);
    CHECK(hit, msg);
    if (hit) {
        CHECK(z.commandId == expectCommand, msg);
    }
}

static void test_hit_test_probes(void) {
    CsbTouchClickZonePc34Compat z;
    printf("[hit-test-probes]\n");
    probe(CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, 240, 130,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 1u,
          "left click in the turn-left arrow dispatches C001");
    probe(CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, 240, 130,
          TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 83u,
          "right click on the arrows falls through to C083 screen toggle");
    probe(CSB_TOUCH_CLICK_VIEW_MOVEMENT_PC34_COMPAT, 10, 40,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 80u,
          "left click inside the dungeon viewport dispatches C080");
    probe(CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, 10, 10,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 12u,
          "left click in champion-0 status box dispatches C012");
    probe(CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, 10, 10,
          TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 7u,
          "right click in champion-0 status box dispatches C007 toggle");
    probe(CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, 50, 5,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 7u,
          "left click on champion-0 bar graphs dispatches C007 (source order)");
    probe(CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, 240, 50,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 100u,
          "left click in the spell-area parent dispatches C100");
    probe(CSB_TOUCH_CLICK_VIEW_INTERFACE_PC34_COMPAT, 0, 199,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 147u,
          "the 2x2 freeze-game box dispatches C147");
    probe(CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, 240, 55,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 101u,
          "spell symbol 1 dispatches C101");
    probe(CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, 240, 45,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 109u,
          "set-magic-caster strip dispatches C109");
    probe(CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, 10, 3,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 16u,
          "champion-0 name strip dispatches C016");
    probe(CSB_TOUCH_CLICK_VIEW_CHAMPION_NAMES_HANDS_PC34_COMPAT, 10, 15,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 20u,
          "champion-0 ready hand dispatches C020");
    probe(CSB_TOUCH_CLICK_VIEW_ACTION_AREA_NAMES_PC34_COMPAT, 240, 90,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 113u,
          "action row 0 dispatches C113");
    probe(CSB_TOUCH_CLICK_VIEW_ACTION_AREA_ICONS_PC34_COMPAT, 240, 90,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 116u,
          "champion-0 action icon dispatches C116");
    /* A point outside every route in a view must miss. */
    CHECK(!CSB_TOUCHCLICK_Compat_HitTestInView(
              CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, 10, 150,
              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &z),
          "spell-area view misses outside its zones");
    /* Views are isolated: a movement point must not hit in spell view. */
    CHECK(!CSB_TOUCHCLICK_Compat_HitTestInView(
              CSB_TOUCH_CLICK_VIEW_SPELL_AREA_PC34_COMPAT, 240, 130,
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
                CsbTouchClickZonePc34Compat z;
                FsGestureZone gz;
                FsGestureZoneAuditReport report;
                M11HitZoneFit fit;
                int pw;
                int ph;
                int pmin;
                if (!CSB_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z))
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
        CsbTouchClickZonePc34Compat z;
        int minSide;
        int lift;
        if (!CSB_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
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

    /* ── Pinned shipped-geometry contract (CSB dungeon view) ── */
    CHECK(decisionCount[0] == 1,
          "exactly one never-lifts zone (hidden 2x2 freeze-game debug box)");
    CHECK(decisionCount[1] == 16, "floor-at-1x count pinned");
    CHECK(decisionCount[2] == 22,
          "needs-2x count (14px icons, 21px arrows, 20px action icons, 16px hands)");
    CHECK(decisionCount[3] == 12,
          "needs-3x count (13x11 runes, 85x11 action rows, 87x8 caster strip, ...)");
    CHECK(decisionCount[4] == 5,
          "needs-4x count (43x7 champion names + 35x7 action.pass)");
    CHECK(expectedBelowMin[1] == 40, "1x below-minimum count pinned");
    CHECK(expectedBelowMin[2] == 18, "2x below-minimum count pinned");
    CHECK(expectedBelowMin[3] == 6,  "3x below-minimum count pinned");
    CHECK(expectedBelowMin[4] == 1,  "4x below-minimum count pinned");
    CHECK(expectedBelowRec[1] == 53, "1x below-recommended count pinned");
    CHECK(expectedBelowRec[2] == 40, "2x below-recommended count pinned");
    CHECK(expectedBelowRec[3] == 22, "3x below-recommended count pinned");
    CHECK(expectedBelowRec[4] == 7,  "4x below-recommended count pinned");
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
          "percent 100 is the identity (shipped percent-independent geometry)");
    for (pi = 0; pi < 3; ++pi) {
        int below = 0;
        for (i = 0; i < zoneCount; ++i) {
            CsbTouchClickZonePc34Compat z;
            if (!CSB_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z))
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
    CHECK(hypotheticalBelowMinAt2x[0] == 18,
          "hypothetical at percent 100 matches the shipped 2x count");
    CHECK(hypotheticalBelowMinAt2x[1] == 6,
          "hypothetical UI-150 leaves only the 7px/2px classes sub-floor at 2x");
    CHECK(hypotheticalBelowMinAt2x[2] == 1,
          "hypothetical UI-200 lifts every interactive zone at 2x (freeze exempt)");

    for (i = 0; i < zoneCount; ++i) {
        CsbTouchClickZonePc34Compat z;
        if (!CSB_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        if (m11_hit_zone_min_lifting_scale(zone_min_side(&z), 200) == 2) {
            hypotheticalLiftedAt200++;
        }
    }
    printf("  hypothetical UI-200 zones lifting at 2x: %d\n",
           hypotheticalLiftedAt200);
    CHECK(hypotheticalLiftedAt200 == 17,
          "UI-200 hypothetical would lift 17 of the 18 sub-floor zones at 2x");
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
