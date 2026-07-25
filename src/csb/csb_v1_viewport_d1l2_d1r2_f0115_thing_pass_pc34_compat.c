#include "csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_SIDE_D1L2 = 1,
    CSB_SIDE_D1R2 = 2,
    CSB_CELL_FRONT_LEFT = 0,      /* ReDMCSB DEFS.H:2642. */
    CSB_CELL_FRONT_RIGHT = 1,     /* ReDMCSB DEFS.H:2643. */
    CSB_CELL_BACK_RIGHT = 2,      /* ReDMCSB DEFS.H:2644. */
    CSB_CELL_BACK_LEFT = 3,       /* ReDMCSB DEFS.H:2645. */
    CSB_OBJECT_CREATURE_SHIFT = 0x8000, /* ReDMCSB DEFS.H:3517. */
    CSB_NATIVE_OBJECT_GRAPHIC_MIN = 498,
    CSB_NATIVE_OBJECT_GRAPHIC_MAX = 583,
    CSB_C3014_ZONE_CENTER_EXPLOSION = 3014, /* ReDMCSB DEFS.H:4234. */
    CSB_C3031_ZONE_SIDE_EXPLOSION = 3031,   /* ReDMCSB DEFS.H:4235. */
    CSB_VIEW_SQUARE_D1L = 4,       /* ReDMCSB DEFS.H:2600 M607_VIEW_SQUARE_D1L. */
    CSB_VIEW_SQUARE_D1R = 5,       /* ReDMCSB DEFS.H:2601 M608_VIEW_SQUARE_D1R. */
    CSB_VIEW_DEPTH_D1 = 1,         /* ReDMCSB DUNVIEW.C:372 G2027[4/5]. */
    CSB_VIEW_LANE_D1L = -1,        /* ReDMCSB DUNVIEW.C:371 G2026[4]. */
    CSB_VIEW_LANE_D1R = 1,         /* ReDMCSB DUNVIEW.C:371 G2026[5]. */
    CSB_G2028_D1L = 9,             /* ReDMCSB DUNVIEW.C:373 G2028[4]. */
    CSB_G2028_D1R = 10,            /* ReDMCSB DUNVIEW.C:373 G2028[5]. */
    CSB_G2033_D1L = 9,             /* ReDMCSB DUNVIEW.C:375 G2033[4]. */
    CSB_G2033_D1R = 10,            /* ReDMCSB DUNVIEW.C:375 G2033[5]. */
    CSB_G2034_D1L = 12,            /* ReDMCSB DUNVIEW.C:376 G2034[4]. */
    CSB_G2034_D1R = 13,            /* ReDMCSB DUNVIEW.C:376 G2034[5]. */
    CSB_FIELD_D1L = 11,            /* ReDMCSB DUNVIEW.C:377 G2035[4]. */
    CSB_FIELD_D1R = 12,            /* ReDMCSB DUNVIEW.C:377 G2035[5]. */
    CSB_D1L_ORDER = 0x0032,        /* ReDMCSB DUNVIEW.C:7523/7536. */
    CSB_D1R_ORDER = 0x0041,        /* ReDMCSB DUNVIEW.C:7691/7704. */
    CSB_C10_COLOR_FLESH = 10,      /* ReDMCSB DEFS.H:2088 C10_COLOR_FLESH. */
    CSB_C2500_ZONE_ITEM = 2500,    /* ReDMCSB DEFS.H:4228; DUNVIEW.C:5075. */
    CSB_C2900_ZONE_PROJECTILE = 2900, /* ReDMCSB DEFS.H:4230; DUNVIEW.C:5683. */
    CSB_C3200_ZONE_CREATURE = 3200,   /* ReDMCSB DUNVIEW.C:5615-5617. */
    CSB_C3000_ZONE_EXPLOSION = 3000,  /* ReDMCSB DUNVIEW.C:5998-5999. */
    CSB_CSBWIN_RF1L1 = 15,         /* CSBWin/CSB-lineage Viewport.cpp:340. */
    CSB_CSBWIN_RF1R1 = 16,         /* CSBWin/CSB-lineage Viewport.cpp:342. */
    CSB_CSBWIN_STD_ROOM_OBJECTS = 60006, /* CSBWin/CSB-lineage Viewport.cpp:379. */
    CSB_CSBWIN_F1L1_CONTENTS = 60125,    /* CSBWin/CSB-lineage Viewport.cpp:511. */
    CSB_CSBWIN_F1R1_CONTENTS = 60127,    /* CSBWin/CSB-lineage Viewport.cpp:513. */
    CSB_CSBWIN_DRAWORDER32 = 60272,      /* CSBWin/CSB-lineage Viewport.cpp:676. */
    CSB_CSBWIN_DRAWORDER41 = 60275       /* CSBWin/CSB-lineage Viewport.cpp:679. */
};

static int s_initialized;

static const CSB_V1_ViewportD1L2D1R2F0115ThingPassEvidencePc34 s_evidence = {
    "Source-locked contract-only gate; synthetic fixture only, no real-asset "
    "bitmap parity and no CSB game-data load.",
    "ReDMCSB DUNVIEW.C:7391-7557 F0122_DUNGEONVIEW_DrawSquareD1L",
    "ReDMCSB DUNVIEW.C:7559-7725 F0123_DUNGEONVIEW_DrawSquareD1R",
    "ReDMCSB DUNVIEW.C:4547-4581,4806-4811,4923,5075,5201-5214,"
    "5615-5617,5668-5683,5916-5923,5998-5999 F0115",
    "ReDMCSB DEFS.H:2088,2596-2601,4228-4230,4250-4260",
    "CSBWin Viewport.cpp:1167-1188 F1L1/F1R1 Open room-object bindings",
    "CSB-lineage Viewport.cpp:1167-1188 F1L1/F1R1 Open room-object bindings",
    "CSB-lineage Viewport.cpp:6503-6551 ApplyDecoration/CustomBackgrounds"
};

static const char s_source_evidence[] =
    "Source-locked contract-only gate; no real-asset bitmap parity and no "
    "CSB game-data load. ReDMCSB DUNVIEW.C:6789-6793 selects F0122 for the "
    "view-depth-2 left branch and 6773-6777 selects F0123 for the mirrored "
    "right branch. DUNVIEW.C:7391-7557 F0122 locks the D1L corridor path: "
    "pit/teleporter/corridor fall through 7520-7525, F0108 and F0112 precede "
    "the terminal F0115 call at 7536 with M607_VIEW_SQUARE_D1L and order "
    "0x0032; wall returns through F0107 at 7436-7460 and door-front F0111 is "
    "a separate 7492-7508 path. DUNVIEW.C:7559-7725 F0123 mirrors this for "
    "D1R with order 0x0041 at 7691/7704 and keeps F0107/F0111 on wall/"
    "door-front paths. DUNVIEW.C:4547-4581 defines the F0115 per-cell thing "
    "draw order. DUNVIEW.C:4806-4811 loads G2026/G2027/G2028 rows; 4923 and "
    "5075 bind C2500 item cells; 5201-5214 and 5615-5617 bind C3200 creature "
    "cells; 5668-5683 bind C2900 projectile cells; 5916-5923 and 5998-5999 "
    "bind C3000 explosion cells. DEFS.H:2088 anchors C10_COLOR_FLESH; "
    "DEFS.H:2596-2601 anchors D1 view-square indices; DEFS.H:4228-4230 "
    "anchors C2500/C2900 and the F0115 zone family; DEFS.H:4250-4260 keeps "
    "door-zone metadata out of this corridor thing-pass slice. CSBWin "
    "Viewport.cpp and CSB-lineage Viewport.cpp:1167-1188 bind F1L1/F1R1 "
    "Open through StdDrawRoomObjects with DrawOrder32/DrawOrder41; "
    "CSB-lineage Viewport.cpp:6503-6551 keeps ApplyDecoration/"
    "CustomBackgrounds out of this D1 corridor thing-pass gate.";

static const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 s_fixtures[] = {
    {
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_SIDE_D1L2,
        1,
        1,
        CSB_PRESENT,
        CSB_C10_COLOR_FLESH,
        CSB_ABSENT,
        CSB_PRESENT,
        4,
        CSB_C2500_ZONE_ITEM,
        CSB_C2900_ZONE_PROJECTILE,
        CSB_C3200_ZONE_CREATURE,
        CSB_C3000_ZONE_EXPLOSION,
        "C2500 item / C2900 projectile / C3200 creature / C3000 explosion",
        CSB_VIEW_SQUARE_D1L,
        CSB_VIEW_DEPTH_D1,
        CSB_VIEW_LANE_D1L,
        CSB_G2028_D1L,
        CSB_G2033_D1L,
        CSB_G2034_D1L,
        CSB_FIELD_D1L,
        CSB_D1L_ORDER,
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
        CSB_CSBWIN_RF1L1,
        CSB_CSBWIN_F1L1_CONTENTS,
        CSB_CSBWIN_DRAWORDER32,
        CSB_CSBWIN_STD_ROOM_OBJECTS,
        "D1L2 requested slice mapped to M607 D1L corridor thing-pass",
        "ReDMCSB DUNVIEW.C:7520-7536 F0122 corridor F0115; no F0107/F0111"
    },
    {
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_SIDE_D1R2,
        1,
        1,
        CSB_PRESENT,
        CSB_C10_COLOR_FLESH,
        CSB_ABSENT,
        CSB_PRESENT,
        4,
        CSB_C2500_ZONE_ITEM,
        CSB_C2900_ZONE_PROJECTILE,
        CSB_C3200_ZONE_CREATURE,
        CSB_C3000_ZONE_EXPLOSION,
        "C2500 item / C2900 projectile / C3200 creature / C3000 explosion",
        CSB_VIEW_SQUARE_D1R,
        CSB_VIEW_DEPTH_D1,
        CSB_VIEW_LANE_D1R,
        CSB_G2028_D1R,
        CSB_G2033_D1R,
        CSB_G2034_D1R,
        CSB_FIELD_D1R,
        CSB_D1R_ORDER,
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
        CSB_CSBWIN_RF1R1,
        CSB_CSBWIN_F1R1_CONTENTS,
        CSB_CSBWIN_DRAWORDER41,
        CSB_CSBWIN_STD_ROOM_OBJECTS,
        "D1R2 requested slice mapped to M608 D1R corridor thing-pass",
        "ReDMCSB DUNVIEW.C:7688-7704 F0123 corridor F0115; no F0107/F0111"
    }
};

int csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34(void)
{
    s_initialized = CSB_PRESENT;
    return s_initialized;
}

size_t csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34(void)
{
    return sizeof(s_fixtures) / sizeof(s_fixtures[0]);
}

const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *
csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_at_pc34(size_t index)
{
    if (!s_initialized) csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34();
    if (index >= csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34()) {
        return 0;
    }
    return &s_fixtures[index];
}

const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *
csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_for_square_pc34(int side)
{
    if (!s_initialized) csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_init_pc34();
    for (size_t i = 0;
         i < csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_count_pc34();
         ++i) {
        if (s_fixtures[i].side == side) return &s_fixtures[i];
    }
    return 0;
}

const CSB_V1_ViewportD1L2D1R2F0115ThingPassEvidencePc34 *
csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_evidence_pc34(void)
{
    return &s_evidence;
}

const char *csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_source_evidence_pc34(void)
{
    return s_source_evidence;
}

int csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_real_asset_receipt_pc34(
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixture,
    int source_graphics_dat_bound,
    int native_object_family_bound,
    int no_synthetic_pixels,
    int no_fallback_visuals,
    int source_graphics_item_index,
    size_t source_byte_count,
    uint32_t source_payload_hash,
    CSB_V1_ViewportD1L2D1R2F0115ThingPassRealAssetReceiptPc34 *out_receipt)
{
    CSB_V1_ViewportD1L2D1R2F0115ThingPassRealAssetReceiptPc34 receipt;

    if (out_receipt) {
        *out_receipt =
            (CSB_V1_ViewportD1L2D1R2F0115ThingPassRealAssetReceiptPc34){0};
    }
    if (!fixture || !out_receipt || !source_graphics_dat_bound ||
        !native_object_family_bound || !no_synthetic_pixels ||
        !no_fallback_visuals ||
        source_graphics_item_index < CSB_NATIVE_OBJECT_GRAPHIC_MIN ||
        source_graphics_item_index > CSB_NATIVE_OBJECT_GRAPHIC_MAX ||
        source_byte_count == 0u || source_payload_hash == 0u ||
        fixture->f0115_call_count != 1 ||
        !fixture->c10_transparency_flag ||
        !fixture->f0115_draw_order_objects_first ||
        !fixture->f0115_draw_order_creatures_second ||
        !fixture->f0115_draw_order_projectiles_third ||
        !fixture->f0115_draw_order_explosions_last ||
        fixture->object_g2028_row < 0) {
        return 0;
    }

    receipt =
        (CSB_V1_ViewportD1L2D1R2F0115ThingPassRealAssetReceiptPc34){0};
    receipt.valid = CSB_PRESENT;
    receipt.route_backed_by_real_graphics_dat = CSB_PRESENT;
    receipt.side = fixture->side;
    receipt.source_graphics_dat_bound = CSB_PRESENT;
    receipt.native_object_family_bound = CSB_PRESENT;
    receipt.no_synthetic_pixels = CSB_PRESENT;
    receipt.no_fallback_visuals = CSB_PRESENT;
    receipt.source_graphics_item_index = source_graphics_item_index;
    receipt.source_byte_count = source_byte_count;
    receipt.source_payload_hash = source_payload_hash;
    receipt.native_object_graphic_min = CSB_NATIVE_OBJECT_GRAPHIC_MIN;
    receipt.native_object_graphic_max = CSB_NATIVE_OBJECT_GRAPHIC_MAX;
    receipt.view_square_index = fixture->view_square_index;
    receipt.view_depth = fixture->view_depth;
    receipt.object_g2028_row = fixture->object_g2028_row;
    receipt.creature_g2033_row = fixture->creature_g2033_row;
    receipt.explosion_g2034_row = fixture->explosion_g2034_row;
    receipt.item_zone_base = fixture->item_zone_base;
    receipt.projectile_zone_base = fixture->projectile_zone_base;
    receipt.creature_zone_base = fixture->creature_zone_base;
    receipt.explosion_zone_base = fixture->explosion_zone_base;
    receipt.c10_transparency = fixture->transparent_color;
    receipt.f0115_cell_order = fixture->f0115_cell_order;
    receipt.route_name = fixture->route_name;
    receipt.source_lines = fixture->source_lines;
    *out_receipt = receipt;
    return 1;
}

int csb_v1_viewport_d1l2_d1r2_f0115_item_zone_pc34(
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture) return -1;
    /* ReDMCSB DUNVIEW.C:4923 gates items by G2028 row >= 0 and cell match;
     * 5075 computes C2500 + row*4 + ViewCell with MASK0x8000. */
    if (fixture->object_g2028_row < 0) return -1;
    if (view_cell < CSB_CELL_FRONT_LEFT || view_cell > CSB_CELL_BACK_LEFT)
        return -1;
    return (CSB_C2500_ZONE_ITEM +
            fixture->object_g2028_row * 4 + view_cell) |
           CSB_OBJECT_CREATURE_SHIFT;
}

int csb_v1_viewport_d1l2_d1r2_f0115_projectile_zone_pc34(
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture) return -1;
    /* ReDMCSB DUNVIEW.C:5668-5683 gates projectiles by G2028 row >= 0
     * and cell match; zone = C2900 + row*4 + ViewCell. */
    if (fixture->object_g2028_row < 0) return -1;
    if (view_cell < CSB_CELL_FRONT_LEFT || view_cell > CSB_CELL_BACK_LEFT)
        return -1;
    return CSB_C2900_ZONE_PROJECTILE +
           fixture->object_g2028_row * 4 + view_cell;
}

int csb_v1_viewport_d1l2_d1r2_f0115_creature_zone_pc34(
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture) return -1;
    /* ReDMCSB DUNVIEW.C:5201-5214,5615-5617 gate creatures by G2033 row
     * and cell; zone = C3200 + row*5 + ViewCell with MASK0x8000. */
    if (fixture->creature_g2033_row < 0) return -1;
    if (view_cell < CSB_CELL_FRONT_LEFT || view_cell > CSB_CELL_BACK_LEFT)
        return -1;
    return (CSB_C3200_ZONE_CREATURE +
            fixture->creature_g2033_row * 5 + view_cell) |
           CSB_OBJECT_CREATURE_SHIFT;
}

int csb_v1_viewport_d1l2_d1r2_f0115_centered_explosion_zone_pc34(
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixture)
{
    if (!fixture) return -1;
    /* ReDMCSB DUNVIEW.C:6107 C3014 + G2034 row. */
    if (fixture->explosion_g2034_row < 0) return -1;
    return CSB_C3014_ZONE_CENTER_EXPLOSION + fixture->explosion_g2034_row;
}

int csb_v1_viewport_d1l2_d1r2_f0115_side_explosion_zone_pc34(
    const CSB_V1_ViewportD1L2D1R2F0115ThingPassPc34 *fixture,
    int view_cell)
{
    if (!fixture) return -1;
    /* ReDMCSB DUNVIEW.C:6121-6122 C3031 + G2034_row*2 + ViewCell;
     * only FRONT_LEFT and FRONT_RIGHT cells receive side explosions. */
    if (fixture->explosion_g2034_row < 0) return -1;
    if (view_cell != CSB_CELL_FRONT_LEFT && view_cell != CSB_CELL_FRONT_RIGHT)
        return -1;
    return CSB_C3031_ZONE_SIDE_EXPLOSION +
           fixture->explosion_g2034_row * 2 + view_cell;
}
