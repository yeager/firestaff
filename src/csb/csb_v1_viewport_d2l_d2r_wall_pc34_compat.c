#include "csb_v1_viewport_d2l_d2r_wall_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_VIEW_SQUARE_D2L = 7,       /* ReDMCSB DEFS.H:2603 M604_VIEW_SQUARE_D2L. */
    CSB_VIEW_SQUARE_D2R = 8,       /* ReDMCSB DEFS.H:2604 M605_VIEW_SQUARE_D2R. */
    CSB_VIEW_SQUARE_D3L = 12,      /* ReDMCSB DEFS.H:2608 M601_VIEW_SQUARE_D3L. */
    CSB_VIEW_SQUARE_D3R = 13,      /* ReDMCSB DEFS.H:2609 M602_VIEW_SQUARE_D3R. */
    CSB_RELATIVE_DEPTH_D2 = 2,     /* ReDMCSB DUNVIEW.C:8512-8517 F0128. */
    CSB_RELATIVE_LATERAL_D2L = -1, /* ReDMCSB DUNVIEW.C:8512 F0128. */
    CSB_RELATIVE_LATERAL_D2R = 1,  /* ReDMCSB DUNVIEW.C:8516 F0128. */
    CSB_ELEMENT_WALL = 0,          /* ReDMCSB DUNVIEW.C:6945/7096 C00_ELEMENT_WALL. */
    CSB_WALL_D2R = 7,              /* ReDMCSB DEFS.H:3430 C07_WALL_D2R. */
    CSB_WALL_D2L = 8,              /* ReDMCSB DEFS.H:3431 C08_WALL_D2L. */
    CSB_ZONE_WALL_D2L = 710,       /* ReDMCSB DEFS.H:4050 C710_ZONE_WALL_D2L. */
    CSB_ZONE_WALL_D2R = 711,       /* ReDMCSB DEFS.H:4051 C711_ZONE_WALL_D2R. */
    CSB_VIEW_WALL_D2L_RIGHT = 7,   /* ReDMCSB DEFS.H:2703 M580_VIEW_WALL_D2L_RIGHT. */
    CSB_VIEW_WALL_D2R_LEFT = 8,    /* ReDMCSB DEFS.H:2704 M581_VIEW_WALL_D2R_LEFT. */
    CSB_VIEW_WALL_D2L_FRONT = 9,   /* ReDMCSB DEFS.H:2705 M582_VIEW_WALL_D2L_FRONT. */
    CSB_VIEW_WALL_D2R_FRONT = 11,  /* ReDMCSB DEFS.H:2707 M584_VIEW_WALL_D2R_FRONT. */
    CSB_VIEW_FLOOR_D2L = 5,        /* ReDMCSB DEFS.H:2755 M591_VIEW_FLOOR_D2L. */
    CSB_VIEW_FLOOR_D2R = 7,        /* ReDMCSB DEFS.H:2757 M593_VIEW_FLOOR_D2R. */
    CSB_ASPECT_RIGHT_WALL_ORNAMENT = 551,
    CSB_ASPECT_FRONT_WALL_ORNAMENT = 552,
    CSB_ASPECT_LEFT_WALL_ORNAMENT = 553,
    CSB_F0104 = 104,               /* ReDMCSB DUNVIEW.C:3113-3156. */
    CSB_F0105 = 105,               /* ReDMCSB DUNVIEW.C:3185-3247. */
    CSB_F0107 = 107,               /* ReDMCSB DUNVIEW.C:3502-3938. */
    CSB_F0115_ALCOVE_ORDER = 0,    /* ReDMCSB DUNVIEW.C:6969-6971/7120-7122. */
    CSB_C10_COLOR_FLESH = 10,      /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_D2_WALL_FRAME_HEIGHT = 71,
    CSB_D2_WALL_FRAME_BYTE_WIDTH = 72,
    CSB_D2_WALL_SOURCE_HEIGHT = 71,
    CSB_D2_WALL_SOURCE_WIDTH = 144,
    CSB_VIEWPORT_WIDTH = 224,
    CSB_VIEWPORT_HEIGHT = 136,
    CSB_F0107_DEPTH2_SCALE = 21,
    CSB_F0107_DEPTH2_PALETTE = 2,
    CSB_LINEAGE_OPEN_ROOM_SHAPE = 1192,
    CSB_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE = 1903
};

static const char s_source_evidence[] =
    "Source-locked contract-only CSB V1 D2L/D2R side-wall wall composition; "
    "no real-asset bitmap parity and no CSB game-data load. ReDMCSB "
    "DUNVIEW.C:8504-8521 F0128 draws D2L2/D2R2 first, then D2L at relative "
    "2,-1 and D2R at relative 2,+1 before D2C. ReDMCSB DUNVIEW.C:6900-7049 "
    "F0119 and 7051-7242 F0120 bind the M604/M605 wall cells; DEFS.H:"
    "2603-2604 confirms CSB D2L/D2R are view squares 7/8, while "
    "DEFS.H:2608-2609 C12/C13 are D3L/D3R and are deliberately rejected "
    "for this D2 source lock. Their wall branches at DUNVIEW.C:6945-6973 "
    "and 7096-7124 route C710/C711 through F0104 native wall blits or "
    "F0105 flipped wall blits, call F0107 for side and front wall "
    "ornaments, and return unless the front ornament is an alcove, in "
    "which case F0115 receives order 0. ReDMCSB DUNVIEW.C:3113-3156 F0104 "
    "and 3185-3247 F0105 preserve DEFS.H:2088 C10_COLOR_FLESH "
    "transparency. ReDMCSB DUNVIEW.C:3502-3938 F0107 provides the "
    "D2-distance branch: M580/M581 side views flip/select left/right, "
    "M582/M584 front views shift by +6/-6 where applicable, use scale 21 "
    "and D2 palette changes, and blit with C10. ReDMCSB DUNVIEW.C:581-588 "
    "G0163_aauc_Graphic558_Frame_Walls provides the D2L/D2R wall frame "
    "x/y/byte-width/height/source rows. DEFS.H:2703-2707 binds "
    "M580/M581/M582/M584 wall ornament views, DEFS.H:2755-2757 binds "
    "M591/M593 floor ornament contrast views, DEFS.H:3430-3431 binds "
    "C07/C08 wall set indices, and DEFS.H:4050-4051 binds C710/C711 wall "
    "zones. CSB-lineage Viewport.cpp:1192-1209 and 1903-1915 anchor the "
    "open side-room and door-facing overlay shape.";

static const CSB_V1_D2LD2RWallSpecPc34 s_specs[] = {
    {
        CSB_V1_D2L_D2R_WALL_SIDE_D2L_PC34,
        "D2L M604 side-wall wall composition",
        119,
        CSB_VIEW_SQUARE_D2L,
        CSB_VIEW_SQUARE_D3L,
        CSB_RELATIVE_DEPTH_D2,
        CSB_RELATIVE_LATERAL_D2L,
        2,
        4,
        CSB_ELEMENT_WALL,
        CSB_ZONE_WALL_D2L,
        CSB_WALL_D2L,
        CSB_WALL_D2R,
        4,
        0,
        74,
        20,
        90,
        CSB_D2_WALL_FRAME_BYTE_WIDTH,
        CSB_D2_WALL_FRAME_HEIGHT,
        61,
        0,
        CSB_D2_WALL_SOURCE_WIDTH,
        CSB_ASPECT_RIGHT_WALL_ORNAMENT,
        CSB_ASPECT_FRONT_WALL_ORNAMENT,
        CSB_VIEW_WALL_D2L_RIGHT,
        CSB_VIEW_WALL_D2L_FRONT,
        CSB_VIEW_FLOOR_D2L,
        CSB_F0104,
        CSB_F0105,
        0,
        1,
        2,
        3,
        4,
        CSB_F0115_ALCOVE_ORDER,
        CSB_C10_COLOR_FLESH,
        CSB_F0107_DEPTH2_SCALE,
        CSB_F0107_DEPTH2_PALETTE,
        CSB_LINEAGE_OPEN_ROOM_SHAPE,
        CSB_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE,
        "ReDMCSB DUNVIEW.C F0119:6900-7049; wall branch 6945-6973"
    },
    {
        CSB_V1_D2L_D2R_WALL_SIDE_D2R_PC34,
        "D2R M605 side-wall wall composition",
        120,
        CSB_VIEW_SQUARE_D2R,
        CSB_VIEW_SQUARE_D3R,
        CSB_RELATIVE_DEPTH_D2,
        CSB_RELATIVE_LATERAL_D2R,
        3,
        4,
        CSB_ELEMENT_WALL,
        CSB_ZONE_WALL_D2R,
        CSB_WALL_D2R,
        CSB_WALL_D2L,
        5,
        149,
        223,
        20,
        90,
        CSB_D2_WALL_FRAME_BYTE_WIDTH,
        CSB_D2_WALL_FRAME_HEIGHT,
        0,
        0,
        CSB_D2_WALL_SOURCE_WIDTH,
        CSB_ASPECT_LEFT_WALL_ORNAMENT,
        CSB_ASPECT_FRONT_WALL_ORNAMENT,
        CSB_VIEW_WALL_D2R_LEFT,
        CSB_VIEW_WALL_D2R_FRONT,
        CSB_VIEW_FLOOR_D2R,
        CSB_F0104,
        CSB_F0105,
        0,
        1,
        2,
        5,
        6,
        CSB_F0115_ALCOVE_ORDER,
        CSB_C10_COLOR_FLESH,
        CSB_F0107_DEPTH2_SCALE,
        CSB_F0107_DEPTH2_PALETTE,
        CSB_LINEAGE_OPEN_ROOM_SHAPE,
        CSB_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE,
        "ReDMCSB DUNVIEW.C F0120:7051-7242; wall branch 7096-7124"
    }
};

size_t csb_v1_viewport_d2l_d2r_wall_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D2LD2RWallSpecPc34 *
csb_v1_viewport_d2l_d2r_wall_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d2l_d2r_wall_spec_count_pc34()) return 0;
    return &s_specs[index];
}

const CSB_V1_D2LD2RWallSpecPc34 *
csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < csb_v1_viewport_d2l_d2r_wall_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

uint8_t csb_v1_viewport_d2l_d2r_wall_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /* ReDMCSB DUNVIEW.C:3113-3156 F0104, 3185-3247 F0105, and
     * 3502-3938 F0107 pass DEFS.H:2088 C10_COLOR_FLESH as transparency. */
    return source_pixel == CSB_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

int csb_v1_viewport_d2l_d2r_wall_trace_pair_pc34(
    int use_flipped_wall_bitmaps,
    int front_wall_ornament_is_alcove,
    CSB_V1_D2LD2RWallTracePc34 *out_trace)
{
    const CSB_V1_D2LD2RWallSpecPc34 *d2l =
        csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            CSB_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const CSB_V1_D2LD2RWallSpecPc34 *d2r =
        csb_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            CSB_V1_D2L_D2R_WALL_SIDE_D2R_PC34);
    CSB_V1_D2LD2RWallTracePc34 trace = { 0 };

    if (!d2l || !d2r || !out_trace) return -1;
    trace.source_locked_contract_only = CSB_PRESENT;
    trace.no_real_asset_bitmap_parity = CSB_PRESENT;
    trace.no_game_data_load = CSB_PRESENT;
    trace.d2l_before_d2r =
        d2l->f0128_order_index == 2 && d2r->f0128_order_index == 3;
    trace.d2l_d2r_before_d2c =
        d2l->f0128_order_index < d2l->d2c_f0128_order_index &&
        d2r->f0128_order_index < d2r->d2c_f0128_order_index;
    trace.rejected_c12_c13_as_d2 =
        d2l->view_square != d2l->rejected_d3_view_square &&
        d2r->view_square != d2r->rejected_d3_view_square;
    trace.rear_d2_outer_calls = 2;
    trace.transparent_frame_calls = 2;
    trace.wall_blit_calls = 2;
    trace.f0104_calls = use_flipped_wall_bitmaps ? 0 : 2;
    trace.f0105_calls = use_flipped_wall_bitmaps ? 2 : 0;
    trace.f0107_side_calls = 2;
    trace.f0107_front_calls = 2;
    trace.f0115_calls = front_wall_ornament_is_alcove ? 2 : 0;
    trace.first_wall_zone = d2l->wall_zone;
    trace.second_wall_zone = d2r->wall_zone;
    trace.first_wall_index =
        use_flipped_wall_bitmaps ? d2l->flipped_wall_index : d2l->native_wall_index;
    trace.second_wall_index =
        use_flipped_wall_bitmaps ? d2r->flipped_wall_index : d2r->native_wall_index;
    trace.first_blit_function =
        use_flipped_wall_bitmaps ? d2l->flipped_wall_blit_function :
                                   d2l->native_wall_blit_function;
    trace.second_blit_function =
        use_flipped_wall_bitmaps ? d2r->flipped_wall_blit_function :
                                   d2r->native_wall_blit_function;
    trace.front_alcove_uses_zero_order = front_wall_ornament_is_alcove;
    trace.wall_returns_without_front_alcove = !front_wall_ornament_is_alcove;
    trace.side_ornament_view =
        (d2l->side_wall_ornament_view << 8) | d2r->side_wall_ornament_view;
    trace.front_ornament_view =
        (d2l->front_wall_ornament_view << 8) | d2r->front_wall_ornament_view;
    trace.c10_transparency_preserved =
        csb_v1_viewport_d2l_d2r_wall_blend_c10_pc34(0x5au, 10u) == 0x5au &&
        csb_v1_viewport_d2l_d2r_wall_blend_c10_pc34(0x5au, 0x24u) == 0x24u;
    trace.d2_row_origin_preserved =
        d2l->wall_frame_y1 == 20 && d2r->wall_frame_y1 == 20 &&
        d2l->wall_frame_height == CSB_D2_WALL_FRAME_HEIGHT &&
        d2l->wall_frame_source_x == 61 && d2r->wall_frame_source_x == 0;
    trace.ok = trace.source_locked_contract_only &&
               trace.no_real_asset_bitmap_parity &&
               trace.no_game_data_load &&
               trace.d2l_before_d2r &&
               trace.d2l_d2r_before_d2c &&
               trace.rejected_c12_c13_as_d2 &&
               trace.wall_blit_calls == 2 &&
               trace.f0107_side_calls == 2 &&
               trace.f0107_front_calls == 2 &&
               trace.first_wall_zone == CSB_ZONE_WALL_D2L &&
               trace.second_wall_zone == CSB_ZONE_WALL_D2R &&
               trace.c10_transparency_preserved &&
               trace.d2_row_origin_preserved;
    *out_trace = trace;
    return trace.ok ? 0 : 1;
}

int csb_v1_viewport_d2l_d2r_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_D2LD2RWallSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    int flipped_variant,
    CSB_V1_D2LD2RWallBlitStatsPc34 *stats)
{
    CSB_V1_D2LD2RWallBlitStatsPc34 local = { 0, 0, 0, 0 };

    if (stats) *stats = local;
    if (!spec || !source || !viewport ||
        source_width < spec->wall_frame_source_x + spec->wall_frame_byte_width ||
        source_height < spec->wall_frame_source_y + spec->wall_frame_height ||
        viewport_width <= 0 || viewport_height <= 0) {
        local.rejected = 1;
        if (stats) *stats = local;
        return -1;
    }

    /* ReDMCSB DUNVIEW.C:581-588 supplies the D2L/D2R frame metadata;
     * DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 apply C10 transparency
     * while the flipped variant mirrors each D2 row before the same blit. */
    for (int y = 0; y < spec->wall_frame_height; ++y) {
        const int dst_y = spec->wall_frame_y1 + y;
        const int src_y = spec->wall_frame_source_y + y;

        for (int x = 0; x < spec->wall_frame_byte_width; ++x) {
            int src_x = spec->wall_frame_source_x + x;
            const int dst_x = spec->wall_frame_x1 + x;
            uint8_t pixel;

            if (flipped_variant) {
                src_x = spec->wall_frame_source_x +
                        (spec->wall_frame_byte_width - 1 - x);
            }
            pixel = source[(src_y * source_width) + src_x];
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

int csb_v1_viewport_d2l_d2r_wall_pc34_compat_run(
    CSB_V1_D2LD2RWallTracePc34 *out_trace)
{
    return csb_v1_viewport_d2l_d2r_wall_trace_pair_pc34(
        CSB_ABSENT, CSB_ABSENT, out_trace);
}

const char *csb_v1_viewport_d2l_d2r_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
