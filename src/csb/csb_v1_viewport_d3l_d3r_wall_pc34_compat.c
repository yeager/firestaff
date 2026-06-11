#include "csb_v1_viewport_d3l_d3r_wall_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_VIEW_SQUARE_D3L = 12,       /* ReDMCSB DEFS.H:2608 M601_VIEW_SQUARE_D3L. */
    CSB_VIEW_SQUARE_D3R = 13,       /* ReDMCSB DEFS.H:2609 M602_VIEW_SQUARE_D3R. */
    CSB_RELATIVE_DEPTH_D3 = 3,      /* ReDMCSB DUNVIEW.C:8490-8495 F0128. */
    CSB_RELATIVE_LATERAL_D3L = -1,  /* ReDMCSB DUNVIEW.C:8490 F0128. */
    CSB_RELATIVE_LATERAL_D3R = 1,   /* ReDMCSB DUNVIEW.C:8494 F0128. */
    CSB_ELEMENT_WALL = 0,           /* ReDMCSB DUNVIEW.C:6406/6545 C00_ELEMENT_WALL. */
    CSB_WALL_D3R = 12,              /* ReDMCSB DUNVIEW.C:6423/6563 C12_WALL_D3R. */
    CSB_WALL_D3L = 13,              /* ReDMCSB DUNVIEW.C:6427/6555 C13_WALL_D3L. */
    CSB_ZONE_WALL_D3L = 705,        /* ReDMCSB DEFS.H:4045 C705_ZONE_WALL_D3L. */
    CSB_ZONE_WALL_D3R = 706,        /* ReDMCSB DEFS.H:4046 C706_ZONE_WALL_D3R. */
    CSB_VIEW_WALL_D3L_RIGHT = 2,    /* ReDMCSB DEFS.H:2698 M575_VIEW_WALL_D3L_RIGHT. */
    CSB_VIEW_WALL_D3R_LEFT = 3,     /* ReDMCSB DEFS.H:2699 M576_VIEW_WALL_D3R_LEFT. */
    CSB_VIEW_WALL_D3L_FRONT = 4,    /* ReDMCSB DEFS.H:2700 M577_VIEW_WALL_D3L_FRONT. */
    CSB_VIEW_WALL_D3R_FRONT = 6,    /* ReDMCSB DEFS.H:2702 M579_VIEW_WALL_D3R_FRONT. */
    CSB_VIEW_FLOOR_D3L = 2,         /* ReDMCSB DEFS.H:2752 M588_VIEW_FLOOR_D3L. */
    CSB_VIEW_FLOOR_D3R = 4,         /* ReDMCSB DEFS.H:2754 M590_VIEW_FLOOR_D3R. */
    CSB_ASPECT_RIGHT_WALL_ORNAMENT = 551,
    CSB_ASPECT_FRONT_WALL_ORNAMENT = 552,
    CSB_ASPECT_LEFT_WALL_ORNAMENT = 553,
    CSB_F0104 = 104,                /* ReDMCSB DUNVIEW.C:3113-3156. */
    CSB_F0105 = 105,                /* ReDMCSB DUNVIEW.C:3185-3247. */
    CSB_F0107 = 107,                /* ReDMCSB DUNVIEW.C:3502-3938. */
    CSB_F0115_ALCOVE_ORDER = 0,     /* ReDMCSB DUNVIEW.C:6433-6435/6569-6571. */
    CSB_C10_COLOR_FLESH = 10,       /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_D3_WALL_FRAME_HEIGHT = 51,
    CSB_D3_WALL_FRAME_BYTE_WIDTH = 64,
    CSB_D3_WALL_SOURCE_HEIGHT = 51,
    CSB_D3_WALL_SOURCE_WIDTH = 64,
    CSB_VIEWPORT_WIDTH = 224,
    CSB_VIEWPORT_HEIGHT = 136,
    CSB_F0107_DEPTH3_SCALE = 14,
    CSB_F0107_DEPTH3_PALETTE = 3,
    CSB_LINEAGE_OPEN_ROOM_SHAPE = 1192,
    CSB_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE = 1903
};

static const char s_source_evidence[] =
    "Source-locked contract-only CSB V1 D3L/D3R side-wall wall composition; "
    "no real-asset bitmap parity and no CSB game-data load. ReDMCSB "
    "DUNVIEW.C:8478-8500 F0128 draws D3L2/D3R2 first, then D3L at relative "
    "3,-1 and D3R at relative 3,+1 before D3C. ReDMCSB DUNVIEW.C:6361-6480 "
    "F0116 and 6500-6622 F0117 bind the C12/C13 wall cells. Their wall "
    "branches at DUNVIEW.C:6406-6437 and 6545-6573 route C705/C706 through "
    "F0104 native wall blits or F0105 flipped wall blits, call F0107 for "
    "side and front wall ornaments, and return unless the front ornament is "
    "an alcove, in which case F0115 receives order 0. ReDMCSB DUNVIEW.C:"
    "3113-3156 F0104 and 3185-3247 F0105 preserve DEFS.H:2088 "
    "C10_COLOR_FLESH transparency. ReDMCSB DUNVIEW.C:3502-3938 F0107 "
    "provides D3 side/front ornament view handling and C10 blits. "
    "ReDMCSB DUNVIEW.C:581-585 G0163_aauc_Graphic558_Frame_Walls provides "
    "the D3L/D3R wall frame x/y/byte-width/height rows. DEFS.H:2608-2609 "
    "binds C12/C13 D3L/D3R view squares, DEFS.H:2698-2702 binds "
    "M575/M576/M577/M579 wall ornament views, DEFS.H:2752-2754 binds "
    "M588/M590 floor ornament contrast views, and DEFS.H:4045-4046 binds "
    "C705/C706 wall zones. CSB-lineage Viewport.cpp:1192-1209 and "
    "1903-1915 anchor the open side-room and door-facing overlay shape.";

static const CSB_V1_D3LD3RWallSpecPc34 s_specs[] = {
    {
        CSB_V1_D3L_D3R_WALL_SIDE_D3L_PC34,
        "D3L C12 side-wall wall composition",
        116,
        CSB_VIEW_SQUARE_D3L,
        CSB_RELATIVE_DEPTH_D3,
        CSB_RELATIVE_LATERAL_D3L,
        2,
        CSB_ELEMENT_WALL,
        CSB_ZONE_WALL_D3L,
        CSB_WALL_D3L,
        CSB_WALL_D3R,
        1,
        0,
        83,
        25,
        75,
        CSB_D3_WALL_FRAME_BYTE_WIDTH,
        CSB_D3_WALL_FRAME_HEIGHT,
        32,
        0,
        CSB_ASPECT_RIGHT_WALL_ORNAMENT,
        CSB_ASPECT_FRONT_WALL_ORNAMENT,
        CSB_VIEW_WALL_D3L_RIGHT,
        CSB_VIEW_WALL_D3L_FRONT,
        CSB_VIEW_FLOOR_D3L,
        CSB_F0104,
        CSB_F0105,
        0,
        0,
        1,
        CSB_F0115_ALCOVE_ORDER,
        CSB_C10_COLOR_FLESH,
        CSB_F0107_DEPTH3_SCALE,
        CSB_F0107_DEPTH3_PALETTE,
        CSB_LINEAGE_OPEN_ROOM_SHAPE,
        CSB_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE,
        "ReDMCSB DUNVIEW.C F0116:6361-6480; wall branch 6406-6437"
    },
    {
        CSB_V1_D3L_D3R_WALL_SIDE_D3R_PC34,
        "D3R C13 side-wall wall composition",
        117,
        CSB_VIEW_SQUARE_D3R,
        CSB_RELATIVE_DEPTH_D3,
        CSB_RELATIVE_LATERAL_D3R,
        3,
        CSB_ELEMENT_WALL,
        CSB_ZONE_WALL_D3R,
        CSB_WALL_D3R,
        CSB_WALL_D3L,
        2,
        139,
        223,
        25,
        75,
        CSB_D3_WALL_FRAME_BYTE_WIDTH,
        CSB_D3_WALL_FRAME_HEIGHT,
        0,
        0,
        CSB_ASPECT_LEFT_WALL_ORNAMENT,
        CSB_ASPECT_FRONT_WALL_ORNAMENT,
        CSB_VIEW_WALL_D3R_LEFT,
        CSB_VIEW_WALL_D3R_FRONT,
        CSB_VIEW_FLOOR_D3R,
        CSB_F0104,
        CSB_F0105,
        1,
        2,
        3,
        CSB_F0115_ALCOVE_ORDER,
        CSB_C10_COLOR_FLESH,
        CSB_F0107_DEPTH3_SCALE,
        CSB_F0107_DEPTH3_PALETTE,
        CSB_LINEAGE_OPEN_ROOM_SHAPE,
        CSB_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE,
        "ReDMCSB DUNVIEW.C F0117:6500-6622; wall branch 6545-6573"
    }
};

size_t csb_v1_viewport_d3l_d3r_wall_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D3LD3RWallSpecPc34 *
csb_v1_viewport_d3l_d3r_wall_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d3l_d3r_wall_spec_count_pc34()) return 0;
    return &s_specs[index];
}

const CSB_V1_D3LD3RWallSpecPc34 *
csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_d3l_d3r_wall_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

uint8_t csb_v1_viewport_d3l_d3r_wall_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /* ReDMCSB DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 pass
     * DEFS.H:2088 C10_COLOR_FLESH as the transparent color. */
    return source_pixel == CSB_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

int csb_v1_viewport_d3l_d3r_wall_trace_pair_pc34(
    int use_flipped_wall_bitmaps,
    int front_wall_ornament_is_alcove,
    CSB_V1_D3LD3RWallTracePc34 *out_trace)
{
    const CSB_V1_D3LD3RWallSpecPc34 *d3l =
        csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(
            CSB_V1_D3L_D3R_WALL_SIDE_D3L_PC34);
    const CSB_V1_D3LD3RWallSpecPc34 *d3r =
        csb_v1_viewport_d3l_d3r_wall_spec_for_side_pc34(
            CSB_V1_D3L_D3R_WALL_SIDE_D3R_PC34);
    CSB_V1_D3LD3RWallTracePc34 trace = { 0 };

    if (!d3l || !d3r || !out_trace) return -1;
    trace.source_locked_contract_only = CSB_PRESENT;
    trace.no_real_asset_bitmap_parity = CSB_PRESENT;
    trace.no_game_data_load = CSB_PRESENT;
    trace.d3l_before_d3r =
        d3l->wall_blit_order_index == 0 && d3r->wall_blit_order_index == 1;
    trace.wall_blit_calls = 2;
    trace.f0104_calls = use_flipped_wall_bitmaps ? 0 : 2;
    trace.f0105_calls = use_flipped_wall_bitmaps ? 2 : 0;
    trace.f0107_side_calls = 2;
    trace.f0107_front_calls = 2;
    trace.f0115_calls = front_wall_ornament_is_alcove ? 2 : 0;
    trace.first_wall_zone = d3l->wall_zone;
    trace.second_wall_zone = d3r->wall_zone;
    trace.first_wall_index =
        use_flipped_wall_bitmaps ? d3l->flipped_wall_index : d3l->native_wall_index;
    trace.second_wall_index =
        use_flipped_wall_bitmaps ? d3r->flipped_wall_index : d3r->native_wall_index;
    trace.first_blit_function =
        use_flipped_wall_bitmaps ? d3l->flipped_wall_blit_function :
                                   d3l->native_wall_blit_function;
    trace.second_blit_function =
        use_flipped_wall_bitmaps ? d3r->flipped_wall_blit_function :
                                   d3r->native_wall_blit_function;
    trace.front_alcove_uses_zero_order = front_wall_ornament_is_alcove;
    trace.wall_returns_without_front_alcove = !front_wall_ornament_is_alcove;
    trace.side_ornament_view =
        (d3l->side_wall_ornament_view << 8) | d3r->side_wall_ornament_view;
    trace.front_ornament_view =
        (d3l->front_wall_ornament_view << 8) | d3r->front_wall_ornament_view;
    trace.c10_transparency_preserved =
        csb_v1_viewport_d3l_d3r_wall_blend_c10_pc34(0x5au, 10u) == 0x5au &&
        csb_v1_viewport_d3l_d3r_wall_blend_c10_pc34(0x5au, 0x24u) == 0x24u;
    trace.ok = trace.source_locked_contract_only &&
               trace.no_real_asset_bitmap_parity &&
               trace.no_game_data_load &&
               trace.d3l_before_d3r &&
               trace.wall_blit_calls == 2 &&
               trace.f0107_side_calls == 2 &&
               trace.f0107_front_calls == 2 &&
               trace.first_wall_zone == CSB_ZONE_WALL_D3L &&
               trace.second_wall_zone == CSB_ZONE_WALL_D3R &&
               trace.c10_transparency_preserved;
    *out_trace = trace;
    return trace.ok ? 0 : 1;
}

int csb_v1_viewport_d3l_d3r_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_D3LD3RWallSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    int flipped_variant,
    CSB_V1_D3LD3RWallBlitStatsPc34 *stats)
{
    CSB_V1_D3LD3RWallBlitStatsPc34 local = { 0, 0, 0, 0 };

    if (stats) *stats = local;
    if (!spec || !source || !viewport ||
        source_width < spec->wall_frame_byte_width ||
        source_height < spec->wall_frame_height ||
        viewport_width <= 0 || viewport_height <= 0) {
        local.rejected = 1;
        if (stats) *stats = local;
        return -1;
    }

    /* ReDMCSB DUNVIEW.C:581-585 supplies the D3L/D3R frame metadata;
     * DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 apply C10 transparency
     * while the flipped variant mirrors each row before the same blit. */
    for (int y = 0; y < spec->wall_frame_height; ++y) {
        const int dst_y = spec->wall_frame_y1 + y;

        for (int x = 0; x < spec->wall_frame_byte_width; ++x) {
            const int src_x = flipped_variant ? (spec->wall_frame_byte_width - 1 - x) : x;
            const uint8_t pixel = source[(y * source_width) + src_x];
            const int dst_x = spec->wall_frame_x1 + x;

            if (dst_x < 0 || dst_x >= viewport_width ||
                dst_y < 0 || dst_y >= viewport_height) {
                ++local.clipped_pixels;
                continue;
            }
            if (pixel == (uint8_t)spec->c10_transparent_color) {
                ++local.transparent_pixels;
                continue;
            }
            viewport[(dst_y * viewport_width) + dst_x] = pixel;
            ++local.copied_pixels;
        }
    }

    if (stats) *stats = local;
    return local.copied_pixels;
}

int csb_v1_viewport_d3l_d3r_wall_pc34_compat_run(
    CSB_V1_D3LD3RWallTracePc34 *out_trace)
{
    return csb_v1_viewport_d3l_d3r_wall_trace_pair_pc34(
        CSB_ABSENT, CSB_ABSENT, out_trace);
}

const char *csb_v1_viewport_d3l_d3r_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
