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
    DM1_V1_ViewportClickResultPc34 r;

    r = DM1_V1_Viewport_ResolveClickWithGrabbableMaskPc34Compat(
        16, 96, 0, 10, 20, 1, 1, 0);
    check_int("pickup.empty_mask.cell", r.viewCell, DM1_VIEW_CELL_FRONT_LEFT);
    check_int("pickup.empty_mask.no_grab", r.objectGrabbed, 0);
    check_int("pickup.empty_mask.no_stop", r.stopWaitingForInput, 0);

    r = DM1_V1_Viewport_ResolveClickWithGrabbableMaskPc34Compat(
        16, 96, 0, 10, 20, 1, 1,
        DM1_VIEWPORT_GRABBABLE_CELL_MASK(DM1_VIEW_CELL_FRONT_LEFT));
    check_int("pickup.front_left.grab", r.objectGrabbed, 1);
    check_int("pickup.front_left.stop", r.stopWaitingForInput, 1);
    check_int("pickup.front_left.x", r.targetMapX, 10);
    check_int("pickup.front_left.y", r.targetMapY, 20);

    r = DM1_V1_Viewport_ResolveClickWithGrabbableMaskPc34Compat(
        200, 16, 1, 10, 20, 1, 1,
        DM1_VIEWPORT_GRABBABLE_CELL_MASK(DM1_VIEW_CELL_BACK_RIGHT));
    check_int("pickup.back_right.cell", r.viewCell, DM1_VIEW_CELL_BACK_RIGHT);
    check_int("pickup.back_right.grab", r.objectGrabbed, 1);
    check_int("pickup.back_right.x", r.targetMapX, 11);
    check_int("pickup.back_right.y", r.targetMapY, 20);

    r = DM1_V1_Viewport_ResolveClickWithGrabbableMaskPc34Compat(
        16, 16, 1, 10, 20, 1, 1,
        DM1_VIEWPORT_GRABBABLE_CELL_MASK(DM1_VIEW_CELL_BACK_RIGHT));
    check_int("pickup.wrong_cell.cell", r.viewCell, DM1_VIEW_CELL_BACK_LEFT);
    check_int("pickup.wrong_cell.no_grab", r.objectGrabbed, 0);
    check_int("pickup.wrong_cell.no_stop", r.stopWaitingForInput, 0);
}

static void test_default_resolver_does_not_pickup_without_rendered_state(void)
{
    DM1_V1_ViewportClickResultPc34 r = DM1_V1_Viewport_ResolveClickPc34Compat(
        16, 96, 0, 10, 20, 1, 1);

    check_int("default.no_rendered_state.cell", r.viewCell,
              DM1_VIEW_CELL_FRONT_LEFT);
    check_int("default.no_rendered_state.no_grab", r.objectGrabbed, 0);
    check_int("default.no_rendered_state.no_pile_top", r.pileTopObjectId,
              DM1_VIEWPORT_NO_PILE_TOP_OBJECT);
    check_int("default.no_rendered_state.no_stop", r.stopWaitingForInput, 0);
}

static void test_grabbable_state_wires_pile_top_object(void)
{
    DM1_V1_ViewportGrabbableStatePc34 state;
    DM1_V1_ViewportClickResultPc34 r;

    DM1_V1_ViewportGrabbable_InitPc34Compat(&state);
    check_int("state.init.mask", state.grabbableCellMask,
              DM1_VIEWPORT_GRABBABLE_NO_CELLS);
    check_int("state.init.pile_top",
              DM1_V1_ViewportGrabbable_PileTopPc34Compat(&state, DM1_VIEW_CELL_FRONT_LEFT),
              DM1_VIEWPORT_NO_PILE_TOP_OBJECT);

    check_int("state.set.front_left",
              DM1_V1_ViewportGrabbable_SetPileTopPc34Compat(
                  &state, DM1_VIEW_CELL_FRONT_LEFT, 42),
              1);
    r = DM1_V1_Viewport_ResolveClickWithGrabbableStatePc34Compat(
        16, 96, 0, 10, 20, 1, 1, &state);
    check_int("state.front_left.grab", r.objectGrabbed, 1);
    check_int("state.front_left.pile_top", r.pileTopObjectId, 42);

    r = DM1_V1_Viewport_ResolveClickWithGrabbableStatePc34Compat(
        200, 96, 0, 10, 20, 1, 1, &state);
    check_int("state.wrong_cell.no_grab", r.objectGrabbed, 0);
    check_int("state.wrong_cell.no_pile_top", r.pileTopObjectId,
              DM1_VIEWPORT_NO_PILE_TOP_OBJECT);

    check_int("state.replace_top",
              DM1_V1_ViewportGrabbable_SetPileTopPc34Compat(
                  &state, DM1_VIEW_CELL_FRONT_LEFT, 77),
              1);
    r = DM1_V1_Viewport_ResolveClickWithGrabbableStatePc34Compat(
        16, 96, 0, 10, 20, 1, 1, &state);
    check_int("state.replaced.pile_top", r.pileTopObjectId, 77);

    check_int("state.clear_cell",
              DM1_V1_ViewportGrabbable_SetPileTopPc34Compat(
                  &state, DM1_VIEW_CELL_FRONT_LEFT,
                  DM1_VIEWPORT_NO_PILE_TOP_OBJECT),
              1);
    r = DM1_V1_Viewport_ResolveClickWithGrabbableStatePc34Compat(
        16, 96, 0, 10, 20, 1, 1, &state);
    check_int("state.cleared.no_grab", r.objectGrabbed, 0);
}

static void test_source_evidence_mentions_grabbable_zone_gate(void)
{
    const char *e = DM1_V1_ViewportClick_SourceEvidencePc34Compat();

    check_int("evidence.nonnull", e != NULL, 1);
    if (!e) return;
    check_int("evidence.pile_top", strstr(e, "CLIKVIEW.C:117-126") != NULL, 1);
    check_int("evidence.clickable_boxes", strstr(e, "CLIKVIEW.C:406-438") != NULL, 1);
    check_int("evidence.rendered_zones", strstr(e, "DUNVIEW.C:5113-5178") != NULL, 1);
}

int main(void)
{
    test_floor_pickup_requires_rendered_grabbable_cell();
    test_default_resolver_does_not_pickup_without_rendered_state();
    test_grabbable_state_wires_pile_top_object();
    test_source_evidence_mentions_grabbable_zone_gate();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_click_source_lock failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_viewport_click_source_lock\n");
    return 0;
}
