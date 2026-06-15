#include "dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * DM1 V1 champion-panel portrait box blit dispatch gate tests.
 *
 * This contract-only test pins the F0292 -> F0354 dispatch predicate, the
 * post-call Attributes mask, the F0254 secondary dispatch, and the
 * F0293 champion-index dispatch order without claiming any real-asset
 * bitmap parity. The gate source is locked to ReDMCSB
 * CHAMDRAW.C F0292:757-1110 + TIMELINE.C F0254:1614-1637 + CHAMDRAW.C
 * F0293:1117-1143 + DEFS.H:3783-3793 + DEFS.H:3793.
 *
 * The fixture does not duplicate pass673 (champion panel mouth/eye),
 * pass683 (champion panel food/water), pass760 (champion panel status-
 * hand rotation), pass761 (chest close while mirror candidate open),
 * pass762 (C040 rotation race), or any existing portrait state redraw
 * / portrait source-lock slice.
 */

static int g_assertions = 0;
static int g_failures = 0;

#define CHECK(ID, GOT, WANT, ANCHOR) check_int((ID), (int)(GOT), (int)(WANT), (ANCHOR))
#define CHECK_BOOL(ID, GOT, WANT, ANCHOR) \
    check_int((ID), (GOT) ? 1 : 0, (WANT) ? 1 : 0, (ANCHOR))

static void check_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void check_contains(const char *id, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_inventory_champion_portrait_blit_default(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        DM1_V1_CPBBG_MASK_STATUS_BOX_PC34,
        true,  /* champion_alive */
        true,  /* is_inventory_champion */
        false, /* draw_floor_and_ceiling_requested */
        0,     /* champion_index */
        1,     /* inventory_champion_ordinal */
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("default.f0292_short_circuits", result.f0292_short_circuits, 0,
          "CHAMDRAW.C F0292:757 redraw-mask non-zero");
    CHECK("default.f0292_calls_f0355", result.f0292_calls_f0355, 0,
          "CHAMDRAW.C F0292:767-770 F0355 not armed");
    CHECK("default.f0292_enters_status_box_branch",
          result.f0292_enters_status_box_branch, 1,
          "CHAMDRAW.C F0292:771 MASK0x1000_STATUS_BOX set");
    CHECK("default.f0292_calls_f0354", result.f0292_calls_f0354, 1,
          "CHAMDRAW.C F0292:810-812 F0354 inventory portrait blit");
    CHECK("default.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34,
          "F0354 portrait blit path");
    CHECK("default.post_f0292_mask", (int)result.post_f0292_attributes_mask,
          (int)DM1_V1_CPBBG_POST_F0354_MASK_PC34,
          "CHAMDRAW.C F0292:812 post-F0354 MASK0x0100_STATISTICS only");
    CHECK("default.post_f0292_cleared",
          (int)result.post_f0292_cleared_mask,
          (int)DM1_V1_CPBBG_CLEAR_MASK_PC34,
          "CHAMDRAW.C F0292:1110 nine-bit clear at function exit");
    CHECK("default.status_box_zone", result.status_box_zone, 151,
          "DEFS.H:3783-3793 C151+C151+championIndex status-box zone");
    CHECK("default.portrait_zone", result.portrait_zone, 175,
          "DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX");
    CHECK("default.dispatch_order", result.champion_index_dispatch_order, -1,
          "F0292 single-champion dispatch has no per-tick order");
}

static void test_f0292_redraw_mask_short_circuit(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        0, /* input_redraw_mask: no bits set */
        true,
        true,
        false,
        0,
        1,
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("short.f0292_short_circuits", result.f0292_short_circuits, 1,
          "CHAMDRAW.C F0292:757 no redraw-mask bit set -> early return");
    CHECK("short.f0292_calls_f0355", result.f0292_calls_f0355, 0,
          "F0292 short-circuits before F0355 pre-route");
    CHECK("short.f0292_enters_status_box_branch",
          result.f0292_enters_status_box_branch, 0,
          "F0292 short-circuits before status-box branch");
    CHECK("short.f0292_calls_f0354", result.f0292_calls_f0354, 0,
          "F0354 is unreachable when F0292 short-circuits");
    CHECK("short.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_NOT_REACHED_PC34,
          "no dispatch path reached");
    CHECK("short.post_f0292_mask", (int)result.post_f0292_attributes_mask, 0,
          "no F0292 body ran so the post-call mask stays at zero");
    CHECK("short.post_f0292_cleared",
          (int)result.post_f0292_cleared_mask, 0,
          "no F0292 body ran so the post-call clear stays at zero");
}

static void test_f0292_non_status_box_mask_skips_f0354(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        DM1_V1_CPBBG_MASK_STATISTICS_PC34 |
            DM1_V1_CPBBG_MASK_LOAD_PC34 |
            DM1_V1_CPBBG_MASK_WOUNDS_PC34 |
            DM1_V1_CPBBG_MASK_ACTION_HAND_PC34,
        true,
        true,
        false,
        1,
        2,
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("no_status_box.f0292_short_circuits",
          result.f0292_short_circuits, 0,
          "F0292 has bits set so it does not short-circuit");
    CHECK("no_status_box.f0292_calls_f0355", result.f0292_calls_f0355, 0,
          "G0297 not set so F0355 pre-route is not armed");
    CHECK("no_status_box.f0292_enters_status_box_branch",
          result.f0292_enters_status_box_branch, 0,
          "CHAMDRAW.C F0292:771 MASK0x1000_STATUS_BOX is not set");
    CHECK("no_status_box.f0292_calls_f0354", result.f0292_calls_f0354, 0,
          "F0354 unreachable without MASK0x1000_STATUS_BOX");
    CHECK("no_status_box.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_NOT_REACHED_PC34,
          "no F0354 path without status-box mask");
    CHECK("no_status_box.post_f0292_cleared",
          (int)result.post_f0292_cleared_mask,
          (int)DM1_V1_CPBBG_CLEAR_MASK_PC34,
          "F0292 still runs the body and clears the mask at exit");
}

static void test_f0292_dead_champion_skips_f0354(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        DM1_V1_CPBBG_MASK_STATUS_BOX_PC34,
        false, /* champion_alive */
        true,
        false,
        2,
        3,
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("dead.f0292_short_circuits", result.f0292_short_circuits, 0,
          "MASK0x1000_STATUS_BOX set so F0292 does not short-circuit");
    CHECK("dead.f0292_calls_f0355", result.f0292_calls_f0355, 0,
          "G0297 not set so F0355 pre-route is not armed");
    CHECK("dead.f0292_enters_status_box_branch",
          result.f0292_enters_status_box_branch, 1,
          "MASK0x1000_STATUS_BOX set so status-box branch is entered");
    CHECK("dead.f0292_calls_f0354", result.f0292_calls_f0354, 0,
          "CHAMDRAW.C F0292:784 dead branch skips F0354");
    CHECK("dead.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_DEAD_STATUS_BOX_PC34,
          "dead-status-box path");
    CHECK("dead.status_box_zone", result.status_box_zone, 153,
          "DEFS.H:3783-3793 C151+championIndex status-box zone (champion 2)");
    CHECK("dead.post_f0292_mask", (int)result.post_f0292_attributes_mask, 0,
          "dead branch takes the early return and clears the mask at exit");
}

static void test_f0292_non_inventory_champion_fallback(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        DM1_V1_CPBBG_MASK_STATUS_BOX_PC34,
        true,
        false, /* is_inventory_champion */
        false,
        1,
        2, /* inventory champion is champion 2, not champion 1 */
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("non_inventory.f0292_short_circuits",
          result.f0292_short_circuits, 0,
          "MASK0x1000_STATUS_BOX set so F0292 does not short-circuit");
    CHECK("non_inventory.f0292_calls_f0355",
          result.f0292_calls_f0355, 0,
          "champion is not the inventory champion so F0355 is not armed");
    CHECK("non_inventory.f0292_enters_status_box_branch",
          result.f0292_enters_status_box_branch, 1,
          "MASK0x1000_STATUS_BOX set so status-box branch is entered");
    CHECK("non_inventory.f0292_calls_f0354", result.f0292_calls_f0354, 0,
          "CHAMDRAW.C F0292:810 non-inventory branch skips F0354");
    CHECK("non_inventory.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_NON_INVENTORY_REDRAW_PC34,
          "non-inventory redraw-mask fallback path");
    CHECK("non_inventory.post_f0292_mask",
          (int)result.post_f0292_attributes_mask,
          (int)DM1_V1_CPBBG_NON_INVENTORY_MASK_PC34,
          "CHAMDRAW.C F0292:813-814 NAME_TITLE|STATISTICS|WOUNDS|ACTION_HAND");
    CHECK("non_inventory.status_box_zone", result.status_box_zone, 152,
          "DEFS.H:3783-3793 C151+championIndex status-box zone (champion 1)");
}

static void test_f0292_f0355_inventory_pre_route(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        DM1_V1_CPBBG_MASK_STATUS_BOX_PC34,
        true,
        true,
        true, /* draw_floor_and_ceiling_requested */
        0,
        1,
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("f0355.f0292_calls_f0355", result.f0292_calls_f0355, 1,
          "CHAMDRAW.C F0292:767-770 inventory + G0297 -> F0355 pre-route");
    CHECK("f0355.f0292_enters_status_box_branch",
          result.f0292_enters_status_box_branch, 1,
          "F0355 returns and the status-box branch still runs");
    CHECK("f0355.f0292_calls_f0354", result.f0292_calls_f0354, 1,
          "F0292 still calls F0354 after F0355 pre-route");
    CHECK("f0355.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34,
          "F0354 portrait blit path is the final outcome");
}

static void test_f0292_full_mask_still_lands_on_f0354(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        DM1_V1_CPBBG_CLEAR_MASK_PC34,
        true,
        true,
        false,
        3,
        4,
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("full_mask.f0292_short_circuits", result.f0292_short_circuits, 0,
          "all nine bits set so F0292 does not short-circuit");
    CHECK("full_mask.f0292_calls_f0354", result.f0292_calls_f0354, 1,
          "all nine bits + inventory + alive still calls F0354");
    CHECK("full_mask.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34,
          "F0354 portrait blit path with the full redraw mask");
    CHECK("full_mask.status_box_zone", result.status_box_zone, 154,
          "DEFS.H:3783-3793 C151+championIndex status-box zone (champion 3)");
    CHECK("full_mask.portrait_zone", result.portrait_zone, 178,
          "DEFS.H:3793 C175+championIndex portrait zone (champion 3)");
    CHECK("full_mask.post_f0292_mask",
          (int)result.post_f0292_attributes_mask,
          (int)DM1_V1_CPBBG_POST_F0354_MASK_PC34,
          "F0354 still owns the post-call mask regardless of input bits");
}

static void test_f0292_zone_bases_per_champion(void)
{
    int champion;
    int expected_status_box;
    int expected_portrait;
    DM1_V1_CPBBG_GateResultPc34Compat result;

    for (champion = 0; champion < DM1_V1_CPBBG_CHAMPION_COUNT_PC34;
         ++champion) {
        memset(&result, 0, sizeof(result));
        DM1_V1_CPBBG_BuildGatePc34Compat(
            DM1_V1_CPBBG_MASK_STATUS_BOX_PC34,
            true,
            true,
            false,
            champion,
            champion + 1,
            DM1_V1_CPBBG_DISPATCH_F0292_PC34,
            &result);
        expected_status_box =
            DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 + champion;
        expected_portrait =
            DM1_V1_CPBBG_PORTRAIT_ZONE_BASE_PC34 + champion;

        char id_status[64];
        char id_portrait[64];
        snprintf(id_status, sizeof(id_status),
                 "zone.champion%d.status_box_zone", champion);
        snprintf(id_portrait, sizeof(id_portrait),
                 "zone.champion%d.portrait_zone", champion);

        CHECK(id_status, result.status_box_zone, expected_status_box,
              "DEFS.H:3783-3793 C151+championIndex status-box zone stride");
        CHECK(id_portrait, result.portrait_zone, expected_portrait,
              "DEFS.H:3793 C175+championIndex portrait zone stride");
    }
}

static void test_f0254_inventory_routes_through_f0354(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        0, /* F0254 does not use the F0292 redraw mask */
        true,
        true,
        false,
        0,
        1,
        DM1_V1_CPBBG_DISPATCH_F0254_PC34,
        &result);

    CHECK("f0254_inventory.dispatch_site", (int)result.dispatch_site,
          (int)DM1_V1_CPBBG_DISPATCH_F0254_PC34,
          "TIMELINE.C F0254:1614 secondary F0354 dispatch site");
    CHECK("f0254_inventory.f0292_calls_f0354", result.f0292_calls_f0354, 1,
          "TIMELINE.C F0254:1630 inventory champion -> F0354 blit");
    CHECK("f0254_inventory.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34,
          "F0254 inventory route still ends in F0354 portrait blit");
    CHECK("f0254_inventory.portrait_zone", result.portrait_zone, 175,
          "DEFS.H:3793 C175_ZONE_FIRST_CHAMPION_STATUS_BOX for champion 0");
}

static void test_f0254_dead_champion_short_circuit(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        0,
        false, /* dead champion */
        true,
        false,
        0,
        1,
        DM1_V1_CPBBG_DISPATCH_F0254_PC34,
        &result);

    CHECK("f0254_dead.f0292_calls_f0354", result.f0292_calls_f0354, 0,
          "TIMELINE.C F0254:1624 dead champion short-circuits before F0354");
    CHECK("f0254_dead.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_DEAD_STATUS_BOX_PC34,
          "F0254 dead route skips F0354");
}

static void test_f0254_non_inventory_routes_through_f0292(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        0,
        true,
        false, /* non-inventory champion */
        false,
        1,
        2, /* inventory champion is champion 2, not champion 1 */
        DM1_V1_CPBBG_DISPATCH_F0254_PC34,
        &result);

    CHECK("f0254_non_inventory.f0292_calls_f0354",
          result.f0292_calls_f0354, 0,
          "TIMELINE.C F0254:1635 non-inventory champion -> F0292, not F0354");
    CHECK("f0254_non_inventory.path", (int)result.path,
          (int)DM1_V1_CPBBG_PATH_NON_INVENTORY_REDRAW_PC34,
          "F0254 non-inventory route takes the F0292 redraw-mask path");
    CHECK("f0254_non_inventory.post_f0292_mask",
          (int)result.post_f0292_attributes_mask,
          (int)DM1_V1_CPBBG_F0254_NON_INVENTORY_MASK_PC34,
          "TIMELINE.C F0254:1635 sets NAME_TITLE only on the Attributes mask");
}

static void test_f0293_champion_index_dispatch_order(void)
{
    int champion;
    DM1_V1_CPBBG_GateResultPc34Compat result;

    for (champion = 0; champion < DM1_V1_CPBBG_CHAMPION_COUNT_PC34;
         ++champion) {
        bool is_inventory = (champion + 1) == 2;
        int expected_order = is_inventory ? champion : -1;

        memset(&result, 0, sizeof(result));
        DM1_V1_CPBBG_BuildGatePc34Compat(
            DM1_V1_CPBBG_CLEAR_MASK_PC34,
            true,
            is_inventory,
            false,
            champion,
            2, /* inventory champion is champion 2 */
            DM1_V1_CPBBG_DISPATCH_F0293_PC34,
            &result);

        char id[64];
        snprintf(id, sizeof(id), "f0293.champion%d.dispatch_order", champion);
        CHECK(id, result.champion_index_dispatch_order, expected_order,
              "CHAMDRAW.C F0293:1134 champion-index loop counter");
        snprintf(id, sizeof(id), "f0293.champion%d.calls_f0354", champion);
        CHECK(id, result.f0292_calls_f0354, is_inventory ? 1 : 0,
              "CHAMDRAW.C F0293 -> F0292 -> F0354 only for inventory champion");
    }
}

static void test_invalid_champion_index(void)
{
    DM1_V1_CPBBG_GateResultPc34Compat result;

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        DM1_V1_CPBBG_MASK_STATUS_BOX_PC34,
        true,
        true,
        false,
        -1, /* invalid champion_index */
        1,
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("invalid_index.f0292_calls_f0354", result.f0292_calls_f0354, 1,
          "F0354 is reached but the zone stays at -1 for invalid index");
    CHECK("invalid_index.status_box_zone", result.status_box_zone, -1,
          "invalid champion_index leaves the status-box zone unanchored");
    CHECK("invalid_index.portrait_zone", result.portrait_zone, -1,
          "invalid champion_index leaves the portrait zone unanchored");

    memset(&result, 0, sizeof(result));
    DM1_V1_CPBBG_BuildGatePc34Compat(
        DM1_V1_CPBBG_MASK_STATUS_BOX_PC34,
        true,
        true,
        false,
        4, /* out-of-range champion_index */
        1,
        DM1_V1_CPBBG_DISPATCH_F0292_PC34,
        &result);

    CHECK("oor_index.f0292_calls_f0354", result.f0292_calls_f0354, 1,
          "F0354 is reached but the zone stays at -1 for out-of-range index");
    CHECK("oor_index.status_box_zone", result.status_box_zone, -1,
          "out-of-range champion_index leaves the status-box zone unanchored");
    CHECK("oor_index.portrait_zone", result.portrait_zone, -1,
          "out-of-range champion_index leaves the portrait zone unanchored");
}

static void test_redraw_mask_table(void)
{
    CHECK("mask.NAME_TITLE", (int)DM1_V1_CPBBG_MASK_NAME_TITLE_PC34, 0x0080,
          "CHAMDRAW.C F0292:757 NAME_TITLE");
    CHECK("mask.STATISTICS", (int)DM1_V1_CPBBG_MASK_STATISTICS_PC34, 0x0100,
          "CHAMDRAW.C F0292:812 STATISTICS post-F0354 mask");
    CHECK("mask.LOAD", (int)DM1_V1_CPBBG_MASK_LOAD_PC34, 0x0200,
          "CHAMDRAW.C F0292:757 LOAD");
    CHECK("mask.ICON", (int)DM1_V1_CPBBG_MASK_ICON_PC34, 0x0400,
          "CHAMDRAW.C F0292:757 ICON");
    CHECK("mask.PANEL", (int)DM1_V1_CPBBG_MASK_PANEL_PC34, 0x0800,
          "CHAMDRAW.C F0292:757 PANEL");
    CHECK("mask.STATUS_BOX", (int)DM1_V1_CPBBG_MASK_STATUS_BOX_PC34, 0x1000,
          "CHAMDRAW.C F0292:771 STATUS_BOX gate");
    CHECK("mask.WOUNDS", (int)DM1_V1_CPBBG_MASK_WOUNDS_PC34, 0x2000,
          "CHAMDRAW.C F0292:757 WOUNDS");
    CHECK("mask.VIEWPORT", (int)DM1_V1_CPBBG_MASK_VIEWPORT_PC34, 0x4000,
          "CHAMDRAW.C F0292:757 VIEWPORT");
    CHECK("mask.ACTION_HAND", (int)DM1_V1_CPBBG_MASK_ACTION_HAND_PC34, 0x8000,
          "CHAMDRAW.C F0292:757 ACTION_HAND");
    CHECK("mask.cleared", (int)DM1_V1_CPBBG_CLEAR_MASK_PC34,
          0x0080 | 0x0100 | 0x0200 | 0x0400 | 0x0800 |
              0x1000 | 0x2000 | 0x4000 | 0x8000,
          "CHAMDRAW.C F0292:1110 nine-bit clear");
    CHECK("mask.post_f0354", (int)DM1_V1_CPBBG_POST_F0354_MASK_PC34, 0x0100,
          "CHAMDRAW.C F0292:812 STATISTICS only after F0354");
    CHECK("mask.non_inventory", (int)DM1_V1_CPBBG_NON_INVENTORY_MASK_PC34,
          0x0080 | 0x0100 | 0x2000 | 0x8000,
          "CHAMDRAW.C F0292:813-814 NAME_TITLE|STATISTICS|WOUNDS|ACTION_HAND");
    CHECK("mask.f0254_non_inventory",
          (int)DM1_V1_CPBBG_F0254_NON_INVENTORY_MASK_PC34, 0x0080,
          "TIMELINE.C F0254:1635 NAME_TITLE only on Attributes mask");
}

static void test_source_evidence(void)
{
    const char *evidence = DM1_V1_CPBBG_SourceEvidencePc34Compat();

    check_contains("evidence.f0292_short_circuit", evidence,
                   "CHAMDRAW.C F0292:757-760", "redraw-mask short-circuit");
    check_contains("evidence.f0292_f0355_pre_route", evidence,
                   "CHAMDRAW.C F0292:767-770", "F0355 pre-route");
    check_contains("evidence.f0292_status_box_branch", evidence,
                   "CHAMDRAW.C F0292:771",
                   "MASK0x1000_STATUS_BOX branch");
    check_contains("evidence.f0292_dead_branch", evidence,
                   "CHAMDRAW.C F0292:784", "dead champion branch");
    check_contains("evidence.f0292_f0354_call", evidence,
                   "CHAMDRAW.C F0292:810-812",
                   "F0354 call site + post-call mask");
    check_contains("evidence.f0292_non_inventory_fallback", evidence,
                   "CHAMDRAW.C F0292:813-814",
                   "non-inventory redraw-mask fallback");
    check_contains("evidence.f0292_clear_at_exit", evidence,
                   "CHAMDRAW.C F0292:1110", "nine-bit clear at exit");
    check_contains("evidence.f0254_secondary_dispatch", evidence,
                   "TIMELINE.C F0254:1614-1637",
                   "HideDamageReceived secondary F0354 dispatch");
    check_contains("evidence.f0293_dispatch_loop", evidence,
                   "CHAMDRAW.C F0293:1117-1143",
                   "DrawAllChampionStates champion-index loop");
    check_contains("evidence.defs_status_box_zones", evidence,
                   "DEFS.H:3783-3793", "C151 status-box zone stride");
    check_contains("evidence.defs_portrait_zone", evidence,
                   "DEFS.H:3793", "C175_ZONE_FIRST_CHAMPION_STATUS_BOX");
    check_contains("evidence.no_real_asset_claim", evidence,
                   "no real-asset bitmap parity claim",
                   "contract-only no-claim marker");
}

int main(void)
{
    test_inventory_champion_portrait_blit_default();
    test_f0292_redraw_mask_short_circuit();
    test_f0292_non_status_box_mask_skips_f0354();
    test_f0292_dead_champion_skips_f0354();
    test_f0292_non_inventory_champion_fallback();
    test_f0292_f0355_inventory_pre_route();
    test_f0292_full_mask_still_lands_on_f0354();
    test_f0292_zone_bases_per_champion();
    test_f0254_inventory_routes_through_f0354();
    test_f0254_dead_champion_short_circuit();
    test_f0254_non_inventory_routes_through_f0292();
    test_f0293_champion_index_dispatch_order();
    test_invalid_champion_index();
    test_redraw_mask_table();
    test_source_evidence();

    if (g_failures) {
        printf("FAIL test_dm1_v1_champion_panel_portrait_box_blit_gate "
               "failures=%d assertions=%d\n",
               g_failures, g_assertions);
        printf("Assertions: %d\n", g_assertions);
        printf("Failures: %d\n", g_failures);
        return 1;
    }

    printf("PASS test_dm1_v1_champion_panel_portrait_box_blit_gate "
           "failures=0 assertions=%d\n",
           g_assertions);
    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    return 0;
}
