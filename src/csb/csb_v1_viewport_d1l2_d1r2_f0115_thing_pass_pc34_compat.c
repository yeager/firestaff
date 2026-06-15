#include "csb_v1_viewport_d1l2_d1r2_f0115_thing_pass_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_SIDE_D1L2 = 1,
    CSB_SIDE_D1R2 = 2,
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
