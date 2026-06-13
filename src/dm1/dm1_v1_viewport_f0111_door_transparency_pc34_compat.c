#include "firestaff/dm1/v1/viewport/f0111_door_transparency_pc34_compat.h"

#include <stddef.h>
#include <string.h>

enum {
    DM1_DOOR_STATE_OPEN = 0,
    DM1_DOOR_STATE_ONE_FOURTH = 1,
    DM1_DOOR_STATE_HALF = 2,
    DM1_DOOR_STATE_THREE_FOURTH = 3,
    DM1_DOOR_STATE_CLOSED = 4,
    DM1_VIEW_SQUARE_D1C = 3,
    DM1_VIEW_SQUARE_D2C = 6,
    DM1_VIEW_SQUARE_D2L = 7,
    DM1_VIEW_SQUARE_D2R = 8,
    DM1_VIEW_SQUARE_D2L2 = 9,
    DM1_VIEW_SQUARE_D2R2 = 10,
    DM1_ZONE_WALL_D2L2 = 707,
    DM1_ZONE_WALL_D2R2 = 708,
    DM1_ZONE_DOOR_D2L = 3750,
    DM1_ZONE_DOOR_D2C = 3760,
    DM1_ZONE_DOOR_D2R = 3770,
    DM1_ZONE_DOOR_D1C = 3790,
    DM1_F0128_D2L_LINE = 8513,
    DM1_F0128_D2R_LINE = 8517,
    DM1_F0128_D2C_LINE = 8521,
    DM1_F0128_D1C_LINE = 8533,
    DM1_DOOR_THING_SENTINEL = 0x0123,
    DM1_SECOND_HALF_SHIFT =
        3 | DM1_V1_F0111_DOOR_TRANSPARENCY_MASK0X4000_PC34
};

static const DM1_V1_F0111DoorTransparencyRouteSpecPc34 s_routes[] = {
    {
        DM1_V1_F0111_DOOR_TRANSPARENCY_ROUTE_D1C_VERTICAL_PC34,
        "D1C vertical partly-open F0111 zone",
        DM1_VIEW_SQUARE_D1C,
        1,
        0,
        DM1_ZONE_DOOR_D1C,
        DM1_F0128_D1C_LINE,
        1,
        1,
        0,
        "G0186_s_Graphic558_Frames_Door_D1C.Vertical[state-1]"
    },
    {
        DM1_V1_F0111_DOOR_TRANSPARENCY_ROUTE_D1C_HORIZONTAL_PC34,
        "D1C horizontal F0111 transparency halves",
        DM1_VIEW_SQUARE_D1C,
        1,
        0,
        DM1_ZONE_DOOR_D1C,
        DM1_F0128_D1C_LINE,
        1,
        0,
        1,
        "G0186_s_Graphic558_Frames_Door_D1C.LeftHorizontal/RightHorizontal[state-1]"
    },
    {
        DM1_V1_F0111_DOOR_TRANSPARENCY_ROUTE_D2L_HORIZONTAL_PC34,
        "D2L horizontal F0111 transparency halves",
        DM1_VIEW_SQUARE_D2L,
        2,
        -1,
        DM1_ZONE_DOOR_D2L,
        DM1_F0128_D2L_LINE,
        1,
        0,
        1,
        "G0182_s_Graphic558_Frames_Door_D2L.LeftHorizontal/RightHorizontal[state-1]"
    },
    {
        DM1_V1_F0111_DOOR_TRANSPARENCY_ROUTE_D2R_HORIZONTAL_PC34,
        "D2R horizontal F0111 transparency halves",
        DM1_VIEW_SQUARE_D2R,
        2,
        1,
        DM1_ZONE_DOOR_D2R,
        DM1_F0128_D2R_LINE,
        1,
        0,
        1,
        "G0184_s_Graphic558_Frames_Door_D2R.LeftHorizontal/RightHorizontal[state-1]"
    }
};

static const char s_source_evidence[] =
    "DM1 V1 F0111 door transparency source-lock, contract-only and asset-free. "
    "ReDMCSB DUNVIEW.C F0111:4218-4337 anchors the door panel; "
    "F0111:4308 decrements C1..C3 partly-open door state, F0111:4311-4313 "
    "selects LeftHorizontal/RightHorizontal for horizontal doors, "
    "F0111:4317-4318 adds P2084_i_ZoneIndex + state, F0111:4320-4324 "
    "performs the first half blit through zone + C6_UNKNOWN with "
    "C10_COLOR_FLESH, F0111:4325 shifts the final half by 3 | MASK0x4000, "
    "and F0111:4334 blits the final half with C10. DUNVIEW.C "
    "F0121:7244-7389 is the D2C-only center anchor at M628 and is not the "
    "D1C/D2L/D2R route. DUNVIEW.C F0128:8508-8533 dispatches D2L, D2R, "
    "D2C, then D1C. DUNGEON.C F0163:1769-1838 links a door thing to the "
    "map cell thing list and F0164:1840-1905 clears/unlinks it before "
    "F0172 exposes M556_DOOR_STATE and M557_DOOR_THING_INDEX. DEFS.H:2088 "
    "C10_COLOR_FLESH, DEFS.H:2605-2606 C09/C10 D2L2/D2R2 wall ordinals, "
    "DEFS.H:3508 C6_UNKNOWN, DEFS.H:3516 MASK0x4000, and DEFS.H:4047-4048 "
    "C707/C708 D2L2/D2R2 wall zones are pinned. Disjointness: existing "
    "F0111 gates pin per-square D2C/D1C/D0C/D1L2/D1R2/D2L/D2R/D2L2/D2R2/D3C "
    "composition; this gate pins the shared C10 transparency byte flow, "
    "D1C vertical zone path, row-1/2 horizontal zone shift, and map-cell "
    "door thing-list source lock.";

static DM1_V1_F0111DoorTransparencySelfTestResultPc34 s_last;

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> ((unsigned int)i * 8U)) & 0xffU;
        hash *= 16777619U;
    }
    return hash;
}

static uint32_t mix_string(uint32_t hash, const char *text)
{
    if (!text) return mix_u32(hash, 0xffffffffU);
    while (*text) {
        hash ^= (unsigned char)*text++;
        hash *= 16777619U;
    }
    return hash;
}

static void check_int(const char *id, int got, int want)
{
    ++s_last.assertions;
    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, id);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)got);
    s_last.deterministic_hash = mix_u32(s_last.deterministic_hash, (uint32_t)want);
    if (got != want) ++s_last.failures;
}

static void check_contains(const char *id, const char *haystack, const char *needle)
{
    const int found = haystack && needle && strstr(haystack, needle) != NULL;

    s_last.deterministic_hash = mix_string(s_last.deterministic_hash, needle);
    check_int(id, found, 1);
}

const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *
dm1_v1_viewport_f0111_door_transparency_route_at_pc34(unsigned int index)
{
    const unsigned int count = (unsigned int)(sizeof(s_routes) / sizeof(s_routes[0]));

    return index < count ? &s_routes[index] : NULL;
}

const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *
dm1_v1_viewport_f0111_door_transparency_route_for_square_pc34(int view_square)
{
    size_t i;

    for (i = 0; i < sizeof(s_routes) / sizeof(s_routes[0]); ++i) {
        if (s_routes[i].view_square == view_square) return &s_routes[i];
    }
    return NULL;
}

uint8_t dm1_v1_viewport_f0111_door_transparency_blend_pc34(
    uint8_t destination,
    uint8_t source)
{
    return source == DM1_V1_F0111_DOOR_TRANSPARENCY_C10_COLOR_FLESH_PC34
        ? destination
        : source;
}

int dm1_v1_viewport_f0111_door_transparency_is_d2l2_d2r2_wall_pc34(
    int view_square,
    int zone)
{
    return (view_square == DM1_VIEW_SQUARE_D2L2 && zone == DM1_ZONE_WALL_D2L2) ||
           (view_square == DM1_VIEW_SQUARE_D2R2 && zone == DM1_ZONE_WALL_D2R2);
}

int dm1_v1_viewport_f0111_door_transparency_trace_pc34(
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *spec,
    int door_state,
    int map_square_flags,
    int door_thing_index,
    DM1_V1_F0111DoorTransparencyTracePc34 *out)
{
    uint8_t background;

    if (!spec || !out) return 0;
    memset(out, 0, sizeof(*out));
    out->input_state = door_state;
    out->map_square_had_thing_list =
        (map_square_flags &
         DM1_V1_F0111_DOOR_TRANSPARENCY_THING_LIST_PRESENT_PC34) != 0;
    out->f0163_links_door_to_cell = out->map_square_had_thing_list;
    out->f0164_unlinks_door_from_cell = out->map_square_had_thing_list;
    out->door_state_from_m556 = door_state;
    out->door_thing_from_m557 = door_thing_index;
    out->base_zone_from_p2084 = spec->door_zone;

    if (!out->map_square_had_thing_list ||
        door_thing_index != DM1_DOOR_THING_SENTINEL) {
        out->branch = DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_INVALID_PC34;
        return 1;
    }
    if (door_state == DM1_DOOR_STATE_OPEN) {
        out->branch = DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_OPEN_PC34;
        return 1;
    }
    if (door_state == DM1_DOOR_STATE_CLOSED) {
        out->branch = DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_CLOSED_PC34;
        return 1;
    }
    if (door_state < DM1_DOOR_STATE_ONE_FOURTH ||
        door_state > DM1_DOOR_STATE_THREE_FOURTH) {
        out->branch = DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_INVALID_PC34;
        return 1;
    }

    out->decremented_state = door_state - 1;
    background = (uint8_t)(0x40 + spec->view_square);
    if (spec->uses_d1c_vertical_slot) {
        out->branch = DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_PARTLY_VERTICAL_PC34;
        out->vertical_bitmap = 1;
        out->final_zone = spec->door_zone + out->decremented_state;
        out->final_uses_c10 = 1;
        out->final_pass_pixel =
            dm1_v1_viewport_f0111_door_transparency_blend_pc34(background, 10);
        return 1;
    }

    out->branch = DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_PARTLY_HORIZONTAL_PC34;
    out->left_horizontal_bitmap = 1;
    out->right_horizontal_bitmap = 1;
    out->first_half_zone_with_c6 = spec->door_zone + out->decremented_state +
        DM1_V1_F0111_DOOR_TRANSPARENCY_C6_UNKNOWN_PC34;
    out->first_half_uses_c10 = 1;
    out->second_half_shift = DM1_SECOND_HALF_SHIFT;
    out->final_zone = spec->door_zone + out->decremented_state +
        DM1_SECOND_HALF_SHIFT;
    out->final_uses_c10 = 1;
    out->first_pass_pixel =
        dm1_v1_viewport_f0111_door_transparency_blend_pc34(background, 10);
    out->final_pass_pixel =
        dm1_v1_viewport_f0111_door_transparency_blend_pc34(background, 0x55);
    return 1;
}

const char *
dm1_v1_viewport_f0111_door_transparency_source_evidence_pc34(void)
{
    return s_source_evidence;
}

static void check_route_spec(
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *spec,
    int view_square,
    int depth,
    int lateral,
    int zone,
    int dispatch_line)
{
    check_int("route.present", spec != NULL, 1);
    if (!spec) return;
    check_int("route.view.square", spec->view_square, view_square);
    check_int("route.depth", spec->relative_depth, depth);
    check_int("route.lateral", spec->relative_lateral, lateral);
    check_int("route.zone", spec->door_zone, zone);
    check_int("route.f0128.line", spec->f0128_dispatch_line, dispatch_line);
    check_int("route.d2c.anchor.only", spec->f0121_is_only_d2c_anchor, 1);
    check_int("route.frame.symbol", spec->frame_symbol != NULL, 1);
    ++s_last.route_count;
}

static void check_trace(
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *spec,
    int state)
{
    DM1_V1_F0111DoorTransparencyTracePc34 trace;
    const int flags = DM1_V1_F0111_DOOR_TRANSPARENCY_THING_LIST_PRESENT_PC34;

    check_int("trace.ok",
              dm1_v1_viewport_f0111_door_transparency_trace_pc34(
                  spec, state, flags, DM1_DOOR_THING_SENTINEL, &trace),
              1);
    check_int("trace.map.cell", trace.map_square_had_thing_list, 1);
    check_int("trace.f0163", trace.f0163_links_door_to_cell, 1);
    check_int("trace.f0164", trace.f0164_unlinks_door_from_cell, 1);
    check_int("trace.m556", trace.door_state_from_m556, state);
    check_int("trace.m557", trace.door_thing_from_m557, DM1_DOOR_THING_SENTINEL);
    check_int("trace.p2084", trace.base_zone_from_p2084, spec->door_zone);
    check_int("trace.decremented", trace.decremented_state, state - 1);
    ++s_last.map_cell_checks;

    if (spec->uses_d1c_vertical_slot) {
        check_int("trace.branch.vertical", trace.branch,
                  DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_PARTLY_VERTICAL_PC34);
        check_int("trace.vertical.bitmap", trace.vertical_bitmap, 1);
        check_int("trace.no.left.horizontal", trace.left_horizontal_bitmap, 0);
        check_int("trace.vertical.final.zone", trace.final_zone,
                  spec->door_zone + state - 1);
        check_int("trace.vertical.c10", trace.final_uses_c10, 1);
        check_int("trace.vertical.c10.preserve", trace.final_pass_pixel,
                  0x40 + spec->view_square);
        ++s_last.d1c_vertical_partly;
    } else {
        check_int("trace.branch.horizontal", trace.branch,
                  DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_PARTLY_HORIZONTAL_PC34);
        check_int("trace.left.horizontal", trace.left_horizontal_bitmap, 1);
        check_int("trace.right.horizontal", trace.right_horizontal_bitmap, 1);
        check_int("trace.first.half.zone", trace.first_half_zone_with_c6,
                  spec->door_zone + state - 1 + 6);
        check_int("trace.first.c10", trace.first_half_uses_c10, 1);
        check_int("trace.shift", trace.second_half_shift, DM1_SECOND_HALF_SHIFT);
        check_int("trace.final.zone", trace.final_zone,
                  spec->door_zone + state - 1 + DM1_SECOND_HALF_SHIFT);
        check_int("trace.final.c10", trace.final_uses_c10, 1);
        check_int("trace.first.c10.preserve", trace.first_pass_pixel,
                  0x40 + spec->view_square);
        check_int("trace.final.opaque.write", trace.final_pass_pixel, 0x55);
        ++s_last.horizontal_partly;
    }
    s_last.c10_transparency_checks += 2;
}

static void check_rejection(
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *spec,
    int state,
    DM1_V1_F0111DoorTransparencyBranchPc34 branch,
    int *counter)
{
    DM1_V1_F0111DoorTransparencyTracePc34 trace;
    const int flags = DM1_V1_F0111_DOOR_TRANSPARENCY_THING_LIST_PRESENT_PC34;

    check_int("reject.ok",
              dm1_v1_viewport_f0111_door_transparency_trace_pc34(
                  spec, state, flags, DM1_DOOR_THING_SENTINEL, &trace),
              1);
    check_int("reject.branch", trace.branch, branch);
    check_int("reject.no.final.c10", trace.final_uses_c10, 0);
    ++*counter;
}

int run_dm1_v1_viewport_f0111_door_transparency_self_test(void)
{
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *d1c_vertical;
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *d1c_horizontal;
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *d2l;
    const DM1_V1_F0111DoorTransparencyRouteSpecPc34 *d2r;
    DM1_V1_F0111DoorTransparencyTracePc34 invalid;

    memset(&s_last, 0, sizeof(s_last));
    s_last.deterministic_hash = 2166136261U;

    check_contains("source.f0111", s_source_evidence, "DUNVIEW.C F0111:4218-4337");
    check_contains("source.partly.horizontal", s_source_evidence, "F0111:4311-4313");
    check_contains("source.p2084", s_source_evidence, "P2084_i_ZoneIndex");
    check_contains("source.c6", s_source_evidence, "C6_UNKNOWN");
    check_contains("source.mask", s_source_evidence, "MASK0x4000");
    check_contains("source.c10", s_source_evidence, "C10_COLOR_FLESH");
    check_contains("source.f0121", s_source_evidence, "F0121:7244-7389");
    check_contains("source.f0128", s_source_evidence, "F0128:8508-8533");
    check_contains("source.f0163", s_source_evidence, "F0163:1769-1838");
    check_contains("source.f0164", s_source_evidence, "F0164:1840-1905");
    check_contains("source.defs.2605", s_source_evidence, "DEFS.H:2605-2606");
    check_contains("source.defs.4047", s_source_evidence, "DEFS.H:4047-4048");
    check_contains("source.disjoint", s_source_evidence, "Disjointness:");

    d1c_vertical = dm1_v1_viewport_f0111_door_transparency_route_at_pc34(0);
    d1c_horizontal = dm1_v1_viewport_f0111_door_transparency_route_at_pc34(1);
    d2l = dm1_v1_viewport_f0111_door_transparency_route_at_pc34(2);
    d2r = dm1_v1_viewport_f0111_door_transparency_route_at_pc34(3);

    check_route_spec(d1c_vertical, DM1_VIEW_SQUARE_D1C, 1, 0,
                     DM1_ZONE_DOOR_D1C, DM1_F0128_D1C_LINE);
    check_route_spec(d1c_horizontal, DM1_VIEW_SQUARE_D1C, 1, 0,
                     DM1_ZONE_DOOR_D1C, DM1_F0128_D1C_LINE);
    check_route_spec(d2l, DM1_VIEW_SQUARE_D2L, 2, -1,
                     DM1_ZONE_DOOR_D2L, DM1_F0128_D2L_LINE);
    check_route_spec(d2r, DM1_VIEW_SQUARE_D2R, 2, 1,
                     DM1_ZONE_DOOR_D2R, DM1_F0128_D2R_LINE);
    check_int("route.at.bounds",
              dm1_v1_viewport_f0111_door_transparency_route_at_pc34(4) == NULL,
              1);
    check_int("route.d2c.not.claimed",
              dm1_v1_viewport_f0111_door_transparency_route_for_square_pc34(
                  DM1_VIEW_SQUARE_D2C) == NULL,
              1);

    check_trace(d1c_vertical, DM1_DOOR_STATE_ONE_FOURTH);
    check_trace(d1c_vertical, DM1_DOOR_STATE_HALF);
    check_trace(d1c_vertical, DM1_DOOR_STATE_THREE_FOURTH);
    check_trace(d1c_horizontal, DM1_DOOR_STATE_HALF);
    check_trace(d2l, DM1_DOOR_STATE_HALF);
    check_trace(d2r, DM1_DOOR_STATE_THREE_FOURTH);

    check_rejection(d2l, DM1_DOOR_STATE_OPEN,
                    DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_OPEN_PC34,
                    &s_last.open_rejections);
    check_rejection(d2r, DM1_DOOR_STATE_CLOSED,
                    DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_CLOSED_PC34,
                    &s_last.closed_rejections);

    check_int("invalid.no.map.cell",
              dm1_v1_viewport_f0111_door_transparency_trace_pc34(
                  d1c_vertical, DM1_DOOR_STATE_HALF, 0,
                  DM1_DOOR_THING_SENTINEL, &invalid),
              1);
    check_int("invalid.no.map.branch", invalid.branch,
              DM1_V1_F0111_DOOR_TRANSPARENCY_BRANCH_INVALID_PC34);

    check_int("d2l2.wall.exclude",
              dm1_v1_viewport_f0111_door_transparency_is_d2l2_d2r2_wall_pc34(
                  DM1_VIEW_SQUARE_D2L2, DM1_ZONE_WALL_D2L2),
              1);
    check_int("d2r2.wall.exclude",
              dm1_v1_viewport_f0111_door_transparency_is_d2l2_d2r2_wall_pc34(
                  DM1_VIEW_SQUARE_D2R2, DM1_ZONE_WALL_D2R2),
              1);
    check_int("d2l.not.wall.exclude",
              dm1_v1_viewport_f0111_door_transparency_is_d2l2_d2r2_wall_pc34(
                  DM1_VIEW_SQUARE_D2L, DM1_ZONE_DOOR_D2L),
              0);
    s_last.d2l2_d2r2_wall_exclusions += 3;

    check_int("blend.c10",
              dm1_v1_viewport_f0111_door_transparency_blend_pc34(0x66, 10),
              0x66);
    check_int("blend.opaque",
              dm1_v1_viewport_f0111_door_transparency_blend_pc34(0x66, 0x44),
              0x44);
    s_last.c10_transparency_checks += 2;

    check_int("route.count", s_last.route_count, 4);
    check_int("d1c.vertical.count", s_last.d1c_vertical_partly, 3);
    check_int("horizontal.count", s_last.horizontal_partly, 3);
    check_int("open.rejections", s_last.open_rejections, 1);
    check_int("closed.rejections", s_last.closed_rejections, 1);
    check_int("map.cell.checks", s_last.map_cell_checks, 6);
    check_int("d2.wall.exclusions", s_last.d2l2_d2r2_wall_exclusions, 3);
    check_int("c10.checks", s_last.c10_transparency_checks, 14);
    check_int("hash.changed", s_last.deterministic_hash != 2166136261U, 1);

    return s_last.failures == 0 ? 0 : 1;
}

const DM1_V1_F0111DoorTransparencySelfTestResultPc34 *
dm1_v1_viewport_f0111_door_transparency_last_self_test_result_pc34(void)
{
    return &s_last;
}
