#include "csb/csb_v1_viewport_d1l_d1r_wall_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_SIDE_D1L = 1,
    CSB_SIDE_D1R = 2,
    CSB_ELEMENT_WALL = 0,          /* ReDMCSB DUNVIEW.C:7436/7604 C00_ELEMENT_WALL. */
    CSB_ELEMENT_TELEPORTER = 5,    /* ReDMCSB DUNVIEW.C:7538/7706 C05_ELEMENT_TELEPORTER. */
    CSB_VIEW_SQUARE_D1C = 3,       /* ReDMCSB DEFS.H:2599 M606_VIEW_SQUARE_D1C. */
    CSB_VIEW_SQUARE_D1L = 4,       /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L. */
    CSB_VIEW_SQUARE_D1R = 5,       /* ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R. */
    CSB_VIEW_DEPTH_D1 = 1,         /* ReDMCSB DUNVIEW.C:8524/8528 F0128. */
    CSB_VIEW_LATERAL_D1L = -1,     /* ReDMCSB DUNVIEW.C:8524 F0128. */
    CSB_VIEW_LATERAL_D1R = 1,      /* ReDMCSB DUNVIEW.C:8528 F0128. */
    CSB_WALL_D1R = 2,              /* ReDMCSB DEFS.H:3425 C02_WALL_D1R. */
    CSB_WALL_D1L = 3,              /* ReDMCSB DEFS.H:3426 C03_WALL_D1L. */
    CSB_ZONE_WALL_D1C = 712,       /* ReDMCSB DEFS.H:4052 C712_ZONE_WALL_D1C. */
    CSB_ZONE_WALL_D1L = 713,       /* ReDMCSB DEFS.H:4053 C713_ZONE_WALL_D1L. */
    CSB_ZONE_WALL_D1R = 714,       /* ReDMCSB DEFS.H:4054 C714_ZONE_WALL_D1R. */
    CSB_ZONE_WALL_D0L = 716,       /* ReDMCSB DEFS.H:4040-4057 C716 viewport spec. */
    CSB_ZONE_WALL_D0R = 717,       /* ReDMCSB DEFS.H:4040-4057 C717 viewport spec. */
    CSB_C10_COLOR_FLESH = 10,      /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_SCREEN_WIDTH = 320,        /* ReDMCSB COORD.C:1713 G2071_C320_ScreenPixelWidth. */
    CSB_SCREEN_HEIGHT = 200,       /* ReDMCSB COORD.C:1714 G2072_C200_ScreenPixelHeight. */
    CSB_VIEWPORT_WIDTH = 224,      /* ReDMCSB COORD.C:1721 G2073_C224_ViewportPixelWidth. */
    CSB_VIEWPORT_HEIGHT = 136,     /* ReDMCSB COORD.C:1722 G2074_C136_ViewportHeight. */
    CSB_SOURCE_WIDTH = 256,
    CSB_SOURCE_HEIGHT = 111,
    CSB_D1_CLIP_WIDTH = 64,
    CSB_D1_CLIP_HEIGHT = 111,
    CSB_D1L_FRAME_ROW = 7,         /* ReDMCSB DUNVIEW.C:590 G0163 D1L row. */
    CSB_D1R_FRAME_ROW = 8,         /* ReDMCSB DUNVIEW.C:591 G0163 D1R row. */
    CSB_ZONE_MATH_BASE = 709
};

static const char s_source_evidence[] =
    "Source-locked contract-only CSB V1 D1L/D1R wall pair; no real-asset "
    "bitmap parity and no CSB game-data load. ReDMCSB DUNVIEW.C:7391-7560 "
    "F0122_DUNGEONVIEW_DrawSquareD1L and DUNVIEW.C:7559-7725 "
    "F0123_DUNGEONVIEW_DrawSquareD1R read square aspects through "
    "DUNGEON.C:2466-2523 F0172. Their C00_ELEMENT_WALL bodies at "
    "DUNVIEW.C:7436-7460 and 7604-7628 use F0104 native row blits "
    "from DUNVIEW.C:3113-3156 or F0105 row-local scratch flips from "
    "DUNVIEW.C:3185-3247, preserving DEFS.H:2088 C10_COLOR_FLESH. "
    "DUNVIEW.C:4547-4581 and 5668-5671 anchor the F0115 thing-pass "
    "follow-up keep-out for this wall slice; DUNGEON.C:1769-1838 F0163 "
    "and 1840-1905 F0164 thing-list mutations are not reached. "
    "DUNVIEW.C:8318-8542 F0128 dispatches D1L then D1R and follows with "
    "D1C; DUNVIEW.C:8294 cross-references the F0127 D1C boundary. "
    "COORD.C:1713-1722 and COMMAND.C:1126-1127 bind 320x200 screen and "
    "224x136 viewport edge clipping. DEFS.H:4040-4057 includes C716/C717 "
    "viewport-zone spec while this D1 wall pair binds C713/C714. "
    "CSB-lineage Viewport.cpp:1192-1209 and 1903-1915 bind D1L/D1R row "
    "composition and distinguish this explicit D1L/D1R wall gate from the "
    "existing CSB D1L2/D1R2 wall and D1L/D1R F0111 door gates.";

static const CSB_V1_D1LD1RWallSpecPc34 s_specs[] = {
    {
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
        CSB_PRESENT, CSB_PRESENT,
        CSB_SIDE_D1L, 122, 0,
        CSB_VIEW_SQUARE_D1L, CSB_VIEW_DEPTH_D1, CSB_VIEW_LATERAL_D1L,
        CSB_ELEMENT_WALL, CSB_ELEMENT_TELEPORTER,
        CSB_ZONE_WALL_D1L, CSB_ZONE_WALL_D1C,
        CSB_WALL_D1L, CSB_WALL_D1R,
        CSB_D1L_FRAME_ROW, 0, 63, 9, 119, 128, CSB_D1_CLIP_HEIGHT,
        192, 0, CSB_D1_CLIP_WIDTH, CSB_D1_CLIP_HEIGHT,
        CSB_C10_COLOR_FLESH, CSB_SCREEN_WIDTH, CSB_SCREEN_HEIGHT,
        CSB_VIEWPORT_WIDTH, CSB_VIEWPORT_HEIGHT,
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
        CSB_ABSENT, CSB_ABSENT, CSB_PRESENT,
        CSB_PRESENT, CSB_PRESENT,
        "D1L wall body via F0122",
        "G0163_aauc_Graphic558_Frame_Walls[M607] {0,63,9,119,128,111,192,0}",
        "G2107_WallSet[C03_WALL_D1L] native / C02_WALL_D1R flipped",
        "ReDMCSB DUNVIEW.C:7391-7560 F0122 D1L wall body"
    },
    {
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
        CSB_PRESENT, CSB_PRESENT,
        CSB_SIDE_D1R, 123, 1,
        CSB_VIEW_SQUARE_D1R, CSB_VIEW_DEPTH_D1, CSB_VIEW_LATERAL_D1R,
        CSB_ELEMENT_WALL, CSB_ELEMENT_TELEPORTER,
        CSB_ZONE_WALL_D1R, CSB_ZONE_WALL_D1C,
        CSB_WALL_D1R, CSB_WALL_D1L,
        CSB_D1R_FRAME_ROW, 160, 223, 9, 119, 128, CSB_D1_CLIP_HEIGHT,
        0, 0, CSB_D1_CLIP_WIDTH, CSB_D1_CLIP_HEIGHT,
        CSB_C10_COLOR_FLESH, CSB_SCREEN_WIDTH, CSB_SCREEN_HEIGHT,
        CSB_VIEWPORT_WIDTH, CSB_VIEWPORT_HEIGHT,
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
        CSB_PRESENT, CSB_PRESENT, CSB_PRESENT,
        CSB_ABSENT, CSB_ABSENT, CSB_PRESENT,
        CSB_PRESENT, CSB_PRESENT,
        "D1R wall body via F0123",
        "G0163_aauc_Graphic558_Frame_Walls[M608] {160,223,9,119,128,111,0,0}",
        "G2107_WallSet[C02_WALL_D1R] native / C03_WALL_D1L flipped",
        "ReDMCSB DUNVIEW.C:7559-7725 F0123 D1R wall body"
    }
};

size_t csb_v1_viewport_d1l_d1r_wall_spec_count_pc34(void)
{
    return sizeof(s_specs) / sizeof(s_specs[0]);
}

const CSB_V1_D1LD1RWallSpecPc34 *
csb_v1_viewport_d1l_d1r_wall_spec_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d1l_d1r_wall_spec_count_pc34()) return 0;
    return &s_specs[index];
}

const CSB_V1_D1LD1RWallSpecPc34 *
csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(int side)
{
    for (size_t i = 0; i < csb_v1_viewport_d1l_d1r_wall_spec_count_pc34(); ++i) {
        if (s_specs[i].side == side) return &s_specs[i];
    }
    return 0;
}

int csb_v1_viewport_d1l_d1r_wall_zone_for_square_pc34(int view_square)
{
    if (view_square != CSB_VIEW_SQUARE_D1L && view_square != CSB_VIEW_SQUARE_D1R) {
        return -1;
    }
    return CSB_ZONE_MATH_BASE + view_square;
}

int csb_v1_viewport_d1l_d1r_wall_map_viewport_x_to_source_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    int viewport_x,
    int flipped_variant,
    int *out_source_x)
{
    int local_x;

    if (!spec || !out_source_x) return -1;
    if (viewport_x < spec->frame_x1 || viewport_x > spec->frame_x2) return 1;
    local_x = viewport_x - spec->frame_x1;
    if (local_x < 0 || local_x >= spec->clip_width) return -1;
    *out_source_x = spec->frame_source_x +
                    (flipped_variant ? (spec->clip_width - 1 - local_x) : local_x);
    return 0;
}

int csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    const uint8_t *source,
    int source_width,
    int source_height,
    uint8_t *viewport,
    int viewport_width,
    int viewport_height,
    int flipped_variant,
    CSB_V1_D1LD1RWallBlitStatsPc34 *stats)
{
    CSB_V1_D1LD1RWallBlitStatsPc34 local = { 0, 0, 0, 0 };

    if (stats) *stats = local;
    if (!spec || !source || !viewport ||
        source_width <= 0 || source_height <= 0 ||
        viewport_width <= 0 || viewport_height <= 0) {
        local.rejected = 1;
        if (stats) *stats = local;
        return -1;
    }
    if (source_width < spec->frame_source_x + spec->clip_width ||
        source_height < spec->frame_source_y + spec->clip_height) {
        local.rejected = 1;
        if (stats) *stats = local;
        return -1;
    }

    /* ReDMCSB DUNVIEW.C:3113-3156 F0104 and 3185-3247 F0105 preserve
     * DEFS.H:2088 C10_COLOR_FLESH while clipping D1L/D1R rows to the
     * 224x136 viewport bound by COORD.C:1713-1722 and COMMAND.C:1126-1127. */
    for (int y = 0; y < spec->clip_height; ++y) {
        const int dst_y = spec->frame_y1 + y;
        const int src_y = spec->frame_source_y + y;

        for (int x = 0; x < spec->clip_width; ++x) {
            int src_x = spec->frame_source_x + x;
            const int dst_x = spec->frame_x1 + x;
            uint8_t pixel;

            if (flipped_variant) {
                src_x = spec->frame_source_x + (spec->clip_width - 1 - x);
            }
            pixel = source[(src_y * source_width) + src_x];
            if (dst_x < 0 || dst_x >= viewport_width ||
                dst_y < 0 || dst_y >= viewport_height) {
                ++local.clipped_pixels;
                continue;
            }
            if (pixel == (uint8_t)spec->transparent_color) {
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

uint16_t csb_v1_viewport_d1l_d1r_wall_preserve_first_thing_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    uint16_t first_thing)
{
    /* ReDMCSB DUNVIEW.C:7436-7460 and 7604-7628 return from the wall cases
     * before F0115; DUNVIEW.C:4547-4581/5668-5671 and DUNGEON.C:1769-1905
     * are follow-up/thing-list anchors, not writes in this wall slice. */
    if (!spec || !spec->f0115_thing_pass_keepout) return 0xffffu;
    return first_thing;
}

int csb_v1_viewport_d1l_d1r_wall_preserve_map_identity_pc34(
    const CSB_V1_D1LD1RWallSpecPc34 *spec,
    int in_map_x,
    int in_map_y,
    int *out_map_x,
    int *out_map_y)
{
    /* ReDMCSB DUNGEON.C:2466-2523 F0172 reads square aspect metadata; the
     * synthetic wall route does not emulate or mutate real dungeon map data. */
    if (!spec || !out_map_x || !out_map_y || !spec->f0172_square_aspect_read) {
        return -1;
    }
    *out_map_x = in_map_x;
    *out_map_y = in_map_y;
    return 0;
}

int csb_v1_viewport_d1l_d1r_wall_pc34_compat_run(
    CSB_V1_D1LD1RWallRunResultPc34 *out_result)
{
    const CSB_V1_D1LD1RWallSpecPc34 *d1l =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(CSB_SIDE_D1L);
    const CSB_V1_D1LD1RWallSpecPc34 *d1r =
        csb_v1_viewport_d1l_d1r_wall_spec_for_side_pc34(CSB_SIDE_D1R);
    uint8_t left_source[CSB_SOURCE_WIDTH * CSB_SOURCE_HEIGHT];
    uint8_t right_source[CSB_SOURCE_WIDTH * CSB_SOURCE_HEIGHT];
    uint8_t viewport[CSB_VIEWPORT_WIDTH * CSB_VIEWPORT_HEIGHT];
    CSB_V1_D1LD1RWallBlitStatsPc34 left_stats;
    CSB_V1_D1LD1RWallBlitStatsPc34 right_stats;
    CSB_V1_D1LD1RWallRunResultPc34 result;
    int left_copied;
    int right_copied;

    if (!d1l || !d1r) return -1;
    for (size_t i = 0; i < sizeof(left_source); ++i) left_source[i] = CSB_C10_COLOR_FLESH;
    for (size_t i = 0; i < sizeof(right_source); ++i) right_source[i] = CSB_C10_COLOR_FLESH;
    for (size_t i = 0; i < sizeof(viewport); ++i) viewport[i] = 0xeeu;

    left_source[(0 * CSB_SOURCE_WIDTH) + 192] = 0x41u;
    left_source[(0 * CSB_SOURCE_WIDTH) + 255] = 0x42u;
    left_source[(110 * CSB_SOURCE_WIDTH) + 193] = 0x43u;
    right_source[(0 * CSB_SOURCE_WIDTH) + 0] = 0x51u;
    right_source[(0 * CSB_SOURCE_WIDTH) + 63] = 0x52u;
    right_source[(110 * CSB_SOURCE_WIDTH) + 62] = 0x53u;

    left_copied = csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
        d1l, left_source, CSB_SOURCE_WIDTH, CSB_SOURCE_HEIGHT,
        viewport, CSB_VIEWPORT_WIDTH, CSB_VIEWPORT_HEIGHT, 0, &left_stats);
    right_copied = csb_v1_viewport_d1l_d1r_wall_apply_c10_frame_clip_pc34(
        d1r, right_source, CSB_SOURCE_WIDTH, CSB_SOURCE_HEIGHT,
        viewport, CSB_VIEWPORT_WIDTH, CSB_VIEWPORT_HEIGHT, 0, &right_stats);

    result.route_count = (int)csb_v1_viewport_d1l_d1r_wall_spec_count_pc34();
    result.identities_ok =
        d1l->redmcsb_function_number == 122 &&
        d1r->redmcsb_function_number == 123 &&
        d1l->side == CSB_SIDE_D1L &&
        d1r->side == CSB_SIDE_D1R;
    result.coordinates_ok =
        d1l->view_square == CSB_VIEW_SQUARE_D1L &&
        d1r->view_square == CSB_VIEW_SQUARE_D1R &&
        d1l->frame_row == CSB_D1L_FRAME_ROW &&
        d1r->frame_row == CSB_D1R_FRAME_ROW &&
        d1l->frame_x1 == 0 &&
        d1l->frame_x2 == 63 &&
        d1r->frame_x1 == 160 &&
        d1r->frame_x2 == 223;
    result.c10_transparency_ok =
        d1l->transparent_color == CSB_C10_COLOR_FLESH &&
        d1r->transparent_color == CSB_C10_COLOR_FLESH &&
        left_stats.transparent_pixels == (CSB_D1_CLIP_WIDTH * CSB_D1_CLIP_HEIGHT) - 3 &&
        right_stats.transparent_pixels == (CSB_D1_CLIP_WIDTH * CSB_D1_CLIP_HEIGHT) - 3;
    result.row_local_flip_ok =
        d1l->f0104_native_body_row_blit &&
        d1r->f0104_native_body_row_blit &&
        d1l->f0105_row_local_scratch_flip &&
        d1r->f0105_row_local_scratch_flip &&
        d1l->native_wall_index == CSB_WALL_D1L &&
        d1r->native_wall_index == CSB_WALL_D1R &&
        d1l->flipped_wall_index == CSB_WALL_D1R &&
        d1r->flipped_wall_index == CSB_WALL_D1L;
    result.edge_clip_ok =
        left_copied == 3 &&
        right_copied == 3 &&
        viewport[(9 * CSB_VIEWPORT_WIDTH) + 0] == 0x41u &&
        viewport[(9 * CSB_VIEWPORT_WIDTH) + 63] == 0x42u &&
        viewport[(9 * CSB_VIEWPORT_WIDTH) + 64] == 0xeeu &&
        viewport[(9 * CSB_VIEWPORT_WIDTH) + 159] == 0xeeu &&
        viewport[(9 * CSB_VIEWPORT_WIDTH) + 160] == 0x51u &&
        viewport[(9 * CSB_VIEWPORT_WIDTH) + 223] == 0x52u;
    result.f0128_followup_ok =
        d1l->f0128_draw_order_index == 0 &&
        d1r->f0128_draw_order_index == 1 &&
        d1l->f0128_resets_map_coordinates_after_side &&
        d1r->f0128_resets_map_coordinates_after_side &&
        d1l->f0128_draws_d1c_after_pair &&
        d1r->f0128_draws_d1c_after_pair &&
        d1l->f0127_d1c_followup_boundary &&
        d1r->f0127_d1c_followup_boundary;
    result.first_thing_before = 0x2345u;
    result.first_thing_after =
        csb_v1_viewport_d1l_d1r_wall_preserve_first_thing_pc34(
            d1l, result.first_thing_before);
    result.map_x_before = 17;
    result.map_y_before = 29;
    result.map_x_after = -1;
    result.map_y_after = -1;
    csb_v1_viewport_d1l_d1r_wall_preserve_map_identity_pc34(
        d1r, result.map_x_before, result.map_y_before,
        &result.map_x_after, &result.map_y_after);
    result.dungeon_identity_ok =
        d1l->f0172_square_aspect_read &&
        d1r->f0172_square_aspect_read &&
        d1l->f0163_link_thing_keepout == 0 &&
        d1r->f0164_unlink_thing_keepout == 0 &&
        result.first_thing_before == result.first_thing_after &&
        result.map_x_before == result.map_x_after &&
        result.map_y_before == result.map_y_after;
    result.scope_keepout_ok =
        d1l->no_real_asset_bitmap_parity &&
        d1l->no_game_data_load &&
        d1l->no_c17_door_ornament &&
        d1l->no_door_state_byte &&
        d1l->no_f0111_dispatch &&
        d1l->no_f0108_floor_ornament_coupling &&
        d1l->no_c15_destroyed_mask &&
        d1l->not_csb_d1l2_d1r2_wall_gate &&
        d1l->not_dm1_wall_or_stairs_pit_dispatch;
    result.d1l_copied_pixels = left_copied;
    result.d1r_copied_pixels = right_copied;
    result.ok = result.route_count == 2 &&
                result.identities_ok &&
                result.coordinates_ok &&
                result.c10_transparency_ok &&
                result.row_local_flip_ok &&
                result.edge_clip_ok &&
                result.f0128_followup_ok &&
                result.dungeon_identity_ok &&
                result.scope_keepout_ok;

    if (out_result) *out_result = result;
    return result.ok ? 0 : 1;
}

const char *csb_v1_viewport_d1l_d1r_wall_source_evidence_pc34(void)
{
    (void)CSB_ZONE_WALL_D0L;
    (void)CSB_ZONE_WALL_D0R;
    (void)CSB_VIEW_SQUARE_D1C;
    return s_source_evidence;
}
