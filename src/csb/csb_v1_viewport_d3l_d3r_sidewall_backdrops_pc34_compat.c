#include "csb_v1_viewport_d3l_d3r_sidewall_backdrops_pc34_compat.h"

enum {
    CSB_VIEW_SQUARE_D3L = 12,
    CSB_VIEW_SQUARE_D3R = 13,
    CSB_RELATIVE_DEPTH_D3 = 3,
    CSB_RELATIVE_LATERAL_D3L = -1,
    CSB_RELATIVE_LATERAL_D3R = 1,
    CSB_ORDER_AFTER_BACKDROPS_D3L = 2,
    CSB_ORDER_AFTER_BACKDROPS_D3R = 3,
    CSB_PRECEDING_D3L2_D3R2_BACKDROPS = 2,
    CSB_ZONE_WALL_D3L = 705,
    CSB_ZONE_WALL_D3R = 706,
    CSB_WALL_D3R = 12,
    CSB_WALL_D3L = 13,
    CSB_VIEW_WALL_D3L_RIGHT = 2,
    CSB_VIEW_WALL_D3R_LEFT = 3,
    CSB_VIEW_WALL_D3L_FRONT = 4,
    CSB_VIEW_WALL_D3R_FRONT = 6,
    CSB_VIEW_FLOOR_D3L = 2,
    CSB_VIEW_FLOOR_D3R = 4,
    CSB_C10_COLOR_FLESH = 10,
    CSB_LINEAGE_DRAWORDER02 = 60288,
    CSB_LINEAGE_DRAWORDER01 = 60287,
    CSB_LINEAGE_DRAWORDER218 = 60279,
    CSB_LINEAGE_DRAWORDER349 = 60280
};

static const char s_source_evidence[] =
    "CSB V1 D3L/D3R side-wall composition gate; contract-only, no real-asset "
    "bitmap parity and no CSB game-data load. ReDMCSB DUNVIEW.C:8478-8500 "
    "F0128 draws D3L2 and D3R2 first, then D3L at relative 3,-1 and D3R at "
    "relative 3,+1 before D3C. ReDMCSB DUNVIEW.C:6361-6480 F0116 and "
    "6500-6622 F0117 bind D3L/D3R wall composition: the WALL branch blits "
    "C705/C706 through F0104 or F0105, calls F0107 for side and front wall "
    "ornaments, returns without F0115 unless the front ornament is an alcove, "
    "and otherwise sends alcove objects through F0115 order 0. Corridor, pit, "
    "teleporter, and stairs-front routes call F0108 then F0115 with 0x3421 "
    "for D3L and 0x4312 for D3R. Door-side/stairs-side routes call F0108 "
    "then F0115 with 0x0321 for D3L and 0x0412 for D3R. Door-front routes "
    "call F0108, F0115 rear pass 0x0218/0x0128, F0111, then F0115 front pass "
    "0x0349/0x0439. ReDMCSB DUNVIEW.C:3502-3938 F0107 provides C10 "
    "transparent wall-ornament blits and DUNVIEW.C:3940-4009 F0108 provides "
    "C10 transparent floor-ornament blits. ReDMCSB DUNVIEW.C:4547-4581 F0115 "
    "defines the object/creature/projectile/explosion pass order. ReDMCSB "
    "DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 keep C10_COLOR_FLESH "
    "transparent. DEFS.H:2608-2609 binds C12/C13 D3L/D3R view squares; "
    "DEFS.H:2668-2677 binds the D3L/D3R cell orders; DEFS.H:2698-2702 and "
    "2752-2754 bind wall/floor ornament view indices; DEFS.H:4045-4046 binds "
    "C705/C706 wall zones. CSB-lineage Viewport.cpp:1192-1209 anchors the "
    "open side-room room-object ordering and Viewport.cpp:1903-1915 anchors "
    "the two-pass door-facing room-object overlay shape mirrored by the "
    "D3L/D3R rear/front F0115 split.";

static const CSB_V1_D3LD3RSidewallBackdropSpecPc34 s_specs[] = {
    {
        CSB_V1_D3L_D3R_SIDEWALL_SIDE_D3L_PC34,
        "D3L side-wall after D3L2/D3R2 backdrops",
        CSB_VIEW_SQUARE_D3L,
        CSB_RELATIVE_DEPTH_D3,
        CSB_RELATIVE_LATERAL_D3L,
        CSB_ORDER_AFTER_BACKDROPS_D3L,
        CSB_PRECEDING_D3L2_D3R2_BACKDROPS,
        CSB_ZONE_WALL_D3L,
        CSB_WALL_D3L,
        CSB_WALL_D3R,
        0,
        83,
        25,
        75,
        64,
        51,
        CSB_VIEW_WALL_D3L_RIGHT,
        CSB_VIEW_WALL_D3L_FRONT,
        CSB_VIEW_FLOOR_D3L,
        0x3421u,
        0x0321u,
        0x0218u,
        0x0349u,
        CSB_C10_COLOR_FLESH,
        CSB_LINEAGE_DRAWORDER02,
        CSB_LINEAGE_DRAWORDER218,
        CSB_LINEAGE_DRAWORDER349,
        "ReDMCSB DUNVIEW.C:6361-6480 F0116; F0128 lines 8490-8491"
    },
    {
        CSB_V1_D3L_D3R_SIDEWALL_SIDE_D3R_PC34,
        "D3R side-wall after D3L2/D3R2 backdrops",
        CSB_VIEW_SQUARE_D3R,
        CSB_RELATIVE_DEPTH_D3,
        CSB_RELATIVE_LATERAL_D3R,
        CSB_ORDER_AFTER_BACKDROPS_D3R,
        CSB_PRECEDING_D3L2_D3R2_BACKDROPS,
        CSB_ZONE_WALL_D3R,
        CSB_WALL_D3R,
        CSB_WALL_D3L,
        139,
        223,
        25,
        75,
        64,
        51,
        CSB_VIEW_WALL_D3R_LEFT,
        CSB_VIEW_WALL_D3R_FRONT,
        CSB_VIEW_FLOOR_D3R,
        0x4312u,
        0x0412u,
        0x0128u,
        0x0439u,
        CSB_C10_COLOR_FLESH,
        CSB_LINEAGE_DRAWORDER01,
        CSB_LINEAGE_DRAWORDER218,
        CSB_LINEAGE_DRAWORDER349,
        "ReDMCSB DUNVIEW.C:6500-6622 F0117; F0128 lines 8494-8495"
    }
};

size_t csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *
csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_count_pc34()) {
        return 0;
    }
    return &s_specs[index];
}

const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *
csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_d3l_d3r_sidewall_backdrops_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

static int is_open_style_element(int element)
{
    return element == CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_CORRIDOR_PC34 ||
           element == CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_PIT_PC34 ||
           element == CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_TELEPORTER_PC34 ||
           element == CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_STAIRS_FRONT_PC34;
}

int csb_v1_viewport_d3l_d3r_sidewall_backdrops_trace_pc34(
    const CSB_V1_D3LD3RSidewallBackdropSpecPc34 *spec,
    int element,
    int front_wall_ornament_is_alcove,
    CSB_V1_D3LD3RSidewallBackdropTracePc34 *out_trace)
{
    CSB_V1_D3LD3RSidewallBackdropTracePc34 trace = { 0 };

    if (!spec || !out_trace) return -1;
    trace.preceding_backdrops = spec->preceding_backdrop_count;
    trace.d3l_before_d3r = 1;
    trace.c10_transparency_preserved =
        csb_v1_viewport_d3l_d3r_sidewall_backdrops_blend_c10_pc34(0x5au, 10u) == 0x5a;

    if (element == CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_WALL_PC34) {
        trace.wall_blit_calls = 1;
        trace.f0104_calls = front_wall_ornament_is_alcove ? 0 : 1;
        trace.f0105_calls = front_wall_ornament_is_alcove ? 1 : 0;
        trace.f0107_calls = 2;
        trace.f0115_calls = front_wall_ornament_is_alcove ? 1 : 0;
        trace.first_f0115_order = front_wall_ornament_is_alcove ? 0u : 0xffffu;
        trace.wall_returns_without_front_alcove = !front_wall_ornament_is_alcove;
        trace.front_alcove_uses_f0115_zero_order = front_wall_ornament_is_alcove;
    } else if (is_open_style_element(element)) {
        trace.f0108_calls = 1;
        trace.f0115_calls = 1;
        trace.first_f0115_order = spec->open_order;
    } else if (element == CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_DOOR_SIDE_PC34 ||
               element == CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_STAIRS_SIDE_PC34) {
        trace.f0108_calls = 1;
        trace.f0115_calls = 1;
        trace.first_f0115_order = spec->door_side_order;
    } else if (element == CSB_V1_D3L_D3R_SIDEWALL_ELEMENT_DOOR_FRONT_PC34) {
        trace.f0108_calls = 1;
        trace.f0111_calls = 1;
        trace.f0115_calls = 2;
        trace.first_f0115_order = spec->door_rear_order;
        trace.second_f0115_order = spec->door_front_order;
    } else {
        return -1;
    }

    trace.ok = trace.preceding_backdrops == 2 && trace.d3l_before_d3r &&
               trace.c10_transparency_preserved;
    *out_trace = trace;
    return 0;
}

uint8_t csb_v1_viewport_d3l_d3r_sidewall_backdrops_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /* ReDMCSB: DUNVIEW.C F0104 lines 3113-3156 and F0105 lines 3185-3247
     * pass DEFS.H line 2088 C10_COLOR_FLESH as the transparent color. */
    return source_pixel == CSB_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

const char *csb_v1_viewport_d3l_d3r_sidewall_backdrops_source_evidence_pc34(void)
{
    return s_source_evidence;
}
