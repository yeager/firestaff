#include "csb_v1_viewport_d3c_f0107_f0108_first_backdrop_pc34_compat.h"

#include <string.h>

enum {
    CSB_PRESENT = 1,
    CSB_VIEW_SQUARE_D3C = 11,              /* ReDMCSB: DEFS.H:2607 M600_VIEW_SQUARE_D3C. */
    CSB_VIEW_WALL_D3C_FRONT = 5,           /* ReDMCSB: DEFS.H:2701 M578_VIEW_WALL_D3C_FRONT. */
    CSB_VIEW_FLOOR_D3C = 3,                /* ReDMCSB: DEFS.H:2753 M589_VIEW_FLOOR_D3C. */
    CSB_ZONE_WALL_ORNAMENT = 1004,         /* ReDMCSB: DEFS.H:4222 C1004_ZONE_WALL_ORNAMENT. */
    CSB_WALL_COORDINATE_SET_STRIDE = 15,   /* ReDMCSB: DUNVIEW.C:3587 CSB/I34 wall stride. */
    CSB_ZONE_FLOOR_ORNAMENT = 1500,        /* ReDMCSB: DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT. */
    CSB_FLOOR_COORDINATE_SET_STRIDE = 11,  /* ReDMCSB: DUNVIEW.C:3998 CSB/I34 floor stride. */
    CSB_TRANSPARENT_COLOR = 10,            /* ReDMCSB: DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_BACKDROP_COLOR = 31,
    CSB_WALL_ORNAMENT_COLOR = 47,
    CSB_FLOOR_ORNAMENT_COLOR = 63,
    CSB_BACKGROUND_COLOR = 0,
    CSB_EXPECTED_WALL_ORDINAL = 3,
    CSB_EXPECTED_FLOOR_ORDINAL = 5,
    CSB_EXPECTED_BACKDROP_ROOM_SLOT = 0,
    CSB_SYNTHETIC_COORDINATE_SET = 0
};

static const char s_source_evidence[] =
    "Contract-only synthetic pixel-composition gate; no real-asset bitmap "
    "parity is claimed. ReDMCSB DRAWVIEW.C:709-722 defines "
    "F0097_DUNGEONVIEW_DrawViewport as the final viewport commit request. "
    "DUNVIEW.C F0098:2962-3002 copies base floor/ceiling pixels before cell "
    "routes, and F0128:8337-8339 invokes it before the D3C sequence. "
    "DUNVIEW.C F0107:3502-3938 handles wall-ornament ordinal decrement, "
    "coordinate-set zone calculation at 3587, alcove/front metadata at "
    "3722-3747, CSB/I34 scaled draw path at 3817-3857, and the C10 "
    "transparent blit at 3922. DUNVIEW.C F0108:3940-4011 handles floor "
    "ornaments, computes C1500 + CoordinateSet*11 + ViewFloor at 3998, "
    "uses C10 transparency, and recurses for footprint masks at 4007-4008. "
    "DUNVIEW.C F0118:6642-6763 dispatches D3C wall/door/pit/teleporter "
    "routes; 6716 calls F0107 for M578_VIEW_WALL_D3C_FRONT, 6722 and 6814 "
    "call F0108 for M589_VIEW_FLOOR_D3C. DUNVIEW.C F0115:4547-4581 defines "
    "the thing-pass draw loop. DUNVIEW.C F0127:8294 is the post-D3C wall/"
    "thing follow-up reference, and F0128:8478-8508 commits D3L2/D3R2 and "
    "D3C then follows with near cells. DEFS.H:4056-4057 C716/C717 and "
    "2588-2598 M610/M611 anchor wall/floor/ceiling zone families; "
    "DEFS.H:2749-2760 anchors CSB/I34 floor views. CustomBackgrounds is "
    "cross-checked against CSB-lineage Viewport.cpp:1192-1209 open-cell "
    "draw order, 1903-1915 floor-decoration-before-front-door draw order, "
    "6507-6548 masked decoration blit, and the first-backdrop room/cell "
    "dispatch around 6800-6840.";

static const CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract s_contract = {
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_WIDTH_PC34,
    CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_HEIGHT_PC34,
    CSB_VIEW_SQUARE_D3C,
    CSB_VIEW_WALL_D3C_FRONT,
    CSB_VIEW_FLOOR_D3C,
    CSB_ZONE_WALL_ORNAMENT,
    CSB_WALL_COORDINATE_SET_STRIDE,
    CSB_ZONE_FLOOR_ORNAMENT,
    CSB_FLOOR_COORDINATE_SET_STRIDE,
    CSB_TRANSPARENT_COLOR,
    CSB_EXPECTED_WALL_ORDINAL,
    CSB_EXPECTED_WALL_ORDINAL - 1,
    CSB_EXPECTED_FLOOR_ORDINAL,
    CSB_EXPECTED_FLOOR_ORDINAL - 1,
    CSB_EXPECTED_BACKDROP_ROOM_SLOT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    { 74, 25, 149, 75 },
    { 88, 35, 135, 64 },
    { 96, 57, 127, 80 },
    "ReDMCSB DRAWVIEW.C:709-722 F0097_DUNGEONVIEW_DrawViewport",
    "ReDMCSB DUNVIEW.C F0098:2962-3002; F0128:8337-8339",
    "ReDMCSB DUNVIEW.C F0107:3502-3938; zone 3587; blit 3922",
    "ReDMCSB DUNVIEW.C F0108:3940-4011; zone 3998; mask recursion 4007-4008",
    "ReDMCSB DUNVIEW.C F0118:6642-6763; F0107 at 6716; F0108 at 6722/6814",
    "ReDMCSB DUNVIEW.C F0115:4547-4581",
    "ReDMCSB DUNVIEW.C F0127:8294 post-D3C wall/thing follow-up anchor",
    "ReDMCSB DUNVIEW.C F0128:8478-8508 D3C dispatch and view commit sequence",
    "ReDMCSB DEFS.H:4056-4057 C716/C717; 2588-2598 M610/M611; 2749-2760 floor views; 4222-4223 zones",
    "CSB-lineage Viewport.cpp:1192-1209,1903-1915,6507-6548,6800-6840",
    s_source_evidence
};

static int rect_contains(
    CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 rect,
    int x,
    int y)
{
    return x >= rect.x1 && x <= rect.x2 && y >= rect.y1 && y <= rect.y2;
}

static int rect_area(CSB_V1_ViewportD3cF0107F0108FirstBackdropRectPc34 rect)
{
    return ((rect.x2 - rect.x1) + 1) * ((rect.y2 - rect.y1) + 1);
}

static size_t pixel_offset(int x, int y)
{
    return ((size_t)y *
            CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_WIDTH_PC34) +
           (size_t)x;
}

static int floor_mask_is_transparent(int x, int y)
{
    return (x == 104 || x == 105) && (y == 60 || y == 61);
}

const CSB_V1_ViewportD3cF0107F0108FirstBackdropPc34Contract *
csb_v1_viewport_d3c_f0107_f0108_first_backdrop_contract_pc34(void)
{
    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 place backdrop/base pixels
     * before the D3C dispatch at lines 8498-8499. F0118 line 6716 anchors
     * the D3C-front F0107 wall-ornament route, while lines 6722 and 6814
     * anchor the D3C F0108 floor-ornament route. */
    return &s_contract;
}

const char *
csb_v1_viewport_d3c_f0107_f0108_first_backdrop_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int csb_v1_viewport_d3c_f0107_f0108_first_backdrop_plan_pc34(
    int wall_ornament_ordinal,
    int floor_ornament_ordinal,
    int first_backdrop_room_slot,
    CSB_V1_ViewportD3cF0107F0108FirstBackdropPlanPc34 *out_plan)
{
    CSB_V1_ViewportD3cF0107F0108FirstBackdropPlanPc34 plan;

    if (!out_plan) return -1;
    memset(&plan, 0, sizeof(plan));

    plan.wall_ornament_ordinal = wall_ornament_ordinal;
    plan.wall_ornament_index = wall_ornament_ordinal - 1;
    plan.floor_ornament_ordinal = floor_ornament_ordinal;
    plan.floor_ornament_index = floor_ornament_ordinal - 1;
    plan.first_backdrop_room_slot = first_backdrop_room_slot;
    plan.wall_ornament_zone = CSB_ZONE_WALL_ORNAMENT +
        (CSB_SYNTHETIC_COORDINATE_SET * CSB_WALL_COORDINATE_SET_STRIDE) +
        CSB_VIEW_WALL_D3C_FRONT;
    plan.floor_ornament_zone = CSB_ZONE_FLOOR_ORNAMENT +
        (CSB_SYNTHETIC_COORDINATE_SET * CSB_FLOOR_COORDINATE_SET_STRIDE) +
        CSB_VIEW_FLOOR_D3C;
    plan.backdrop_color = CSB_BACKDROP_COLOR;
    plan.wall_ornament_color = CSB_WALL_ORNAMENT_COLOR;
    plan.floor_ornament_color = CSB_FLOOR_ORNAMENT_COLOR;
    plan.masked_floor_source_color = CSB_TRANSPARENT_COLOR;
    plan.f0107_before_f0108 = CSB_PRESENT;
    plan.f0108_mask_preserves_f0107 = CSB_PRESENT;
    plan.distinct_layer_colors = CSB_PRESENT;
    plan.d3c_window = s_contract.d3c_window;
    plan.wall_ornament_window = s_contract.wall_ornament_window;
    plan.floor_ornament_window = s_contract.floor_ornament_window;
    plan.backdrop_only_x = 80;
    plan.backdrop_only_y = 30;
    plan.wall_only_x = 90;
    plan.wall_only_y = 40;
    plan.floor_opaque_x = 110;
    plan.floor_opaque_y = 70;
    plan.overlap_masked_x = 104;
    plan.overlap_masked_y = 60;
    plan.source_evidence = s_source_evidence;

    if (wall_ornament_ordinal == CSB_EXPECTED_WALL_ORDINAL &&
        floor_ornament_ordinal == CSB_EXPECTED_FLOOR_ORDINAL &&
        first_backdrop_room_slot == CSB_EXPECTED_BACKDROP_ROOM_SLOT) {
        /* ReDMCSB: DUNVIEW.C F0107 lines 3571-3576 convert an ordinal to
         * a zero-based wall-ornament index. F0108 lines 3973-3975 do the
         * same for floor ornaments, and F0108 line 3998 uses the D3C floor
         * view in the CSB/I34 eleven-slot floor zone namespace. */
        plan.ok = CSB_PRESENT;
        plan.draw_step_count = 3;
        plan.draw_steps[0] =
            CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_BACKDROP;
        plan.draw_steps[1] =
            CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_F0107_WALL_ORNAMENT;
        plan.draw_steps[2] =
            CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_STEP_F0108_FLOOR_ORNAMENT;
    }

    *out_plan = plan;
    return plan.ok ? 0 : 1;
}

int csb_v1_viewport_d3c_f0107_f0108_first_backdrop_run_pc34(
    const CSB_V1_ViewportD3cF0107F0108FirstBackdropPlanPc34 *plan,
    uint8_t *viewport,
    size_t viewport_len,
    CSB_V1_ViewportD3cF0107F0108FirstBackdropResultPc34 *out_result)
{
    CSB_V1_ViewportD3cF0107F0108FirstBackdropResultPc34 result;
    const size_t required_len =
        (size_t)CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_WIDTH_PC34 *
        (size_t)CSB_V1_D3C_F0107_F0108_FIRST_BACKDROP_VIEWPORT_HEIGHT_PC34;

    if (!plan || !viewport || !out_result || viewport_len < required_len ||
        !plan->ok || plan->draw_step_count != 3) {
        return -1;
    }

    memset(&result, 0, sizeof(result));
    memset(viewport, CSB_BACKGROUND_COLOR, required_len);
    result.draw_step_count = plan->draw_step_count;
    result.draw_steps[0] = plan->draw_steps[0];
    result.draw_steps[1] = plan->draw_steps[1];
    result.draw_steps[2] = plan->draw_steps[2];
    result.source_evidence = s_source_evidence;

    /* ReDMCSB: DUNVIEW.C F0098 lines 2962-3002 and F0128 lines 8337-8339
     * put the backdrop/base pass under later D3C cell work. */
    for (int y = plan->d3c_window.y1; y <= plan->d3c_window.y2; ++y) {
        for (int x = plan->d3c_window.x1; x <= plan->d3c_window.x2; ++x) {
            viewport[pixel_offset(x, y)] = (uint8_t)plan->backdrop_color;
            ++result.backdrop_pixels;
        }
    }

    /* ReDMCSB: DUNVIEW.C F0107 lines 3502-3938 draw the D3C-front wall
     * ornament with C10 transparency before any later floor-ornament layer
     * modeled by this composition gate. */
    for (int y = plan->wall_ornament_window.y1;
         y <= plan->wall_ornament_window.y2;
         ++y) {
        for (int x = plan->wall_ornament_window.x1;
             x <= plan->wall_ornament_window.x2;
             ++x) {
            viewport[pixel_offset(x, y)] = (uint8_t)plan->wall_ornament_color;
            ++result.wall_ornament_pixels;
        }
    }

    /* ReDMCSB: DUNVIEW.C F0108 lines 3940-4011 draw floor ornaments through
     * C10 transparency. The C10 source samples below must not clear pixels
     * already written by F0107 at the same D3C viewport coordinate. */
    for (int y = plan->floor_ornament_window.y1;
         y <= plan->floor_ornament_window.y2;
         ++y) {
        for (int x = plan->floor_ornament_window.x1;
             x <= plan->floor_ornament_window.x2;
             ++x) {
            uint8_t *pixel = &viewport[pixel_offset(x, y)];
            if (rect_contains(plan->wall_ornament_window, x, y)) {
                ++result.overlap_pixels;
            }
            if (x == plan->overlap_masked_x && y == plan->overlap_masked_y) {
                result.pixel_before_f0108_at_masked_overlap = *pixel;
            }
            if (x == plan->floor_opaque_x && y == plan->floor_opaque_y) {
                result.pixel_before_f0108_at_opaque_floor = *pixel;
            }
            if (floor_mask_is_transparent(x, y)) {
                ++result.floor_ornament_masked_pixels;
            } else {
                *pixel = (uint8_t)plan->floor_ornament_color;
                ++result.floor_ornament_opaque_pixels;
            }
            if (x == plan->overlap_masked_x && y == plan->overlap_masked_y) {
                result.pixel_after_f0108_at_masked_overlap = *pixel;
            }
            if (x == plan->floor_opaque_x && y == plan->floor_opaque_y) {
                result.pixel_after_f0108_at_opaque_floor = *pixel;
            }
        }
    }

    result.final_backdrop_only_pixel =
        viewport[pixel_offset(plan->backdrop_only_x, plan->backdrop_only_y)];
    result.final_wall_only_pixel =
        viewport[pixel_offset(plan->wall_only_x, plan->wall_only_y)];
    result.final_floor_opaque_pixel =
        viewport[pixel_offset(plan->floor_opaque_x, plan->floor_opaque_y)];
    result.final_overlap_masked_pixel =
        viewport[pixel_offset(plan->overlap_masked_x, plan->overlap_masked_y)];
    result.f0108_mask_did_not_erase_f0107 =
        result.pixel_before_f0108_at_masked_overlap == plan->wall_ornament_color &&
        result.pixel_after_f0108_at_masked_overlap == plan->wall_ornament_color &&
        result.final_overlap_masked_pixel == plan->wall_ornament_color;
    result.f0108_opaque_pixel_overwrote_destination =
        result.pixel_before_f0108_at_opaque_floor == plan->backdrop_color &&
        result.pixel_after_f0108_at_opaque_floor == plan->floor_ornament_color;
    result.ok =
        result.backdrop_pixels == rect_area(plan->d3c_window) &&
        result.wall_ornament_pixels == rect_area(plan->wall_ornament_window) &&
        result.floor_ornament_opaque_pixels +
            result.floor_ornament_masked_pixels ==
            rect_area(plan->floor_ornament_window) &&
        result.f0108_mask_did_not_erase_f0107 &&
        result.f0108_opaque_pixel_overwrote_destination;

    *out_result = result;
    return result.ok ? 0 : 1;
}
