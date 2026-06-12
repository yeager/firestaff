#include <dm1_v1_viewport_d2l_d2r_wall_pc34_compat.h>

#include <string.h>

enum {
    DM1_PRESENT = 1,
    DM1_ABSENT = 0,
    DM1_ELEMENT_WALL = 0,               /* ReDMCSB DUNVIEW.C:6945/7096 C00_ELEMENT_WALL. */
    DM1_VIEW_SQUARE_D2L = 4,            /* ReDMCSB DEFS.H:2582 M604_VIEW_SQUARE_D2L. */
    DM1_VIEW_SQUARE_D2R = 5,            /* ReDMCSB DEFS.H:2583 M605_VIEW_SQUARE_D2R. */
    DM1_RELATIVE_DEPTH_D2 = 2,          /* ReDMCSB DUNVIEW.C:8512/8516 F0128. */
    DM1_RELATIVE_LATERAL_D2L = -1,      /* ReDMCSB DUNVIEW.C:8512 F0128. */
    DM1_RELATIVE_LATERAL_D2R = 1,       /* ReDMCSB DUNVIEW.C:8516 F0128. */
    DM1_WALL_D2R = 7,                   /* ReDMCSB DEFS.H:3430 C07_WALL_D2R. */
    DM1_WALL_D2L = 8,                   /* ReDMCSB DEFS.H:3431 C08_WALL_D2L. */
    DM1_ZONE_WALL_D2L = 710,            /* ReDMCSB DEFS.H:4050 C710_ZONE_WALL_D2L. */
    DM1_ZONE_WALL_D2R = 711,            /* ReDMCSB DEFS.H:4051 C711_ZONE_WALL_D2R. */
    DM1_VIEW_WALL_D2L_RIGHT = 7,        /* ReDMCSB DEFS.H:2703 M580_VIEW_WALL_D2L_RIGHT. */
    DM1_VIEW_WALL_D2R_LEFT = 8,         /* ReDMCSB DEFS.H:2704 M581_VIEW_WALL_D2R_LEFT. */
    DM1_VIEW_WALL_D2L_FRONT = 9,        /* ReDMCSB DEFS.H:2705 M582_VIEW_WALL_D2L_FRONT. */
    DM1_VIEW_WALL_D2R_FRONT = 11,       /* ReDMCSB DEFS.H:2707 M584_VIEW_WALL_D2R_FRONT. */
    DM1_VIEW_FLOOR_D2L = 5,             /* ReDMCSB DEFS.H:2755 M591_VIEW_FLOOR_D2L. */
    DM1_VIEW_FLOOR_D2R = 7,             /* ReDMCSB DEFS.H:2757 M593_VIEW_FLOOR_D2R. */
    DM1_ASPECT_RIGHT_WALL_ORNAMENT = 551,
    DM1_ASPECT_FRONT_WALL_ORNAMENT = 552,
    DM1_ASPECT_LEFT_WALL_ORNAMENT = 553,
    DM1_F0104 = 104,                    /* ReDMCSB DUNVIEW.C:3113-3156. */
    DM1_F0105 = 105,                    /* ReDMCSB DUNVIEW.C:3185-3247. */
    DM1_F0107 = 107,                    /* ReDMCSB DUNVIEW.C:3502-3938. */
    DM1_F0115 = 115,
    DM1_C10_COLOR_FLESH = 10,           /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    DM1_F0128_D2L_ORDER = 4,            /* ReDMCSB DUNVIEW.C:8510-8513. */
    DM1_F0128_D2R_ORDER = 5,            /* ReDMCSB DUNVIEW.C:8514-8517. */
    DM1_F0128_D2C_ORDER = 6,            /* ReDMCSB DUNVIEW.C:8518-8521. */
    DM1_LINEAGE_OPEN_ROOM_SHAPE = 1192,
    DM1_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE = 1903
};

static const char s_source_evidence[] =
    "Source-locked contract-only DM1 V1 D2L/D2R side-wall composition; no "
    "real-asset bitmap parity and no game-data load. ReDMCSB DUNVIEW.C:"
    "8503-8521 F0128 draws D2L2/D2R2, then D2L at relative 2,-1 and D2R "
    "at relative 2,+1 before D2C at 2,0. ReDMCSB DUNVIEW.C:6900-6973 "
    "F0119_DUNGEONVIEW_DrawSquareD2L binds M604/C710 and DUNVIEW.C:"
    "7051-7166 F0120_DUNGEONVIEW_DrawSquareD2R_CPSF binds M605/C711. "
    "Their wall branches at DUNVIEW.C:6945-6973 and 7096-7166 draw the "
    "rear D2L/D2R frame, route native C08/C07 wall bitmaps through "
    "F0104 or flipped C07/C08 wall bitmaps through F0105, call F0107 for "
    "side/front wall ornaments, and enter the first-backdrop F0115 pass "
    "only when the front ornament is an alcove. ReDMCSB DUNVIEW.C:"
    "3048-3058 F0100 supplies the transparent D2LCR wall frame blit; "
    "its G0163 frame rows use ByteWidth=72, which is 144 packed PC34 "
    "pixels for the synthetic indexed-pixel edge gate; "
    "DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 preserve DEFS.H:2088 "
    "C10_COLOR_FLESH transparency. ReDMCSB DUNVIEW.C:3502-3938 F0107 "
    "uses DM1 D2 wall ornament views M580/M581/M582/M584, not the D3 "
    "M575/M576/M577/M579 set. ReDMCSB DUNVIEW.C:581-588 provides the "
    "D2L/D2R wall frame rows. DEFS.H:2582-2583 binds M604/M605 view "
    "squares, DEFS.H:2703-2707 binds M580/M581/M582/M584 D2 wall "
    "ornament views, DEFS.H:3430-3431 binds C07/C08 wall indexes, and "
    "DEFS.H:4050-4051 binds C710/C711 wall zones. CSB-lineage "
    "Viewport.cpp:1192-1209 and 1903-1915 anchor the open side-room and "
    "door-facing overlay shape used as lineage FOV-shape evidence.";

static const DM1_V1_D2LD2RWallSpecPc34 s_specs[] = {
    {
        DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34,
        "D2L M604 side-wall wall composition",
        119,
        DM1_VIEW_SQUARE_D2L,
        DM1_RELATIVE_DEPTH_D2,
        DM1_RELATIVE_LATERAL_D2L,
        DM1_F0128_D2L_ORDER,
        DM1_ELEMENT_WALL,
        DM1_ZONE_WALL_D2L,
        DM1_WALL_D2L,
        DM1_WALL_D2R,
        4,
        0,
        74,
        20,
        90,
        DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34,
        DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34,
        61,
        0,
        DM1_ASPECT_RIGHT_WALL_ORNAMENT,
        DM1_ASPECT_FRONT_WALL_ORNAMENT,
        DM1_VIEW_WALL_D2L_RIGHT,
        DM1_VIEW_WALL_D2L_FRONT,
        DM1_VIEW_FLOOR_D2L,
        DM1_F0104,
        DM1_F0105,
        DM1_C10_COLOR_FLESH,
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        DM1_ABSENT,
        DM1_LINEAGE_OPEN_ROOM_SHAPE,
        DM1_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE,
        "ReDMCSB DUNVIEW.C F0119:6900-6973; wall branch 6945-6973"
    },
    {
        DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34,
        "D2R M605 side-wall wall composition",
        120,
        DM1_VIEW_SQUARE_D2R,
        DM1_RELATIVE_DEPTH_D2,
        DM1_RELATIVE_LATERAL_D2R,
        DM1_F0128_D2R_ORDER,
        DM1_ELEMENT_WALL,
        DM1_ZONE_WALL_D2R,
        DM1_WALL_D2R,
        DM1_WALL_D2L,
        5,
        149,
        223,
        20,
        90,
        DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34,
        DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34,
        0,
        0,
        DM1_ASPECT_LEFT_WALL_ORNAMENT,
        DM1_ASPECT_FRONT_WALL_ORNAMENT,
        DM1_VIEW_WALL_D2R_LEFT,
        DM1_VIEW_WALL_D2R_FRONT,
        DM1_VIEW_FLOOR_D2R,
        DM1_F0104,
        DM1_F0105,
        DM1_C10_COLOR_FLESH,
        7,
        8,
        9,
        10,
        11,
        12,
        13,
        DM1_PRESENT,
        DM1_LINEAGE_OPEN_ROOM_SHAPE,
        DM1_LINEAGE_DOOR_FRONT_OVERLAY_SHAPE,
        "ReDMCSB DUNVIEW.C F0120:7051-7166; wall branch 7096-7166"
    }
};

size_t dm1_v1_viewport_d2l_d2r_wall_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const DM1_V1_D2LD2RWallSpecPc34 *
dm1_v1_viewport_d2l_d2r_wall_spec_at_pc34(size_t index)
{
    if (index >= dm1_v1_viewport_d2l_d2r_wall_spec_count_pc34()) return 0;
    return &s_specs[index];
}

const DM1_V1_D2LD2RWallSpecPc34 *
dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(int side)
{
    size_t i;

    for (i = 0; i < dm1_v1_viewport_d2l_d2r_wall_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

uint8_t dm1_v1_viewport_d2l_d2r_wall_blend_c10_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /* ReDMCSB DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 pass
     * DEFS.H:2088 C10_COLOR_FLESH as the transparent color. */
    return source_pixel == DM1_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

int dm1_v1_viewport_d2l_d2r_wall_apply_frame_pixel_pc34(
    const DM1_V1_D2LD2RWallSpecPc34 *spec,
    int viewport_y,
    int viewport_x,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_D2LD2RWallFramePixelPc34 *out)
{
    int local_x;
    int local_y;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->spec = spec;
    out->row = viewport_y;
    out->viewport_x = viewport_x;
    if (!spec) return 0;

    if (viewport_y < spec->wall_frame_y1 ||
        viewport_y > spec->wall_frame_y2 ||
        viewport_x < spec->wall_frame_x1 ||
        viewport_x > spec->wall_frame_x2) {
        out->no_write_metadata = DM1_PRESENT;
        return 1;
    }
    if (!source || !viewport) return 0;

    local_x = viewport_x - spec->wall_frame_x1;
    local_y = viewport_y - spec->wall_frame_y1;
    out->in_clip = DM1_PRESENT;
    out->source_x = spec->wall_frame_source_x + local_x;
    out->source_y = spec->wall_frame_source_y + local_y;
    out->source_offset =
        (size_t)out->source_y *
        (size_t)DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 +
        (size_t)out->source_x;
    out->viewport_offset =
        (size_t)viewport_y * (size_t)DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 +
        (size_t)viewport_x;
    if (out->source_x < 0 ||
        out->source_x >= DM1_V1_D2L_D2R_WALL_SOURCE_PIXEL_WIDTH_PC34 ||
        out->source_y < 0 ||
        out->source_y >= DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34 ||
        out->source_offset >= source_len ||
        out->viewport_offset >= viewport_len) {
        return 0;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip = out->source_pixel == DM1_C10_COLOR_FLESH;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_d2l_d2r_wall_blend_c10_pc34(
            viewport[out->viewport_offset], out->source_pixel);
    out->pixel_after = viewport[out->viewport_offset];
    return 1;
}

static int source_offset(int y, int x)
{
    return y * DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 + x;
}

static int record_c10_blit(
    const DM1_V1_D2LD2RWallSpecPc34 *spec,
    int kind,
    int order_index,
    int view_wall_index,
    int wall_index,
    int blit_function,
    int flipped,
    uint8_t opaque_pixel,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    DM1_V1_D2LD2RWallTracePc34 *trace)
{
    uint8_t source[DM1_V1_D2L_D2R_WALL_SOURCE_WIDTH_PC34 *
                   DM1_V1_D2L_D2R_WALL_SOURCE_HEIGHT_PC34];
    DM1_V1_D2LD2RWallBlitRecordPc34 *record;
    const int dst_x = spec->wall_frame_x1;
    const int dst_y = spec->wall_frame_y1 + (kind % 7);
    const int src_transparent_x = flipped ? 1 : 0;
    const int src_opaque_x = flipped ? 0 : 1;
    int transparent_offset;
    int opaque_offset;
    int transparent_dst;
    int opaque_dst;

    if (trace->blit_count >= DM1_V1_D2L_D2R_WALL_MAX_BLITS_PC34) return -1;
    if (!viewport || viewport_width <= dst_x + 1 || viewport_height <= dst_y) return -1;

    memset(source, DM1_C10_COLOR_FLESH, sizeof(source));
    source[source_offset(0, src_opaque_x)] = opaque_pixel;

    transparent_dst = (dst_y * viewport_width) + dst_x;
    opaque_dst = (dst_y * viewport_width) + dst_x + 1;
    transparent_offset = source_offset(0, src_transparent_x);
    opaque_offset = source_offset(0, src_opaque_x);

    record = &trace->blits[trace->blit_count++];
    memset(record, 0, sizeof(*record));
    record->side = spec->side;
    record->kind = kind;
    record->order_index = order_index;
    record->view_square = spec->view_square;
    record->view_wall_index = view_wall_index;
    record->wall_index = wall_index;
    record->wall_zone = spec->wall_zone;
    record->blit_function = blit_function;
    record->transparent_color = spec->c10_transparent_color;
    record->flipped = flipped;
    record->dst_x = dst_x;
    record->dst_y = dst_y;
    record->source_x = src_opaque_x;
    record->source_y = 0;
    record->source_transparent_sample = source[transparent_offset];
    record->source_opaque_sample = source[opaque_offset];
    record->destination_before_transparent = viewport[transparent_dst];
    viewport[transparent_dst] = dm1_v1_viewport_d2l_d2r_wall_blend_c10_pc34(
        viewport[transparent_dst], source[transparent_offset]);
    record->destination_after_transparent = viewport[transparent_dst];
    viewport[opaque_dst] = dm1_v1_viewport_d2l_d2r_wall_blend_c10_pc34(
        viewport[opaque_dst], source[opaque_offset]);
    record->destination_after_opaque = viewport[opaque_dst];
    record->stats.transparent_pixels = 1;
    record->stats.copied_pixels = 1;
    return 0;
}

static int compose_side(
    const DM1_V1_D2LD2RWallSpecPc34 *spec,
    int use_flipped_wall_bitmaps,
    int front_wall_ornament_is_alcove,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    DM1_V1_D2LD2RWallTracePc34 *trace)
{
    const int wall_index =
        use_flipped_wall_bitmaps ? spec->flipped_wall_index : spec->native_wall_index;
    const int wall_function =
        use_flipped_wall_bitmaps ? spec->flipped_wall_blit_function :
                                   spec->native_wall_blit_function;
    const uint8_t side_base =
        spec->side == DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34 ? 0x20u : 0x50u;

    if (record_c10_blit(spec, DM1_V1_D2L_D2R_WALL_BLIT_REAR_BACKDROP_PC34,
                        spec->rear_backdrop_order_index, 0, 0, DM1_F0104,
                        DM1_ABSENT, (uint8_t)(side_base + 1u), viewport,
                        viewport_width, viewport_height, trace) != 0) return -1;
    if (record_c10_blit(spec, DM1_V1_D2L_D2R_WALL_BLIT_C10_FRAME_TOP_PC34,
                        spec->frame_top_order_index, 0, 0, DM1_F0104,
                        DM1_ABSENT, (uint8_t)(side_base + 2u), viewport,
                        viewport_width, viewport_height, trace) != 0) return -1;
    if (record_c10_blit(spec, DM1_V1_D2L_D2R_WALL_BLIT_C10_FRAME_SIDE_PC34,
                        spec->frame_side_order_index, 0, 0, DM1_F0105,
                        spec->c10_frame_side_is_flipped,
                        (uint8_t)(side_base + 3u), viewport,
                        viewport_width, viewport_height, trace) != 0) return -1;
    if (record_c10_blit(spec, DM1_V1_D2L_D2R_WALL_BLIT_WALL_BITMAP_PC34,
                        spec->wall_bitmap_order_index, 0, wall_index,
                        wall_function, use_flipped_wall_bitmaps,
                        (uint8_t)(side_base + 4u), viewport,
                        viewport_width, viewport_height, trace) != 0) return -1;
    if (record_c10_blit(spec, DM1_V1_D2L_D2R_WALL_BLIT_SIDE_ORNAMENT_PC34,
                        spec->side_ornament_order_index,
                        spec->side_wall_ornament_view, wall_index, DM1_F0107,
                        spec->side == DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34,
                        (uint8_t)(side_base + 5u), viewport,
                        viewport_width, viewport_height, trace) != 0) return -1;
    if (record_c10_blit(spec, DM1_V1_D2L_D2R_WALL_BLIT_FRONT_ORNAMENT_PC34,
                        spec->front_ornament_order_index,
                        spec->front_wall_ornament_view, wall_index, DM1_F0107,
                        spec->side == DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34,
                        (uint8_t)(side_base + 6u), viewport,
                        viewport_width, viewport_height, trace) != 0) return -1;
    if (front_wall_ornament_is_alcove) {
        if (record_c10_blit(
                spec, DM1_V1_D2L_D2R_WALL_BLIT_FRONT_FIRST_BACKDROP_PC34,
                spec->front_first_backdrop_order_index,
                spec->front_wall_ornament_view, wall_index, DM1_F0115,
                DM1_ABSENT, (uint8_t)(side_base + 7u), viewport,
                viewport_width, viewport_height, trace) != 0) return -1;
    }
    return 0;
}

int dm1_v1_viewport_d2l_d2r_wall_compose(
    const DM1_V1_D2LD2RWallComposeStatePc34 *state,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    DM1_V1_D2LD2RWallTracePc34 *out_trace)
{
    const DM1_V1_D2LD2RWallSpecPc34 *d2l =
        dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34);
    const DM1_V1_D2LD2RWallSpecPc34 *d2r =
        dm1_v1_viewport_d2l_d2r_wall_spec_for_side_pc34(
            DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34);
    DM1_V1_D2LD2RWallComposeStatePc34 local_state = { 0, 0 };
    DM1_V1_D2LD2RWallTracePc34 trace;
    int i;

    if (!out_trace || !viewport || !d2l || !d2r ||
        viewport_width < DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 ||
        viewport_height < DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34) {
        return -1;
    }
    if (state) local_state = *state;
    memset(&trace, 0, sizeof(trace));

    trace.source_locked_contract_only = DM1_PRESENT;
    trace.no_real_asset_bitmap_parity = DM1_PRESENT;
    trace.no_game_data_load = DM1_PRESENT;
    trace.d2c_order_index = DM1_F0128_D2C_ORDER;
    trace.d2l_view_square = d2l->view_square;
    trace.d2r_view_square = d2r->view_square;
    trace.d2l_side_ornament_view = d2l->side_wall_ornament_view;
    trace.d2r_side_ornament_view = d2r->side_wall_ornament_view;
    trace.d2l_front_ornament_view = d2l->front_wall_ornament_view;
    trace.d2r_front_ornament_view = d2r->front_wall_ornament_view;
    trace.first_wall_zone = d2l->wall_zone;
    trace.second_wall_zone = d2r->wall_zone;
    trace.first_wall_index =
        local_state.use_flipped_wall_bitmaps ? d2l->flipped_wall_index :
                                               d2l->native_wall_index;
    trace.second_wall_index =
        local_state.use_flipped_wall_bitmaps ? d2r->flipped_wall_index :
                                               d2r->native_wall_index;
    trace.d2l_before_d2c = d2l->f0128_order_index < DM1_F0128_D2C_ORDER;
    trace.d2r_before_d2c = d2r->f0128_order_index < DM1_F0128_D2C_ORDER;
    trace.d2l_before_d2r = d2l->f0128_order_index < d2r->f0128_order_index;

    if (compose_side(d2l, local_state.use_flipped_wall_bitmaps,
                     local_state.front_wall_ornament_is_alcove, viewport,
                     viewport_width, viewport_height, &trace) != 0 ||
        compose_side(d2r, local_state.use_flipped_wall_bitmaps,
                     local_state.front_wall_ornament_is_alcove, viewport,
                     viewport_width, viewport_height, &trace) != 0) {
        return -1;
    }

    trace.all_blits_use_c10 = DM1_PRESENT;
    trace.all_blits_preserve_c10 = DM1_PRESENT;
    for (i = 0; i < trace.blit_count; ++i) {
        const DM1_V1_D2LD2RWallBlitRecordPc34 *record = &trace.blits[i];
        if (record->side == DM1_V1_D2L_D2R_WALL_SIDE_D2L_PC34) {
            ++trace.d2l_blit_count;
        } else if (record->side == DM1_V1_D2L_D2R_WALL_SIDE_D2R_PC34) {
            ++trace.d2r_blit_count;
        }
        if (record->transparent_color != DM1_C10_COLOR_FLESH) {
            trace.all_blits_use_c10 = DM1_ABSENT;
        }
        if (record->destination_before_transparent !=
            record->destination_after_transparent) {
            trace.all_blits_preserve_c10 = DM1_ABSENT;
        }
        if (record->blit_function == DM1_F0104) ++trace.f0104_calls;
        if (record->blit_function == DM1_F0105) ++trace.f0105_calls;
        if (record->kind == DM1_V1_D2L_D2R_WALL_BLIT_SIDE_ORNAMENT_PC34) {
            ++trace.f0107_side_calls;
        }
        if (record->kind == DM1_V1_D2L_D2R_WALL_BLIT_FRONT_ORNAMENT_PC34) {
            ++trace.f0107_front_calls;
        }
        if (record->kind ==
            DM1_V1_D2L_D2R_WALL_BLIT_FRONT_FIRST_BACKDROP_PC34) {
            ++trace.f0115_first_backdrop_calls;
        }
    }

    trace.ok = trace.source_locked_contract_only &&
               trace.no_real_asset_bitmap_parity &&
               trace.no_game_data_load &&
               trace.d2l_before_d2r &&
               trace.d2l_before_d2c &&
               trace.d2r_before_d2c &&
               trace.all_blits_use_c10 &&
               trace.all_blits_preserve_c10 &&
               trace.d2l_blit_count >= 6 &&
               trace.d2r_blit_count >= 6 &&
               trace.first_wall_zone == DM1_ZONE_WALL_D2L &&
               trace.second_wall_zone == DM1_ZONE_WALL_D2R &&
               trace.d2l_view_square == DM1_VIEW_SQUARE_D2L &&
               trace.d2r_view_square == DM1_VIEW_SQUARE_D2R;
    *out_trace = trace;
    return trace.ok ? 0 : 1;
}

int dm1_v1_viewport_d2l_d2r_wall_pc34_compat_run(
    DM1_V1_D2LD2RWallTracePc34 *out_trace)
{
    uint8_t viewport[DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34 *
                     DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34];
    DM1_V1_D2LD2RWallComposeStatePc34 state = { 0, 0 };

    memset(viewport, 0xee, sizeof(viewport));
    return dm1_v1_viewport_d2l_d2r_wall_compose(
        &state, viewport, DM1_V1_D2L_D2R_WALL_VIEWPORT_WIDTH_PC34,
        DM1_V1_D2L_D2R_WALL_VIEWPORT_HEIGHT_PC34, out_trace);
}

const char *dm1_v1_viewport_d2l_d2r_wall_source_evidence_pc34(void)
{
    return s_source_evidence;
}
