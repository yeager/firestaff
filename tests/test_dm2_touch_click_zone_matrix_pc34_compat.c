/*
 * test_dm2_touch_click_zone_matrix — DM2 per-view zone inventory
 * sanity + hit-zone touch-target audit, sibling to
 * test_csb_touch_click_zone_matrix_pc34_compat (CSB lane).
 *
 * Consumes the LIVE DM2 zone inventory from
 * src/dm2/dm2_touch_click_zone_matrix_pc34_compat.c (source-locked
 * against SKWIN skval1.h DM2 MOUSE_INPUT route tables in relocated
 * _1031_07d6 form + the GDAT rect pool) — no zone geometry is
 * duplicated here, so the audit can never drift from the shipped
 * inventory.
 *
 * Sections:
 *   1. inventory sanity — pinned total/per-view counts, positive
 *      extents, 320x200 bounds, names + source evidence on every zone;
 *   2. source-disjoint sets — zone families the source lays out as
 *      grids must not overlap inside one view (movement arrows, spell
 *      runes, moneybox, container, backpack, scabbards/pouches,
 *      savegame slots);
 *   3. hit-test probes — source-ordered per-view dispatch incl. button
 *      masking and the aux 0x10 menu mask;
 *   4. hit-zone audit — per-zone classification cross-checked through
 *      fs_gesture_audit_zones at UI 100/150/200 x 1x..4x, pinned
 *      aggregate decisions and floor counts (same contract as the M11
 *      audit), UI-scale independence finding.
 *
 * Data-free: pure arithmetic over the live zone table, no assets.
 */

#include "dm2_touch_click_zone_matrix_pc34_compat.h"
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

static int zone_min_side(const Dm2TouchClickZonePc34Compat* z) {
    return z->w < z->h ? z->w : z->h;
}

/* ── 1. inventory sanity ────────────────────────────────────────────── */
static int test_inventory(void) {
    unsigned int count = DM2_TOUCHCLICK_Compat_GetZoneCount();
    unsigned int i;
    printf("[inventory] zones=%u\n", count);
    CHECK(count == 421, "DM2 full route-table inventory is the pinned 421 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT) == 5,
          "view main_menu carries 5 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_CREDITS_PC34_COMPAT) == 2,
          "view credits carries 2 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_SLEEP_PC34_COMPAT) == 3,
          "view sleep carries 3 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_PAUSE_PC34_COMPAT) == 2,
          "view pause carries 2 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_DIALOG_PC34_COMPAT) == 34,
          "view dialog carries 34 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT) == 136,
          "view dungeon carries 136 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_CHAMPION_RIBBON_PC34_COMPAT) == 3,
          "view champion_ribbon carries 3 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_CHAMPION_PC34_COMPAT) == 37,
          "view champion carries 37 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT) == 165,
          "view inventory carries 165 zones");
    CHECK(DM2_TOUCHCLICK_Compat_GetViewZoneCount(
              DM2_TOUCH_CLICK_VIEW_SAVEGAME_SLOTS_PC34_COMPAT) == 34,
          "view savegame_slots carries 34 zones");
    for (i = 0; i < count; ++i) {
        Dm2TouchClickZonePc34Compat z;
        CHECK(DM2_TOUCHCLICK_Compat_GetZone(i, &z),
              "every ordinal resolvable");
        if (!DM2_TOUCHCLICK_Compat_GetZone(i, &z)) continue;
        CHECK(z.w > 0 && z.h > 0, "every zone has positive extent");
        CHECK(z.coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
              "every DM2 zone is screen-relative (origins baked at decode)");
        CHECK(z.x >= 0 && z.y >= 0 && z.x + z.w <= 320 && z.y + z.h <= 200,
              "every zone inside the 320x200 source screen");
        CHECK(z.groupName && z.groupName[0], "every zone is named");
        CHECK(z.sourceEvidence && z.sourceEvidence[0],
              "every zone carries source evidence");
    }
    /* Per-view ordinals must round-trip through the flat table. */
    {
        unsigned int v;
        unsigned int total = 0;
        for (v = DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT;
             v <= DM2_TOUCH_CLICK_VIEW_SAVEGAME_SLOTS_PC34_COMPAT; ++v) {
            unsigned int vc = DM2_TOUCHCLICK_Compat_GetViewZoneCount(
                (Dm2TouchClickViewPc34Compat)v);
            unsigned int k;
            total += vc;
            for (k = 0; k < vc; ++k) {
                Dm2TouchClickZonePc34Compat z;
                CHECK(DM2_TOUCHCLICK_Compat_GetViewZone(
                          (Dm2TouchClickViewPc34Compat)v, k, &z),
                      "per-view ordinal resolvable");
                CHECK(z.view == (Dm2TouchClickViewPc34Compat)v,
                      "per-view zone reports its own view");
            }
        }
        CHECK(total == count, "per-view counts sum to the flat count");
    }
    return (int)count;
}

/* ── 2. source-disjoint zone families (per view) ────────────────────── */
static int rects_overlap(const Dm2TouchClickZonePc34Compat* a,
                         const Dm2TouchClickZonePc34Compat* b) {
    return a->x < b->x + b->w && b->x < a->x + a->w &&
           a->y < b->y + b->h && b->y < a->y + a->h;
}

static void check_family_disjoint(Dm2TouchClickViewPc34Compat view,
                                  const char* prefix, int expected,
                                  int zoneCount) {
    Dm2TouchClickZonePc34Compat zones[40];
    int n = 0;
    int i;
    int j;
    for (i = 0; i < zoneCount && n < 40; ++i) {
        Dm2TouchClickZonePc34Compat z;
        if (!DM2_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        if (z.view != view) continue;
        if (strncmp(z.groupName, prefix, strlen(prefix)) != 0) continue;
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
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT,
                          "movement.", 6, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT,
                          "spell.rune_", 6, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT,
                          "moneybox.slot_", 6, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT,
                          "container.slot_", 8, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT,
                          "champion1.hand_", 2, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT,
                          "backpack.slot_", 17, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_CHAMPION_PC34_COMPAT,
                          "backpack.slot_", 17, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT,
                          "inventory.scabbard_", 4, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT,
                          "inventory.pouch_", 2, zoneCount);
    check_family_disjoint(DM2_TOUCH_CLICK_VIEW_SAVEGAME_SLOTS_PC34_COMPAT,
                          "savegame.slot_", 34, zoneCount);
    /* The magic_map.rune_ family intentionally repeats each rune rect
     * across the four runtime-gated champion variants (24 overlapping
     * pairs) — that is source gate nesting, not an overlap defect, and
     * is therefore not asserted disjoint. */
}

/* ── 3. hit-test probes ─────────────────────────────────────────────── */
static void probe(Dm2TouchClickViewPc34Compat view, int x, int y,
                  unsigned int button, unsigned int expectCommand,
                  const char* msg) {
    Dm2TouchClickZonePc34Compat z;
    int hit = DM2_TOUCHCLICK_Compat_HitTestInView(view, x, y, button, &z);
    CHECK(hit, msg);
    if (hit) {
        CHECK(z.commandId == expectCommand, msg);
    }
}

static void test_hit_test_probes(void) {
    Dm2TouchClickZonePc34Compat z;
    printf("[hit-test-probes]\n");
    /* main menu */
    probe(DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT, 115, 60,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xD7u,
          "left click on the menu start box dispatches UI_EVENTCODE_START_NEW_GAME");
    probe(DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT, 115, 60,
          0x0010u, 0xD8u,
          "aux-0x10 click on the same box dispatches the 0xD8 menu option");
    probe(DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT, 40, 75,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xD9u,
          "left click on resume dispatches UI_EVENTCODE_RESUME_GAME");
    probe(DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT, 50, 105,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xE0u,
          "left click on quit dispatches 0xE0");
    probe(DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT, 90, 168,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xDAu,
          "left click on credits dispatches UI_EVENTCODE_SHOW_CREDITS");
    /* credits screen: any button quits the credits */
    probe(DM2_TOUCH_CLICK_VIEW_CREDITS_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xEFu,
          "left click anywhere quits the credits");
    probe(DM2_TOUCH_CLICK_VIEW_CREDITS_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 0xEFu,
          "right click anywhere quits the credits");
    /* sleep / pause */
    probe(DM2_TOUCH_CLICK_VIEW_SLEEP_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x92u,
          "left click on the sleep screen wakes (0x92)");
    probe(DM2_TOUCH_CLICK_VIEW_PAUSE_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x94u,
          "left click on the pause screen ends pause (0x94)");
    /* dialog */
    probe(DM2_TOUCH_CLICK_VIEW_DIALOG_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xDBu,
          "left click on dialog button row dispatches UI_EVENTCODE_DIALOG_BUTTON_1");
    /* dungeon view */
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 235, 135,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x01u,
          "left click in the turn-left arrow dispatches UI_EVENTCODE_TURN_LEFT");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 265, 135,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x03u,
          "left click in the forward arrow dispatches UI_EVENTCODE_MOVE_FORWARD");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 300, 160,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x04u,
          "left click in the move-right arrow dispatches UI_EVENTCODE_MOVE_RIGHT");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 10, 50,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x50u,
          "left click inside the dungeon viewport dispatches UI_EVENTCODE_CLICK_VIEWPORT");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 0x52u,
          "right click anywhere falls through to UI_EVENTCODE_VIEW_LEADER");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 0, 199,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x93u,
          "the 1x1 bottom-left pause box dispatches UI_EVENTCODE_PAUSE");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 50, 5,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x0Bu,
          "left click in the champion ribbon dispatches UI_EVENTCODE_RETURN_VIEWPORT (source order)");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 240, 70,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x5Fu,
          "left click on the top-left spell-or-leader panel dispatches 0x5F");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 260, 70,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x7Du,
          "left click on the top-left party position dispatches 0x7D");
    probe(DM2_TOUCH_CLICK_VIEW_DUNGEON_PC34_COMPAT, 240, 60,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x74u,
          "left click on champion-1 action hand dispatches 0x74");
    /* champion ribbon view */
    probe(DM2_TOUCH_CLICK_VIEW_CHAMPION_RIBBON_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x50u,
          "ribbon view left click in the viewport dispatches 0x50");
    probe(DM2_TOUCH_CLICK_VIEW_CHAMPION_RIBBON_PC34_COMPAT, 10, 10,
          TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 0x52u,
          "ribbon view right click dispatches VIEW_LEADER");
    /* champion view: revive box */
    probe(DM2_TOUCH_CLICK_VIEW_CHAMPION_PC34_COMPAT, 100, 100,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xA0u,
          "left click on the revive box dispatches UI_EVENTCODE_REVIVE_CHAMPION");
    /* inventory view */
    probe(DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT, 14, 104,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x1Cu,
          "left click on the inventory right hand dispatches 0x1C");
    probe(DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT, 20, 64,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x47u,
          "left click on the eye dispatches UI_EVENTCODE_EYE");
    probe(DM2_TOUCH_CLICK_VIEW_INVENTORY_PC34_COMPAT, 175, 48,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0x8Cu,
          "left click on the disk icon dispatches UI_EVENTCODE_DISK_OP");
    /* savegame slots */
    probe(DM2_TOUCH_CLICK_VIEW_SAVEGAME_SLOTS_PC34_COMPAT, 130, 157,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xA5u,
          "left click on savegame slot line dispatches 0xA5");
    probe(DM2_TOUCH_CLICK_VIEW_SAVEGAME_SLOTS_PC34_COMPAT, 190, 157,
          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 0xA6u,
          "left click on the slot side box dispatches 0xA6");
    /* A point outside every route in a view must miss. */
    CHECK(!DM2_TOUCHCLICK_Compat_HitTestInView(
              DM2_TOUCH_CLICK_VIEW_SAVEGAME_SLOTS_PC34_COMPAT, 60, 50,
              TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &z),
          "savegame view misses outside its zones");
    /* Views are isolated: a dungeon arrow point must not hit in the
     * main menu (the dialog view legitimately covers it with its wide
     * button rows, so the menu is the clean isolation witness). */
    CHECK(!DM2_TOUCHCLICK_Compat_HitTestInView(
              DM2_TOUCH_CLICK_VIEW_MAIN_MENU_PC34_COMPAT, 235, 135,
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
                Dm2TouchClickZonePc34Compat z;
                FsGestureZone gz;
                FsGestureZoneAuditReport report;
                M11HitZoneFit fit;
                int pw;
                int ph;
                int pmin;
                if (!DM2_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z))
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
        Dm2TouchClickZonePc34Compat z;
        int minSide;
        int lift;
        if (!DM2_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
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

    /* ── Pinned shipped-geometry contract (DM2 full route tables) ── */
    CHECK(decisionCount[0] == 5,
          "exactly five never-lifts zones (the hidden 1x1 pause box repeated in five views)");
    CHECK(decisionCount[1] == 69, "floor-at-1x count pinned");
    CHECK(decisionCount[2] == 215,
          "needs-2x count (16x16 inventory/hand slots, 21x21 positions, 23px arrows, 13x13 disk, ...)");
    CHECK(decisionCount[3] == 132,
          "needs-3x count (13x11 runes, 11px savegame strips, 9x9 map runes, 9px hand-quit strips, ...)");
    CHECK(decisionCount[4] == 0,
          "no zone needs 4x (the 1x1 pause box is the only sub-floor class at 3x)");
    CHECK(expectedBelowMin[1] == 352, "1x below-minimum count pinned");
    CHECK(expectedBelowMin[2] == 137, "2x below-minimum count pinned");
    CHECK(expectedBelowMin[3] == 5,   "3x below-minimum count pinned");
    CHECK(expectedBelowMin[4] == 5,   "4x below-minimum count pinned");
    CHECK(expectedBelowRec[1] == 392, "1x below-recommended count pinned");
    CHECK(expectedBelowRec[2] == 346, "2x below-recommended count pinned");
    CHECK(expectedBelowRec[3] == 142, "3x below-recommended count pinned");
    CHECK(expectedBelowRec[4] == 108, "4x below-recommended count pinned");
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
            Dm2TouchClickZonePc34Compat z;
            if (!DM2_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z))
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
    CHECK(hypotheticalBelowMinAt2x[0] == 137,
          "hypothetical at percent 100 matches the shipped 2x count");
    CHECK(hypotheticalBelowMinAt2x[1] == 5,
          "hypothetical UI-150 leaves only the 1px pause box sub-floor at 2x");
    CHECK(hypotheticalBelowMinAt2x[2] == 5,
          "hypothetical UI-200 lifts every interactive zone at 2x (pause boxes exempt)");

    for (i = 0; i < zoneCount; ++i) {
        Dm2TouchClickZonePc34Compat z;
        if (!DM2_TOUCHCLICK_Compat_GetZone((unsigned int)i, &z)) continue;
        if (m11_hit_zone_min_lifting_scale(zone_min_side(&z), 200) == 2) {
            hypotheticalLiftedAt200++;
        }
    }
    printf("  hypothetical UI-200 zones lifting at 2x: %d\n",
           hypotheticalLiftedAt200);
    CHECK(hypotheticalLiftedAt200 == 132,
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
