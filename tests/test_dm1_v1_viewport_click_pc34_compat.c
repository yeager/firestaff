#include "dm1_v1_viewport_click_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void check_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        printf("FAIL %s actual=%d expected=%d\n", name, actual, expected);
        ++g_failures;
    }
}

static void test_floor_pickup_requires_rendered_grabbable_cell(void)
{
    M11_ViewportClickResult r;

    r = m11_viewport_resolve_click_with_grabbable_mask(
        16, 96, 0, 10, 20, 1, 1, 0);
    check_int("pickup.empty_mask.cell", r.viewCell, DM1_VIEW_CELL_FRONT_LEFT);
    check_int("pickup.empty_mask.no_grab", r.objectGrabbed, 0);
    check_int("pickup.empty_mask.no_stop", r.stopWaitingForInput, 0);

    r = m11_viewport_resolve_click_with_grabbable_mask(
        16, 96, 0, 10, 20, 1, 1,
        DM1_VIEWPORT_GRABBABLE_CELL_MASK(DM1_VIEW_CELL_FRONT_LEFT));
    check_int("pickup.front_left.grab", r.objectGrabbed, 1);
    check_int("pickup.front_left.stop", r.stopWaitingForInput, 1);
    check_int("pickup.front_left.x", r.targetMapX, 10);
    check_int("pickup.front_left.y", r.targetMapY, 20);

    r = m11_viewport_resolve_click_with_grabbable_mask(
        200, 16, 1, 10, 20, 1, 1,
        DM1_VIEWPORT_GRABBABLE_CELL_MASK(DM1_VIEW_CELL_BACK_RIGHT));
    check_int("pickup.back_right.cell", r.viewCell, DM1_VIEW_CELL_BACK_RIGHT);
    check_int("pickup.back_right.grab", r.objectGrabbed, 1);
    check_int("pickup.back_right.x", r.targetMapX, 11);
    check_int("pickup.back_right.y", r.targetMapY, 20);

    r = m11_viewport_resolve_click_with_grabbable_mask(
        16, 16, 1, 10, 20, 1, 1,
        DM1_VIEWPORT_GRABBABLE_CELL_MASK(DM1_VIEW_CELL_BACK_RIGHT));
    check_int("pickup.wrong_cell.cell", r.viewCell, DM1_VIEW_CELL_BACK_LEFT);
    check_int("pickup.wrong_cell.no_grab", r.objectGrabbed, 0);
    check_int("pickup.wrong_cell.no_stop", r.stopWaitingForInput, 0);
}

static void test_legacy_resolver_keeps_existing_all_cells_contract(void)
{
    M11_ViewportClickResult r = m11_viewport_resolve_click(
        16, 96, 0, 10, 20, 1, 1);

    check_int("legacy.all_cells.cell", r.viewCell, DM1_VIEW_CELL_FRONT_LEFT);
    check_int("legacy.all_cells.grab", r.objectGrabbed, 1);
    check_int("legacy.all_cells.stop", r.stopWaitingForInput, 1);
}

static void test_source_evidence_mentions_grabbable_zone_gate(void)
{
    const char *e = m11_viewport_click_source_evidence();

    check_int("evidence.nonnull", e != NULL, 1);
    if (!e) return;
    check_int("evidence.pile_top", strstr(e, "CLIKVIEW.C:117-126") != NULL, 1);
    check_int("evidence.clickable_boxes", strstr(e, "CLIKVIEW.C:406-438") != NULL, 1);
    check_int("evidence.rendered_zones", strstr(e, "DUNVIEW.C:5113-5165") != NULL, 1);
}

int main(void)
{
    test_floor_pickup_requires_rendered_grabbable_cell();
    test_legacy_resolver_keeps_existing_all_cells_contract();
    test_source_evidence_mentions_grabbable_zone_gate();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_click_source_lock failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_click_source_lock\n");
    return 0;
}
