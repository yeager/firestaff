#include "csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pc34_compat.h"

/*
 * Source lock:
 * - ReDMCSB DUNVIEW.C F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF
 *   lines 3502-3938 draws wall ornaments and returns the alcove flag; lines
 *   3907-3923 keep C10_COLOR_FLESH transparent for ornament blits.
 * - ReDMCSB DUNVIEW.C F0121_DUNGEONVIEW_DrawSquareD2C lines 7244-7342 binds
 *   the D2C wall and center-door routes: line 7308 calls F0107 on
 *   M552/M583, line 7309 switches alcove order to C0x0000, and lines
 *   7313-7341 draw D2C door-front floor/object pass, frames, F0111, then the
 *   front object pass.
 * - ReDMCSB DUNVIEW.C F0111_DUNGEONVIEW_DrawDoor lines 4218-4339 draws the
 *   closed door panel; lines 4297-4299 blit the closed/destroyed bitmap and
 *   line 4334 is the PC34 zone draw with C10 transparency.
 * - ReDMCSB DEFS.H lines 2537-2539, 2581, 2657-2677, 2688-2690, 4030-4049,
 *   and 4238-4257 define square-aspect slots, D2C view square, cell-order
 *   constants, D2 wall indices, wall zones, and M628_ZONE_DOOR_D2C.
 * - CSB-lineage Viewport.cpp lines 1016-1024 preserve the F2 wall-decoration
 *   Alcove/JumpZ helper, lines 1865-1879 preserve the F2 door-facing helper,
 *   lines 2596-2616 call DrawDoor, and lines 2949-2955 call
 *   DrawWallDecoration.
 */

enum {
    CSB_PRESENT = 1,
    CSB_ABSENT = 0,
    CSB_ELEMENT_WALL = 0,
    CSB_ELEMENT_DOOR_FRONT = 17,
    CSB_VIEW_SQUARE_D2C = 6,
    CSB_D2_DEPTH = 2,
    CSB_CENTER_LATERAL = 0,
    CSB_M552_FRONT_WALL_ORNAMENT_ORDINAL = 5,
    CSB_M583_VIEW_WALL_D2C_FRONT = 10,
    CSB_C09_WALL_D2C = 9,
    CSB_C709_ZONE_WALL_D2C = 709,
    CSB_CELL_ORDER_ALCOVE = 0x0000,
    CSB_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT = 0x0218,
    CSB_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT = 0x0349,
    CSB_C4_DOOR_STATE_CLOSED = 4,
    CSB_M628_ZONE_DOOR_D2C = 3760,
    CSB_C1_VIEW_DOOR_ORNAMENT_D2LCR = 1,
    CSB_D2C_DOOR_WIDTH = 64,
    CSB_D2C_DOOR_HEIGHT = 61,
    CSB_C10_COLOR_FLESH = 10,
    CSB_COLOR_WALL = 1,
    CSB_COLOR_ORNAMENT_LEFT = 21,
    CSB_COLOR_ORNAMENT_CENTER = 22,
    CSB_COLOR_ORNAMENT_RIGHT = 23,
    CSB_COLOR_DOOR = 40,
    CSB_ORNAMENT_X1 = 8,
    CSB_ORNAMENT_X2 = 103,
    CSB_ORNAMENT_CENTER_X1 = 24,
    CSB_ORNAMENT_CENTER_X2 = 87,
    CSB_DOOR_X1 = 24,
    CSB_DOOR_X2 = 87,
    CSB_LEFT_VISIBLE_X1 = 8,
    CSB_LEFT_VISIBLE_X2 = 23,
    CSB_RIGHT_VISIBLE_X1 = 88,
    CSB_RIGHT_VISIBLE_X2 = 103
};

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C:3502-3938 F0107_DUNGEONVIEW_"
    "IsDrawnWallOrnamentAnAlcove_CPSF draws a D2C front wall ornament and "
    "returns the alcove flag; DUNVIEW.C:7308-7312 F0121_DUNGEONVIEW_"
    "DrawSquareD2C binds M552_FRONT_WALL_ORNAMENT_ORDINAL to "
    "M583_VIEW_WALL_D2C_FRONT and C0x0000_CELL_ORDER_ALCOVE. "
    "DUNVIEW.C:7313-7341 binds the C17_ELEMENT_DOOR_FRONT path, "
    "F0115 order 0x0218 before F0111, F0111_DUNGEONVIEW_DrawDoor at "
    "7336/7339 with G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, "
    "C1_VIEW_DOOR_ORNAMENT_D2LCR and M628_ZONE_DOOR_D2C, then order "
    "0x0349. DUNVIEW.C:4218-4339 F0111 line 4298 draws the closed door "
    "bitmap and line 4334 uses C10_COLOR_FLESH transparency. DEFS.H:2537-"
    "2539,2581,2657-2677,2688-2690,4030-4049,4238-4257 anchor the square "
    "aspect, D2C view, cell orders, wall zones and door zone. CSB-lineage "
    "Viewport.cpp:1016-1024 F2 wall-decoration Alcove/JumpZ helper and "
    "Viewport.cpp:1865-1879 F2 door-facing helper correspond; "
    "Viewport.cpp:2596-2616 StdDrawDoor and 2949-2955 StdDrawWallDecoration "
    "are the helper-level door/ornament cross-check.";

static const CSB_V1_ViewportD2CF0107F0111SpecPc34 s_spec = {
    CSB_PRESENT,
    CSB_PRESENT,
    CSB_VIEW_SQUARE_D2C,
    CSB_D2_DEPTH,
    CSB_CENTER_LATERAL,
    CSB_ELEMENT_WALL,
    CSB_ELEMENT_DOOR_FRONT,
    CSB_M552_FRONT_WALL_ORNAMENT_ORDINAL,
    CSB_M583_VIEW_WALL_D2C_FRONT,
    CSB_C09_WALL_D2C,
    CSB_C709_ZONE_WALL_D2C,
    CSB_VIEW_SQUARE_D2C,
    CSB_PRESENT,
    CSB_CELL_ORDER_ALCOVE,
    CSB_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT,
    CSB_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT,
    CSB_C4_DOOR_STATE_CLOSED,
    CSB_M628_ZONE_DOOR_D2C,
    CSB_C1_VIEW_DOOR_ORNAMENT_D2LCR,
    CSB_D2C_DOOR_WIDTH,
    CSB_D2C_DOOR_HEIGHT,
    CSB_C10_COLOR_FLESH,
    CSB_ORNAMENT_X1,
    CSB_ORNAMENT_X2,
    CSB_ORNAMENT_CENTER_X1,
    CSB_ORNAMENT_CENTER_X2,
    CSB_DOOR_X1,
    CSB_DOOR_X2,
    CSB_LEFT_VISIBLE_X1,
    CSB_LEFT_VISIBLE_X2,
    CSB_RIGHT_VISIBLE_X1,
    CSB_RIGHT_VISIBLE_X2,
    "ReDMCSB DUNVIEW.C:3502-3938 F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF; D2C call at 7308",
    "ReDMCSB DUNVIEW.C:4218-4339 F0111_DUNGEONVIEW_DrawDoor; closed-door blit 4297-4299 and PC34 draw 4334",
    "ReDMCSB DUNVIEW.C:7244-7342 F0121_DUNGEONVIEW_DrawSquareD2C; wall 7289-7312 and door 7313-7341",
    "ReDMCSB DEFS.H:2537-2539,2581,2657-2677,2688-2690,4030-4049,4238-4257",
    "CSB-lineage Viewport.cpp:1016-1024 F2 stone, 1865-1879 F2 door, 2596-2616 StdDrawDoor, 2949-2955 StdDrawWallDecoration",
    s_source_evidence
};

const CSB_V1_ViewportD2CF0107F0111SpecPc34 *
csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_spec_pc34(void)
{
    return &s_spec;
}

static size_t pixel_offset(int x, int y)
{
    return (size_t)y * CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34 + (size_t)x;
}

int csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_pixel_pc34(
    const uint8_t *canvas,
    size_t canvas_size,
    int x,
    int y)
{
    if (!canvas) return -1;
    if (x < 0 || x >= CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34) return -1;
    if (y < 0 || y >= CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34) return -1;
    if (canvas_size < (size_t)(CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34 *
                               CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34)) {
        return -1;
    }
    return canvas[pixel_offset(x, y)];
}

int csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_render_pc34(
    uint8_t *canvas,
    size_t canvas_size,
    CSB_V1_ViewportD2CF0107F0111TracePc34 *out_trace)
{
    CSB_V1_ViewportD2CF0107F0111TracePc34 trace = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, s_source_evidence
    };
    const size_t needed =
        (size_t)CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34 *
        (size_t)CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34;

    if (!canvas || !out_trace || canvas_size < needed) return -1;

    for (int y = 0; y < CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34; ++y) {
        for (int x = 0; x < CSB_V1_D2C_F0107_F0111_VIEWPORT_WIDTH_PC34; ++x) {
            canvas[pixel_offset(x, y)] = (uint8_t)CSB_COLOR_WALL;
            ++trace.wall_pixels;
        }
    }

    /* F0107 first: draw the D2C alcove ornament on the back wall. */
    for (int y = 0; y < CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34; ++y) {
        for (int x = CSB_ORNAMENT_X1; x <= CSB_ORNAMENT_X2; ++x) {
            uint8_t color = (uint8_t)CSB_COLOR_ORNAMENT_CENTER;
            if (x < CSB_DOOR_X1) color = (uint8_t)CSB_COLOR_ORNAMENT_LEFT;
            if (x > CSB_DOOR_X2) color = (uint8_t)CSB_COLOR_ORNAMENT_RIGHT;
            canvas[pixel_offset(x, y)] = color;
            ++trace.ornament_pixels_before_door;
        }
    }

    /* F0111 second: the closed D2C door bitmap is opaque over the cell column. */
    for (int y = 0; y < CSB_D2C_DOOR_HEIGHT; ++y) {
        for (int x = CSB_DOOR_X1; x <= CSB_DOOR_X2; ++x) {
            if (canvas[pixel_offset(x, y)] == (uint8_t)CSB_COLOR_ORNAMENT_CENTER) {
                ++trace.ornament_center_pixels_covered_by_door;
            }
            canvas[pixel_offset(x, y)] = (uint8_t)CSB_COLOR_DOOR;
            ++trace.door_pixels;
        }
    }

    for (int y = 0; y < CSB_V1_D2C_F0107_F0111_VIEWPORT_HEIGHT_PC34; ++y) {
        for (int x = CSB_LEFT_VISIBLE_X1; x <= CSB_LEFT_VISIBLE_X2; ++x) {
            if (canvas[pixel_offset(x, y)] == (uint8_t)CSB_COLOR_ORNAMENT_LEFT) {
                ++trace.ornament_left_pixels_visible_after_door;
            }
        }
        for (int x = CSB_RIGHT_VISIBLE_X1; x <= CSB_RIGHT_VISIBLE_X2; ++x) {
            if (canvas[pixel_offset(x, y)] == (uint8_t)CSB_COLOR_ORNAMENT_RIGHT) {
                ++trace.ornament_right_pixels_visible_after_door;
            }
        }
    }

    trace.center_samples_opaque =
        canvas[pixel_offset(24, 0)] == (uint8_t)CSB_COLOR_DOOR &&
        canvas[pixel_offset(56, 30)] == (uint8_t)CSB_COLOR_DOOR &&
        canvas[pixel_offset(87, 60)] == (uint8_t)CSB_COLOR_DOOR;
    trace.left_samples_visible =
        canvas[pixel_offset(8, 0)] == (uint8_t)CSB_COLOR_ORNAMENT_LEFT &&
        canvas[pixel_offset(16, 30)] == (uint8_t)CSB_COLOR_ORNAMENT_LEFT &&
        canvas[pixel_offset(23, 60)] == (uint8_t)CSB_COLOR_ORNAMENT_LEFT;
    trace.right_samples_visible =
        canvas[pixel_offset(88, 0)] == (uint8_t)CSB_COLOR_ORNAMENT_RIGHT &&
        canvas[pixel_offset(96, 30)] == (uint8_t)CSB_COLOR_ORNAMENT_RIGHT &&
        canvas[pixel_offset(103, 60)] == (uint8_t)CSB_COLOR_ORNAMENT_RIGHT;

    trace.ok =
        trace.draw_order_f0107 < trace.draw_order_f0111 &&
        trace.ornament_center_pixels_covered_by_door == trace.door_pixels &&
        trace.ornament_left_pixels_visible_after_door == 16 * 61 &&
        trace.ornament_right_pixels_visible_after_door == 16 * 61 &&
        trace.center_samples_opaque &&
        trace.left_samples_visible &&
        trace.right_samples_visible;

    *out_trace = trace;
    return trace.ok ? 0 : 1;
}

const char *
csb_v1_viewport_d2c_f0107_wall_ornament_with_f0111_door_front_source_evidence_pc34(void)
{
    return s_source_evidence;
}
