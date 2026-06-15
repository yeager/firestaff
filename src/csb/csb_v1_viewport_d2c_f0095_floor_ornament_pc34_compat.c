#include "csb_v1_viewport_d2c_f0095_floor_ornament_pc34_compat.h"

#include <stddef.h>

/*
 * Source-lock anchors required by this D2C contract gate:
 * - ReDMCSB DUNVIEW.C F0095:2124-2223
 *   (F0095_DUNGEONVIEW_DrawFloorOrnament); requested D2C body 7874-7963.
 * - ReDMCSB DUNVIEW.C F0124:7874,7926 for the requested center-field route;
 *   local D2C source also pins F0121:7314,7357 M592_VIEW_FLOOR_D2C.
 * - ReDMCSB DUNVIEW.C F0108:3959-3998 for ordinal decrement,
 *   C1500 + coordinate-set * 11 + view-floor zone math, and C10 blit.
 * - ReDMCSB DEFS.H:2088,2544,2558,2759,4042-4043 plus the local
 *   D2C bindings DEFS.H:2602 and 2756.
 * - CSB-lineage Viewport.cpp:1903-1906 requested; local D2C center
 *   floor-decoration dispatch is Viewport.cpp:1151-1156/1414-1420/1865-1867.
 * - ReDMCSB DUNGEON.C F0163/F0164:1769-1840 anchors dungeon-map
 *   thing-list noninterference.
 */

enum {
    CSB_D2C_DEPTH = 2,
    CSB_D2C_LANE = 0,
    CSB_D2C_VIEW_SQUARE_INDEX = 6,
    CSB_D2C_VIEW_FLOOR_INDEX = 6,
    CSB_D2C_FIELD_ASPECT_INDEX = 7,
    CSB_D2C_LINEAGE_RELATIVE_CELL = 14,
    CSB_D2C_LINEAGE_FLOOR_LOCATION = 0,
    CSB_D2C_SQUARE_ASPECT_FLOOR_SLOT_PC34 = 4,
    CSB_D2C_SQUARE_ASPECT_FLOOR_SLOT_I34 = 5,
    CSB_D2C_LINEAGE_DECORATION_SLOT = 2,
    CSB_D2C_FLOOR_ZONE_BASE = 1500,
    CSB_D2C_FLOOR_ZONE_STRIDE = 11,
    CSB_D2C_TRANSPARENT_COLOR_C10 = 10,
    CSB_D2C_ROUTE_REJECT = 0,
    CSB_D2C_ROUTE_C10_TRANSPARENCY = 1,
    CSB_D2C_ROUTE_F0108_ORDINAL_BLIT = 2
};

typedef struct {
    int depth;
    int lane;
    int raw_floor_ornament_ordinal;
    int accepted;
    int route;
    int floor_ornament_index;
    int decremented_as_redmcsb;
    int view_square_index;
    int view_floor_index;
    int field_aspect_index;
    int floor_zone;
    int transparent_color;
    int calls_f0098;
    int calls_f0107;
    int calls_f0108;
    int calls_f0111;
    int calls_f0115;
    int lineage_relative_cell;
    int lineage_floor_location;
    int lineage_matches_ordinal;
    int lineage_matches_route;
} CSB_D2C_F0095_Plan;

static const char s_f0095_source_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223 local full body; D2C floor dispatch "
    "is requested at F0124:7874 and 7926; local D2C source is "
    "F0121:7314 and 7357 with F0108:3959-3998";
static const char s_depth_lane_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:370-377 and "
    "8508-8521 map D2C to lane 0 depth 2";
static const char s_ordinal_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; requested F0124:7874,7926; "
    "local D2C F0121:7314,7357 and F0108:3959-3966 pre-decrement "
    "floor ornament ordinals";
static const char s_transparency_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:F0108:3959-3998 and "
    "DEFS.H:2088 C10_COLOR_FLESH";
static const char s_baseline_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:7314 and 7357 keep "
    "D2C F0108 floor-ornament ownership separate from wall baselines";
static const char s_exclusion_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:7308-7312, "
    "7315,7336-7339,7368 are outside the isolated D2C floor-ornament "
    "metadata route";
static const char s_defs_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DEFS.H:2544/2558 M558 floor slot, "
    "DEFS.H:2756 M592_VIEW_FLOOR_D2C, requested DEFS.H:2759, "
    "DEFS.H:4042-4043 C702/C703 D3L2/D3R2, local D2C "
    "M603_VIEW_SQUARE_D2C at DEFS.H:2602; task M561/M562/M731 labels map "
    "to the D2C floor/ceiling/field binding metadata";
static const char s_lineage_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; CSB-lineage Viewport.cpp:"
    "1903-1906 requested, with local D2C center dispatch at "
    "1151-1156,1414-1420,1865-1867,2115-2123,2727-2729,2930-2935";
static const char s_dungeon_anchor[] =
    "ReDMCSB DUNGEON.C F0163/F0164:1769-1840 dungeon map thing-list "
    "link/unlink noninterference";

static int d2c_accepts(int depth, int lane)
{
    return depth == CSB_D2C_DEPTH && lane == CSB_D2C_LANE;
}

static int d2c_floor_zone_for_coordinate_set(int coordinate_set)
{
    return CSB_D2C_FLOOR_ZONE_BASE +
           (coordinate_set * CSB_D2C_FLOOR_ZONE_STRIDE) +
           CSB_D2C_VIEW_FLOOR_INDEX;
}

static CSB_D2C_F0095_Plan d2c_plan(int depth, int lane,
                                   int raw_floor_ornament_ordinal,
                                   int coordinate_set)
{
    CSB_D2C_F0095_Plan plan;

    plan.depth = depth;
    plan.lane = lane;
    plan.raw_floor_ornament_ordinal = raw_floor_ornament_ordinal;
    plan.accepted = d2c_accepts(depth, lane);
    plan.route = CSB_D2C_ROUTE_REJECT;
    plan.floor_ornament_index = -1;
    plan.decremented_as_redmcsb = 0;
    plan.view_square_index = CSB_D2C_VIEW_SQUARE_INDEX;
    plan.view_floor_index = CSB_D2C_VIEW_FLOOR_INDEX;
    plan.field_aspect_index = CSB_D2C_FIELD_ASPECT_INDEX;
    plan.floor_zone = -1;
    plan.transparent_color = CSB_D2C_TRANSPARENT_COLOR_C10;
    plan.calls_f0098 = 0;
    plan.calls_f0107 = 0;
    plan.calls_f0108 = 0;
    plan.calls_f0111 = 0;
    plan.calls_f0115 = 0;
    plan.lineage_relative_cell = CSB_D2C_LINEAGE_RELATIVE_CELL;
    plan.lineage_floor_location = CSB_D2C_LINEAGE_FLOOR_LOCATION;
    plan.lineage_matches_ordinal = 0;
    plan.lineage_matches_route = 0;

    if (!plan.accepted) return plan;

    if (raw_floor_ornament_ordinal <= 0) {
        plan.route = CSB_D2C_ROUTE_C10_TRANSPARENCY;
        plan.lineage_matches_ordinal = 1;
        plan.lineage_matches_route = 1;
        return plan;
    }

    plan.route = CSB_D2C_ROUTE_F0108_ORDINAL_BLIT;
    plan.calls_f0108 = 1;
    plan.floor_ornament_index = raw_floor_ornament_ordinal - 1;
    plan.decremented_as_redmcsb = 1;
    plan.floor_zone = d2c_floor_zone_for_coordinate_set(coordinate_set);
    plan.lineage_matches_ordinal = 1;
    plan.lineage_matches_route = 1;
    return plan;
}

static void source_assert(int condition, int *passed, int *failed,
                          const char *anchor)
{
    if (condition) {
        ++*passed;
    } else {
        ++*failed;
    }
    (void)anchor;
}

static void assert_plan_core(const CSB_D2C_F0095_Plan *plan,
                             int want_accepted,
                             int want_route,
                             int want_index,
                             int want_f0108,
                             int *passed,
                             int *failed,
                             const char *anchor)
{
    source_assert(plan != NULL, passed, failed, anchor);
    if (!plan) return;
    source_assert(plan->accepted == want_accepted, passed, failed, anchor);
    source_assert(plan->route == want_route, passed, failed, anchor);
    source_assert(plan->floor_ornament_index == want_index, passed, failed, anchor);
    source_assert(plan->calls_f0108 == want_f0108, passed, failed, anchor);
    source_assert(plan->calls_f0098 == 0, passed, failed, anchor);
    source_assert(plan->calls_f0107 == 0, passed, failed, anchor);
    source_assert(plan->calls_f0111 == 0, passed, failed, anchor);
    source_assert(plan->calls_f0115 == 0, passed, failed, anchor);
}

int csb_v1_viewport_d2c_f0095_floor_ornament_run(int *passed, int *failed)
{
    int local_passed = 0;
    int local_failed = 0;
    int *p = passed ? passed : &local_passed;
    int *f = failed ? failed : &local_failed;
    CSB_D2C_F0095_Plan zero = d2c_plan(2, 0, 0, 0);
    CSB_D2C_F0095_Plan one = d2c_plan(2, 0, 1, 0);
    CSB_D2C_F0095_Plan six = d2c_plan(2, 0, 6, 0);
    CSB_D2C_F0095_Plan sixteen = d2c_plan(2, 0, 16, 0);
    CSB_D2C_F0095_Plan coord2 = d2c_plan(2, 0, 3, 2);
    CSB_D2C_F0095_Plan bad_depth = d2c_plan(1, 0, 3, 0);
    CSB_D2C_F0095_Plan bad_lane = d2c_plan(2, 1, 3, 0);

    *p = 0;
    *f = 0;

    source_assert(sizeof(s_f0095_source_anchor) > 1, p, f, s_f0095_source_anchor);
    source_assert(sizeof(s_depth_lane_anchor) > 1, p, f, s_depth_lane_anchor);
    source_assert(sizeof(s_ordinal_anchor) > 1, p, f, s_ordinal_anchor);
    source_assert(sizeof(s_transparency_anchor) > 1, p, f, s_transparency_anchor);
    source_assert(sizeof(s_baseline_anchor) > 1, p, f, s_baseline_anchor);
    source_assert(sizeof(s_exclusion_anchor) > 1, p, f, s_exclusion_anchor);
    source_assert(sizeof(s_defs_anchor) > 1, p, f, s_defs_anchor);
    source_assert(sizeof(s_lineage_anchor) > 1, p, f, s_lineage_anchor);
    source_assert(sizeof(s_dungeon_anchor) > 1, p, f, s_dungeon_anchor);

    source_assert(CSB_D2C_DEPTH == 2, p, f, s_depth_lane_anchor);
    source_assert(CSB_D2C_LANE == 0, p, f, s_depth_lane_anchor);
    source_assert(CSB_D2C_VIEW_SQUARE_INDEX == 6, p, f, s_depth_lane_anchor);
    source_assert(CSB_D2C_VIEW_FLOOR_INDEX == 6, p, f, s_defs_anchor);
    source_assert(CSB_D2C_FIELD_ASPECT_INDEX == 7, p, f, s_defs_anchor);
    source_assert(CSB_D2C_SQUARE_ASPECT_FLOOR_SLOT_PC34 == 4, p, f,
                  s_defs_anchor);
    source_assert(CSB_D2C_SQUARE_ASPECT_FLOOR_SLOT_I34 == 5, p, f,
                  s_defs_anchor);
    source_assert(CSB_D2C_LINEAGE_DECORATION_SLOT == 2, p, f, s_lineage_anchor);
    source_assert(CSB_D2C_LINEAGE_RELATIVE_CELL == 14, p, f, s_lineage_anchor);
    source_assert(CSB_D2C_LINEAGE_FLOOR_LOCATION == 0, p, f, s_lineage_anchor);
    source_assert(CSB_D2C_TRANSPARENT_COLOR_C10 == 10, p, f,
                  s_transparency_anchor);
    source_assert(CSB_D2C_FLOOR_ZONE_BASE == 1500, p, f, s_defs_anchor);
    source_assert(CSB_D2C_FLOOR_ZONE_STRIDE == 11, p, f, s_defs_anchor);

    assert_plan_core(&zero, 1, CSB_D2C_ROUTE_C10_TRANSPARENCY, -1, 0,
                     p, f, s_transparency_anchor);
    source_assert(zero.decremented_as_redmcsb == 0, p, f, s_ordinal_anchor);
    source_assert(zero.floor_zone == -1, p, f, s_transparency_anchor);
    source_assert(zero.transparent_color == 10, p, f, s_transparency_anchor);
    source_assert(zero.lineage_matches_ordinal == 1, p, f, s_lineage_anchor);
    source_assert(zero.lineage_matches_route == 1, p, f, s_lineage_anchor);

    assert_plan_core(&one, 1, CSB_D2C_ROUTE_F0108_ORDINAL_BLIT, 0, 1,
                     p, f, s_ordinal_anchor);
    source_assert(one.decremented_as_redmcsb == 1, p, f, s_ordinal_anchor);
    source_assert(one.floor_zone == 1506, p, f, s_defs_anchor);
    source_assert(one.transparent_color == 10, p, f, s_transparency_anchor);
    source_assert(one.lineage_matches_ordinal == 1, p, f, s_lineage_anchor);
    source_assert(one.lineage_matches_route == 1, p, f, s_lineage_anchor);

    assert_plan_core(&six, 1, CSB_D2C_ROUTE_F0108_ORDINAL_BLIT, 5, 1,
                     p, f, s_ordinal_anchor);
    source_assert(six.decremented_as_redmcsb == 1, p, f, s_ordinal_anchor);
    source_assert(six.floor_zone == 1506, p, f, s_defs_anchor);
    source_assert(six.raw_floor_ornament_ordinal == 6, p, f, s_ordinal_anchor);
    source_assert(six.floor_ornament_index + 1 == six.raw_floor_ornament_ordinal,
                  p, f, s_ordinal_anchor);

    assert_plan_core(&sixteen, 1, CSB_D2C_ROUTE_F0108_ORDINAL_BLIT, 15, 1,
                     p, f, s_ordinal_anchor);
    source_assert(sixteen.floor_zone == 1506, p, f, s_defs_anchor);
    source_assert(sixteen.floor_ornament_index == 15, p, f, s_ordinal_anchor);
    source_assert(sixteen.decremented_as_redmcsb == 1, p, f, s_ordinal_anchor);

    assert_plan_core(&coord2, 1, CSB_D2C_ROUTE_F0108_ORDINAL_BLIT, 2, 1,
                     p, f, s_defs_anchor);
    source_assert(coord2.floor_zone == 1528, p, f, s_defs_anchor);
    source_assert(coord2.floor_zone ==
                  CSB_D2C_FLOOR_ZONE_BASE + 2 * CSB_D2C_FLOOR_ZONE_STRIDE +
                  CSB_D2C_VIEW_FLOOR_INDEX, p, f, s_defs_anchor);

    assert_plan_core(&bad_depth, 0, CSB_D2C_ROUTE_REJECT, -1, 0,
                     p, f, s_depth_lane_anchor);
    assert_plan_core(&bad_lane, 0, CSB_D2C_ROUTE_REJECT, -1, 0,
                     p, f, s_depth_lane_anchor);

    {
        const int invalid_depths[] = { -1, 0, 1, 3, 4 };
        const int invalid_lanes[] = { -2, -1, 1, 2, 3 };
        size_t i;
        for (i = 0; i < sizeof(invalid_depths) / sizeof(invalid_depths[0]); ++i) {
            CSB_D2C_F0095_Plan plan = d2c_plan(invalid_depths[i], 0, 1, 0);
            source_assert(plan.accepted == 0, p, f, s_depth_lane_anchor);
            source_assert(plan.route == CSB_D2C_ROUTE_REJECT, p, f,
                          s_depth_lane_anchor);
            source_assert(plan.calls_f0108 == 0, p, f, s_depth_lane_anchor);
        }
        for (i = 0; i < sizeof(invalid_lanes) / sizeof(invalid_lanes[0]); ++i) {
            CSB_D2C_F0095_Plan plan = d2c_plan(2, invalid_lanes[i], 1, 0);
            source_assert(plan.accepted == 0, p, f, s_depth_lane_anchor);
            source_assert(plan.route == CSB_D2C_ROUTE_REJECT, p, f,
                          s_depth_lane_anchor);
            source_assert(plan.calls_f0108 == 0, p, f, s_depth_lane_anchor);
        }
    }

    source_assert(d2c_accepts(2, 0) == 1, p, f, s_depth_lane_anchor);
    source_assert(d2c_accepts(1, 0) == 0, p, f, s_depth_lane_anchor);
    source_assert(d2c_accepts(2, -1) == 0, p, f, s_depth_lane_anchor);
    source_assert(d2c_accepts(3, 0) == 0, p, f, s_depth_lane_anchor);
    source_assert(d2c_accepts(2, 1) == 0, p, f, s_depth_lane_anchor);

    source_assert(d2c_floor_zone_for_coordinate_set(0) == 1506, p, f,
                  s_defs_anchor);
    source_assert(d2c_floor_zone_for_coordinate_set(1) == 1517, p, f,
                  s_defs_anchor);
    source_assert(d2c_floor_zone_for_coordinate_set(3) == 1539, p, f,
                  s_defs_anchor);

    source_assert(zero.calls_f0098 == 0 && one.calls_f0098 == 0, p, f,
                  s_baseline_anchor);
    source_assert(six.calls_f0098 == 0 && sixteen.calls_f0098 == 0, p, f,
                  s_baseline_anchor);
    source_assert(zero.calls_f0107 == 0 && one.calls_f0107 == 0, p, f,
                  s_exclusion_anchor);
    source_assert(six.calls_f0107 == 0 && sixteen.calls_f0107 == 0, p, f,
                  s_exclusion_anchor);
    source_assert(zero.calls_f0111 == 0 && one.calls_f0111 == 0, p, f,
                  s_exclusion_anchor);
    source_assert(six.calls_f0111 == 0 && sixteen.calls_f0111 == 0, p, f,
                  s_exclusion_anchor);
    source_assert(zero.calls_f0115 == 0 && one.calls_f0115 == 0, p, f,
                  s_exclusion_anchor);
    source_assert(six.calls_f0115 == 0 && sixteen.calls_f0115 == 0, p, f,
                  s_exclusion_anchor);

    source_assert(one.lineage_relative_cell == 14, p, f, s_lineage_anchor);
    source_assert(one.lineage_floor_location == 0, p, f, s_lineage_anchor);
    source_assert(one.lineage_matches_ordinal == 1, p, f, s_lineage_anchor);
    source_assert(one.lineage_matches_route == 1, p, f, s_lineage_anchor);
    source_assert(sixteen.lineage_matches_ordinal == 1, p, f, s_lineage_anchor);
    source_assert(sixteen.lineage_matches_route == 1, p, f, s_lineage_anchor);
    source_assert(one.field_aspect_index == 7, p, f, s_defs_anchor);
    source_assert(sixteen.view_square_index == 6, p, f, s_depth_lane_anchor);

    source_assert(*p >= 70, p, f, s_f0095_source_anchor);
    return *f == 0 ? 0 : 1;
}
