#include "csb_v1_viewport_d2c_f0111_door_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_VIEW_SQUARE_D2C = 6,        /* ReDMCSB: DEFS.H:2602 M603_VIEW_SQUARE_D2C. */
    CSB_VIEW_DEPTH_D2 = 2,          /* ReDMCSB: DUNVIEW.C:372 G2027[6]. */
    CSB_VIEW_LANE_CENTER = 0,       /* ReDMCSB: DUNVIEW.C:371 G2026[6]. */
    CSB_ELEMENT_DOOR_FRONT = 17,    /* ReDMCSB: DEFS.H:1015 C17_ELEMENT_DOOR_FRONT. */
    CSB_D2C_DOOR_WIDTH = 64,        /* ReDMCSB: DUNVIEW.C:7336 M075_BITMAP_BYTE_COUNT(64, 61). */
    CSB_D2C_DOOR_HEIGHT = 61,       /* ReDMCSB: DUNVIEW.C:7336 M075_BITMAP_BYTE_COUNT(64, 61). */
    CSB_D1_DOOR_BYTE_COUNT = 4224,  /* ReDMCSB: DUNVIEW.C:7905 D1C 96x88 guard. */
    CSB_C1_VIEW_DOOR_ORNAMENT_D2LCR = 1, /* ReDMCSB: DEFS.H:2790. */
    CSB_C2_VIEW_DOOR_BUTTON_D2C = 2,     /* ReDMCSB: DEFS.H:2796. */
    CSB_M628_ZONE_DOOR_D2C = 3760,       /* ReDMCSB: DEFS.H:4256 PC34 form. */
    CSB_DOOR_GRAPHICS_F2 = 1,            /* CSB-lineage Viewport.cpp:2568-2571. */
    CSB_DOORPASS1_ORDER = 0x0218,        /* ReDMCSB: DUNVIEW.C:7315. */
    CSB_DOORPASS2_ORDER = 0x0349,        /* ReDMCSB: DUNVIEW.C:7341. */
    CSB_C730_TOP_TRACK_D2C = 730,        /* ReDMCSB: DUNVIEW.C:7330 / DEFS.H. */
    CSB_C724_FRAME_LEFT_D2C = 724,       /* ReDMCSB: DUNVIEW.C:7331 / DEFS.H. */
    CSB_C725_FRAME_RIGHT_D2C = 725,      /* ReDMCSB: DUNVIEW.C:7332 / DEFS.H. */
    CSB_DOOR_STATE_OPEN = 0,             /* ReDMCSB: DEFS.H:1039 C0_DOOR_STATE_OPEN. */
    CSB_DOOR_STATE_CLOSED = 4,           /* ReDMCSB: DEFS.H:1043 C4_DOOR_STATE_CLOSED. */
    CSB_DOOR_STATE_DESTROYED = 5,        /* ReDMCSB: DEFS.H:1044 C5_DOOR_STATE_DESTROYED. */
    CSB_C15_DESTROYED_MASK = 15,         /* ReDMCSB: DEFS.H:2466. */
    CSB_C6_UNKNOWN = 6,                  /* ReDMCSB: DEFS.H:3508. */
    CSB_MASK_4000_SHIFT = 0x4000,        /* ReDMCSB: DEFS.H:3516. */
    CSB_C10_COLOR_FLESH = 10             /* ReDMCSB: DEFS.H:2088. */
};

static const char s_source_evidence[] =
    "Source-locked contract gate only; no real-asset bitmap parity and no "
    "CSB game-data load. ReDMCSB DUNVIEW.C:7244-7389 "
    "F0121_DUNGEONVIEW_DrawSquareD2C locks the center D2C route. "
    "DUNVIEW.C:7313-7341 is the C17_ELEMENT_DOOR_FRONT branch: F0108 "
    "floor ornament, F0115 order 0x0218, D2C top/left/right door frames, "
    "optional C2_VIEW_DOOR_BUTTON_D2C, F0111_DUNGEONVIEW_DrawDoor with "
    "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, "
    "M075_BITMAP_BYTE_COUNT(64, 61), C1_VIEW_DOOR_ORNAMENT_D2LCR, "
    "and G0183_s_Graphic558_Frames_Door_D2C, then F0115 order 0x0349. "
    "DUNVIEW.C:4218-4337 F0111 skips open doors, applies the C15 "
    "destroyed mask, uses C10_COLOR_FLESH transparency, and shifts partial "
    "horizontal doors with C6_UNKNOWN and MASK0x4000. DEFS.H:2159 anchors "
    "M075_BITMAP_BYTE_COUNT, DEFS.H:2790 anchors C1_VIEW_DOOR_ORNAMENT_D2LCR, "
    "and DEFS.H:4256 anchors M628_ZONE_DOOR_D2C. DUNVIEW.C:8508-8533 "
    "F0128 dispatches D2L, D2R, then D2C before D1L/D1R/D1C; this gate "
    "does not use F0119_DUNGEONVIEW_DrawSquareD2L, "
    "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF, or "
    "F0124_DUNGEONVIEW_DrawSquareD1C. CSB-lineage Viewport.cpp:1903-1915 "
    "is the requested center-door dispatch anchor; local "
    "StdDrawF2DoorFacing at Viewport.cpp:1865-1879 binds F2, DrawOrder218, "
    "StdDoorFacingTopTrackBitmapF2, left/right frame blits, DoorSwitch, "
    "F2DoorRecordIndex/F2DoorState, StdDoorGraphicsF2, StdDoorRectsF2, "
    "StdDrawDoor, and DrawOrder349. ReDMCSB DUNGEON.C:F0163/F0164 "
    "lines 1769-1840 are thing-list link/unlink anchors and are explicitly "
    "non-interfering for this viewport classification gate.";

static const CSB_V1_ViewportD2CF0111DoorPc34Contract s_contract = {
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_VIEW_SQUARE_D2C,
    CSB_VIEW_DEPTH_D2,
    CSB_VIEW_LANE_CENTER,
    CSB_ELEMENT_DOOR_FRONT,
    CSB_D2C_DOOR_WIDTH,
    CSB_D2C_DOOR_HEIGHT,
    ((CSB_D2C_DOOR_WIDTH >> 1) * CSB_D2C_DOOR_HEIGHT),
    (((CSB_D2C_DOOR_WIDTH >> 1) * CSB_D2C_DOOR_HEIGHT) !=
     CSB_D1_DOOR_BYTE_COUNT),
    CSB_C1_VIEW_DOOR_ORNAMENT_D2LCR,
    CSB_C2_VIEW_DOOR_BUTTON_D2C,
    CSB_M628_ZONE_DOOR_D2C,
    CSB_DOOR_GRAPHICS_F2,
    CSB_DOORPASS1_ORDER,
    CSB_DOORPASS2_ORDER,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_C730_TOP_TRACK_D2C,
    CSB_C724_FRAME_LEFT_D2C,
    CSB_C725_FRAME_RIGHT_D2C,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_C15_DESTROYED_MASK,
    CSB_PRESENT,
    CSB_MASK_4000_SHIFT,
    CSB_C10_COLOR_FLESH,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_ABSENT,
    CSB_ABSENT,
    CSB_ABSENT,
    CSB_PRESENT,
    CSB_PRESENT,
    "ReDMCSB DUNVIEW.C:4218-4337 F0111_DUNGEONVIEW_DrawDoor",
    "ReDMCSB DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C; "
    "door branch 7313-7341",
    "ReDMCSB DEFS.H:2159,2790,2796,3508,3516,4256; "
    "DUNVIEW.C:7330-7339",
    "ReDMCSB DUNVIEW.C:8508-8533 F0128_DUNGEONVIEW_Draw_CPSF",
    "ReDMCSB DUNGEON.C:F0163/F0164:1769-1840",
    "CSB-lineage Viewport.cpp:1903-1915 requested; "
    "StdDrawF2DoorFacing local lines 1865-1879",
    "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR",
    "M075_BITMAP_BYTE_COUNT(64, 61)",
    "C1_VIEW_DOOR_ORNAMENT_D2LCR",
    "G0183_s_Graphic558_Frames_Door_D2C",
    "M628_ZONE_DOOR_D2C",
    s_source_evidence
};

const CSB_V1_ViewportD2CF0111DoorPc34Contract *
csb_v1_viewport_d2c_f0111_door_pc34_contract(void)
{
    return &s_contract;
}

int csb_v1_viewport_d2c_f0111_door_byte_count_pc34(int width, int height)
{
    if (width <= 0 || height <= 0) return -1;
    return (width >> 1) * height;
}

int csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *contract,
    int door_state)
{
    if (!contract) return -1;
    if (door_state == CSB_DOOR_STATE_OPEN) return -1;
    if (door_state < 0 || door_state > CSB_DOOR_STATE_DESTROYED) return -1;
    if (door_state == CSB_DOOR_STATE_CLOSED ||
        door_state == CSB_DOOR_STATE_DESTROYED) {
        return contract->door_zone_d2c;
    }
    return contract->door_zone_d2c + door_state;
}

int csb_v1_viewport_d2c_f0111_door_horizontal_half_zone_pc34(
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *contract,
    int door_state,
    int right_half)
{
    const int shifted =
        csb_v1_viewport_d2c_f0111_door_zone_for_state_pc34(contract, door_state);
    if (shifted < 0 || door_state >= CSB_DOOR_STATE_CLOSED) return -1;
    if (!right_half) return shifted + CSB_C6_UNKNOWN;
    return shifted + (3 | contract->horizontal_second_half_mask);
}

int csb_v1_viewport_d2c_f0111_door_apply_c10_blit_pc34(
    const CSB_V1_ViewportD2CF0111DoorPc34Contract *contract,
    const uint8_t *source,
    int source_stride,
    uint8_t *destination,
    int destination_stride,
    int width,
    int height)
{
    int copied = 0;

    if (!contract || !source || !destination) return -1;
    if (width <= 0 || height <= 0) return -1;
    if (source_stride < width || destination_stride < width) return -1;

    /* ReDMCSB: DUNVIEW.C F0111 lines 4322-4334 and DEFS.H line 2088
     * keep C10_COLOR_FLESH transparent when drawing the D2C door bitmap. */
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)contract->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const char *csb_v1_viewport_d2c_f0111_door_source_evidence_pc34(void)
{
    return s_source_evidence;
}
