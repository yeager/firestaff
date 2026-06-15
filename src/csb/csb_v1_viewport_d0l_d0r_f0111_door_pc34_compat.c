#include "csb_v1_viewport_d0l_d0r_f0111_door_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_ELEMENT_CORRIDOR = 1,       /* ReDMCSB DUNVIEW.C:7999/8103. */
    CSB_ELEMENT_DOOR_SIDE = 16,     /* ReDMCSB DUNVIEW.C:8000/8104. */
    CSB_ELEMENT_TELEPORTER = 5,     /* ReDMCSB DUNVIEW.C:8001/8105. */
    CSB_VIEW_SQUARE_D0L = 1,        /* ReDMCSB DEFS.H:2597 M610_VIEW_SQUARE_D0L. */
    CSB_VIEW_SQUARE_D0R = 2,        /* ReDMCSB DEFS.H:2598 M611_VIEW_SQUARE_D0R. */
    CSB_VIEW_DEPTH_D0 = 0,          /* ReDMCSB DUNVIEW.C:372 G2027[1/2]. */
    CSB_VIEW_LANE_LEFT = -1,        /* ReDMCSB DUNVIEW.C:371 G2026[1]. */
    CSB_VIEW_LANE_RIGHT = 1,        /* ReDMCSB DUNVIEW.C:371 G2026[2]. */
    CSB_D0L_F0115_ORDER = 0x0002,   /* ReDMCSB DUNVIEW.C:8005; DEFS.H:2660. */
    CSB_D0R_F0115_ORDER = 0x0001,   /* ReDMCSB DUNVIEW.C:8115; DEFS.H:2659. */
    CSB_C716_WALL_D0L = 716,        /* ReDMCSB DEFS.H:4056. */
    CSB_C717_WALL_D0R = 717,        /* ReDMCSB DEFS.H:4057. */
    CSB_C870_CEILING_PIT_D0L = 870, /* ReDMCSB DUNVIEW.C:8003; DEFS.H:4217. */
    CSB_C872_CEILING_PIT_D0R = 872  /* ReDMCSB DUNVIEW.C:8113; DEFS.H:4219. */
};

static const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatEvidence s_evidence = {
    "Source-locked contract gate only; no real-asset bitmap parity and no "
    "CSB game-data load. Chosen slice: D0L/D0R F0111 door boundary because "
    "the D0 lateral ring had no dedicated F0111 gate, and source locks it "
    "as an absence/front-clip boundary rather than a positive F0111 draw.",
    "ReDMCSB DUNVIEW.C:7976-8060 F0125_DUNGEONVIEW_DrawSquareD0L",
    "ReDMCSB DUNVIEW.C:8080-8159 F0126_DUNGEONVIEW_DrawSquareD0R",
    "ReDMCSB DUNVIEW.C:3502-3938 "
    "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF",
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor",
    "CSBWin Viewport.cpp:1903-1915 StdDrawF1DoorFacing center-door dispatch",
    "CSB-lineage Viewport.cpp:1930-1944 StdDrawF0L1/F0R1 return",
    "CSB-lineage Viewport.cpp:6503-6551 ApplyDecoration/CustomBackgrounds"
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset bitmap parity and no "
    "CSB game-data load. Chosen slice: D0L/D0R F0111 door boundary because "
    "D0L/D0R were the untested D0 lateral ring. ReDMCSB DUNVIEW.C:7976-8060 "
    "locks D0L: corridor, door-side, and teleporter cases draw the D0L "
    "ceiling pit at 8003 and dispatch only F0115 with C0x0002 at 8005; wall "
    "draws return at 8038; teleporter field uses C716 at 8059. ReDMCSB "
    "DUNVIEW.C:8080-8159 locks D0R with the mirrored F0115 C0x0001 at 8115 "
    "and field zone C717 at 8159. ReDMCSB DUNVIEW.C:3502-3938 F0107 is the "
    "wall-ornament/alcove helper, but F0125/F0126 do not call it. ReDMCSB "
    "DUNVIEW.C:4218-4337 F0111 is the door bitmap routine, but F0125/F0126 "
    "do not dispatch it and no D0L/D0R door-zone constants exist in "
    "DEFS.H:4250-4260. CSBWin Viewport.cpp:1903-1915 is the positive "
    "center-door StdDrawDoor contrast for F1, while CSB-lineage "
    "Viewport.cpp:1930-1944 makes F0L1/F0R1 return-only and "
    "Viewport.cpp:6503-6551 keeps CustomBackgrounds/ApplyDecoration "
    "separate from this D0 F0111 gate.";

static const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant s_invariants[] = {
    {
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_VIEW_SQUARE_D0L,
        CSB_VIEW_DEPTH_D0,
        CSB_VIEW_LANE_LEFT,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_D0L_F0115_ORDER,
        CSB_C716_WALL_D0L,
        CSB_C716_WALL_D0L,
        CSB_C870_CEILING_PIT_D0L,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        "D0L F0111 absence/front-clip boundary",
        "ReDMCSB DUNVIEW.C:7976-8060 F0125 D0L; no F0107/F0111"
    },
    {
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_VIEW_SQUARE_D0R,
        CSB_VIEW_DEPTH_D0,
        CSB_VIEW_LANE_RIGHT,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_D0R_F0115_ORDER,
        CSB_C717_WALL_D0R,
        CSB_C717_WALL_D0R,
        CSB_C872_CEILING_PIT_D0R,
        CSB_ABSENT,
        CSB_ABSENT,
        CSB_PRESENT,
        CSB_PRESENT,
        CSB_PRESENT,
        "D0R F0111 absence/front-clip boundary",
        "ReDMCSB DUNVIEW.C:8080-8159 F0126 D0R; no F0107/F0111"
    }
};

size_t csb_v1_viewport_d0l_d0r_f0111_door_pc34_count(void)
{
    return sizeof(s_invariants) / sizeof(s_invariants[0]);
}

const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *
csb_v1_viewport_d0l_d0r_f0111_door_pc34_at(size_t index)
{
    if (index >= csb_v1_viewport_d0l_d0r_f0111_door_pc34_count()) return NULL;
    return &s_invariants[index];
}

const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *
csb_v1_viewport_d0l_d0r_f0111_door_pc34_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_d0l_d0r_f0111_door_pc34_count(); ++i) {
        if (s_invariants[i].view_square == view_square) return &s_invariants[i];
    }
    return NULL;
}

int csb_v1_viewport_d0l_d0r_f0111_door_allows_f0111_pc34(
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *invariant,
    int element)
{
    if (!invariant) return 0;
    if (element != CSB_ELEMENT_CORRIDOR &&
        element != CSB_ELEMENT_DOOR_SIDE &&
        element != CSB_ELEMENT_TELEPORTER) {
        return 0;
    }
    return invariant->f0111_route_present;
}

const char *csb_v1_viewport_d0l_d0r_f0111_door_route_label_pc34(
    const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatInvariant *invariant)
{
    if (!invariant) return NULL;
    return invariant->route_name;
}

const CSB_V1_ViewportD0LD0RF0111DoorPc34CompatEvidence *
csb_v1_viewport_d0l_d0r_f0111_door_evidence_pc34(void)
{
    return &s_evidence;
}

const char *csb_v1_viewport_d0l_d0r_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
