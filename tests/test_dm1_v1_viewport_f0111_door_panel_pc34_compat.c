/*
 * ReDMCSB evidence:
 * - DUNVIEW.C:4218-4339 F0111_DUNGEONVIEW_DrawDoor: C0 open skip,
 *   G0074 temporary door composition, C4/C5/partly-open branches, D1C
 *   Thieves Eye mask, and final F0791 viewport blit with C10 transparency.
 * - DEFS.H:1039-1044 C0..C5 door states; DEFS.H:2088 C10_COLOR_FLESH;
 *   DEFS.H:3516 MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR;
 *   DEFS.H:4250-4260 MEDIA720 door zones.
 */
#include "dm1_v1_viewport_f0111_door_panel_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static DM1_V1_F0111DoorPanelTracePc34 resolve(
    DM1_V1_F0111DoorPanelInputPc34 input,
    const char *id,
    const char *anchor)
{
    DM1_V1_F0111DoorPanelTracePc34 out;
    expect_int(id, dm1_v1_viewport_f0111_door_panel_resolve_pc34(&input, &out) ? 1 : 0,
               1, anchor);
    return out;
}

static void test_spec_metadata(void)
{
    const DM1_V1_F0111DoorPanelSpecPc34 *spec =
        dm1_v1_viewport_f0111_door_panel_spec_pc34();

    expect_int("spec.contract_only", spec->source_locked_contract_only ? 1 : 0, 1,
               "DUNVIEW.C:4218-4339 F0111 source-lock contract");
    expect_int("spec.no_real_asset_pixel_parity", spec->no_real_asset_pixel_parity ? 1 : 0, 1,
               "contract-only, no real-asset pixel parity claim");
    expect_int("spec.open_state", spec->open_state, 0,
               "DEFS.H:1039 C0_DOOR_STATE_OPEN");
    expect_int("spec.closed_state", spec->closed_state, 4,
               "DEFS.H:1043 C4_DOOR_STATE_CLOSED");
    expect_int("spec.destroyed_state", spec->destroyed_state, 5,
               "DEFS.H:1044 C5_DOOR_STATE_DESTROYED");
    expect_int("spec.c10", spec->transparent_color, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("spec.d1c_zone", spec->d1c_zone_index, 3790,
               "DEFS.H:4259 M631_ZONE_DOOR_D1C");
    expect_int("spec.mask4000", spec->half_zone_shift_mask, 0x4000,
               "DEFS.H:3516 MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR");
}

static void test_open_state_skips_temporary_and_viewport_blit(void)
{
    const DM1_V1_F0111DoorPanelInputPc34 input = {
        DM1_V1_F0111_DOOR_STATE_OPEN_PC34, false, false, 0, 3790, 96, 1
    };
    const DM1_V1_F0111DoorPanelTracePc34 out = resolve(
        input, "open.resolve", "DUNVIEW.C:4248 F0111 C0 open guard");

    expect_int("open.branch", (int)out.branch,
               DM1_V1_F0111_DOOR_BRANCH_OPEN_NO_DRAW_PC34,
               "DUNVIEW.C:4248 skips all door drawing for C0");
    expect_int("open.no_copy", out.copied_native_panel_to_temporary ? 1 : 0, 0,
               "DUNVIEW.C:4255-4334 guarded by C0");
    expect_int("open.no_ornament", out.drew_base_ornament_to_temporary ? 1 : 0, 0,
               "DUNVIEW.C:4262 guarded by C0");
    expect_int("open.no_final_blit", out.final_viewport_blit ? 1 : 0, 0,
               "DUNVIEW.C:4334 guarded by C0");
}

static void test_closed_and_destroyed_composition_paths(void)
{
    const DM1_V1_F0111DoorPanelInputPc34 closed = {
        DM1_V1_F0111_DOOR_STATE_CLOSED_PC34, true, true, 7, 3790, 96, 2
    };
    const DM1_V1_F0111DoorPanelInputPc34 destroyed = {
        DM1_V1_F0111_DOOR_STATE_DESTROYED_PC34, true, false, 0, 3790, 96, 0
    };
    const DM1_V1_F0111DoorPanelTracePc34 closed_out = resolve(
        closed, "closed.resolve", "DUNVIEW.C:4297-4298 C4 closed branch");
    const DM1_V1_F0111DoorPanelTracePc34 destroyed_out = resolve(
        destroyed, "destroyed.resolve", "DUNVIEW.C:4301-4304 C5 destroyed branch");

    expect_int("closed.copy", closed_out.copied_native_panel_to_temporary ? 1 : 0, 1,
               "DUNVIEW.C:4260 copies native panel to G0074");
    expect_int("closed.base_ornament", closed_out.drew_base_ornament_to_temporary ? 1 : 0, 1,
               "DUNVIEW.C:4262 draws base ornament into G0074");
    expect_int("closed.flip_flags", closed_out.flip_flags, 3,
               "DUNVIEW.C:4273-4285 animated door flip flags");
    expect_int("closed.thieves_eye", closed_out.applied_thieves_eye_mask ? 1 : 0, 1,
               "DUNVIEW.C:4292-4294 D1C Thieves Eye mask");
    expect_int("closed.no_destroyed_mask", closed_out.applied_destroyed_mask ? 1 : 0, 0,
               "DUNVIEW.C:4301 C5 branch not taken");
    expect_int("closed.final_zone", closed_out.final_zone_index, 3790,
               "DUNVIEW.C:4334 final F0791 zone");
    expect_int("closed.final_blit", closed_out.final_viewport_blit ? 1 : 0, 1,
               "DUNVIEW.C:4334 F0791 viewport blit");

    expect_int("destroyed.branch", (int)destroyed_out.branch,
               DM1_V1_F0111_DOOR_BRANCH_DESTROYED_PC34,
               "DUNVIEW.C:4301-4304 C5 destroyed branch");
    expect_int("destroyed.mask", destroyed_out.applied_destroyed_mask ? 1 : 0, 1,
               "DUNVIEW.C:4302 applies C15 destroyed mask");
    expect_int("destroyed.closed_frame", destroyed_out.drew_closed_or_destroyed_frame ? 1 : 0, 1,
               "DUNVIEW.C:4304 draws ClosedOrDestroyed frame");
    expect_int("destroyed.no_flip", destroyed_out.flip_flags, 0,
               "DUNVIEW.C:4284-4285 non-animated no flip");
}

static void test_partly_open_vertical_and_horizontal_zone_math(void)
{
    const DM1_V1_F0111DoorPanelInputPc34 vertical = {
        DM1_V1_F0111_DOOR_STATE_CLOSED_HALF_PC34, true, false, 0, 3750, 64, 0
    };
    const DM1_V1_F0111DoorPanelInputPc34 horizontal = {
        DM1_V1_F0111_DOOR_STATE_CLOSED_THREE_FOURTH_PC34, false, false, 0, 3750, 64, 0
    };
    const DM1_V1_F0111DoorPanelTracePc34 vertical_out = resolve(
        vertical, "partial_vertical.resolve", "DUNVIEW.C:4317-4327 PC34 partly-open zone math");
    const DM1_V1_F0111DoorPanelTracePc34 horizontal_out = resolve(
        horizontal, "partial_horizontal.resolve", "DUNVIEW.C:4319-4325 horizontal split");

    expect_int("partial_vertical.branch", (int)vertical_out.branch,
               DM1_V1_F0111_DOOR_BRANCH_PARTLY_VERTICAL_PC34,
               "DUNVIEW.C:4317-4318 vertical partly-open zone");
    expect_int("partial_vertical.zone", vertical_out.final_zone_index, 3752,
               "DUNVIEW.C:4318 P2084_i_ZoneIndex += P0125_ui_DoorState");
    expect_int("partial_vertical.no_half", vertical_out.drew_horizontal_front_half ? 1 : 0, 0,
               "DUNVIEW.C:4319 horizontal-only split guard");
    expect_int("partial_vertical.final_blit", vertical_out.final_viewport_blit ? 1 : 0, 1,
               "DUNVIEW.C:4334 final blit");

    expect_int("partial_horizontal.branch", (int)horizontal_out.branch,
               DM1_V1_F0111_DOOR_BRANCH_PARTLY_HORIZONTAL_PC34,
               "DUNVIEW.C:4319-4325 horizontal branch");
    expect_int("partial_horizontal.front_half_zone", horizontal_out.front_half_zone_index, 3759,
               "DUNVIEW.C:4322 first half uses zone + state + C6_UNKNOWN");
    expect_int("partial_horizontal.shift_x", horizontal_out.zone_shift_x, 32,
               "DUNVIEW.C:4320 G2154_i_ZoneShiftX = temp width >> 1");
    expect_int("partial_horizontal.final_zone", horizontal_out.final_zone_index, 20140,
               "DUNVIEW.C:4325 adds 3 | MASK0x4000 before final blit");
    expect_int("partial_horizontal.final_blit", horizontal_out.final_viewport_blit ? 1 : 0, 1,
               "DUNVIEW.C:4334 final blit");
}

static void test_transparency_and_invalid_inputs(void)
{
    DM1_V1_F0111DoorPanelTracePc34 out;
    const DM1_V1_F0111DoorPanelInputPc34 bad_state = {
        6, false, false, 0, 3790, 96, 0
    };

    expect_int("invalid.null_out",
               dm1_v1_viewport_f0111_door_panel_resolve_pc34(NULL, NULL) ? 1 : 0, 0,
               "contract rejects null output");
    expect_int("invalid.null_input",
               dm1_v1_viewport_f0111_door_panel_resolve_pc34(NULL, &out) ? 1 : 0, 0,
               "contract rejects null input");
    expect_int("invalid.bad_state",
               dm1_v1_viewport_f0111_door_panel_resolve_pc34(&bad_state, &out) ? 1 : 0, 0,
               "DEFS.H:1039-1044 only C0..C5 door states");
    expect_int("blend.c10",
               dm1_v1_viewport_f0111_door_panel_blend_pixel_pc34(0x44, 10, 10), 0x44,
               "DUNVIEW.C:4334 F0791 C10 transparent blit");
    expect_int("blend.opaque",
               dm1_v1_viewport_f0111_door_panel_blend_pixel_pc34(0x44, 0x55, 10), 0x55,
               "DUNVIEW.C:4334 F0791 opaque pixel writes");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const DM1_V1_F0111DoorPanelSpecPc34 *spec =
        dm1_v1_viewport_f0111_door_panel_spec_pc34();
    const char *e = dm1_v1_viewport_f0111_door_panel_source_evidence_pc34();

    expect_int("evidence.pointer", spec->source_lines == e ? 1 : 0, 1,
               "source evidence pointer");
    expect_contains("evidence.f0111", e, "DUNVIEW.C:4218-4339",
                    "ReDMCSB F0111 source block");
    expect_contains("evidence.open", e, "line 4248",
                    "DUNVIEW.C:4248 open guard");
    expect_contains("evidence.copy", e, "line 4260",
                    "DUNVIEW.C:4260 G0074 copy");
    expect_contains("evidence.thieves_eye", e, "4292-4294",
                    "DUNVIEW.C:4292-4294 Thieves Eye");
    expect_contains("evidence.zone_math", e, "4317-4325",
                    "DUNVIEW.C:4317-4325 partly-open zone math");
    expect_contains("evidence.final_blit", e, "line 4334",
                    "DUNVIEW.C:4334 F0791 blit");
    expect_contains("evidence.defs", e, "DEFS.H:1039-1044",
                    "DEFS.H door states");
    expect_contains("evidence.non_overlap", e, "existing door-front occlusion gates",
                    "non-duplication note");
}

int main(void)
{
    test_spec_metadata();
    test_open_state_skips_temporary_and_viewport_blit();
    test_closed_and_destroyed_composition_paths();
    test_partly_open_vertical_and_horizontal_zone_math();
    test_transparency_and_invalid_inputs();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_f0111_door_panel_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_f0111_door_panel_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}
