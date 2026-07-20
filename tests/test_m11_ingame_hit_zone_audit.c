/*
 * test_m11_ingame_hit_zone_audit — M11 in-game hit-zone touch-target
 * audit per UI scale, the M11 half of the remaining "UI scaling and
 * touch-target audit across launcher and game views" TODO (sibling to
 * the launcher-side m12_menu_row_hit_height_audit).
 *
 * Consumes the LIVE DM1 V1 hit-zone inventory from
 * touch_click_zone_matrix_pc34_compat.c (source-locked against ReDMCSB
 * COMMAND.C route tables + the I34E layout-696 ZONES table) — no zone
 * geometry is duplicated here, so the audit can never drift from the
 * shipped hit-test table.
 *
 * Per zone, per UI scale percent (100/150/200), per presentation scale
 * (1x..4x) the audit records:
 *   1. classification — presented shorter side vs the 24 px floor /
 *      44 px recommendation, cross-checked through
 *      fs_gesture_audit_zones (the same contract the launcher audits
 *      use);
 *   2. per-zone decision — the minimum presentation scale that lifts
 *      the zone's shorter side to the floor
 *      (floor-at-1x / needs-2x / needs-3x / needs-4x /
 *      never-lifts-exempt);
 *   3. the UI-scale finding — zone geometry is UI-scale independent
 *      today (M11_UIScale has no hit-test/HUD-geometry consumer), so
 *      floor counts are identical at 100/150/200; the hypothetical
 *      M11_UIScale_Apply-adjusted sizes are recorded alongside.
 *
 * Aggregate floor/recommended counts per presentation scale are pinned
 * as the shipped-geometry contract: any zone-table change that alters
 * touch-target safety fails this audit.
 *
 * Data-free: pure arithmetic over the live zone table, no assets.
 */

#include "touch_click_zone_matrix_pc34_compat.h"
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

static int zone_min_side(const TouchClickZonePc34Compat* z) {
    return z->w < z->h ? z->w : z->h;
}

/* ── 1. inventory sanity ────────────────────────────────────────────── */
static int test_inventory(void) {
    unsigned int count = TOUCHCLICK_Compat_GetZoneCount();
    unsigned int i;
    printf("[inventory] zones=%u\n", count);
    CHECK(count == 104, "DM1 V1 zone inventory is the pinned 104 zones");
    for (i = 0; i < count; ++i) {
        TouchClickZonePc34Compat z;
        CHECK(TOUCHCLICK_Compat_GetZone(i, &z), "every ordinal resolvable");
        if (!TOUCHCLICK_Compat_GetZone(i, &z)) continue;
        CHECK(z.w > 0 && z.h > 0, "every zone has positive extent");
        CHECK(z.groupName && z.groupName[0], "every zone is named");
        CHECK(z.sourceEvidence && z.sourceEvidence[0],
              "every zone carries source evidence");
    }
    return (int)count;
}

/* ── 2. per-zone classification cross-check at every scale ──────────── */
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
                TouchClickZonePc34Compat z;
                FsGestureZone gz;
                FsGestureZoneAuditReport report;
                M11HitZoneFit fit;
                int pw;
                int ph;
                int pmin;
                if (!TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
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

/* ── 3. per-zone decisions + aggregate floor-count contract ─────────── */
static void test_decisions_and_counts(int zoneCount) {
    /* Derived from the live table; pinned as the shipped-geometry
     * contract.  min-side classes: >=24 -> floor-at-1x, 12..23 ->
     * needs-2x, 8..11 -> needs-3x, 5..7 -> needs-4x, else exempt. */
    int expectedBelowMin[M11_HIT_ZONE_PRESENT_SCALE_MAX + 1] = {0, 0, 0, 0, 0};
    int expectedBelowRec[M11_HIT_ZONE_PRESENT_SCALE_MAX + 1] = {0, 0, 0, 0, 0};
    int decisionCount[5] = {0, 0, 0, 0, 0}; /* index = min-lifting scale, 0 = exempt */
    int scale;
    int i;

    printf("[decisions]\n");
    for (i = 0; i < zoneCount; ++i) {
        TouchClickZonePc34Compat z;
        int minSide;
        int lift;
        if (!TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        minSide = zone_min_side(&z);
        lift = m11_hit_zone_min_lifting_scale(minSide, 100);
        decisionCount[lift]++;
        for (scale = 1; scale <= M11_HIT_ZONE_PRESENT_SCALE_MAX; ++scale) {
            int presented = m11_hit_zone_presented_px(minSide, 100, scale);
            if (m11_hit_zone_classify(presented) == M11_HIT_ZONE_FIT_BELOW_MINIMUM) {
                expectedBelowMin[scale]++;
            }
            if (m11_hit_zone_classify(presented) != M11_HIT_ZONE_FIT_RECOMMENDED) {
                expectedBelowRec[scale]++;
            }
        }
        /* The decision must agree with the direct classification at the
         * lifting scale, and must fail one scale below it. */
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

    /* ── Pinned shipped-geometry contract (DM1 V1 zone table) ── */
    CHECK(decisionCount[0] == 1,
          "exactly one never-lifts zone (hidden 2x2 freeze-game debug box)");
    CHECK(decisionCount[1] == 19, "floor-at-1x count pinned");
    CHECK(decisionCount[2] == 62, "needs-2x count pinned");
    CHECK(decisionCount[3] == 17,
          "needs-3x count (13x11 runes, 85x11 action rows, 11x11 icons, ...)");
    CHECK(decisionCount[4] == 5,
          "needs-4x count (43x7 champion names + 35x7 action.pass)");
    CHECK(expectedBelowMin[1] == 85, "1x below-minimum count pinned");
    CHECK(expectedBelowMin[2] == 23, "2x below-minimum count pinned");
    CHECK(expectedBelowMin[3] == 6,  "3x below-minimum count pinned");
    CHECK(expectedBelowMin[4] == 1,  "4x below-minimum count pinned");
    CHECK(expectedBelowRec[1] == 98, "1x below-recommended count pinned");
    CHECK(expectedBelowRec[2] == 85, "2x below-recommended count pinned");
    CHECK(expectedBelowRec[3] == 27, "3x below-recommended count pinned");
    CHECK(expectedBelowRec[4] == 8,  "4x below-recommended count pinned");

    /* Every interactive zone must clear the floor at 4x presentation;
     * the single exemption is the hidden source debug box. */
    CHECK(decisionCount[0] == 1,
          "all interactive zones lift within the audited scale range");
}

/* ── 4. named per-zone decisions for the TODO's exemplar zones ──────── */
static void test_named_zone_decisions(int zoneCount) {
    int i;
    int runeChecked = 0;
    int freezeChecked = 0;
    printf("[named-decisions]\n");
    for (i = 0; i < zoneCount; ++i) {
        TouchClickZonePc34Compat z;
        if (!TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        if (strncmp(z.groupName, "spell.symbol", 12) == 0) {
            /* The TODO's exemplar: 13x11 spell runes lift only at 3x
             * presentation (2x leaves the 11 px side at 22 < 24). */
            CHECK(m11_hit_zone_min_lifting_scale(zone_min_side(&z), 100) == 3,
                  "13x11 spell runes need 3x presentation to clear the floor");
            runeChecked++;
        } else if (strcmp(z.groupName, "system.freeze_game") == 0) {
            /* Hidden source debug box (COMMAND.C:394): never lifts,
             * explicitly exempt — it is not a user touch target. */
            CHECK(m11_hit_zone_min_lifting_scale(zone_min_side(&z), 100) == 0,
                  "freeze-game debug box is the recorded never-lifts exemption");
            freezeChecked++;
        } else if (strncmp(z.groupName, "champion", 8) == 0 &&
                   strstr(z.groupName, ".name") != NULL) {
            /* 43x7 champion name strips are the smallest named
             * interactive targets: they need 4x presentation. */
            CHECK(m11_hit_zone_min_lifting_scale(zone_min_side(&z), 100) == 4,
                  "43x7 champion name strips need 4x presentation");
        }
    }
    CHECK(runeChecked == 6, "all six spell runes audited by name");
    CHECK(freezeChecked == 1, "freeze-game exemption audited by name");
}

/* ── 5. UI-scale independence finding + hypothetical re-audit ───────── */
static void test_ui_scale_finding(int zoneCount) {
    int hypotheticalBelowMinAt2x[3];
    static const int percents[3] = {100, 150, 200};
    int hypotheticalLiftedAt200 = 0;
    int pi;
    int i;
    printf("[ui-scale-finding]\n");
    /* Shipped contract: the hit-test consumes raw 320x200 source
     * coordinates (TOUCHCLICK_Compat_HitTest family) with no
     * M11_UIScale consumer anywhere in the geometry path, so shipped
     * zone sizes are percent-independent by construction — pinned via
     * the Apply mirror identity at percent 100. */
    CHECK(m11_hit_zone_apply_ui_scale(11, 100) == 11 &&
              m11_hit_zone_apply_ui_scale(7, 100) == 7 &&
              m11_hit_zone_apply_ui_scale(2, 100) == 2,
          "percent 100 is the identity (shipped percent-independent geometry)");
    CHECK(m11_hit_zone_apply_ui_scale(11, 200) == 22,
          "Apply mirror matches M11_UIScale_Apply integer math");

    /* Hypothetical re-audit: IF HUD geometry ever consumed
     * M11_UIScale, how would the floor counts move?  Recorded as
     * decisions, not shipped behavior: at 150% the six stubborn
     * sub-floor classes shrink to the 43x7/35x7 group plus freeze;
     * at 200% every interactive zone lifts at >= 2x presentation
     * (Apply(11,200)=22 -> 44, Apply(7,200)=14 -> 28) and only the
     * hidden freeze box stays exempt. */
    for (pi = 0; pi < 3; ++pi) {
        int below = 0;
        for (i = 0; i < zoneCount; ++i) {
            TouchClickZonePc34Compat z;
            if (!TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
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
    CHECK(hypotheticalBelowMinAt2x[0] == 23,
          "hypothetical at percent 100 matches the shipped 2x count");
    CHECK(hypotheticalBelowMinAt2x[1] == 6,
          "hypothetical UI-150 leaves only the 7px/2px classes sub-floor at 2x");
    CHECK(hypotheticalBelowMinAt2x[2] == 1,
          "hypothetical UI-200 lifts every interactive zone at 2x (freeze exempt)");

    for (i = 0; i < zoneCount; ++i) {
        TouchClickZonePc34Compat z;
        if (!TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        if (m11_hit_zone_min_lifting_scale(zone_min_side(&z), 200) == 2) {
            hypotheticalLiftedAt200++;
        }
    }
    printf("  hypothetical UI-200 zones lifting at 2x: %d\n",
           hypotheticalLiftedAt200);
    CHECK(hypotheticalLiftedAt200 == 22,
          "UI-200 hypothetical would lift 22 of the 23 sub-floor zones at 2x");
}

int main(void) {
    int zoneCount = test_inventory();
    test_classification_crosscheck(zoneCount);
    test_decisions_and_counts(zoneCount);
    test_named_zone_decisions(zoneCount);
    test_ui_scale_finding(zoneCount);

    printf("\nResult: %s (%d failure%s)\n",
           g_failures == 0 ? "PASS" : "FAIL",
           g_failures, g_failures == 1 ? "" : "s");
    return g_failures == 0 ? 0 : 1;
}
