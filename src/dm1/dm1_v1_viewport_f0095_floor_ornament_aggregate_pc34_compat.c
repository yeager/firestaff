#include "dm1_v1_viewport_f0095_floor_ornament_aggregate_pc34_compat.h"

#include <stddef.h>
#include <string.h>

enum {
    DM1_F0095_ROUTE_REJECTED_LINE = -1,
    DM1_F0095_FLOOR_ZONE_BASE = 1500,
    DM1_F0095_FLOOR_ZONE_STRIDE = 11
};

/*
 * Contract-only source-lock gate.
 * ReDMCSB anchors:
 * - DUNVIEW.C F0095:2124-2223 keeps the wall-set/G0095 binding in the
 *   loaded graphics family that this row must not conflate with F0108.
 * - DUNVIEW.C F0108:3959-4008 gates floor ornament ordinals, decrements them,
 *   resolves native bitmap/coordinate metadata, and blits with C10.
 * - DUNVIEW.C F0124:7874-7926 supplies the D1C center floor-ornament route.
 * - DUNVIEW.C F0128:8318-8542 dispatches center squares D3C, D2C, D1C, D0C.
 * - DUNVIEW.C F0098:2962-3002 owns the floor/ceiling pixels before the row.
 * - DUNGEON.C F0163:1769-1838 is cited as thing-list noninterference.
 * - DEFS.H:2088,2544,2558,2759,4042-4043 bind C10, M558, D1C, C702/C703.
 * - CSB-lineage Viewport.cpp:1903-1915 matches F1 floor-decoration before
 *   room-object pass one, door overlays, then room-object pass two.
 */

static const DM1_V1_F0095CenterSquareSpecPc34 s_center_specs[] = {
    {
        DM1_V1_F0095_CENTER_D3C_PC34,
        "D3C",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
        8499,
        3,
        0,
        11,
        3,
        6722,
        6814,
        6723,
        6816,
        true,
        false
    },
    {
        DM1_V1_F0095_CENTER_D2C_PC34,
        "D2C",
        "F0121_DUNGEONVIEW_DrawSquareD2C",
        8521,
        2,
        0,
        6,
        6,
        7314,
        7357,
        7315,
        7368,
        true,
        false
    },
    {
        DM1_V1_F0095_CENTER_D1C_PC34,
        "D1C",
        "F0124_DUNGEONVIEW_DrawSquareD1C",
        8533,
        1,
        0,
        3,
        9,
        7874,
        7926,
        7875,
        7937,
        true,
        false
    },
    {
        DM1_V1_F0095_CENTER_D0C_PC34,
        "D0C",
        "F0127_DUNGEONVIEW_DrawSquareD0C",
        8542,
        0,
        0,
        0,
        -1,
        DM1_F0095_ROUTE_REJECTED_LINE,
        DM1_F0095_ROUTE_REJECTED_LINE,
        8294,
        8294,
        false,
        true
    }
};

static const DM1_V1_F0095FloorOrnamentBindingPc34 s_binding = {
    10,
    4,
    5,
    DM1_F0095_FLOOR_ZONE_BASE,
    DM1_F0095_FLOOR_ZONE_STRIDE,
    702,
    703,
    true,
    true,
    true,
    false
};

static const char s_source_lock[] =
    "contract_only=1; no per-square runtime or real-asset bitmap parity claim. "
    "DUNVIEW.C F0095:2124-2223 loads wall-set graphics and preserves the "
    "G0095 WallD3LCR native binding outside the F0108 floor-ornament route. "
    "DUNVIEW.C F0108:3959-4008 gates floor ornament ordinals, decrements the "
    "ordinal to an index, applies variant/palette/flip metadata, and blits "
    "through C1500+coordinateSet*11+viewFloor with C10 transparency. "
    "DUNVIEW.C F0124:7874-7926 anchors the D1C center route; the aggregate "
    "also locks D3C 6722/6814 and D2C 7314/7357 as the D*-C family, with D0C "
    "kept out because F0127:8164-8310 has no F0108 route. "
    "DUNVIEW.C F0128:8318-8542 calls F0098 first when requested, then D3C, "
    "D2C, D1C, and D0C. DUNVIEW.C F0098:2962-3002 owns floor/ceiling pixels "
    "before F0108 overlays. DUNGEON.C F0163:1769-1838 anchors thing-list "
    "noninterference. DEFS.H:2088/2544/2558/2759/4042-4043 bind C10, M558, "
    "M595, C702 and C703. CSB-lineage Viewport.cpp:1903-1915 matches floor "
    "decoration before room-object overlay pass one and door overlays.";

static const DM1_V1_F0095CenterSquareSpecPc34 *spec_for_square(
    DM1_V1_F0095CenterSquarePc34 square)
{
    size_t i;
    for (i = 0; i < sizeof(s_center_specs) / sizeof(s_center_specs[0]); ++i) {
        if (s_center_specs[i].square == square) return &s_center_specs[i];
    }
    return NULL;
}

static const char *anchor_for_square(const DM1_V1_F0095CenterSquareSpecPc34 *spec)
{
    if (!spec) return "ReDMCSB DUNVIEW.C F0128:8318-8542 aggregate reject";
    if (spec->square == DM1_V1_F0095_CENTER_D1C_PC34) {
        return "ReDMCSB DUNVIEW.C F0124:7874-7926; F0108:3959-4008";
    }
    if (spec->square == DM1_V1_F0095_CENTER_D2C_PC34) {
        return "ReDMCSB DUNVIEW.C F0121:7314-7357; F0108:3959-4008";
    }
    if (spec->square == DM1_V1_F0095_CENTER_D3C_PC34) {
        return "ReDMCSB DUNVIEW.C F0118:6722-6814; F0108:3959-4008";
    }
    return "ReDMCSB DUNVIEW.C F0127:8164-8310 keep-out; F0098:2962-3002";
}

int dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_count_pc34(void)
{
    return (int)(sizeof(s_center_specs) / sizeof(s_center_specs[0]));
}

const DM1_V1_F0095CenterSquareSpecPc34 *
dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_at_pc34(int index)
{
    if (index < 0 ||
        index >= dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_count_pc34()) {
        return NULL;
    }
    return &s_center_specs[index];
}

const DM1_V1_F0095FloorOrnamentBindingPc34 *
dm1_v1_viewport_f0095_floor_ornament_aggregate_binding_pc34(void)
{
    return &s_binding;
}

bool dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
    DM1_V1_F0095CenterSquarePc34 square,
    DM1_V1_F0095CenterElementPc34 element,
    int floor_ornament_ordinal,
    int coordinate_set,
    DM1_V1_F0095FloorOrnamentAggregateResultPc34 *out)
{
    const DM1_V1_F0095CenterSquareSpecPc34 *spec;
    bool door_route;
    bool open_route;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->floor_ornament_index = -1;
    out->floor_ornament_zone = -1;
    out->f0108_source_line = DM1_F0095_ROUTE_REJECTED_LINE;
    out->f0115_source_line = DM1_F0095_ROUTE_REJECTED_LINE;

    spec = spec_for_square(square);
    out->source_anchor = anchor_for_square(spec);
    if (!spec || floor_ornament_ordinal < 0 || coordinate_set < 0) return false;

    out->accepted = true;
    out->f0098_precedes_f0128_center_row = true;
    out->c10_transparency = true;
    door_route = element == DM1_V1_F0095_CENTER_ELEMENT_DOOR_FRONT_PC34;
    open_route = element == DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34 ||
        element == DM1_V1_F0095_CENTER_ELEMENT_PIT_PC34;
    out->bug64_pit_overlay_contract =
        element == DM1_V1_F0095_CENTER_ELEMENT_PIT_PC34 &&
        spec->center_square_can_call_f0108;

    if (!spec->center_square_can_call_f0108) {
        out->calls_f0098_inside_center_square = spec->d0c_ceiling_keepout;
        return true;
    }
    if (!door_route && !open_route) return true;

    out->calls_f0108 = true;
    out->f0108_source_line = door_route ? spec->door_front_f0108_line :
        spec->open_f0108_line;
    out->f0115_source_line = door_route ? spec->door_front_first_f0115_line :
        spec->open_first_f0115_line;
    out->f0108_precedes_f0115_when_present =
        out->f0108_source_line > 0 && out->f0115_source_line > out->f0108_source_line;
    if (floor_ornament_ordinal > 0) {
        out->decrements_ordinal = true;
        out->floor_ornament_index = floor_ornament_ordinal - 1;
        out->floor_ornament_zone = DM1_F0095_FLOOR_ZONE_BASE +
            coordinate_set * DM1_F0095_FLOOR_ZONE_STRIDE + spec->view_floor_index;
    }
    return true;
}

const char *
dm1_v1_viewport_f0095_floor_ornament_aggregate_source_lock_pc34(void)
{
    return s_source_lock;
}

static void source_assert(int condition, int *passed, int *failed)
{
    if (condition) {
        ++*passed;
    } else {
        ++*failed;
    }
}

int dm1_v1_viewport_f0095_floor_ornament_aggregate_run_pc34(
    int *passed,
    int *failed)
{
    int local_passed = 0;
    int local_failed = 0;
    int *p = passed ? passed : &local_passed;
    int *f = failed ? failed : &local_failed;
    int i;

    *p = 0;
    *f = 0;

    source_assert(dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_count_pc34() == 4,
                  p, f);
    source_assert(s_binding.c10_transparent_color == 10, p, f);
    source_assert(s_binding.m558_floor_slot_pc34 == 4, p, f);
    source_assert(s_binding.m558_floor_slot_i34 == 5, p, f);
    source_assert(s_binding.floor_zone_base == 1500, p, f);
    source_assert(s_binding.floor_zone_stride == 11, p, f);
    source_assert(s_binding.wall_zone_d3l2_c702 == 702, p, f);
    source_assert(s_binding.wall_zone_d3r2_c703 == 703, p, f);
    source_assert(s_binding.f0095_wallset_loads_g0095_native_wall_binding, p, f);
    source_assert(s_binding.g0109_champion_portrait_box_is_not_floor_ornament, p, f);
    source_assert(s_binding.contract_only, p, f);
    source_assert(!s_binding.real_asset_runtime_parity, p, f);
    source_assert(strstr(s_source_lock, "DUNVIEW.C F0095:2124-2223") != NULL, p, f);
    source_assert(strstr(s_source_lock, "DUNVIEW.C F0108:3959-4008") != NULL, p, f);
    source_assert(strstr(s_source_lock, "DUNVIEW.C F0124:7874-7926") != NULL, p, f);
    source_assert(strstr(s_source_lock, "DUNVIEW.C F0128:8318-8542") != NULL, p, f);
    source_assert(strstr(s_source_lock, "DUNVIEW.C F0098:2962-3002") != NULL, p, f);
    source_assert(strstr(s_source_lock, "DUNGEON.C F0163:1769-1838") != NULL, p, f);
    source_assert(strstr(s_source_lock, "DEFS.H:2088/2544/2558/2759/4042-4043") != NULL,
                  p, f);
    source_assert(strstr(s_source_lock, "Viewport.cpp:1903-1915") != NULL, p, f);

    for (i = 0; i < dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_count_pc34();
         ++i) {
        const DM1_V1_F0095CenterSquareSpecPc34 *spec =
            dm1_v1_viewport_f0095_floor_ornament_aggregate_spec_at_pc34(i);
        DM1_V1_F0095FloorOrnamentAggregateResultPc34 door;
        DM1_V1_F0095FloorOrnamentAggregateResultPc34 open;
        DM1_V1_F0095FloorOrnamentAggregateResultPc34 pit;
        DM1_V1_F0095FloorOrnamentAggregateResultPc34 wall;
        DM1_V1_F0095FloorOrnamentAggregateResultPc34 stairs;

        source_assert(spec != NULL, p, f);
        if (!spec) continue;
        source_assert(spec->lane == 0, p, f);
        source_assert(spec->depth == 3 - i, p, f);
        source_assert(spec->f0128_dispatch_line > 0, p, f);
        source_assert(spec->view_square_index >= 0, p, f);
        source_assert(anchor_for_square(spec) != NULL, p, f);

        source_assert(dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          spec->square, DM1_V1_F0095_CENTER_ELEMENT_DOOR_FRONT_PC34,
                          6, 2, &door), p, f);
        source_assert(dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          spec->square, DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34,
                          1, 0, &open), p, f);
        source_assert(dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          spec->square, DM1_V1_F0095_CENTER_ELEMENT_PIT_PC34,
                          0, 3, &pit), p, f);
        source_assert(dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          spec->square, DM1_V1_F0095_CENTER_ELEMENT_WALL_PC34,
                          4, 0, &wall), p, f);
        source_assert(dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          spec->square, DM1_V1_F0095_CENTER_ELEMENT_STAIRS_PC34,
                          4, 0, &stairs), p, f);

        source_assert(door.accepted && open.accepted && pit.accepted, p, f);
        source_assert(wall.accepted && stairs.accepted, p, f);
        source_assert(door.f0098_precedes_f0128_center_row, p, f);
        source_assert(open.f0098_precedes_f0128_center_row, p, f);
        source_assert(pit.f0098_precedes_f0128_center_row, p, f);
        source_assert(door.c10_transparency && open.c10_transparency, p, f);
        source_assert(pit.c10_transparency, p, f);
        source_assert(!wall.calls_f0108, p, f);
        source_assert(!stairs.calls_f0108, p, f);

        if (spec->center_square_can_call_f0108) {
            source_assert(door.calls_f0108, p, f);
            source_assert(open.calls_f0108, p, f);
            source_assert(pit.calls_f0108, p, f);
            source_assert(door.f0108_source_line == spec->door_front_f0108_line, p, f);
            source_assert(open.f0108_source_line == spec->open_f0108_line, p, f);
            source_assert(door.f0115_source_line == spec->door_front_first_f0115_line,
                          p, f);
            source_assert(open.f0115_source_line == spec->open_first_f0115_line, p, f);
            source_assert(door.f0108_precedes_f0115_when_present, p, f);
            source_assert(open.f0108_precedes_f0115_when_present, p, f);
            source_assert(door.decrements_ordinal, p, f);
            source_assert(door.floor_ornament_index == 5, p, f);
            source_assert(door.floor_ornament_zone ==
                          1500 + 2 * 11 + spec->view_floor_index, p, f);
            source_assert(open.decrements_ordinal, p, f);
            source_assert(open.floor_ornament_index == 0, p, f);
            source_assert(open.floor_ornament_zone == 1500 + spec->view_floor_index,
                          p, f);
            source_assert(pit.bug64_pit_overlay_contract, p, f);
            source_assert(!pit.decrements_ordinal, p, f);
            source_assert(pit.floor_ornament_index == -1, p, f);
            source_assert(!door.calls_f0098_inside_center_square, p, f);
            source_assert(!open.calls_f0098_inside_center_square, p, f);
        } else {
            source_assert(!door.calls_f0108, p, f);
            source_assert(!open.calls_f0108, p, f);
            source_assert(!pit.calls_f0108, p, f);
            source_assert(door.calls_f0098_inside_center_square, p, f);
            source_assert(open.calls_f0098_inside_center_square, p, f);
            source_assert(pit.calls_f0098_inside_center_square, p, f);
            source_assert(door.floor_ornament_index == -1, p, f);
            source_assert(open.floor_ornament_zone == -1, p, f);
        }
    }

    source_assert(s_center_specs[0].square == DM1_V1_F0095_CENTER_D3C_PC34, p, f);
    source_assert(s_center_specs[1].square == DM1_V1_F0095_CENTER_D2C_PC34, p, f);
    source_assert(s_center_specs[2].square == DM1_V1_F0095_CENTER_D1C_PC34, p, f);
    source_assert(s_center_specs[3].square == DM1_V1_F0095_CENTER_D0C_PC34, p, f);
    source_assert(s_center_specs[0].f0128_dispatch_line < s_center_specs[1].f0128_dispatch_line,
                  p, f);
    source_assert(s_center_specs[1].f0128_dispatch_line < s_center_specs[2].f0128_dispatch_line,
                  p, f);
    source_assert(s_center_specs[2].f0128_dispatch_line < s_center_specs[3].f0128_dispatch_line,
                  p, f);
    source_assert(s_center_specs[2].view_floor_index == 9, p, f);
    source_assert(s_center_specs[1].view_floor_index == 6, p, f);
    source_assert(s_center_specs[0].view_floor_index == 3, p, f);
    source_assert(s_center_specs[3].view_floor_index == -1, p, f);
    source_assert(spec_for_square((DM1_V1_F0095CenterSquarePc34)99) == NULL, p, f);
    {
        DM1_V1_F0095FloorOrnamentAggregateResultPc34 rejected;
        source_assert(!dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          (DM1_V1_F0095CenterSquarePc34)99,
                          DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34, 1, 0, &rejected),
                      p, f);
        source_assert(!dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          DM1_V1_F0095_CENTER_D1C_PC34,
                          DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34, -1, 0, &rejected),
                      p, f);
        source_assert(!dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          DM1_V1_F0095_CENTER_D1C_PC34,
                          DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34, 1, -1, &rejected),
                      p, f);
        source_assert(!dm1_v1_viewport_f0095_floor_ornament_aggregate_eval_pc34(
                          DM1_V1_F0095_CENTER_D1C_PC34,
                          DM1_V1_F0095_CENTER_ELEMENT_OPEN_PC34, 1, 0, NULL),
                      p, f);
    }

    source_assert(*p >= 160, p, f);
    return *f == 0 ? 0 : 1;
}
