#include "csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat.h"

#include <stdlib.h>
#include <string.h>

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_SIDE_D0L2 = 1,
    CSB_SIDE_D0R2 = 2,
    CSB_VIEW_SQUARE_D0L = 1,      /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L. */
    CSB_VIEW_SQUARE_D0R = 2,      /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R. */
    CSB_VIEW_DEPTH_D0 = 0,        /* ReDMCSB DUNVIEW.C:372 G2027[1/2]. */
    CSB_VIEW_LANE_D0L = -1,       /* ReDMCSB DUNVIEW.C:371 G2026[1]. */
    CSB_VIEW_LANE_D0R = 1,        /* ReDMCSB DUNVIEW.C:371 G2026[2]. */
    CSB_G2028_DISABLED = -1,      /* ReDMCSB DUNVIEW.C:373 G2028[1/2]. */
    CSB_G2033_D0L = 11,           /* ReDMCSB DUNVIEW.C:375 G2033[1]. */
    CSB_G2033_D0R = 12,           /* ReDMCSB DUNVIEW.C:375 G2033[2]. */
    CSB_G2034_D0L = 15,           /* ReDMCSB DUNVIEW.C:376 G2034[1]. */
    CSB_G2034_D0R = 16,           /* ReDMCSB DUNVIEW.C:376 G2034[2]. */
    CSB_FIELD_D0L = 14,           /* ReDMCSB DUNVIEW.C:377 G2035[1]. */
    CSB_FIELD_D0R = 15,           /* ReDMCSB DUNVIEW.C:377 G2035[2]. */
    CSB_D0L_ORDER = 0x0002,       /* ReDMCSB DUNVIEW.C:8005; DEFS.H:2660. */
    CSB_D0R_ORDER = 0x0001,       /* ReDMCSB DUNVIEW.C:8115; DEFS.H:2659. */
    CSB_CELL_FRONT_LEFT = 0,      /* ReDMCSB DEFS.H:2642. */
    CSB_CELL_FRONT_RIGHT = 1,     /* ReDMCSB DEFS.H:2643. */
    CSB_CELL_BACK_RIGHT = 2,      /* ReDMCSB DEFS.H:2644. */
    CSB_CELL_BACK_LEFT = 3,       /* ReDMCSB DEFS.H:2645. */
    CSB_WALL_FRAME_D0L_ROW = 10,  /* ReDMCSB DUNVIEW.C:592-594 G0163 table. */
    CSB_WALL_FRAME_D0R_ROW = 11,  /* ReDMCSB DUNVIEW.C:592-594 G0163 table. */
    CSB_C10_COLOR_FLESH = 10,     /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_C2500_ZONE_ITEM = 2500,   /* ReDMCSB DEFS.H:4228; DUNVIEW.C:5075. */
    CSB_C2900_ZONE_PROJECTILE = 2900, /* ReDMCSB DEFS.H:4230; DUNVIEW.C:5683. */
    CSB_C3014_ZONE_CENTER_EXPLOSION = 3014, /* ReDMCSB DEFS.H:4234; DUNVIEW.C:6107. */
    CSB_C3031_ZONE_SIDE_EXPLOSION = 3031,   /* ReDMCSB DEFS.H:4235; DUNVIEW.C:6122. */
    CSB_C3200_ZONE_CREATURE = 3200,         /* ReDMCSB DEFS.H:4236; DUNVIEW.C:5615. */
    CSB_OBJECT_CREATURE_SHIFT = 0x8000,
    CSB_C716_ZONE_WALL_D0L = 716, /* ReDMCSB DEFS.H:4056; DUNVIEW.C:8059. */
    CSB_C717_ZONE_WALL_D0R = 717, /* ReDMCSB DEFS.H:4057; DUNVIEW.C:8159. */
    CSB_C870_ZONE_CEILING_D0L = 870, /* ReDMCSB DUNVIEW.C:8003. */
    CSB_C872_ZONE_CEILING_D0R = 872, /* ReDMCSB DUNVIEW.C:8113. */
    CSB_LINEAGE_RF0L1 = 18,       /* CSB-lineage Viewport.cpp:343. */
    CSB_LINEAGE_RF0R1 = 19,       /* CSB-lineage Viewport.cpp:345. */
    CSB_LINEAGE_F0L1_CONTENTS = 60128, /* CSB-lineage Viewport.cpp:514. */
    CSB_LINEAGE_F0R1_CONTENTS = 60130, /* CSB-lineage Viewport.cpp:516. */
    CSB_LINEAGE_STD_ROOM_OBJECTS = 60006, /* CSB-lineage Viewport.cpp:379. */
    CSB_LINEAGE_DRAWORDER02 = 60288, /* CSB-lineage Viewport.cpp:690/2620. */
    CSB_LINEAGE_DRAWORDER01 = 60287, /* CSB-lineage Viewport.cpp:689/2619. */
    CSB_LINEAGE_DRAWORDER218 = 60279, /* CSB-lineage Viewport.cpp:681/1906. */
    CSB_LINEAGE_DRAWORDER349 = 60280  /* CSB-lineage Viewport.cpp:682/1915. */
};

static int s_initialized;

static const CSB_V1_D0L2D0R2F0115ThingPassEvidencePc34 s_evidence = {
    "Source-locked contract-only gate; synthetic fixture only, no real-asset "
    "bitmap parity and no CSB game-data load.",
    "ReDMCSB DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L",
    "ReDMCSB DUNVIEW.C:8064-8162 F0126_DUNGEONVIEW_DrawSquareD0R",
    "ReDMCSB DUNVIEW.C:8536-8541 F0128 draws D0L then D0R",
    "ReDMCSB DUNVIEW.C:4547-4581,4806-4811,4923,5201-5214,5295,"
    "5615-5617,5668-5683,5916-5923,5998-5999,6107,6122 F0115",
    "ReDMCSB DUNVIEW.C:2995-3015 F0674_F0128_sub per-frame bitmap copy",
    "ReDMCSB DUNVIEW.C:581-594 G0163 frame rows D0L/D0R",
    "ReDMCSB DEFS.H:2088,2596-2598,2642-2660,4056-4057,4228-4236",
    "CSB-lineage Viewport.cpp:1192-1209 F0L1/F0R1 open StdDrawRoomObjects",
    "CSB-lineage Viewport.cpp:1903-1915 center door-facing two-pass dispatch"
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate; source_locked_contract_only=1; "
    "no_real_asset_bitmap_parity=1; no_game_data_load=1. ReDMCSB "
    "DUNVIEW.C:7960-8062 F0125_DUNGEONVIEW_DrawSquareD0L routes corridor, "
    "door-side, teleporter, and pit fallthrough through F0112 at 8003 and "
    "F0115 at 8005 with M610_VIEW_SQUARE_D0L and "
    "C0x0002_CELL_ORDER_BACKRIGHT; wall returns through the separate "
    "F0100/F0104 path at 8007-8038. DUNVIEW.C:8064-8162 "
    "F0126_DUNGEONVIEW_DrawSquareD0R mirrors the right side with F0112 at "
    "8113 and F0115 at 8115 using M611_VIEW_SQUARE_D0R and "
    "C0x0001_CELL_ORDER_BACKLEFT; wall returns at 8117-8144. "
    "DUNVIEW.C:8536-8541 F0128 dispatches D0L at relative offset 0,-1 "
    "before D0R at 0,1. DUNVIEW.C:2995-3015 F0674_F0128_sub copies the "
    "per-frame ceiling/floor bitmaps before the view-square draw order; "
    "DUNVIEW.C:581-594 G0163 anchors the clipped D0L/D0R wall-frame rows "
    "{0,31,0,135,16,136,0,0} and {192,223,0,135,16,136,0,0}. "
    "DUNVIEW.C:4547-4581 defines the F0115 thing-pass order; "
    "4806-4811 loads G2026/G2027/G2028 rows; 4923 and 5668-5683 skip "
    "items/projectiles because G2028[1/2] is negative; 5201-5214, 5295, "
    "and 5615-5617 gate D0L creatures to BACKRIGHT and D0R creatures to "
    "BACKLEFT with C3200 zones; 5916-5923, 5998-5999, 6107, and 6122 bind "
    "explosion zones C3000/C3014/C3031. DUNVIEW.C:8050-8059 and "
    "8150-8159 draw teleporter fields after the F0115 route using "
    "G2035/C716/C717 wall zones. DEFS.H:2088 anchors C10_COLOR_FLESH; "
    "DEFS.H:2596-2598 anchors M609/M610/M611; DEFS.H:2642-2660 anchors "
    "view-cell ordinals; DEFS.H:4056-4057 and 4228-4236 anchor wall and "
    "thing-zone bases. CSB-lineage Viewport.cpp:1192-1209 binds F0L1/F0R1 "
    "open routes through DrawOrder02/DrawOrder01 StdDrawRoomObjects, while "
    "CSB-lineage Viewport.cpp:1903-1915 is the distinct center F1 "
    "door-facing dispatch with DrawOrder218/DrawOrder349.";

static const CSB_V1_D0L2D0R2F0115ThingPassPc34 s_fixtures[] = {
    {
        CSB_SIDE_D0L2,
        "D0L2 near-left corridor/open-floor thing pass",
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        1,
        1,
        CSB_VIEW_SQUARE_D0L,
        CSB_VIEW_DEPTH_D0,
        CSB_VIEW_LANE_D0L,
        CSB_D0L_ORDER,
        CSB_CELL_BACK_RIGHT,
        1,
        CSB_WALL_FRAME_D0L_ROW,
        0,
        31,
        0,
        135,
        16,
        136,
        0,
        0,
        0,
        31,
        0,
        135,
        0,
        135,
        CSB_PRESENT,
        CSB_C10_COLOR_FLESH,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_G2028_DISABLED,
        CSB_PRESENT,
        CSB_G2033_D0L,
        CSB_CELL_BACK_RIGHT,
        CSB_G2034_D0L,
        CSB_FIELD_D0L,
        CSB_C716_ZONE_WALL_D0L,
        CSB_C870_ZONE_CEILING_D0L,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_LINEAGE_RF0L1,
        CSB_LINEAGE_F0L1_CONTENTS,
        CSB_LINEAGE_DRAWORDER02,
        CSB_LINEAGE_STD_ROOM_OBJECTS,
        CSB_LINEAGE_DRAWORDER218,
        CSB_LINEAGE_DRAWORDER349,
        "ReDMCSB DUNVIEW.C:8003/8005/8059; F0128 8536-8537 D0L first",
        "ReDMCSB DUNVIEW.C:4547-4581; 4923/5295/5615-5617/5668-5683/6107/6122",
        s_source_evidence
    },
    {
        CSB_SIDE_D0R2,
        "D0R2 near-right corridor/open-floor thing pass",
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        1,
        1,
        CSB_VIEW_SQUARE_D0R,
        CSB_VIEW_DEPTH_D0,
        CSB_VIEW_LANE_D0R,
        CSB_D0R_ORDER,
        CSB_CELL_BACK_LEFT,
        1,
        CSB_WALL_FRAME_D0R_ROW,
        192,
        223,
        0,
        135,
        16,
        136,
        0,
        0,
        192,
        223,
        0,
        135,
        0,
        135,
        CSB_PRESENT,
        CSB_C10_COLOR_FLESH,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_G2028_DISABLED,
        CSB_PRESENT,
        CSB_G2033_D0R,
        CSB_CELL_BACK_LEFT,
        CSB_G2034_D0R,
        CSB_FIELD_D0R,
        CSB_C717_ZONE_WALL_D0R,
        CSB_C872_ZONE_CEILING_D0R,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_LINEAGE_RF0R1,
        CSB_LINEAGE_F0R1_CONTENTS,
        CSB_LINEAGE_DRAWORDER01,
        CSB_LINEAGE_STD_ROOM_OBJECTS,
        CSB_LINEAGE_DRAWORDER218,
        CSB_LINEAGE_DRAWORDER349,
        "ReDMCSB DUNVIEW.C:8113/8115/8159; F0128 8540-8541 D0R second",
        "ReDMCSB DUNVIEW.C:4547-4581; 4923/5295/5615-5617/5668-5683/6107/6122",
        s_source_evidence
    }
};

int csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34(void)
{
    /* ReDMCSB: F0125/F0126 lines 8005/8115 seed contract metadata only;
     * no DUNGEON.DAT/GRAPHICS.DAT is read by this source-lock gate. */
    s_initialized = CSB_PRESENT;
    return s_initialized;
}

size_t csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34(void)
{
    if (!s_initialized) csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_init_pc34();
    return sizeof(s_fixtures) / sizeof(s_fixtures[0]);
}

const CSB_V1_D0L2D0R2F0115ThingPassPc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_at_pc34(size_t index)
{
    if (index >= csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34()) {
        return 0;
    }
    return &s_fixtures[index];
}

const CSB_V1_D0L2D0R2F0115ThingPassPc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(int side)
{
    size_t i;

    for (i = 0;
         i < csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_count_pc34();
         ++i) {
        if (s_fixtures[i].side == side) return &s_fixtures[i];
    }
    return 0;
}

const CSB_V1_D0L2D0R2F0115ThingPassEvidencePc34 *
csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_evidence_pc34(void)
{
    return &s_evidence;
}

int csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int x,
    int y)
{
    if (!fixture) return 0;
    return x >= fixture->viewport_clip_x1 &&
           x <= fixture->viewport_clip_x2 &&
           y >= fixture->viewport_clip_y1 &&
           y <= fixture->viewport_clip_y2;
}

int csb_v1_viewport_d0l2_d0r2_f0115_source_y_visible_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int source_y)
{
    if (!fixture) return 0;
    return source_y >= fixture->source_clip_y1 &&
           source_y <= fixture->source_clip_y2;
}

unsigned char csb_v1_viewport_d0l2_d0r2_f0115_blend_pixel_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    unsigned char destination,
    unsigned char source)
{
    if (!fixture || source == (unsigned char)fixture->transparent_color) {
        return destination;
    }
    return source;
}

int csb_v1_viewport_d0l2_d0r2_f0115_apply_pixel_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int x,
    int y,
    int source_y,
    unsigned char source,
    unsigned char *destination)
{
    if (!fixture || !destination) return 0;
    if (!csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
            fixture, x, y)) {
        return 0;
    }
    if (!csb_v1_viewport_d0l2_d0r2_f0115_source_y_visible_pc34(
            fixture, source_y)) {
        return 0;
    }
    if (source == (unsigned char)fixture->transparent_color) return 0;
    *destination = csb_v1_viewport_d0l2_d0r2_f0115_blend_pixel_pc34(
        fixture, *destination, source);
    return 1;
}

int csb_v1_viewport_d0l2_d0r2_f0115_item_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    (void)view_cell;
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 line 4923 rejects D0L/D0R items because
     * G2028[1] and G2028[2] are -1 (DUNVIEW.C:373). */
    return fixture->item_projectile_row < 0 ? -1 :
        ((CSB_C2500_ZONE_ITEM + fixture->item_projectile_row * 4 + view_cell) |
         CSB_OBJECT_CREATURE_SHIFT);
}

int csb_v1_viewport_d0l2_d0r2_f0115_projectile_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    (void)view_cell;
    if (!fixture) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5668-5683 use the same negative row
     * gate for D0L/D0R projectiles. */
    return fixture->item_projectile_row < 0 ? -1 :
        CSB_C2900_ZONE_PROJECTILE + fixture->item_projectile_row * 4 + view_cell;
}

int csb_v1_viewport_d0l2_d0r2_f0115_creature_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture || view_cell != fixture->creature_cell_gate) return -1;
    /* ReDMCSB: DUNVIEW.C F0115 lines 5295 and 5615-5617 gate D0L quarter
     * creatures to BACKRIGHT and D0R to BACKLEFT. */
    return (CSB_C3200_ZONE_CREATURE +
            fixture->creature_row * 5 +
            view_cell) | CSB_OBJECT_CREATURE_SHIFT;
}

int csb_v1_viewport_d0l2_d0r2_f0115_centered_explosion_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture)
{
    if (!fixture) return -1;
    return CSB_C3014_ZONE_CENTER_EXPLOSION + fixture->explosion_row;
}

int csb_v1_viewport_d0l2_d0r2_f0115_side_explosion_zone_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture ||
        (view_cell != CSB_CELL_FRONT_LEFT && view_cell != CSB_CELL_FRONT_RIGHT)) {
        return -1;
    }
    return CSB_C3031_ZONE_SIDE_EXPLOSION + fixture->explosion_row * 2 + view_cell;
}

int csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_real_asset_receipt_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    int source_graphics_dat_bound,
    int no_synthetic_pixels,
    int no_fallback_visuals,
    int source_graphics_item_index,
    size_t source_byte_count,
    uint32_t source_payload_hash,
    CSB_V1_D0L2D0R2F0115ThingPassRealAssetReceiptPc34 *out_receipt)
{
    CSB_V1_D0L2D0R2F0115ThingPassRealAssetReceiptPc34 receipt;

    if (out_receipt) {
        *out_receipt =
            (CSB_V1_D0L2D0R2F0115ThingPassRealAssetReceiptPc34){0};
    }
    if (!fixture || !out_receipt || !source_graphics_dat_bound ||
        !no_synthetic_pixels || !no_fallback_visuals ||
        source_graphics_item_index != fixture->wall_frame_row ||
        source_byte_count == 0u || source_payload_hash == 0u ||
        !fixture->f0674_per_frame_bitmap_copy ||
        !fixture->item_projectile_disabled_by_g2028 ||
        fixture->f0115_call_count != 1) {
        return 0;
    }

    receipt =
        (CSB_V1_D0L2D0R2F0115ThingPassRealAssetReceiptPc34){0};
    receipt.valid = CSB_PRESENT;
    receipt.route_backed_by_real_graphics_dat = CSB_PRESENT;
    receipt.side = fixture->side;
    receipt.source_graphics_dat_bound = CSB_PRESENT;
    receipt.no_synthetic_pixels = CSB_PRESENT;
    receipt.no_fallback_visuals = CSB_PRESENT;
    receipt.wall_frame_row = fixture->wall_frame_row;
    receipt.source_graphics_item_index = source_graphics_item_index;
    receipt.source_byte_count = source_byte_count;
    receipt.source_payload_hash = source_payload_hash;
    receipt.viewport_clip_x1 = fixture->viewport_clip_x1;
    receipt.viewport_clip_x2 = fixture->viewport_clip_x2;
    receipt.viewport_clip_y1 = fixture->viewport_clip_y1;
    receipt.viewport_clip_y2 = fixture->viewport_clip_y2;
    receipt.item_projectile_disabled_by_g2028 =
        fixture->item_projectile_disabled_by_g2028;
    receipt.creature_cell_gate = fixture->creature_cell_gate;
    receipt.redmcsb_f0115_anchor = fixture->redmcsb_f0115_anchor;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    const CSB_V1_D0L2D0R2F0115TeleporterSourceRasterPc34 *source,
    unsigned char *viewport,
    size_t viewport_size,
    int viewport_stride,
    CSB_V1_D0L2D0R2F0115TeleporterRenderReceiptPc34 *out_receipt)
{
    CSB_V1_D0L2D0R2F0115TeleporterRenderReceiptPc34 receipt = {0};
    int x;
    int y;

    if (out_receipt) {
        *out_receipt = (CSB_V1_D0L2D0R2F0115TeleporterRenderReceiptPc34){0};
    }
    if (!fixture || !source || !viewport || !out_receipt ||
        !source->source_graphics_dat_bound || !source->no_synthetic_pixels ||
        !source->no_fallback_visuals || !source->pixels ||
        source->source_payload_hash == 0u ||
        source->source_graphics_item_index != fixture->field_aspect_index ||
        source->width != fixture->wall_frame_x2 - fixture->wall_frame_x1 + 1 ||
        source->height != fixture->wall_frame_height ||
        source->stride < source->width ||
        source->pixel_count < (size_t)source->stride * (size_t)source->height ||
        viewport_stride < fixture->viewport_clip_x2 + 1 ||
        viewport_size < (size_t)viewport_stride *
                            (size_t)(fixture->viewport_clip_y2 + 1)) {
        return 0;
    }

    /* F0125/F0126 lay this field over the completed F0115 thing pass.  Both
     * source and destination coordinates are constrained to G0163's D0 row;
     * C10 remains transparent exactly as the native bitmap blit does. */
    for (y = 0; y < source->height; ++y) {
        int dst_y = fixture->viewport_clip_y1 + y;
        for (x = 0; x < source->width; ++x) {
            int dst_x = fixture->wall_frame_x1 + x;
            unsigned char pixel;

            if (!csb_v1_viewport_d0l2_d0r2_f0115_viewport_clip_contains_pc34(
                    fixture, dst_x, dst_y)) {
                ++receipt.clipped_pixels;
                continue;
            }
            pixel = source->pixels[(size_t)y * (size_t)source->stride + (size_t)x];
            if (pixel == CSB_C10_COLOR_FLESH) {
                ++receipt.transparent_pixels;
                continue;
            }
            viewport[(size_t)dst_y * (size_t)viewport_stride + (size_t)dst_x] =
                pixel;
            ++receipt.copied_pixels;
        }
    }

    if (receipt.copied_pixels == 0u) {
        return 0;
    }
    receipt.valid = CSB_PRESENT;
    receipt.side = fixture->side;
    receipt.field_aspect_index = fixture->field_aspect_index;
    receipt.source_graphics_item_index = source->source_graphics_item_index;
    receipt.source_payload_hash = source->source_payload_hash;
    *out_receipt = receipt;
    return 1;
}

static uint32_t csb_v1_viewport_d0_f0115_fnv1a_pc34(
    const unsigned char *bytes,
    size_t byte_count)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!bytes || byte_count == 0u) {
        return 0u;
    }
    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int csb_v1_viewport_d0_f0115_cache_palette_matches_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsDatPaletteSourceReceipt *palette)
{
    CSB_V1_CSBGraphicsEntrySpan current_span;
    unsigned char decoded[CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES];
    size_t written = 0u;

    if (!cache || !palette || !cache->loaded || !cache->file_buffer ||
        cache->file_size == 0u || cache->resolved_path[0] == '\0' ||
        cache->matched_md5[0] == '\0' || !palette->valid ||
        strcmp(palette->source_path, cache->resolved_path) != 0 ||
        strcmp(palette->source_md5, cache->matched_md5) != 0 ||
        palette->entry_span.compressed_size == 0u ||
        palette->entry_span.decompressed_size !=
            CSB_V1_CSBGRAPHICS_DAT_PALETTE_BYTES ||
        palette->decoded_fnv1a != csb_v1_viewport_d0_f0115_fnv1a_pc34(
            palette->decoded_bytes, sizeof(palette->decoded_bytes)) ||
        csb_v1_csbgraphics_dat_entry_span(cache->file_buffer,
                                           cache->file_size,
                                           palette->entry_span.entry_index,
                                           &current_span) !=
            CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        current_span.compressed_size != palette->entry_span.compressed_size ||
        current_span.decompressed_size != palette->entry_span.decompressed_size ||
        csb_v1_csbgraphics_dat_decode_entry(cache->file_buffer,
                                             cache->file_size,
                                             palette->entry_span.entry_index,
                                             decoded,
                                             sizeof(decoded),
                                             &written) !=
            CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        written != sizeof(decoded) ||
        memcmp(decoded, palette->decoded_bytes, sizeof(decoded)) != 0) {
        return 0;
    }
    return 1;
}

int csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_from_cache_pc34(
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *fixture,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_D0L2D0R2F0115CacheTeleporterRequestPc34 *request,
    unsigned char *viewport,
    size_t viewport_size,
    int viewport_stride,
    CSB_V1_D0L2D0R2F0115CacheTeleporterReceiptPc34 *out_receipt)
{
    CSB_V1_CSBGraphicsEntrySpan span;
    CSB_V1_D0L2D0R2F0115TeleporterSourceRasterPc34 source;
    CSB_V1_D0L2D0R2F0115TeleporterRenderReceiptPc34 composed;
    CSB_V1_D0L2D0R2F0115CacheTeleporterReceiptPc34 receipt;
    unsigned char *decoded = NULL;
    size_t expected_size;
    size_t written = 0u;
    uint32_t raster_hash;
    uint32_t palette_hash;
    int rc;
    int drawn;

    if (out_receipt) {
        *out_receipt = (CSB_V1_D0L2D0R2F0115CacheTeleporterReceiptPc34){0};
    }
    if (!fixture || !cache || !request || !out_receipt || !viewport ||
        !csb_v1_viewport_d0_f0115_cache_palette_matches_pc34(
            cache, request->palette_receipt) ||
        fixture->field_aspect_index < 0 || fixture->wall_frame_height <= 0 ||
        fixture->wall_frame_x2 < fixture->wall_frame_x1) {
        return 0;
    }

    expected_size = (size_t)(fixture->wall_frame_x2 - fixture->wall_frame_x1 + 1) *
                    (size_t)fixture->wall_frame_height;
    if (expected_size == 0u || expected_size > 65535u ||
        csb_v1_csbgraphics_dat_entry_span(cache->file_buffer,
                                           cache->file_size,
                                           request->csbgraphics_entry_index,
                                           &span) !=
            CSB_V1_CSBGRAPHICS_CLASSIFY_OK ||
        span.compressed_size == 0u || span.decompressed_size != expected_size) {
        return 0;
    }

    decoded = (unsigned char *)malloc(expected_size);
    if (!decoded) {
        return 0;
    }
    rc = csb_v1_csbgraphics_dat_decode_entry(cache->file_buffer,
                                              cache->file_size,
                                              request->csbgraphics_entry_index,
                                              decoded,
                                              expected_size,
                                              &written);
    if (rc != CSB_V1_CSBGRAPHICS_CLASSIFY_OK || written != expected_size) {
        free(decoded);
        return 0;
    }
    raster_hash = csb_v1_viewport_d0_f0115_fnv1a_pc34(decoded, written);
    palette_hash = csb_v1_viewport_d0_f0115_fnv1a_pc34(
        request->palette_receipt->decoded_bytes,
        sizeof(request->palette_receipt->decoded_bytes));
    if (raster_hash == 0u || palette_hash == 0u) {
        free(decoded);
        return 0;
    }

    source = (CSB_V1_D0L2D0R2F0115TeleporterSourceRasterPc34){
        1,
        1,
        1,
        fixture->field_aspect_index,
        decoded,
        written,
        fixture->wall_frame_x2 - fixture->wall_frame_x1 + 1,
        fixture->wall_frame_height,
        fixture->wall_frame_x2 - fixture->wall_frame_x1 + 1,
        raster_hash
    };
    drawn = csb_v1_viewport_d0l2_d0r2_f0115_render_teleporter_field_pc34(
        fixture, &source, viewport, viewport_size, viewport_stride, &composed);
    free(decoded);
    if (!drawn) {
        return 0;
    }

    receipt.valid = 1;
    receipt.consumed_hash_admitted_csbgraphics_dat = 1;
    receipt.no_synthetic_pixels = 1;
    receipt.no_fallback_visuals = 1;
    receipt.side = fixture->side;
    receipt.field_aspect_index = fixture->field_aspect_index;
    receipt.csbgraphics_entry_index = request->csbgraphics_entry_index;
    receipt.palette_entry_index = request->palette_receipt->entry_span.entry_index;
    receipt.decoded_raster_hash = raster_hash;
    receipt.palette_hash = palette_hash;
    receipt.copied_pixels = composed.copied_pixels;
    receipt.transparent_pixels = composed.transparent_pixels;
    *out_receipt = receipt;
    return 1;
}

const char *csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}
