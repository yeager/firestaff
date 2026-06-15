#include "csb_v1_viewport_d1c_f0111_door_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_VIEW_SQUARE_D1C = 3,
    CSB_VIEW_DEPTH_D1 = 1,
    CSB_VIEW_LANE_CENTER = 0,
    CSB_ELEMENT_DOOR_FRONT = 17,
    CSB_D1C_DOOR_NATIVE_WIDTH = 96,
    CSB_D1C_DOOR_NATIVE_HEIGHT = 88,
    CSB_D1C_DOOR_NATIVE_BYTE_COUNT = 4224,
    CSB_C2_VIEW_DOOR_ORNAMENT_D1LCR = 2,
    CSB_M631_ZONE_DOOR_D1C = 3790,
    CSB_DOORPASS1_ORDER = 0x0218,
    CSB_DOORPASS2_ORDER = 0x0349
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; contract_only=1 and no fixture state. "
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:7873-7911 locks "
    "the D1C C17_ELEMENT_DOOR_FRONT path. DUNVIEW.C:F0124:7905-7908 "
    "draws the D1C F0111 door with "
    "G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, "
    "M075_BITMAP_BYTE_COUNT(96, 88), C2_VIEW_DOOR_ORNAMENT_D1LCR, and "
    "&G0186_s_Graphic558_Frames_Door_D1C, with the PC34 zone form using "
    "M631_ZONE_DOOR_D1C. ReDMCSB DEFS.H:2159 defines "
    "M075_BITMAP_BYTE_COUNT(width, height); DEFS.H:2791 defines "
    "C2_VIEW_DOOR_ORNAMENT_D1LCR; DEFS.H:4259 defines M631_ZONE_DOOR_D1C. "
    "DUNVIEW.C:F0124:7784-7872 is the D1C wall block with F0101/F0100 "
    "before the door case; DUNVIEW.C:F0124:7937 is the terminal D1C "
    "F0115 pass after the door case. DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:"
    "8524-8533 dispatches F0122 D1L, F0123 D1R, then F0124 D1C; the D1C "
    "door contract does not use F0122_DUNGEONVIEW_DrawSquareD1L or "
    "F0123_DUNGEONVIEW_DrawSquareD1R. CSB-lineage Viewport.cpp:"
    "StdDrawF1DoorFacing:1903-1915 binds center F1 door dispatch to "
    "StdDoorGraphicsF1, StdDoorRectsF1, StdDrawDoor, and the same "
    "DrawOrder218/DrawOrder349 object pass split.";

static const CSB_V1_ViewportD1CF0111DoorPc34Contract s_contract = {
    CSB_PRESENT,
    CSB_VIEW_SQUARE_D1C,
    CSB_VIEW_DEPTH_D1,
    CSB_VIEW_LANE_CENTER,
    CSB_ELEMENT_DOOR_FRONT,
    CSB_D1C_DOOR_NATIVE_WIDTH,
    CSB_D1C_DOOR_NATIVE_HEIGHT,
    CSB_D1C_DOOR_NATIVE_BYTE_COUNT,
    CSB_C2_VIEW_DOOR_ORNAMENT_D1LCR,
    CSB_M631_ZONE_DOOR_D1C,
    CSB_DOORPASS1_ORDER,
    CSB_DOORPASS2_ORDER,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_ABSENT,
    CSB_ABSENT,
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:7905-7908",
    "ReDMCSB DEFS.H:M075_BITMAP_BYTE_COUNT:2159; "
    "DEFS.H:C2_VIEW_DOOR_ORNAMENT_D1LCR:2791; "
    "DEFS.H:M631_ZONE_DOOR_D1C:4259",
    "ReDMCSB DUNVIEW.C:F0124_DUNGEONVIEW_DrawSquareD1C:"
    "7784-7872,7873-7911,7937-7937",
    "ReDMCSB DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF:8524-8533",
    "CSB-lineage Viewport.cpp:StdDrawF1DoorFacing:1903-1915",
    "G0695_ai_DoorNativeBitmapIndex_Front_D1LCR",
    "M075_BITMAP_BYTE_COUNT(96, 88)",
    "C2_VIEW_DOOR_ORNAMENT_D1LCR",
    "G0186_s_Graphic558_Frames_Door_D1C",
    s_source_evidence
};

const CSB_V1_ViewportD1CF0111DoorPc34Contract *
csb_v1_viewport_d1c_f0111_door_pc34_contract(void)
{
    return &s_contract;
}

const char *
csb_v1_viewport_d1c_f0111_door_pc34_source_evidence(void)
{
    return s_source_evidence;
}
