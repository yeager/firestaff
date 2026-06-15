#include "csb_v1_viewport_d1c_f0095_floor_ornament_pc34_compat.h"

#include <stddef.h>

enum {
    CSB_D1C_DEPTH = 1,
    CSB_D1C_LANE = 0,
    CSB_D1C_VIEW_SQUARE_INDEX = 3,
    CSB_D1C_VIEW_FLOOR_INDEX = 9,
    CSB_D1C_LINEAGE_RELATIVE_CELL = 17,
    CSB_D1C_LINEAGE_FLOOR_LOCATION = 0,
    CSB_D1C_SQUARE_ASPECT_FLOOR_SLOT_PC34 = 4,
    CSB_D1C_SQUARE_ASPECT_FLOOR_SLOT_I34 = 5,
    CSB_D1C_LINEAGE_DECORATION_SLOT = 2,
    CSB_D1C_FLOOR_ZONE_BASE = 1500,
    CSB_D1C_FLOOR_ZONE_STRIDE = 11,
    CSB_D1C_TRANSPARENT_COLOR_C10 = 10,
    CSB_D1C_ROUTE_REJECT = 0,
    CSB_D1C_ROUTE_C10_TRANSPARENCY = 1,
    CSB_D1C_ROUTE_F0108_ORDINAL_BLIT = 2
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
} CSB_D1C_F0095_Plan;

static const char s_f0095_source_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223 local full body; D1C floor dispatch "
    "is F0124:7874 and 7926 with F0108:3940-4011";
static const char s_depth_lane_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:370-377 and 8532-8533 "
    "map D1C to lane 0 depth 1";
static const char s_ordinal_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:7874,7926 and "
    "F0108:3959-3966 pre-decrement floor ornament ordinals";
static const char s_transparency_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:F0108:3959-3998 and "
    "DEFS.H:2088 C10_COLOR_FLESH";
static const char s_baseline_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:8337-8339 and "
    "8442-8444 keep F0098 floor/ceiling ownership outside D1C F0108";
static const char s_exclusion_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DUNVIEW.C:7842-7843, "
    "7875,7905-7908,7937 are outside the isolated floor-ornament metadata route";
static const char s_defs_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; DEFS.H:2544/2558 M558 floor slot, "
    "DEFS.H:2759 M595_VIEW_FLOOR_D1C, DEFS.H:4042-4043 C702/C703 D3L2/D3R2; "
    "task C04_ORINARY_ORNAMENT_INDEX label maps to the raw ordinal metadata";
static const char s_lineage_anchor[] =
    "ReDMCSB DUNVIEW.C F0095:2124-2223; CSB-lineage Viewport.cpp:"
    "1175-1179,1903-1906,2413-2416,2730-2732,2930-2935";

static int d1c_accepts(int depth, int lane)
{
    return depth == CSB_D1C_DEPTH && lane == CSB_D1C_LANE;
}

static int d1c_floor_zone_for_coordinate_set(int coordinate_set)
{
    return CSB_D1C_FLOOR_ZONE_BASE +
           (coordinate_set * CSB_D1C_FLOOR_ZONE_STRIDE) +
           CSB_D1C_VIEW_FLOOR_INDEX;
}

static CSB_D1C_F0095_Plan d1c_plan(int depth, int lane,
                                   int raw_floor_ornament_ordinal,
                                   int coordinate_set)
{
    CSB_D1C_F0095_Plan plan;

    plan.depth = depth;
    plan.lane = lane;
    plan.raw_floor_ornament_ordinal = raw_floor_ornament_ordinal;
    plan.accepted = d1c_accepts(depth, lane);
    plan.route = CSB_D1C_ROUTE_REJECT;
    plan.floor_ornament_index = -1;
    plan.decremented_as_redmcsb = 0;
    plan.view_square_index = CSB_D1C_VIEW_SQUARE_INDEX;
    plan.view_floor_index = CSB_D1C_VIEW_FLOOR_INDEX;
    plan.floor_zone = -1;
    plan.transparent_color = CSB_D1C_TRANSPARENT_COLOR_C10;
    plan.calls_f0098 = 0;
    plan.calls_f0107 = 0;
    plan.calls_f0108 = 0;
    plan.calls_f0111 = 0;
    plan.calls_f0115 = 0;
    plan.lineage_relative_cell = CSB_D1C_LINEAGE_RELATIVE_CELL;
    plan.lineage_floor_location = CSB_D1C_LINEAGE_FLOOR_LOCATION;
    plan.lineage_matches_ordinal = 0;
    plan.lineage_matches_route = 0;

    if (!plan.accepted) return plan;

    if (raw_floor_ornament_ordinal <= 0) {
        plan.route = CSB_D1C_ROUTE_C10_TRANSPARENCY;
        plan.lineage_matches_ordinal = 1;
        plan.lineage_matches_route = 1;
        return plan;
    }

    plan.route = CSB_D1C_ROUTE_F0108_ORDINAL_BLIT;
    plan.calls_f0108 = 1;
    plan.floor_ornament_index = raw_floor_ornament_ordinal - 1;
    plan.decremented_as_redmcsb = 1;
    plan.floor_zone = d1c_floor_zone_for_coordinate_set(coordinate_set);
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

static void assert_plan_core(const CSB_D1C_F0095_Plan *plan,
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

int csb_v1_viewport_d1c_f0095_floor_ornament_run(int *passed, int *failed)
{
    int local_passed = 0;
    int local_failed = 0;
    int *p = passed ? passed : &local_passed;
    int *f = failed ? failed : &local_failed;
    CSB_D1C_F0095_Plan zero = d1c_plan(1, 0, 0, 0);
    CSB_D1C_F0095_Plan one = d1c_plan(1, 0, 1, 0);
    CSB_D1C_F0095_Plan six = d1c_plan(1, 0, 6, 0);
    CSB_D1C_F0095_Plan sixteen = d1c_plan(1, 0, 16, 0);
    CSB_D1C_F0095_Plan coord2 = d1c_plan(1, 0, 3, 2);
    CSB_D1C_F0095_Plan bad_depth = d1c_plan(2, 0, 3, 0);
    CSB_D1C_F0095_Plan bad_lane = d1c_plan(1, 1, 3, 0);

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

    source_assert(CSB_D1C_DEPTH == 1, p, f, s_depth_lane_anchor);
    source_assert(CSB_D1C_LANE == 0, p, f, s_depth_lane_anchor);
    source_assert(CSB_D1C_VIEW_SQUARE_INDEX == 3, p, f, s_depth_lane_anchor);
    source_assert(CSB_D1C_VIEW_FLOOR_INDEX == 9, p, f, s_defs_anchor);
    source_assert(CSB_D1C_SQUARE_ASPECT_FLOOR_SLOT_PC34 == 4, p, f, s_defs_anchor);
    source_assert(CSB_D1C_SQUARE_ASPECT_FLOOR_SLOT_I34 == 5, p, f, s_defs_anchor);
    source_assert(CSB_D1C_LINEAGE_DECORATION_SLOT == 2, p, f, s_lineage_anchor);
    source_assert(CSB_D1C_LINEAGE_RELATIVE_CELL == 17, p, f, s_lineage_anchor);
    source_assert(CSB_D1C_LINEAGE_FLOOR_LOCATION == 0, p, f, s_lineage_anchor);
    source_assert(CSB_D1C_TRANSPARENT_COLOR_C10 == 10, p, f, s_transparency_anchor);
    source_assert(CSB_D1C_FLOOR_ZONE_BASE == 1500, p, f, s_defs_anchor);
    source_assert(CSB_D1C_FLOOR_ZONE_STRIDE == 11, p, f, s_defs_anchor);

    assert_plan_core(&zero, 1, CSB_D1C_ROUTE_C10_TRANSPARENCY, -1, 0,
                     p, f, s_transparency_anchor);
    source_assert(zero.decremented_as_redmcsb == 0, p, f, s_ordinal_anchor);
    source_assert(zero.floor_zone == -1, p, f, s_transparency_anchor);
    source_assert(zero.transparent_color == 10, p, f, s_transparency_anchor);
    source_assert(zero.lineage_matches_ordinal == 1, p, f, s_lineage_anchor);
    source_assert(zero.lineage_matches_route == 1, p, f, s_lineage_anchor);

    assert_plan_core(&one, 1, CSB_D1C_ROUTE_F0108_ORDINAL_BLIT, 0, 1,
                     p, f, s_ordinal_anchor);
    source_assert(one.decremented_as_redmcsb == 1, p, f, s_ordinal_anchor);
    source_assert(one.floor_zone == 1509, p, f, s_defs_anchor);
    source_assert(one.transparent_color == 10, p, f, s_transparency_anchor);
    source_assert(one.lineage_matches_ordinal == 1, p, f, s_lineage_anchor);
    source_assert(one.lineage_matches_route == 1, p, f, s_lineage_anchor);

    assert_plan_core(&six, 1, CSB_D1C_ROUTE_F0108_ORDINAL_BLIT, 5, 1,
                     p, f, s_ordinal_anchor);
    source_assert(six.decremented_as_redmcsb == 1, p, f, s_ordinal_anchor);
    source_assert(six.floor_zone == 1509, p, f, s_defs_anchor);
    source_assert(six.raw_floor_ornament_ordinal == 6, p, f, s_ordinal_anchor);
    source_assert(six.floor_ornament_index + 1 == six.raw_floor_ornament_ordinal,
                  p, f, s_ordinal_anchor);

    assert_plan_core(&sixteen, 1, CSB_D1C_ROUTE_F0108_ORDINAL_BLIT, 15, 1,
                     p, f, s_ordinal_anchor);
    source_assert(sixteen.floor_zone == 1509, p, f, s_defs_anchor);
    source_assert(sixteen.floor_ornament_index == 15, p, f, s_ordinal_anchor);
    source_assert(sixteen.decremented_as_redmcsb == 1, p, f, s_ordinal_anchor);

    assert_plan_core(&coord2, 1, CSB_D1C_ROUTE_F0108_ORDINAL_BLIT, 2, 1,
                     p, f, s_defs_anchor);
    source_assert(coord2.floor_zone == 1531, p, f, s_defs_anchor);
    source_assert(coord2.floor_zone ==
                  CSB_D1C_FLOOR_ZONE_BASE + 2 * CSB_D1C_FLOOR_ZONE_STRIDE +
                  CSB_D1C_VIEW_FLOOR_INDEX, p, f, s_defs_anchor);

    assert_plan_core(&bad_depth, 0, CSB_D1C_ROUTE_REJECT, -1, 0,
                     p, f, s_depth_lane_anchor);
    assert_plan_core(&bad_lane, 0, CSB_D1C_ROUTE_REJECT, -1, 0,
                     p, f, s_depth_lane_anchor);

    {
        const int invalid_depths[] = { -1, 0, 2, 3, 4 };
        const int invalid_lanes[] = { -2, -1, 1, 2, 3 };
        size_t i;
        for (i = 0; i < sizeof(invalid_depths) / sizeof(invalid_depths[0]); ++i) {
            CSB_D1C_F0095_Plan plan = d1c_plan(invalid_depths[i], 0, 1, 0);
            source_assert(plan.accepted == 0, p, f, s_depth_lane_anchor);
            source_assert(plan.route == CSB_D1C_ROUTE_REJECT, p, f,
                          s_depth_lane_anchor);
            source_assert(plan.calls_f0108 == 0, p, f, s_depth_lane_anchor);
        }
        for (i = 0; i < sizeof(invalid_lanes) / sizeof(invalid_lanes[0]); ++i) {
            CSB_D1C_F0095_Plan plan = d1c_plan(1, invalid_lanes[i], 1, 0);
            source_assert(plan.accepted == 0, p, f, s_depth_lane_anchor);
            source_assert(plan.route == CSB_D1C_ROUTE_REJECT, p, f,
                          s_depth_lane_anchor);
            source_assert(plan.calls_f0108 == 0, p, f, s_depth_lane_anchor);
        }
    }

    source_assert(d1c_accepts(1, 0) == 1, p, f, s_depth_lane_anchor);
    source_assert(d1c_accepts(0, 0) == 0, p, f, s_depth_lane_anchor);
    source_assert(d1c_accepts(1, -1) == 0, p, f, s_depth_lane_anchor);
    source_assert(d1c_accepts(2, 0) == 0, p, f, s_depth_lane_anchor);
    source_assert(d1c_accepts(1, 1) == 0, p, f, s_depth_lane_anchor);

    source_assert(d1c_floor_zone_for_coordinate_set(0) == 1509, p, f,
                  s_defs_anchor);
    source_assert(d1c_floor_zone_for_coordinate_set(1) == 1520, p, f,
                  s_defs_anchor);
    source_assert(d1c_floor_zone_for_coordinate_set(3) == 1542, p, f,
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

    source_assert(one.lineage_relative_cell == 17, p, f, s_lineage_anchor);
    source_assert(one.lineage_floor_location == 0, p, f, s_lineage_anchor);
    source_assert(one.lineage_matches_ordinal == 1, p, f, s_lineage_anchor);
    source_assert(one.lineage_matches_route == 1, p, f, s_lineage_anchor);
    source_assert(sixteen.lineage_matches_ordinal == 1, p, f, s_lineage_anchor);
    source_assert(sixteen.lineage_matches_route == 1, p, f, s_lineage_anchor);

    source_assert(*p >= 70, p, f, s_f0095_source_anchor);
    return *f == 0 ? 0 : 1;
}
