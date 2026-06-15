#include "csb_v1_viewport_d2c_f0111_door_front_pc34_compat.h"

enum {
    CSB_PRESENT = 1,
    CSB_VIEW_SQUARE_D2C = 6,
    CSB_DEPTH_D2 = 2,
    CSB_LATERAL_CENTER = 0,
    CSB_F0128_D2C_DISPATCH_LINE = 8521,
    CSB_F0121_FUNCTION_ID = 121,
    CSB_ELEMENT_DOOR_FRONT = 17,
    CSB_FLOOR_VIEW_D2C = 6,
    CSB_DOORPASS1_BACKLEFT_BACKRIGHT = 0x0218,
    CSB_DOORPASS2_FRONTLEFT_FRONTRIGHT = 0x0349,
    CSB_DOOR_FRONT_BITMAP_D2LCR = 694,
    CSB_DOOR_ORNAMENT_D2LCR = 1,
    CSB_WALL_ZONE_D2C = 709,
    CSB_DOOR_ZONE_D2C = 3760,
    CSB_D2C_DOOR_WIDTH = 64,
    CSB_D2C_DOOR_HEIGHT = 61,
    CSB_C10_COLOR_FLESH = 10,
    CSB_DOORPASS1_MARKER = 8,
    CSB_DOORPASS2_MARKER = 9
};

/*
 * ReDMCSB: DUNVIEW.C F0121 lines 7313-7341 is the D2C C17 door-front
 * composition: F0108, F0115(0x0218), frames/button, F0111, then
 * F0115(0x0349). DUNVIEW.C F0111 lines 4218-4339 supplies open/closed/
 * destroyed/partly-open door behavior and line 4334 passes C10. DUNVIEW.C
 * F0115 lines 4547-4581 documents door-front marker nibbles and cell order.
 * DUNGEON.C F0172 lines 2466-2523 supplies square aspect; F0163 lines
 * 1769-1838 and F0164 lines 1840-1905 are mutation anchors not called by
 * this draw contract. DRAWVIEW.C F0097 lines 709-722 hands the prepared
 * viewport bitmap to the screen. DEFS.H lines 2088, 2533-2559, 2602,
 * 2657-2677, 2756, 2790, 4049, and 4256 define the C10/square/aspect/cell/
 * floor/door-zone fields. CSB-lineage Viewport.cpp lines 1865-1879 and
 * 1903-1915 mirror F2/F1 door-facing helper order; 1192-1209 is the open
 * room-object contrast.
 */
static const char s_source_evidence[] =
    "pass705 CSB V1 D2C F0111 door-front composition source-lock; "
    "contract-only, asset-free, and no CSB game-data load. ReDMCSB "
    "DUNVIEW.C:7244-7389 F0121_DUNGEONVIEW_DrawSquareD2C reaches the "
    "single D2C C17_ELEMENT_DOOR_FRONT branch at 7313-7341: line 7314 "
    "calls F0108 floor ornament with M558_FLOOR_ORNAMENT_ORDINAL and "
    "M592_VIEW_FLOOR_D2C, line 7315 calls F0115 with M603_VIEW_SQUARE_D2C "
    "and C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT, lines 7317-7333 "
    "draw top/left/right frames and optional C2_VIEW_DOOR_BUTTON_D2C, "
    "lines 7336/7339 call F0111_DUNGEONVIEW_DrawDoor with "
    "G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_"
    "D2LCR, G0183_s_Graphic558_Frames_Door_D2C, and M628_ZONE_DOOR_D2C, "
    "then lines 7341/7368 call F0115 again with "
    "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT. ReDMCSB "
    "DUNVIEW.C:4218-4339 F0111 skips open doors at 4248-4253, copies the "
    "native bitmap at 4255-4262, draws closed/destroyed doors at 4297-4305, "
    "handles partly-open state at 4307-4325, and line 4334 draws with "
    "C10_COLOR_FLESH transparency. ReDMCSB DUNVIEW.C:4547-4581 F0115 "
    "defines door-front marker nibbles and cell order processing. "
    "ReDMCSB DUNVIEW.C:8508-8533 F0128 dispatches the D2C square at line "
    "8521 after D2 side lanes and before D1/D0 lanes. ReDMCSB "
    "DUNGEON.C:1769-1838 F0163 and 1840-1905 F0164 are thing-list mutation "
    "anchors that this draw-only contract must not call; DUNGEON.C:2466-"
    "2523 F0172 supplies M550/M556/M557/M558 square-aspect input. "
    "ReDMCSB DRAWVIEW.C:709-722 F0097 is the viewport-bitmap handoff after "
    "the off-screen D2C composition. ReDMCSB DEFS.H:2088 anchors "
    "C10_COLOR_FLESH; DEFS.H:2533-2559 anchors square-aspect fields; "
    "DEFS.H:2602 anchors M603_VIEW_SQUARE_D2C; DEFS.H:2657-2677 anchors "
    "door-front cell orders; DEFS.H:2756 anchors M592_VIEW_FLOOR_D2C; "
    "DEFS.H:2790 anchors C1_VIEW_DOOR_ORNAMENT_D2LCR; DEFS.H:4049 anchors "
    "C709_ZONE_WALL_D2C; DEFS.H:4256 anchors M628_ZONE_DOOR_D2C. "
    "CSB-lineage Viewport.cpp:1865-1879 mirrors the F2 door-facing helper "
    "order with DrawOrder218 before StdDrawDoor and DrawOrder349 after it; "
    "Viewport.cpp:1903-1915 is the requested F1 comparison anchor and "
    "Viewport.cpp:1192-1209 is the open-room contrast. This is a single "
    "D2C F0108 -> F0115 rear -> F0111 -> F0115 front pixel-composition "
    "gate, distinct from pass665 D0C and pass703 D0L2/D0R2 surfaces.";

static const CSB_V1_D2CF0111DoorFrontSpecPc34 s_spec = {
    "CSB_V1_D2C_F0111_DOOR_FRONT_COMPOSITION_PC34",
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_VIEW_SQUARE_D2C,
    CSB_DEPTH_D2,
    CSB_LATERAL_CENTER,
    CSB_F0128_D2C_DISPATCH_LINE,
    CSB_F0121_FUNCTION_ID,
    CSB_ELEMENT_DOOR_FRONT,
    CSB_FLOOR_VIEW_D2C,
    CSB_DOORPASS1_BACKLEFT_BACKRIGHT,
    CSB_DOORPASS2_FRONTLEFT_FRONTRIGHT,
    CSB_DOOR_FRONT_BITMAP_D2LCR,
    CSB_DOOR_ORNAMENT_D2LCR,
    CSB_WALL_ZONE_D2C,
    CSB_DOOR_ZONE_D2C,
    CSB_D2C_DOOR_WIDTH,
    CSB_D2C_DOOR_HEIGHT,
    CSB_C10_COLOR_FLESH,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_DOORPASS1_MARKER,
    CSB_DOORPASS2_MARKER,
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
    "ReDMCSB DUNVIEW.C:7244-7389 F0121; 7313-7341 D2C door branch; "
        "F0111 4218-4339; F0115 4547-4581; F0128 8508-8533",
    "ReDMCSB DUNGEON.C:1769-1838 F0163; 1840-1905 F0164; "
        "2466-2523 F0172",
    "ReDMCSB DRAWVIEW.C:709-722 F0097_DUNGEONVIEW_DrawViewport",
    "ReDMCSB DEFS.H:2088,2533-2559,2602,2657-2677,2756,2790,4049,4256",
    "CSB-lineage Viewport.cpp:1865-1879,1903-1915,1192-1209",
    s_source_evidence
};

size_t csb_v1_viewport_d2c_f0111_door_front_spec_count_pc34(void)
{
    return 1u;
}

const CSB_V1_D2CF0111DoorFrontSpecPc34 *
csb_v1_viewport_d2c_f0111_door_front_spec_pc34(void)
{
    return &s_spec;
}

const CSB_V1_D2CF0111DoorFrontSpecPc34 *
csb_v1_viewport_d2c_f0111_door_front_spec_at_pc34(size_t index)
{
    if (index != 0u) return 0;
    return &s_spec;
}

int csb_v1_viewport_d2c_f0111_door_front_decode_cell_pc34(
    unsigned int order,
    int ordinal)
{
    unsigned int shift;
    unsigned int cell;

    /*
     * ReDMCSB: DUNVIEW.C F0115 lines 4561-4564 and DEFS.H lines
     * 2657-2677 define low-nibble door markers 8/9 plus one-based cell
     * ordinals. Zero terminates and is not a visible cell.
     */
    if (ordinal < 0 || ordinal > 3) return -1;
    shift = (unsigned int)ordinal * 4u;
    cell = (order >> shift) & 0x0fu;
    if (cell == 0u || cell == 8u || cell == 9u) return -1;
    return (int)cell - 1;
}

uint8_t csb_v1_viewport_d2c_f0111_door_front_blend_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel)
{
    /*
     * ReDMCSB: DUNVIEW.C F0111 line 4334 and DEFS.H line 2088 make
     * C10_COLOR_FLESH transparent for the final PC34 D2C door-front blit.
     */
    return source_pixel == CSB_C10_COLOR_FLESH ? destination_pixel : source_pixel;
}

int csb_v1_viewport_d2c_f0111_door_front_compose_pixel_pc34(
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *spec,
    uint8_t base_pixel,
    uint8_t floor_pixel,
    uint8_t rear_pass_pixel,
    uint8_t door_pixel,
    uint8_t front_pass_pixel,
    CSB_V1_D2CF0111DoorFrontTracePc34 *out_trace)
{
    CSB_V1_D2CF0111DoorFrontTracePc34 trace;

    if (!spec || !out_trace) return -1;

    /*
     * ReDMCSB: DUNVIEW.C F0121 lines 7314-7315 and 7336-7368 order the
     * synthetic D2C pixel stack as F0108 floor, rear F0115 door pass,
     * F0111 door, then front F0115 door pass.
     */
    trace.ok = CSB_PRESENT;
    trace.f0108_calls = 1;
    trace.f0115_calls = 2;
    trace.f0111_calls = 1;
    trace.floor_transparent = floor_pixel == CSB_C10_COLOR_FLESH;
    trace.rear_transparent = rear_pass_pixel == CSB_C10_COLOR_FLESH;
    trace.door_transparent = door_pixel == CSB_C10_COLOR_FLESH;
    trace.front_transparent = front_pass_pixel == CSB_C10_COLOR_FLESH;
    trace.after_floor =
        csb_v1_viewport_d2c_f0111_door_front_blend_pc34(base_pixel, floor_pixel);
    trace.after_rear_pass =
        csb_v1_viewport_d2c_f0111_door_front_blend_pc34(trace.after_floor,
                                                        rear_pass_pixel);
    trace.after_door =
        csb_v1_viewport_d2c_f0111_door_front_blend_pc34(trace.after_rear_pass,
                                                        door_pixel);
    trace.after_front_pass =
        csb_v1_viewport_d2c_f0111_door_front_blend_pc34(trace.after_door,
                                                        front_pass_pixel);
    *out_trace = trace;
    return 0;
}

int csb_v1_viewport_d2c_f0111_door_front_is_draw_mutating_pc34(
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *spec)
{
    /*
     * ReDMCSB: DUNGEON.C F0163 lines 1769-1838 and F0164 lines 1840-1905
     * mutate thing lists; DUNVIEW.C F0121 lines 7313-7341 and F0111 lines
     * 4218-4339 only consume F0172 square-aspect fields for this draw gate.
     */
    if (!spec) return -1;
    return !(spec->f0163_not_called_by_draw && spec->f0164_not_called_by_draw);
}

const char *csb_v1_viewport_d2c_f0111_door_front_source_evidence_pc34(void)
{
    return s_source_evidence;
}
