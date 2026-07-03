/*
 * CSB V1 Viewport Rendering — pc34 compat implementation
 *
 * Source-locked to ReDMCSB WIP20210206, Toolchains/Common/Source/:
 *   DUNVIEW.C:6226-6353 F0676/F0677 (CSB back-wall D3L2/D3R2)
 *   DUNVIEW.C:6837-6896 F0678/F0679 (CSB near-wall D2L2/D2R2)
 *   DUNVIEW.C:8318-8542 F0128 (shared DM1/CSB viewport draw core)
 *
 * CSB differences from DM1:
 *   - D3L2/D3R2 back-wall positions render all 8 element types
 *     (WALL, TELEPORTER, STAIRS_FRONT, PIT, CORRIDOR, DOOR_SIDE,
 *      DOOR_FRONT, STAIRS_SIDE)
 *   - D2L2/D2R2 four-sided decoration positions render WALL only
 *     (no stairs, pits, floor ornaments, creatures, items, projectiles)
 *   - Four-sided wall decoration rules differ from DM1 corridor sides
 *   - Custom backgrounds (CSBWin/CSBCode.cpp:26 CustomBackgrounds)
 *   - CSB wall set index selection per current map
 *
 * Reference: CSBWin/Viewport.cpp (7290 lines) · CSBWin/Graphics.cpp (3186 lines)
 *   CSBWin/CSBCode.cpp:26 CustomBackgrounds · CSBWin/CSBCode.cpp:9196
 */

#include "csb_v1_viewport_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include <string.h>

enum {
    CSB_V1_ORNAMENT_SLOT_RIGHT = 1, /* M551_RIGHT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_ORNAMENT_SLOT_FRONT = 2, /* M552_FRONT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_ORNAMENT_SLOT_LEFT = 3,  /* M553_LEFT_WALL_ORNAMENT_ORDINAL */
    CSB_V1_VIEW_WALL_D3L2_RIGHT = 0,
    CSB_V1_VIEW_WALL_D3R2_LEFT = 1,
    CSB_V1_VIEW_WALL_D2L_RIGHT = 7,
    CSB_V1_VIEW_WALL_D2R_LEFT = 8,
    CSB_V1_VIEW_WALL_D2L_FRONT = 9,
    CSB_V1_VIEW_WALL_D2C_FRONT = 10,
    CSB_V1_VIEW_WALL_D2R_FRONT = 11,
    CSB_V1_VIEW_WALL_D1L_RIGHT = 12,
    CSB_V1_VIEW_WALL_D1R_LEFT = 13,
    CSB_V1_VIEW_WALL_D1C_FRONT = 14,
    CSB_V1_NO_ORNAMENT_SLOT = -1,
    CSB_V1_NO_VIEW_WALL = -1,
    CSB_V1_VIEW_FLOOR_D3L2 = 0, /* C00_VIEW_FLOOR_D3L2 */
    CSB_V1_VIEW_FLOOR_D3R2 = 1, /* C01_VIEW_FLOOR_D3R2 */
    CSB_V1_CELL_ORDER_D3L2_DOORPASS1 = 0x0218,
    CSB_V1_CELL_ORDER_D3L2_DOORPASS2 = 0x0349,
    CSB_V1_CELL_ORDER_D3R2_DOORPASS1 = 0x0128,
    CSB_V1_CELL_ORDER_D3R2_DOORPASS2 = 0x0439,
    CSB_V1_CELL_ORDER_D3L2_CORRIDOR = 0x3421,
    CSB_V1_CELL_ORDER_D3R2_CORRIDOR = 0x4312,
    CSB_V1_CELL_ORDER_D3L2_SIDE = 0x0321,
    CSB_V1_CELL_ORDER_D3R2_SIDE = 0x0412,
    CSB_V1_ZONE_DOOR_D3L2 = 3700,
    CSB_V1_ZONE_DOOR_D3R2 = 3710,
    CSB_V1_REDMCSB_VIEW_SQUARE_D3L2 = 14, /* C14_VIEW_SQUARE_D3L2 */
    CSB_V1_REDMCSB_VIEW_SQUARE_D3R2 = 15, /* C15_VIEW_SQUARE_D3R2 */
    CSB_V1_VIEW_DEPTH_D3 = 3,
    CSB_V1_OBJECT_ROW_D3L2 = 3, /* G2028_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_OBJECT_ROW_D3R2 = 4, /* G2028_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_OBJECT_ZONE_BASE = 2500, /* C2500_ZONE_ */
    CSB_V1_OBJECT_ZONE_CELL_STRIDE = 4,
    CSB_V1_OBJECT_SHIFT_SET_D3_FRONT = 5, /* (viewDepth * 2) - 1 - (viewCell >> 1) for D3 back-row cells */
    CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL = 3,
    CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL = 4,
    CSB_V1_CREATURE_ROW_D3L2 = 3, /* G2033_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_CREATURE_ROW_D3R2 = 4, /* G2033_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_CREATURE_ZONE_BASE = 3200, /* C3200_ZONE_ */
    CSB_V1_CREATURE_COORDINATE_SET_STRIDE = 65,
    CSB_V1_CREATURE_ZONE_CELL_STRIDE = 5,
    CSB_V1_CREATURE_SHIFT_MASK = 0x8000, /* MASK0x8000_SHIFT_OBJECTS_AND_CREATURES */
    CSB_V1_PROJECTILE_ROW_D3L2 = 3, /* G2028_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_PROJECTILE_ROW_D3R2 = 4, /* G2028_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_PROJECTILE_ZONE_BASE = 2900, /* C2900_ZONE_ */
    CSB_V1_PROJECTILE_ZONE_STRIDE = 4,
    CSB_V1_EXPLOSION_ROW_D3L2 = 6, /* G2034_ac_ViewSquareIndexTo[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_EXPLOSION_ROW_D3R2 = 7, /* G2034_ac_ViewSquareIndexTo[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_FIELD_ASPECT_D3L2 = 0, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C14_VIEW_SQUARE_D3L2] */
    CSB_V1_FIELD_ASPECT_D3R2 = 1, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C15_VIEW_SQUARE_D3R2] */
    CSB_V1_FIELD_ASPECT_D2L2 = 5, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C09_VIEW_SQUARE_D2L2] */
    CSB_V1_FIELD_ASPECT_D2R2 = 6, /* G2035_ac_ViewSquareIndexToFieldAspectIndex[C10_VIEW_SQUARE_D2R2] */
    CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE = 3000, /* C3000_ZONE_ */
    CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE = 3007, /* C3007_ZONE_ */
    CSB_V1_EXPLOSION_CENTERED_ZONE_BASE = 3014, /* C3014_ZONE_ */
    CSB_V1_EXPLOSION_SIDE_ZONE_BASE = 3031, /* C3031_ZONE_ */
    CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE = 2,
    CSB_V1_FIELD_ZONE_D3L2 = 702, /* C702_ZONE_WALL_D3L2 */
    CSB_V1_FIELD_ZONE_D3R2 = 703, /* C703_ZONE_WALL_D3R2 */
    CSB_V1_FIELD_ZONE_D2L2 = 707, /* C707_ZONE_WALL_D2L2 */
    CSB_V1_FIELD_ZONE_D2R2 = 708, /* C708_ZONE_WALL_D2R2 */
    CSB_V1_WALL_BITMAP_D2R2 = 5, /* DEFS.H:3428 C05_WALL_D2R2 */
    CSB_V1_WALL_BITMAP_D2L2 = 6, /* DEFS.H:3429 C06_WALL_D2L2 */
    CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED = 1,
    CSB_V1_DOOR_PANEL_PARENT_D3L2 = 129,
    CSB_V1_DOOR_PANEL_PARENT_D3R2 = 130,
    CSB_V1_DOOR_PANEL_CLIP_D3 = 126,
    CSB_V1_DOOR_PANEL_NATIVE_W_D3 = 48,
    CSB_V1_DOOR_PANEL_NATIVE_H_D3 = 41,
    CSB_V1_DOOR_PANEL_CLIPPED_H_D3 = 40,
    CSB_V1_DOOR_PANEL_D3L2_X = 24,
    CSB_V1_DOOR_PANEL_D3R2_X = 88,
    CSB_V1_DOOR_PANEL_D3_Y = 28,
    CSB_V1_DOOR_ORNAMENT_D3LCR = 0, /* C0_VIEW_DOOR_ORNAMENT_D3LCR */
    CSB_V1_DOOR_STATE_DESTROYED = 5, /* C5_DOOR_STATE_DESTROYED */
    CSB_V1_DOOR_ORNAMENT_DESTROYED_MASK = 15, /* C15_DOOR_ORNAMENT_DESTROYED_MASK */
    CSB_V1_DOOR_HORIZONTAL_FINAL_SHIFT_MASK = 0x4000, /* MASK0x4000_SHIFT_UNREADABLE_INSCRIPTION_AND_OPEN_VERTICAL_DOOR */
    CSB_V1_DOOR_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_WALL_ORNAMENT_ZONE_BASE = 1004, /* C1004_ZONE_WALL_ORNAMENT */
    CSB_V1_WALL_ORNAMENT_COORD_STRIDE = 15, /* MEDIA720 C15_UNKNOWN */
    CSB_V1_WALL_ORNAMENT_SCALE_X_D3 = 30, /* C30_SCALE_ */
    CSB_V1_WALL_ORNAMENT_SCALE_Y_D3 = 14, /* C14_SCALE_ */
    CSB_V1_WALL_ORNAMENT_SCALE_D2 = 21, /* C21_SCALE_ */
    CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA = -1,
    CSB_V1_WALL_ORNAMENT_D3L2_BITMAP_INCREMENT = 0,
    CSB_V1_WALL_ORNAMENT_D3R2_BITMAP_INCREMENT = 0,
    CSB_V1_WALL_ORNAMENT_D2_SIDE_DERIVED_INCREMENT = 2,
    CSB_V1_WALL_ORNAMENT_D2_FRONT_DERIVED_INCREMENT = 3,
    CSB_V1_WALL_ORNAMENT_D1_SIDE_DERIVED_INCREMENT = 4,
    CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE = -1, /* CM1_DERIVED_BITMAP_NONE */
    CSB_V1_FLOOR_ORNAMENT_ZONE_BASE = 1500, /* C1500_ZONE_FLOOR_ORNAMENT */
    CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE = 11,
    CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA = -1,
    CSB_V1_FLOOR_ORNAMENT_D3L2_BITMAP_INCREMENT = 0,
    CSB_V1_FLOOR_ORNAMENT_D3R2_BITMAP_INCREMENT = 0,
    CSB_V1_FLOOR_ORNAMENT_COORD_SET = 0,
    CSB_V1_FLIP_NONE = 0, /* MASK0x0000_NO_FLIP */
    CSB_V1_FLIP_HORIZONTAL = 1, /* MASK0x0001_FLIP_HORIZONTAL */
    CSB_V1_FLIP_VERTICAL = 2, /* MASK0x0002_FLIP_VERTICAL */
    CSB_V1_PROJECTILE_DERIVED_BITMAP_NONE = -1, /* CM1_DERIVED_BITMAP_NONE */
    CSB_V1_PROJECTILE_TRANSPARENT_COLOR = 10, /* C10_COLOR_FLESH */
    CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_GRAPHIC_ID = 1,
    CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES = 18,
    CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_SKIN_DEF_INDEX = 0,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_SKIN_DEF_INDEX = 1,
    CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_SKIN_DEF_INDEX = 2,
    CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_SKIN_DEF_INDEX = 4,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_SKIN_DEF_INDEX = 5,
    CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_SKIN_DEF_INDEX = 6,
    CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_MIN_BYTES = 64,
    CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_MIN_BYTES = 64,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_MIN_BYTES = 20,
    CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_MIN_BYTES = 7840,
    CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_MIN_BYTES = 3248,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_MIN_BYTES = 4144,
    CSB_V1_CUSTOM_BACKGROUND_NEAR_ROOM_LIMIT = 5
};

/* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 draws the ordinary
 * floor/ceiling backdrop before the square pass sequence, with F0098 lines
 * 2995-3002 consuming G2109/G2108 and clearing the redraw request.  CSBWin
 * adds the CSB-only CustomBackgrounds pass between that baseline backdrop and
 * cell drawing: Viewport.cpp lines 5317-5325 supply room relative positions,
 * 6567-6615 resolves/apply skin bitmaps, and 6919-7140 inserts room slots. */
static const CSB_V1_ViewportCustomBackgroundSlotSpec s_custom_background_slots[] = {
    { 0, 3, -2, 0, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6919 room 0 before F3L1 draw path." },
    { 2, 3, -1, 1, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6920 room 2 before F3L1 draw path." },
    { 1, 3, 2, 2, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6940 room 1 before F3R1 draw path." },
    { 3, 3, 1, 3, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6941 room 3 before F3R1 draw path." },
    { 4, 3, 0, 4, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6961 room 4 before F3 draw path." },
    { 5, 2, -2, 5, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6981 room 5 before F2L1 draw path." },
    { 7, 2, -1, 6, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6982 room 7 before F2L1 draw path." },
    { 6, 2, 2, 7, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7002 room 6 before F2R1 draw path." },
    { 8, 2, 1, 8, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7003 room 8 before F2R1 draw path." },
    { 9, 2, 0, 9, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7023 room 9 before F2 draw path." },
    { 10, 1, -1, 10, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7043 room 10 before F1L1 draw path." },
    { 11, 1, 1, 11, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7063 room 11 before F1R1 draw path." },
    { 12, 1, 0, 12, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7081 room 12 before F1 draw path." },
    { 13, 0, -1, 13, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7102 room 13 before F0L1 draw path." },
    { 14, 0, 1, 14, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7122 room 14 before F0R1 draw path." },
    { 15, 0, 0, 15, 1, 3, 3, 7840, 3248, 4144, "CustomBackgrounds",
      "ReDMCSB DUNVIEW.C:8337-8339 F0128 floor/ceiling baseline; 2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 relpos; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 7140 room 15 before F0 draw path." }
};

/* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 and 8443 draw the base
 * floor/ceiling pass through F0098 lines 2962-3002 before cell drawing.
 * CSBWin extends that baseline with CSD/CSD-I34 background-library bitmaps:
 * Viewport.cpp lines 5402-5412 load the skin-selected bitmap, 6444-6470
 * applies the mask composite, and 6567-6615 gates each runtime layer. */
static const char s_custom_background_application_source[] =
    "ReDMCSB DUNVIEW.C:8337-8339,8443 F0128 floor/ceiling baseline; "
    "2962-3002 F0098 G2109/G2108 draw/reset. CSBWin Viewport.cpp:5317-5325 "
    "relposSid/relposFwd; 5402-5412 GetBitmap selects CSD/CSD-I34 "
    "background-library bitmaps instead of ReDMCSB base bitmaps; 6444-6470 "
    "ApplyBackground masked composite; 6567-6615 CustomBackgrounds runtime "
    "skin/default/mask/bitmap application.";

#define CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(slot_index, applies_near) \
    { \
        &s_custom_background_slots[(slot_index)], \
        1, 1, 1, \
        CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_GRAPHIC_ID, \
        CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_SKIN_DEF_INDEX, \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_MIN_BYTES, \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_MIN_BYTES, \
        (applies_near), \
        CSB_V1_CUSTOM_BACKGROUND_NEAR_ROOM_LIMIT, \
        1, 0, 1, 1, \
        "CustomBackgrounds", \
        s_custom_background_application_source \
    }

static const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec
    s_custom_background_application_specs[] = {
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(0, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(1, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(2, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(3, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(4, 1),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(5, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(6, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(7, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(8, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(9, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(10, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(11, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(12, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(13, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(14, 0),
        CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC(15, 0)
    };

#undef CSB_CUSTOM_BACKGROUND_APPLICATION_SPEC

typedef struct {
    int bitmap_skin_def_index;
    int mask_skin_def_index;
    int bitmap_min_bytes;
    int byte_width;
    int height;
} CSB_V1_CustomBackgroundLayerSelection;

static uint16_t csb_v1_read_le16(const uint8_t *bytes)
{
    return (uint16_t)(bytes[0] | ((uint16_t)bytes[1] << 8));
}

static int csb_v1_viewport_custom_background_layer_for_view(
    CSB_V1_ViewportCustomBackgroundViewIndex view_index,
    CSB_V1_CustomBackgroundLayerSelection *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    switch (view_index) {
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L2:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3L:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3C:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D3R2:
            out->bitmap_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_SKIN_DEF_INDEX;
            out->mask_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_SKIN_DEF_INDEX;
            out->bitmap_min_bytes = CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_MIN_BYTES;
            out->byte_width = 112;
            out->height = 70;
            return 1;
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2L2:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2L:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2C:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2R:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D2R2:
            out->bitmap_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_SKIN_DEF_INDEX;
            out->mask_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_SKIN_DEF_INDEX;
            out->bitmap_min_bytes = CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_MIN_BYTES;
            out->byte_width = 56;
            out->height = 58;
            return 1;
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D1L:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D1C:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D1R:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D0L:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D0C:
        case CSB_V1_CUSTOM_BACKGROUND_VIEW_D0R:
            out->bitmap_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_SKIN_DEF_INDEX;
            out->mask_skin_def_index = CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_SKIN_DEF_INDEX;
            out->bitmap_min_bytes = CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_MIN_BYTES;
            out->byte_width = 56;
            out->height = 74;
            return 1;
        default:
            return 0;
    }
}

static const CSB_V1_ViewportWallOrnamentRouteSpec s_wall_ornament_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        DM1_PC34_ZONE_WALL_D3L2,
        1,
        CSB_V1_ORNAMENT_SLOT_RIGHT,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6254-6263 wall panel then F0107(M551_RIGHT_WALL_ORNAMENT_ORDINAL, C00_VIEW_WALL_D3L2_RIGHT); DEFS.H:2696"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        DM1_PC34_ZONE_WALL_D3R2,
        1,
        CSB_V1_ORNAMENT_SLOT_LEFT,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6321-6330 wall panel then F0107(M553_LEFT_WALL_ORNAMENT_ORDINAL, C01_VIEW_WALL_D3R2_LEFT); DEFS.H:2697"
    },
    {
        (int)DM1_VIEW_SQUARE_D2L2,
        DM1_PC34_ZONE_WALL_D2L2,
        0,
        CSB_V1_NO_ORNAMENT_SLOT,
        CSB_V1_NO_VIEW_WALL,
        "F0678_DrawD2L2",
        "DUNVIEW.C:6848-6865 wall case returns without F0107; teleporter field is the only non-wall draw"
    },
    {
        (int)DM1_VIEW_SQUARE_D2R2,
        DM1_PC34_ZONE_WALL_D2R2,
        0,
        CSB_V1_NO_ORNAMENT_SLOT,
        CSB_V1_NO_VIEW_WALL,
        "F0679_DrawD2R2",
        "DUNVIEW.C:6877-6896 wall case returns without F0107; teleporter field is the only non-wall draw"
    },
};

/* ReDMCSB: DUNVIEW.C F0107 lines 3502-3590, 3817-3829, and
 * 3921-3923; F0676/F0677 lines 6263 and 6330.  The CSB/I34 far-side
 * wall-ornament views return early for ordinal 0, pre-decrement to the
 * current-map wall ornament index, build C1004 + CoordinateSet * 15 +
 * ViewWallIndex, scale through F0675 at C30/C14 with the D3 palette table,
 * flip only C01_VIEW_WALL_D3R2_LEFT, and dispatch the pixels through F0791
 * using C10 transparency. */
static const CSB_V1_ViewportWallOrnamentBlitSpec s_wall_ornament_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        1,
        CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_WALL_ORNAMENT_D3L2_BITMAP_INCREMENT,
        CSB_V1_WALL_ORNAMENT_ZONE_BASE,
        CSB_V1_WALL_ORNAMENT_COORD_STRIDE,
        CSB_V1_WALL_ORNAMENT_SCALE_X_D3,
        CSB_V1_WALL_ORNAMENT_SCALE_Y_D3,
        CSB_V1_FLIP_NONE,
        CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR,
        1,
        1,
        "M551_RIGHT_WALL_ORNAMENT_ORDINAL",
        "F0676_DrawD3L2",
        "DUNVIEW.C:6263 F0107(M551, C00_VIEW_WALL_D3L2_RIGHT); F0107:3571 skips ordinal 0; 3575 ordinal--; 3576 reads native bitmap; 3587 zone C1004 + CoordinateSet*15 + ViewWall; 3817-3819 leaves D3L2 unflipped; 3824-3829 F0675 C30/C14 with G0198 D3 palette; 3921-3923 F0791 C10 blit. DEFS.H:2696,4222; DUNVIEW.C:805-819 G0190 MEDIA720; COORD.C:921-1025 C1000..C1107 layout records."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        1,
        CSB_V1_WALL_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_WALL_ORNAMENT_D3R2_BITMAP_INCREMENT,
        CSB_V1_WALL_ORNAMENT_ZONE_BASE,
        CSB_V1_WALL_ORNAMENT_COORD_STRIDE,
        CSB_V1_WALL_ORNAMENT_SCALE_X_D3,
        CSB_V1_WALL_ORNAMENT_SCALE_Y_D3,
        CSB_V1_FLIP_HORIZONTAL,
        CSB_V1_WALL_ORNAMENT_TRANSPARENT_COLOR,
        1,
        1,
        "M553_LEFT_WALL_ORNAMENT_ORDINAL",
        "F0677_DrawD3R2",
        "DUNVIEW.C:6330 F0107(M553, C01_VIEW_WALL_D3R2_LEFT); F0107:3571 skips ordinal 0; 3575 ordinal--; 3576 reads native bitmap; 3587 zone C1004 + CoordinateSet*15 + ViewWall; 3817-3819 flips C01_VIEW_WALL_D3R2_LEFT; 3824-3829 F0675 C30/C14 with G0198 D3 palette; 3921-3923 F0791 C10 blit. DEFS.H:2697,4222; DUNVIEW.C:805-819 G0190 MEDIA720; COORD.C:921-1025 C1000..C1107 layout records."
    },
};

/* ReDMCSB: DUNVIEW.C F0107 lines 3589, 3608-3753, 3817-3829,
 * and 3921-3928; DUNGEON.C F0149 lines 1330-1347.  The CSB/I34
 * D3L2/D3R2 wall-ornament views still evaluate whether the ornament is an
 * alcove, but because view-wall indices 0/1 are below M585_VIEW_WALL_D1L
 * and are not M587_VIEW_WALL_D1C_FRONT, they stay on the D3 scaled-bitmap
 * path and do not update the D1-front interaction state or champion
 * portrait overlay. */
static const CSB_V1_ViewportWallOrnamentSideEffectSpec s_wall_ornament_side_effects[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_WALL_D3L2_RIGHT,
        1,
        0,
        0,
        0,
        0,
        0,
        1,
        CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6263 calls F0107(C00_VIEW_WALL_D3L2_RIGHT). F0107:3589 evaluates F0149_DUNGEON_IsWallOrnamentAnAlcove; 3608 gates D1-only facing/clickbox state; 3726-3744 updates facing alcove/Vi altar/fountain only inside that D1 branch; 3817-3829 routes C00/C01 through I34 D3 scaled bitmap with CM1_DERIVED_BITMAP_NONE; 3923-3928 champion portrait overlay is only M587_VIEW_WALL_D1C_FRONT. DUNGEON.C:1330-1347 F0149 alcove predicate. DEFS.H:2696/2708-2710."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_WALL_D3R2_LEFT,
        1,
        0,
        0,
        0,
        0,
        0,
        1,
        CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6330 calls F0107(C01_VIEW_WALL_D3R2_LEFT). F0107:3589 evaluates F0149_DUNGEON_IsWallOrnamentAnAlcove; 3608 gates D1-only facing/clickbox state; 3726-3744 updates facing alcove/Vi altar/fountain only inside that D1 branch; 3817-3829 routes C00/C01 through I34 D3 scaled bitmap with CM1_DERIVED_BITMAP_NONE; 3923-3928 champion portrait overlay is only M587_VIEW_WALL_D1C_FRONT. DUNGEON.C:1330-1347 F0149 alcove predicate. DEFS.H:2697/2708-2710."
    },
};

/* ReDMCSB: DUNVIEW.C F0107 lines 3571-3589, 3608-3753,
 * 3817-3860, and 3921-3928; F0119/F0120/F0121/F0122/F0123/F0124
 * wall branches at lines 6968-6969, 7119-7120, 7308, 7459, 7627,
 * and 7842.  These D1/D2 calls are distinct from the CSB-only
 * D3L2/D3R2 path: D2 uses the derived scaled-bitmap route and only
 * front D2 ornaments can return an alcove cell order, while D1 side
 * ornaments use the native bitmap path without updating the D1-front
 * interaction state.  D1C front alone owns the facing/clickbox/portrait
 * side effects. */
#define CSB_D1D2_WALL_ORNAMENT_PATH(square_, view_wall_, slot_, returns_, d2_, d1_, native_inc_, derived_inc_, scale_, flip_, state_, clickbox_, portrait_, fn_, source_) \
    { \
        (int)(square_), \
        (view_wall_), \
        (slot_), \
        (returns_), \
        (d2_), \
        (d1_), \
        (native_inc_), \
        (derived_inc_), \
        (scale_), \
        (flip_), \
        (state_), \
        (clickbox_), \
        (portrait_), \
        CSB_V1_WALL_ORNAMENT_ZONE_BASE, \
        CSB_V1_WALL_ORNAMENT_COORD_STRIDE, \
        (fn_), \
        (source_) \
    }

static const CSB_V1_ViewportWallOrnamentD1D2PathSpec s_wall_ornament_d1d2_paths[] = {
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2L,
        CSB_V1_VIEW_WALL_D2L_RIGHT,
        CSB_V1_ORNAMENT_SLOT_RIGHT,
        0, 1, 0, 0, CSB_V1_WALL_ORNAMENT_D2_SIDE_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0119_DrawSquareD2L",
        "DUNVIEW.C:6968 side F0107(M551, M580_VIEW_WALL_D2L_RIGHT) ignores return; F0107:3571-3589 ordinal/index/zone/alcove; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 2 and G0199 D2 palette; 3921-3923 F0791 C10. DEFS.H:2703,4222; DUNVIEW.C:805-819 G0190; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2L,
        CSB_V1_VIEW_WALL_D2L_FRONT,
        CSB_V1_ORNAMENT_SLOT_FRONT,
        1, 1, 0, 1, CSB_V1_WALL_ORNAMENT_D2_FRONT_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0119_DrawSquareD2L",
        "DUNVIEW.C:6969 front F0107(M552, M582_VIEW_WALL_D2L_FRONT) controls C0x0000 alcove order; F0107:3571-3589 ordinal/index/zone/alcove; 3800-3804 D2L front X adjustment; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 3 and native bitmap +1; 3921-3923 F0791 C10. DEFS.H:2705,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2R,
        CSB_V1_VIEW_WALL_D2R_LEFT,
        CSB_V1_ORNAMENT_SLOT_LEFT,
        0, 1, 0, 0, CSB_V1_WALL_ORNAMENT_D2_SIDE_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_HORIZONTAL, 0, 0, 0,
        "F0120_DrawSquareD2R",
        "DUNVIEW.C:7119 side F0107(M553, M581_VIEW_WALL_D2R_LEFT) ignores return; F0107:3571-3589 ordinal/index/zone/alcove; 3817-3819 sets horizontal flip; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 2 and G0199 D2 palette; 3921-3923 F0791 C10. DEFS.H:2704,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2R,
        CSB_V1_VIEW_WALL_D2R_FRONT,
        CSB_V1_ORNAMENT_SLOT_FRONT,
        1, 1, 0, 1, CSB_V1_WALL_ORNAMENT_D2_FRONT_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0120_DrawSquareD2R",
        "DUNVIEW.C:7120 front F0107(M552, M584_VIEW_WALL_D2R_FRONT) controls C0x0000 alcove order; F0107:3571-3589 ordinal/index/zone/alcove; 3782-3784 D2R front offset; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 3 and native bitmap +1; 3921-3923 F0791 C10. DEFS.H:2707,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D2C,
        CSB_V1_VIEW_WALL_D2C_FRONT,
        CSB_V1_ORNAMENT_SLOT_FRONT,
        1, 1, 0, 1, CSB_V1_WALL_ORNAMENT_D2_FRONT_DERIVED_INCREMENT,
        CSB_V1_WALL_ORNAMENT_SCALE_D2, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0121_DrawSquareD2C",
        "DUNVIEW.C:7308 front F0107(M552, M583_VIEW_WALL_D2C_FRONT) controls C0x0000 alcove order; F0107:3571-3589 ordinal/index/zone/alcove; 3817-3860 D2 C21 scaled derived-bitmap path with G0190 increment 3 and native bitmap +1; 3921-3923 F0791 C10. DEFS.H:2706,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D1L,
        CSB_V1_VIEW_WALL_D1L_RIGHT,
        CSB_V1_ORNAMENT_SLOT_RIGHT,
        0, 0, 1, 0, CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        0, CSB_V1_FLIP_NONE, 0, 0, 0,
        "F0122_DrawSquareD1L",
        "DUNVIEW.C:7459 side F0107(M551, M585_VIEW_WALL_D1L_RIGHT) ignores return; F0107:3571-3589 ordinal/index/zone/alcove; 3608 enters D1 branch, 3755-3760 uses native/CM1_DERIVED_BITMAP_NONE path, 3921-3923 F0791 C10; no 3726-3744 facing state because it is not M587. DEFS.H:2708,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D1R,
        CSB_V1_VIEW_WALL_D1R_LEFT,
        CSB_V1_ORNAMENT_SLOT_LEFT,
        0, 0, 1, 0, CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        0, CSB_V1_FLIP_HORIZONTAL, 0, 0, 0,
        "F0123_DrawSquareD1R",
        "DUNVIEW.C:7627 side F0107(M553, M586_VIEW_WALL_D1R_LEFT) ignores return; F0107:3571-3589 ordinal/index/zone/alcove; 3608 enters D1 branch, 3751-3752 sets horizontal flip, 3755-3760 uses native/CM1_DERIVED_BITMAP_NONE path, 3921-3923 F0791 C10; no 3726-3744 facing state because it is not M587. DEFS.H:2709,4222; COORD.C:921-1025."),
    CSB_D1D2_WALL_ORNAMENT_PATH(
        DM1_VIEW_SQUARE_D1C,
        CSB_V1_VIEW_WALL_D1C_FRONT,
        CSB_V1_ORNAMENT_SLOT_FRONT,
        1, 0, 1, 1, CSB_V1_WALL_ORNAMENT_DERIVED_BITMAP_NONE,
        0, CSB_V1_FLIP_NONE, 1, 1, 1,
        "F0124_DrawSquareD1C",
        "DUNVIEW.C:7842 front F0107(M552, M587_VIEW_WALL_D1C_FRONT) controls C0x0000 alcove F0115; F0107:3571-3589 ordinal/index/zone/alcove; 3608-3744 D1-front branch updates facing alcove/Vi altar/fountain, 3722 native bitmap +1, 3923-3928 copies clickbox and draws champion portrait overlay when present. DEFS.H:2710,4222; COORD.C:921-1025.")
};

#undef CSB_D1D2_WALL_ORNAMENT_PATH

/* ReDMCSB: DUNVIEW.C F0676/F0677 lines 6270-6286 and 6337-6353.
 * The CSB-only D3L2/D3R2 routes call F0108 before the rear F0115 pass,
 * call F0111 for door-front panels, then finish with the front F0115 pass.
 * Pit cases fall through to the same F0108 call, preserving BUG0_64. */
static const CSB_V1_ViewportFloorOrnamentRouteSpec s_floor_ornament_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        1,
        1,
        1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS2,
        CSB_V1_ZONE_DOOR_D3L2,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108 door-front floor ornament; 6271 F0115 pass1; 6272 F0111 C3700_ZONE_DOOR_D3L2; 6282-6286 pit/corridor F0108 then F0115"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        1,
        1,
        1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS2,
        CSB_V1_ZONE_DOOR_D3R2,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108 door-front floor ornament; 6338 F0115 pass1; 6339 F0111 C3710_ZONE_DOOR_D3R2; 6349-6353 pit/corridor F0108 then F0115"
    },
};

/* ReDMCSB: DUNVIEW.C F0108 lines 3940-4008 and F0676/F0677 lines
 * 6270/6284 and 6337/6351.  F0108 draws nothing for ordinal 0, converts
 * ordinal to map floor-ornament index by pre-decrement, adds the per-view
 * G0191 native bitmap increment, and blits to
 * C1500_ZONE_FLOOR_ORNAMENT + CoordinateSet * 11 + ViewFloorIndex using
 * F0791 with C10 transparency.  The CSB/I34 coordinate-set table is all 0,
 * and the CSB/I34 path flips D3R2 horizontally. */
static const CSB_V1_ViewportFloorOrnamentBlitSpec s_floor_ornament_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        1,
        CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_FLOOR_ORNAMENT_D3L2_BITMAP_INCREMENT,
        CSB_V1_FLOOR_ORNAMENT_COORD_SET,
        CSB_V1_FLOOR_ORNAMENT_ZONE_BASE,
        CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE,
        CSB_V1_FLIP_NONE,
        CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR,
        "M552_FRONT_WALL_ORNAMENT_ORDINAL",
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108 uses M552 for door-front D3L2; 6284 uses M558 for pit/corridor. F0108:3959 skips ordinal 0; 3965 ordinal-- plus G0191[0]=0; 3998 F0791 zone C1500 + CoordinateSet*11 + C00_VIEW_FLOOR_D3L2; G0195:1008-1017 gives CSB/I34 CoordinateSet 0; flip 0; C10. DEFS.H:2750, 3522-3523, 4223; COORD.C:903-913."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        1,
        CSB_V1_FLOOR_ORNAMENT_ORDINAL_TO_INDEX_DELTA,
        CSB_V1_FLOOR_ORNAMENT_D3R2_BITMAP_INCREMENT,
        CSB_V1_FLOOR_ORNAMENT_COORD_SET,
        CSB_V1_FLOOR_ORNAMENT_ZONE_BASE,
        CSB_V1_FLOOR_ORNAMENT_COORD_STRIDE,
        CSB_V1_FLIP_HORIZONTAL,
        CSB_V1_FLOOR_ORNAMENT_TRANSPARENT_COLOR,
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "M558_FLOOR_ORNAMENT_ORDINAL",
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108 uses M558 for door-front D3R2; 6351 uses M558 for pit/corridor. F0108:3959 skips ordinal 0; 3965 ordinal-- plus G0191[1]=0; G0195:1008-1017 gives CSB/I34 CoordinateSet 0; 3980-3983 sets MASK0x0001_FLIP_HORIZONTAL for C01_VIEW_FLOOR_D3R2; 3998 F0791 zone C1500 + CoordinateSet*11 + C01_VIEW_FLOOR_D3R2; C10. DEFS.H:2751, 3522-3523, 4223; COORD.C:903-913."
    },
};

/* ReDMCSB: DUNVIEW.C F0676 lines 6270-6286 and F0677 lines 6337-6353.
 * CSB's extra D3L2/D3R2 squares route the same F0115 stack as DM1: objects
 * first, a creature after object cells, projectiles after creatures, and
 * explosions after all cells.  Projectiles restart from the first thing for
 * the current cell and use C2900_ZONE_ + G2028[ViewSquare] * 4 + ViewCell.
 * Door-front squares split F0115 around F0111: rear cells before the door
 * panel/frame, front cells after it. */
static const CSB_V1_ViewportThingPassOrderSpec s_thing_pass_order_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_FLOOR_D3L2,
        0,
        1,
        2,
        3,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3L2_DOORPASS2,
        CSB_V1_CELL_ORDER_D3L2_CORRIDOR,
        CSB_V1_CELL_ORDER_D3L2_SIDE,
        0,
        1,
        2,
        CSB_V1_PROJECTILE_ROW_D3L2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        3,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6270 F0108; 6271 F0115 door rear cells 0x0218; 6272 F0111 door; 6284 F0108 pit/corridor BUG0_64; 6286 F0115 final/corridor/side; F0115:4567-4581 objects/creatures/projectiles, 5668 G2028 row, 5672 D3 front-cell skip, 5679 restart projectile list, 5681 cell match, 5683 C2900 zone, 5881-5883 blit, 5915-5933 explosions"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_FLOOR_D3R2,
        0,
        1,
        2,
        3,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS1,
        CSB_V1_CELL_ORDER_D3R2_DOORPASS2,
        CSB_V1_CELL_ORDER_D3R2_CORRIDOR,
        CSB_V1_CELL_ORDER_D3R2_SIDE,
        0,
        1,
        2,
        CSB_V1_PROJECTILE_ROW_D3R2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        3,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6337 F0108; 6338 F0115 door rear cells 0x0128; 6339 F0111 door; 6351 F0108 pit/corridor BUG0_64; 6353 F0115 final/corridor/side; F0115:4567-4581 objects/creatures/projectiles, 5668 G2028 row, 5672 D3 front-cell skip, 5679 restart projectile list, 5681 cell match, 5683 C2900 zone, 5881-5883 blit, 5915-5933 explosions"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4806-4811 and 4923.
 * For the PC34/I34E path, D3L2/D3R2 map through G2027/G2028 to depth 3
 * rows 3/4.  The object predicate accepts only weapon..junk things on the
 * processed cell; depth 3 squares suppress front-left/front-right cells by
 * requiring AL0126_i_ViewCell > C01_VIEW_CELL_FRONT_RIGHT. */
static const CSB_V1_ViewportObjectVisibilitySpec s_object_visibility_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3L2,
        1,
        1,
        1,
        0,
        CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL,
        CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL,
        "F0676_DrawD3L2",
        "DUNVIEW.C:371-373 G2026/G2027/G2028; 4806-4811 loads lane/depth/object row; 4923 F0115 weapon..junk, L2476>=0, cell match, depth3 front-cell suppression"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3R2,
        1,
        1,
        1,
        0,
        CSB_V1_FIRST_VISIBLE_D3_OBJECT_CELL,
        CSB_V1_LAST_VISIBLE_D3_OBJECT_CELL,
        "F0677_DrawD3R2",
        "DUNVIEW.C:371-373 G2026/G2027/G2028; 4806-4811 loads lane/depth/object row; 4923 F0115 weapon..junk, L2476>=0, cell match, depth3 front-cell suppression"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4923-5110, DEFS.H lines 3517 and
 * 4228, and COORD.C lines 1129-1193.  For the PC34/I34 route,
 * visible weapon..junk objects are drawn through
 * C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES plus the G2028
 * visibility row and view cell.  The shift mask makes COORD.C apply the
 * per-pile G0223/G0217 shift before F0791 blits with C10 transparency. */
static const CSB_V1_ViewportObjectBlitSpec s_object_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3L2,
        CSB_V1_OBJECT_ZONE_BASE,
        CSB_V1_OBJECT_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        CSB_V1_OBJECT_SHIFT_SET_D3_FRONT,
        1,
        10,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:4923 F0115 weapon..junk visible-cell predicate; 5030-5039 D3 object scale/shift set; 5071-5082 C2500_ZONE_ | MASK0x8000 + G2028 row*4 + ViewCell plus pile shift; 5109 F0791 C10 blit. DEFS.H:3517,4228; COORD.C:1129-1193 layout range 2500..2560."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_OBJECT_ROW_D3R2,
        CSB_V1_OBJECT_ZONE_BASE,
        CSB_V1_OBJECT_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        CSB_V1_OBJECT_SHIFT_SET_D3_FRONT,
        1,
        10,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:4923 F0115 weapon..junk visible-cell predicate; 5030-5039 D3 object scale/shift set; 5071-5082 C2500_ZONE_ | MASK0x8000 + G2028 row*4 + ViewCell plus pile shift; 5109 F0791 C10 blit. DEFS.H:3517,4228; COORD.C:1129-1193 layout range 2500..2560."
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 5668-5683 and 5710-5885,
 * DEFS.H line 4230, and COORD.C lines 1194-1239.  The MEDIA709
 * PC34/I34 path restarts from the first thing for the active cell, accepts
 * only projectile things whose stored cell matches, maps D3L2/D3R2 through
 * G2028 rows 3/4, rejects D3 front cells, and sends the scaled projectile
 * bitmap through F0791 with the computed C2900 zone, dynamic flip flags,
 * CM1_DERIVED_BITMAP_NONE for uncached scaled paths, and C10 transparency. */
static const CSB_V1_ViewportProjectileBlitSpec s_projectile_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_PROJECTILE_ROW_D3L2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        1,
        0,
        CSB_V1_PROJECTILE_DERIVED_BITMAP_NONE,
        CSB_V1_PROJECTILE_TRANSPARENT_COLOR,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:5668-5683 F0115 maps C14_VIEW_SQUARE_D3L2 through G2028 row 3, suppresses D3 front cells, restarts the thing list, requires C14_THING_TYPE_PROJECTILE and matching cell, then computes C2900_ZONE_ + row*4 + ViewCell. 5710-5722 scales by depth/cell/kinetic energy; 5859 CM1_DERIVED_BITMAP_NONE for uncached scaled path; 5881-5882 F0791 uses L2474 zone, dynamic flip flags, and C10 transparency. DEFS.H:4230; COORD.C:1194-1239 projectile zone records."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_PROJECTILE_ROW_D3R2,
        CSB_V1_PROJECTILE_ZONE_BASE,
        CSB_V1_PROJECTILE_ZONE_STRIDE,
        1,
        1,
        1,
        1,
        0,
        CSB_V1_PROJECTILE_DERIVED_BITMAP_NONE,
        CSB_V1_PROJECTILE_TRANSPARENT_COLOR,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:5668-5683 F0115 maps C15_VIEW_SQUARE_D3R2 through G2028 row 4, suppresses D3 front cells, restarts the thing list, requires C14_THING_TYPE_PROJECTILE and matching cell, then computes C2900_ZONE_ + row*4 + ViewCell. 5710-5722 scales by depth/cell/kinetic energy; 5859 CM1_DERIVED_BITMAP_NONE for uncached scaled path; 5881-5882 F0791 uses L2474 zone, dynamic flip flags, and C10 transparency. DEFS.H:4230; COORD.C:1194-1239 projectile zone records."
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 4840-4842, 5201-5214, and 5615-5627.
 * The MEDIA720 PC34/I34E path records one group thing while scanning each
 * cell, rejects view squares where G2033 maps to -1, then draws creatures
 * through C3200_ZONE_ + CreatureAspectCoordinateSet * 65 + G2033 row * 5
 * + ViewCell with MASK0x8000_SHIFT_OBJECTS_AND_CREATURES applied so COORD.C
 * adds the object/creature shifts before the F0791 blit. */
static const CSB_V1_ViewportCreatureVisibilitySpec s_creature_visibility_routes[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_CREATURE_ROW_D3L2,
        1,
        1,
        CSB_V1_CREATURE_ZONE_BASE,
        CSB_V1_CREATURE_COORDINATE_SET_STRIDE,
        CSB_V1_CREATURE_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        "F0676_DrawD3L2",
        "DUNVIEW.C:375 G2033 row; 4840-4842 records C04_THING_TYPE_GROUP; 5201-5214 F0115 creature gate rejects G2033<0; 5615-5627 C3200_ZONE_ | MASK0x8000 + CoordinateSet*65 + G2033*5 + ViewCell then F0791; COORD.C:1248-1251 C3200 layout range 3200..3364; 2074-2075 clears shift mask"
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_CREATURE_ROW_D3R2,
        1,
        1,
        CSB_V1_CREATURE_ZONE_BASE,
        CSB_V1_CREATURE_COORDINATE_SET_STRIDE,
        CSB_V1_CREATURE_ZONE_CELL_STRIDE,
        CSB_V1_CREATURE_SHIFT_MASK,
        "F0677_DrawD3R2",
        "DUNVIEW.C:375 G2033 row; 4840-4842 records C04_THING_TYPE_GROUP; 5201-5214 F0115 creature gate rejects G2033<0; 5615-5627 C3200_ZONE_ | MASK0x8000 + CoordinateSet*65 + G2033*5 + ViewCell then F0791; COORD.C:1248-1251 C3200 layout range 3200..3364; 2074-2075 clears shift mask"
    },
};

/* ReDMCSB: DUNVIEW.C F0115 lines 5915-6219, DEFS.H lines 4232-4235
 * and 4042-4043, COORD.C lines 1058-1123 and 1194-1238.  Explosions
 * restart from the first thing after all requested cells are processed.
 * MEDIA720 maps D3L2/D3R2 through G2034/G2035, uses C3000/C3007 for
 * rebirth, C3014 for centered explosions, C3031 + row*2 + cell for side
 * explosions, and defers fluxcage to F0113 as a field overlay. */
static const CSB_V1_ViewportExplosionBlitSpec s_explosion_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_EXPLOSION_ROW_D3L2,
        CSB_V1_FIELD_ASPECT_D3L2,
        1,
        1,
        1,
        CSB_V1_FIELD_ZONE_D3L2,
        CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE,
        CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE,
        CSB_V1_EXPLOSION_CENTERED_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE,
        10,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:5915-5933 restarts explosion pass; 5920-5924 G2034/G2035 visibility rows; 5948 rebirth requires visible row and matching cell; 5998-5999 C3000 + row; 6094-6096 C3007 + row; 6106-6107 C3014 + row; 6121-6122 C3031 + row*2 + ViewCell; 6192-6193 F0791 C10 blit; 6202-6219 fluxcage field C702 + field aspect. DEFS.H:4042,4232-4235; COORD.C:1058-1123,1194-1238."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        CSB_V1_VIEW_DEPTH_D3,
        CSB_V1_EXPLOSION_ROW_D3R2,
        CSB_V1_FIELD_ASPECT_D3R2,
        1,
        1,
        1,
        CSB_V1_FIELD_ZONE_D3R2,
        CSB_V1_EXPLOSION_REBIRTH_STEP1_ZONE_BASE,
        CSB_V1_EXPLOSION_REBIRTH_STEP2_ZONE_BASE,
        CSB_V1_EXPLOSION_CENTERED_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_BASE,
        CSB_V1_EXPLOSION_SIDE_ZONE_CELL_STRIDE,
        10,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:5915-5933 restarts explosion pass; 5920-5924 G2034/G2035 visibility rows; 5948 rebirth requires visible row and matching cell; 5998-5999 C3000 + row; 6094-6096 C3007 + row; 6106-6107 C3014 + row; 6121-6122 C3031 + row*2 + ViewCell; 6192-6193 F0791 C10 blit; 6202-6219 fluxcage field C702 + field aspect. DEFS.H:4043,4232-4235; COORD.C:1058-1123,1194-1238."
    },
};

/* ReDMCSB: DUNVIEW.C F0676/F0677 lines 6288-6290 and 6355-6357,
 * F0678/F0679 lines 6863-6865 and 6894-6896, G2035 at line 377, and
 * DEFS.H lines 4042-4048.  CSB/I34 D3L2/D3R2 teleporters finish their
 * F0108/F0115 path before F0113; D2L2/D2R2 are near-wall teleporter-only
 * routes with no F0108, F0115, or thing pass. */
static const CSB_V1_ViewportTeleporterFieldSpec s_teleporter_fields[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3L2,
        1,
        1,
        CSB_V1_FIELD_ASPECT_D3L2,
        CSB_V1_FIELD_ZONE_D3L2,
        1,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6288-6290 teleporter draws F0113(G0188[G2035[C14_VIEW_SQUARE_D3L2]], C702_ZONE_WALL_D3L2) after 6284 F0108 and 6286 F0115. DUNVIEW.C:377 G2035 maps C14 to field aspect 0; 4382-4409 F0113 clips by zone. DEFS.H:4042 C702_ZONE_WALL_D3L2."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_REDMCSB_VIEW_SQUARE_D3R2,
        1,
        1,
        CSB_V1_FIELD_ASPECT_D3R2,
        CSB_V1_FIELD_ZONE_D3R2,
        1,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6355-6357 teleporter draws F0113(G0188[G2035[C15_VIEW_SQUARE_D3R2]], C703_ZONE_WALL_D3R2) after 6351 F0108 and 6353 F0115. DUNVIEW.C:377 G2035 maps C15 to field aspect 1; 4382-4409 F0113 clips by zone. DEFS.H:4043 C703_ZONE_WALL_D3R2."
    },
    {
        (int)DM1_VIEW_SQUARE_D2L2,
        9,
        1,
        0,
        CSB_V1_FIELD_ASPECT_D2L2,
        CSB_V1_FIELD_ZONE_D2L2,
        1,
        "F0678_DrawD2L2",
        "DUNVIEW.C:6863-6865 teleporter draws F0113(G0188[G2035[C09_VIEW_SQUARE_D2L2]], C707_ZONE_WALL_D2L2) without F0108 and without F0115. DUNVIEW.C:377 G2035 maps C09 to field aspect 5; 4382-4409 F0113 clips by zone. DEFS.H:2605 C09_VIEW_SQUARE_D2L2; DEFS.H:4047 C707_ZONE_WALL_D2L2."
    },
    {
        (int)DM1_VIEW_SQUARE_D2R2,
        10,
        1,
        0,
        CSB_V1_FIELD_ASPECT_D2R2,
        CSB_V1_FIELD_ZONE_D2R2,
        1,
        "F0679_DrawD2R2",
        "DUNVIEW.C:6894-6896 teleporter draws F0113(G0188[G2035[C10_VIEW_SQUARE_D2R2]], C708_ZONE_WALL_D2R2) without F0108 and without F0115. DUNVIEW.C:377 G2035 maps C10 to field aspect 6; 4382-4409 F0113 clips by zone. DEFS.H:2606 C10_VIEW_SQUARE_D2R2; DEFS.H:4048 C708_ZONE_WALL_D2R2."
    },
};

/* ReDMCSB: DUNVIEW.C F0111 lines 4218-4337, F0676/F0677 lines
 * 6271-6273 and 6338-6340, DEFS.H lines 4250-4251, COORD.C lines
 * 1545-1565 and 781-807.  The CSB-only far door panels reuse the
 * D3 native 48x41 door bitmap but clip it through COORD.C record 126's
 * 48x40 viewport sub-zone.  F0111 skips state 0, shifts zone ids by
 * door state for partially-open doors, applies the destroyed-door mask for
 * state 5, and blits with C10 transparency. */
static const CSB_V1_ViewportDoorPanelBlitSpec s_door_panel_blits[] = {
    {
        (int)DM1_VIEW_SQUARE_D3L2,
        CSB_V1_ZONE_DOOR_D3L2,
        CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED,
        CSB_V1_DOOR_PANEL_PARENT_D3L2,
        CSB_V1_DOOR_PANEL_D3L2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_PANEL_CLIP_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_NATIVE_H_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_CLIPPED_H_D3,
        CSB_V1_DOOR_PANEL_D3L2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_ORNAMENT_D3LCR,
        1,
        1,
        6,
        3,
        CSB_V1_DOOR_STATE_DESTROYED,
        CSB_V1_DOOR_ORNAMENT_DESTROYED_MASK,
        1,
        CSB_V1_DOOR_TRANSPARENT_COLOR,
        "F0676_DrawD3L2",
        "DUNVIEW.C:6271 F0115 rear pass; 6272 F0111(... C3700_ZONE_DOOR_D3L2); 6273-6286 F0115 front pass. F0111:4248 skips C0 open, 4301-4302 applies C15 destroyed mask to P0128 view ornament index, 4298-4321 shifts zone by state/horizontal halves, 4334 F0791 blits with C10. DEFS.H:1044,2466,4250; COORD.C:1548-1565 records 120/126/129 and 788-797 zone 3700..3709."
    },
    {
        (int)DM1_VIEW_SQUARE_D3R2,
        CSB_V1_ZONE_DOOR_D3R2,
        CSB_V1_DOOR_PANEL_RECORD_TYPE_CLOSED,
        CSB_V1_DOOR_PANEL_PARENT_D3R2,
        CSB_V1_DOOR_PANEL_D3R2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_PANEL_CLIP_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_NATIVE_H_D3,
        CSB_V1_DOOR_PANEL_NATIVE_W_D3,
        CSB_V1_DOOR_PANEL_CLIPPED_H_D3,
        CSB_V1_DOOR_PANEL_D3R2_X,
        CSB_V1_DOOR_PANEL_D3_Y,
        CSB_V1_DOOR_ORNAMENT_D3LCR,
        1,
        1,
        6,
        3,
        CSB_V1_DOOR_STATE_DESTROYED,
        CSB_V1_DOOR_ORNAMENT_DESTROYED_MASK,
        1,
        CSB_V1_DOOR_TRANSPARENT_COLOR,
        "F0677_DrawD3R2",
        "DUNVIEW.C:6338 F0115 rear pass; 6339 F0111(... C3710_ZONE_DOOR_D3R2); 6340-6353 F0115 front pass. F0111:4248 skips C0 open, 4301-4302 applies C15 destroyed mask to P0128 view ornament index, 4298-4321 shifts zone by state/horizontal halves, 4334 F0791 blits with C10. DEFS.H:1044,2466,4251; COORD.C:1548-1565 records 120/126/130 and 798-807 zone 3710..3719."
    },
};

void csb_v1_viewport_init(CSB_V1_ViewportConfig *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->ambient_color = 0xFF000000; /* black ambient */
    cfg->viewport_pixels = NULL;
    cfg->viewport_stride = 320;
    cfg->dungeon_grid = NULL;
    cfg->dungeon_width = 0;
    cfg->dungeon_height = 0;
}

void csb_v1_viewport_set_wall_set(CSB_V1_ViewportConfig *cfg, int set) {
    if (cfg) cfg->wall_set_index = set;
}

void csb_v1_viewport_set_custom_background(CSB_V1_ViewportConfig *cfg, int bg_id) {
    if (cfg) cfg->custom_background = bg_id;
}

size_t csb_v1_viewport_custom_background_slot_spec_count(void)
{
    return sizeof(s_custom_background_slots) / sizeof(s_custom_background_slots[0]);
}

const CSB_V1_ViewportCustomBackgroundSlotSpec *
csb_v1_viewport_get_custom_background_slot_spec(size_t index)
{
    if (index >= csb_v1_viewport_custom_background_slot_spec_count()) return NULL;
    return &s_custom_background_slots[index];
}

const CSB_V1_ViewportCustomBackgroundSlotSpec *
csb_v1_viewport_get_custom_background_slot_spec_for_room(int room_num)
{
    for (size_t i = 0; i < csb_v1_viewport_custom_background_slot_spec_count(); ++i) {
        if (s_custom_background_slots[i].room_num == room_num) {
            return &s_custom_background_slots[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_custom_background_bitmap_application_spec_count(void)
{
    return sizeof(s_custom_background_application_specs) /
           sizeof(s_custom_background_application_specs[0]);
}

const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *
csb_v1_viewport_get_custom_background_bitmap_application_spec(size_t index)
{
    if (index >= csb_v1_viewport_custom_background_bitmap_application_spec_count()) return NULL;
    return &s_custom_background_application_specs[index];
}

const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *
csb_v1_viewport_get_custom_background_bitmap_application_spec_for_room(int room_num)
{
    for (size_t i = 0;
         i < csb_v1_viewport_custom_background_bitmap_application_spec_count();
         ++i) {
        const CSB_V1_ViewportCustomBackgroundSlotSpec *slot =
            s_custom_background_application_specs[i].room_slot;
        if (slot && slot->room_num == room_num) {
            return &s_custom_background_application_specs[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_custom_background_translate_cell(
    const CSB_V1_ViewportCustomBackgroundBitmapApplicationSpec *spec,
    int party_x,
    int party_y,
    int facing,
    int *out_x,
    int *out_y)
{
    static const int dx_forward[4] = { 0, 1, 0, -1 };
    static const int dy_forward[4] = { -1, 0, 1, 0 };
    static const int dx_side[4] = { 1, 0, -1, 0 };
    static const int dy_side[4] = { 0, 1, 0, -1 };
    const CSB_V1_ViewportCustomBackgroundSlotSpec *slot;

    if (!spec || !spec->room_slot || facing < 0 || facing >= 4 || !out_x || !out_y) {
        return 0;
    }

    slot = spec->room_slot;
    *out_x = party_x + dx_side[facing] * slot->relative_side +
             dx_forward[facing] * slot->relative_forward;
    *out_y = party_y + dy_side[facing] * slot->relative_side +
             dy_forward[facing] * slot->relative_forward;
    return 1;
}

CSB_V1_ViewportCustomBackgroundSelection
csb_v1_viewport_custom_background_load_and_select_pc34(
    const uint8_t *skin_def,
    size_t skin_def_size,
    CSB_V1_ViewportCustomBackgroundViewIndex view_index)
{
    CSB_V1_ViewportCustomBackgroundSelection result;
    CSB_V1_CustomBackgroundLayerSelection layer;
    size_t bitmap_offset;
    size_t mask_offset;

    memset(&result, 0, sizeof(result));

    /* ReDMCSB DRAWVIEW.C has no custom/background/skin-def references; it is
     * only the shared viewport transfer anchor.  The CSB-only path is the
     * CSBWin extension: CSBCode.cpp:26 declares CustomBackgrounds for the
     * CSB display lane rooted at CSBCode.cpp:9196, while Viewport.cpp
     * 6567-6615 loads a skin definition, selects pSkinDef[0/2/1] bitmaps
     * and pSkinDef[4/6/5] masks, then calls ApplyBackground (6444-6470). */
    if (!skin_def ||
        skin_def_size < CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_MIN_BYTES ||
        !csb_v1_viewport_custom_background_layer_for_view(view_index, &layer)) {
        return result;
    }

    bitmap_offset = (size_t)layer.bitmap_skin_def_index * 2u;
    mask_offset = (size_t)layer.mask_skin_def_index * 2u;
    if (bitmap_offset + 2u > skin_def_size || mask_offset + 2u > skin_def_size) {
        return result;
    }

    if (csb_v1_read_le16(skin_def) !=
        (uint16_t)CSB_V1_CUSTOM_BACKGROUND_SKIN_DEF_GRAPHIC_ID) {
        return result;
    }
    if (csb_v1_read_le16(skin_def + bitmap_offset) == 0 ||
        csb_v1_read_le16(skin_def + mask_offset) == 0) {
        return result;
    }

    result.bitmap = skin_def + bitmap_offset;
    result.mask = skin_def + mask_offset;
    result.byte_width = layer.byte_width;
    result.height = layer.height;
    result.is_valid = 1;
    return result;
}

size_t csb_v1_viewport_custom_background_layer_plan_pc34(
    int room_num,
    CSB_V1_ViewportCustomBackgroundLayerPlan *out_layers,
    size_t out_capacity)
{
    CSB_V1_ViewportCustomBackgroundLayerPlan layers[3];
    size_t count = 0;

    /* ReDMCSB: DUNVIEW.C F0128 lines 8337-8339 only establishes the
     * floor/ceiling baseline via F0098 lines 2962-3002. CSBWin extends
     * that baseline at Viewport.cpp lines 6593-6612 by applying large,
     * middle, then near CustomBackgrounds masks, with the near layer gated
     * by roomNum < 5. */
    if (room_num < 0 || room_num >= (int)csb_v1_viewport_custom_background_slot_spec_count()) {
        return 0;
    }

    layers[count++] = (CSB_V1_ViewportCustomBackgroundLayerPlan) {
        CSB_V1_CUSTOM_BACKGROUND_LAYER_LARGE,
        CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_SKIN_DEF_INDEX,
        CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_SKIN_DEF_INDEX,
        CSB_V1_CUSTOM_BACKGROUND_LARGE_BITMAP_MIN_BYTES,
        CSB_V1_CUSTOM_BACKGROUND_LARGE_MASK_MIN_BYTES,
        1
    };
    layers[count++] = (CSB_V1_ViewportCustomBackgroundLayerPlan) {
        CSB_V1_CUSTOM_BACKGROUND_LAYER_MIDDLE,
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_SKIN_DEF_INDEX,
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_SKIN_DEF_INDEX,
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_BITMAP_MIN_BYTES,
        CSB_V1_CUSTOM_BACKGROUND_MIDDLE_MASK_MIN_BYTES,
        1
    };
    if (room_num < CSB_V1_CUSTOM_BACKGROUND_NEAR_ROOM_LIMIT) {
        layers[count++] = (CSB_V1_ViewportCustomBackgroundLayerPlan) {
            CSB_V1_CUSTOM_BACKGROUND_LAYER_NEAR,
            CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_SKIN_DEF_INDEX,
            CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_SKIN_DEF_INDEX,
            CSB_V1_CUSTOM_BACKGROUND_NEAR_BITMAP_MIN_BYTES,
            CSB_V1_CUSTOM_BACKGROUND_NEAR_MASK_MIN_BYTES,
            1
        };
    }

    if (out_layers && out_capacity > 0) {
        size_t copy_count = count < out_capacity ? count : out_capacity;
        memcpy(out_layers, layers, copy_count * sizeof(layers[0]));
    }

    return count;
}

void csb_v1_viewport_set_dungeon_grid(CSB_V1_ViewportConfig *cfg,
                                       const uint8_t *grid,
                                       int width, int height) {
    if (!cfg) return;
    cfg->dungeon_grid = grid;
    cfg->dungeon_width = width;
    cfg->dungeon_height = height;
}

/* csb_v1_viewport_render_frame — integration entry point
 *
 * Renders the CSB dungeon view by delegating to the shared DM1 V1 viewport
 * engine (dm1_viewport_3d_draw_frame).  CSB config provides:
 *   - viewport_pixels + stride: the pixel buffer to draw into
 *   - dungeon_grid/width/height: square type data for element routing
 *   - wall_set_index: selects which GRAPHICS.DAT wall set to use
 *
 * When viewport_pixels is NULL, this is a no-op (allows staged integration).
 *
 * Source: CSBWin/Viewport.cpp F0128 passthrough; ReDMCSB DUNVIEW.C F0128
 */
void csb_v1_viewport_render_frame(CSB_V1_ViewportConfig *cfg,
                                   int party_dir,
                                   int party_x,
                                   int party_y)
{
    if (!cfg) return;

    /* Guard: no viewport buffer means not yet initialised */
    if (!cfg->viewport_pixels) return;

    /* Set up a DM1 viewport state backed by our CSB pixel buffer.
     * We share the exact same pixel format (320×200 indexed, viewport
     * sub-region 224×136 at screen row 33) so the DM1 draw primitives
     * work without modification. */
    DM1_Viewport3DState vp;
    memset(&vp, 0, sizeof(vp));
    vp.viewport_pixels = cfg->viewport_pixels;
    vp.viewport_stride = cfg->viewport_stride > 0 ? cfg->viewport_stride : 320;
    vp.floor_area = cfg->viewport_pixels +
                    DM1_VIEWPORT_FLOOR_Y * vp.viewport_stride;
    vp.floor_ceiling_dirty = true;

    /* Wire dungeon grid for CSB back-wall rendering (D3L2/D3R2/D2L2/D2R2).
     * The dungeon grid enables element-specific routing for CSB four-sided
     * wall decoration: walls, doors, stairs, pits, teleporters, corridors.
     * When dungeon_grid is NULL, CSB back-wall rendering falls back to
     * the generic wall-drawing path (same as DM1).
     *
     * Grid layout: dungeon_grid[y * dungeon_width + x] = raw dungeon cell.
     * Raw cell: low 5 bits = element type (0=WALL, 1=CORRIDOR, 2=PIT,
     * 3=STAIRS, 4=DOOR, 5=TELEPORTER, 6=FAKEWALL).
     *
     * Source: ReDMCSB DUNVIEW.C:6226-6353 F0676/F0677; 6837-6896 F0678/F0679 */
    vp.dungeon_grid   = cfg->dungeon_grid;
    vp.dungeon_width  = cfg->dungeon_width;
    vp.dungeon_height = cfg->dungeon_height;

    /* Wall set index — CSB may use a different wall set than DM1.
     * ReDMCSB DUNVIEW.C F0096 loads wall set based on current map index.
     * Here we accept the index from config; 0 = default CSB wall set. */
    dm1_viewport_3d_load_wall_set(&vp, cfg->wall_set_index, 0);

    /* Main draw call — mirrors ReDMCSB DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF.
     * Draws wall frames for D4 far objects, then D3L/R/C → D2 → D1 → D0
     * back-to-front with correct depth occlusion and parity flip.
     *
     * The CSB-specific elements (back-walls D3L2/D3R2, near-walls D2L2/D2R2)
     * are handled by dm1_viewport_3d_draw_csb_back_wall and
     * dm1_viewport_3d_draw_csb_near_wall, which are invoked from the
     * dm1_viewport_3d_draw_frame wall loop for D3L2/D3R2/D2L2/D2R2 positions.
     *
     * Custom background rendering — TODO (pass604).
     * ReDMCSB DUNVIEW.C F0128 lines 8337-8339 draws the standard
     * floor/ceiling backdrop through F0098 before square rendering begins;
     * CSBWin Viewport.cpp lines 6567-6615 and 6919-7140 add the CSB-only
     * CustomBackgrounds room-slot overlays before individual cell draws.
     * The source-locked slot metadata is exposed by
     * csb_v1_viewport_get_custom_background_slot_spec*(). */

    dm1_viewport_3d_draw_frame(&vp, party_dir, party_x, party_y);
}

/* ReDMCSB: DUNVIEW.C F0678 lines 6848-6862 and F0679 lines
 * 6879-6893.  The CSB/I34 D2L2/D2R2 wall case draws only the wall panel:
 * normal rendering uses its own G2107 wall-set bitmap (C06 for D2L2,
 * C05 for D2R2), while G0076_B_UseFlippedWallAndFootprintsBitmaps swaps
 * to the opposite bitmap and calls the flipped blitter.  The wall branch
 * returns before the teleporter-only F0113 path at lines 6863-6865 and
 * 6894-6896. */
int csb_v1_viewport_near_wall_d2_wall_bitmap_index(int view_square,
                                                   int use_flipped_wall_bitmaps)
{
    if (view_square == (int)DM1_VIEW_SQUARE_D2L2) {
        return use_flipped_wall_bitmaps ? CSB_V1_WALL_BITMAP_D2R2 :
                                          CSB_V1_WALL_BITMAP_D2L2;
    }
    if (view_square == (int)DM1_VIEW_SQUARE_D2R2) {
        return use_flipped_wall_bitmaps ? CSB_V1_WALL_BITMAP_D2L2 :
                                          CSB_V1_WALL_BITMAP_D2R2;
    }
    return -1;
}

int csb_v1_viewport_near_wall_d2_wall_zone(int view_square)
{
    if (view_square == (int)DM1_VIEW_SQUARE_D2L2) return CSB_V1_FIELD_ZONE_D2L2;
    if (view_square == (int)DM1_VIEW_SQUARE_D2R2) return CSB_V1_FIELD_ZONE_D2R2;
    return -1;
}

int csb_v1_viewport_near_wall_d2_wall_uses_flipped_blit(
    int view_square,
    int use_flipped_wall_bitmaps)
{
    if (view_square != (int)DM1_VIEW_SQUARE_D2L2 &&
        view_square != (int)DM1_VIEW_SQUARE_D2R2) {
        return -1;
    }
    return use_flipped_wall_bitmaps ? 1 : 0;
}

size_t csb_v1_viewport_wall_ornament_route_spec_count(void)
{
    return sizeof(s_wall_ornament_routes) / sizeof(s_wall_ornament_routes[0]);
}

const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_route_spec_count()) return NULL;
    return &s_wall_ornament_routes[index];
}

const CSB_V1_ViewportWallOrnamentRouteSpec *csb_v1_viewport_get_wall_ornament_route_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_route_spec_count(); ++i) {
        if (s_wall_ornament_routes[i].view_square == view_square) {
            return &s_wall_ornament_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_wall_ornament_blit_spec_count(void)
{
    return sizeof(s_wall_ornament_blits) / sizeof(s_wall_ornament_blits[0]);
}

const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_blit_spec_count()) return NULL;
    return &s_wall_ornament_blits[index];
}

const CSB_V1_ViewportWallOrnamentBlitSpec *csb_v1_viewport_get_wall_ornament_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_blit_spec_count(); ++i) {
        if (s_wall_ornament_blits[i].view_square == view_square) {
            return &s_wall_ornament_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_wall_ornament_blit_zone(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                            int coordinate_set)
{
    if (!spec || coordinate_set < 0) return -1;
    return spec->zone_base + (coordinate_set * spec->coordinate_set_stride) +
           spec->view_wall_index;
}

int csb_v1_viewport_wall_ornament_native_bitmap_index(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                                      int base_native_bitmap_index)
{
    if (!spec || base_native_bitmap_index < 0) return -1;
    return base_native_bitmap_index + spec->native_bitmap_index_increment;
}

int csb_v1_viewport_wall_ornament_blit_pixels(const CSB_V1_ViewportWallOrnamentBlitSpec *spec,
                                              const uint8_t *source,
                                              int source_stride,
                                              uint8_t *destination,
                                              int destination_stride,
                                              int width,
                                              int height)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride <= 0 || destination_stride <= 0 ||
        width <= 0 || height <= 0) {
        return -1;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sx = spec->horizontal_flip ? (width - 1 - x) : x;
            uint8_t pixel = source[(y * source_stride) + sx];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

size_t csb_v1_viewport_wall_ornament_side_effect_spec_count(void)
{
    return sizeof(s_wall_ornament_side_effects) / sizeof(s_wall_ornament_side_effects[0]);
}

const CSB_V1_ViewportWallOrnamentSideEffectSpec *csb_v1_viewport_get_wall_ornament_side_effect_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_side_effect_spec_count()) return NULL;
    return &s_wall_ornament_side_effects[index];
}

const CSB_V1_ViewportWallOrnamentSideEffectSpec *csb_v1_viewport_get_wall_ornament_side_effect_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_wall_ornament_side_effect_spec_count(); ++i) {
        if (s_wall_ornament_side_effects[i].view_square == view_square) {
            return &s_wall_ornament_side_effects[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_wall_ornament_d1d2_path_spec_count(void)
{
    return sizeof(s_wall_ornament_d1d2_paths) / sizeof(s_wall_ornament_d1d2_paths[0]);
}

const CSB_V1_ViewportWallOrnamentD1D2PathSpec *
csb_v1_viewport_get_wall_ornament_d1d2_path_spec(size_t index)
{
    if (index >= csb_v1_viewport_wall_ornament_d1d2_path_spec_count()) return NULL;
    return &s_wall_ornament_d1d2_paths[index];
}

int csb_v1_viewport_wall_ornament_d1d2_path_zone(
    const CSB_V1_ViewportWallOrnamentD1D2PathSpec *spec,
    int coordinate_set)
{
    if (!spec || coordinate_set < 0) return -1;
    return spec->zone_base + (coordinate_set * spec->coordinate_set_stride) +
           spec->view_wall_index;
}

size_t csb_v1_viewport_floor_ornament_route_spec_count(void)
{
    return sizeof(s_floor_ornament_routes) / sizeof(s_floor_ornament_routes[0]);
}

const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec(size_t index)
{
    if (index >= csb_v1_viewport_floor_ornament_route_spec_count()) return NULL;
    return &s_floor_ornament_routes[index];
}

const CSB_V1_ViewportFloorOrnamentRouteSpec *csb_v1_viewport_get_floor_ornament_route_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_floor_ornament_route_spec_count(); ++i) {
        if (s_floor_ornament_routes[i].view_square == view_square) {
            return &s_floor_ornament_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_floor_ornament_blit_spec_count(void)
{
    return sizeof(s_floor_ornament_blits) / sizeof(s_floor_ornament_blits[0]);
}

const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_floor_ornament_blit_spec_count()) return NULL;
    return &s_floor_ornament_blits[index];
}

const CSB_V1_ViewportFloorOrnamentBlitSpec *csb_v1_viewport_get_floor_ornament_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_floor_ornament_blit_spec_count(); ++i) {
        if (s_floor_ornament_blits[i].view_square == view_square) {
            return &s_floor_ornament_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_floor_ornament_blit_zone(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                             int coordinate_set)
{
    if (!spec || coordinate_set < 0) return -1;
    return spec->zone_base + (coordinate_set * spec->coordinate_set_stride) + spec->floor_view_index;
}

int csb_v1_viewport_floor_ornament_native_bitmap_index(const CSB_V1_ViewportFloorOrnamentBlitSpec *spec,
                                                       int base_native_bitmap_index)
{
    if (!spec || base_native_bitmap_index < 0) return -1;
    return base_native_bitmap_index + spec->native_bitmap_index_increment;
}

size_t csb_v1_viewport_thing_pass_order_spec_count(void)
{
    return sizeof(s_thing_pass_order_routes) / sizeof(s_thing_pass_order_routes[0]);
}

const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec(size_t index)
{
    if (index >= csb_v1_viewport_thing_pass_order_spec_count()) return NULL;
    return &s_thing_pass_order_routes[index];
}

const CSB_V1_ViewportThingPassOrderSpec *csb_v1_viewport_get_thing_pass_order_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_thing_pass_order_spec_count(); ++i) {
        if (s_thing_pass_order_routes[i].view_square == view_square) {
            return &s_thing_pass_order_routes[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_object_visibility_spec_count(void)
{
    return sizeof(s_object_visibility_routes) / sizeof(s_object_visibility_routes[0]);
}

const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec(size_t index)
{
    if (index >= csb_v1_viewport_object_visibility_spec_count()) return NULL;
    return &s_object_visibility_routes[index];
}

const CSB_V1_ViewportObjectVisibilitySpec *csb_v1_viewport_get_object_visibility_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_object_visibility_spec_count(); ++i) {
        if (s_object_visibility_routes[i].view_square == view_square) {
            return &s_object_visibility_routes[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_object_visibility_allows_cell(const CSB_V1_ViewportObjectVisibilitySpec *spec,
                                                  unsigned char cell_ordinal)
{
    if (!spec) return 0;
    if (cell_ordinal < spec->first_visible_cell_ordinal) return 0;
    if (cell_ordinal > spec->last_visible_cell_ordinal) return 0;
    return 1;
}

size_t csb_v1_viewport_object_blit_spec_count(void)
{
    return sizeof(s_object_blits) / sizeof(s_object_blits[0]);
}

const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_object_blit_spec_count()) return NULL;
    return &s_object_blits[index];
}

const CSB_V1_ViewportObjectBlitSpec *csb_v1_viewport_get_object_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_object_blit_spec_count(); ++i) {
        if (s_object_blits[i].view_square == view_square) {
            return &s_object_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_object_blit_layout_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                            unsigned char view_cell)
{
    if (!spec || view_cell > 4 || spec->object_visibility_row < 0) return -1;
    return spec->object_zone_base +
           (spec->object_visibility_row * spec->object_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_object_blit_zone(const CSB_V1_ViewportObjectBlitSpec *spec,
                                     unsigned char view_cell)
{
    int zone = csb_v1_viewport_object_blit_layout_zone(spec, view_cell);
    if (zone < 0) return -1;
    return zone | spec->shifts_objects_and_creatures;
}

size_t csb_v1_viewport_projectile_blit_spec_count(void)
{
    return sizeof(s_projectile_blits) / sizeof(s_projectile_blits[0]);
}

const CSB_V1_ViewportProjectileBlitSpec *csb_v1_viewport_get_projectile_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_projectile_blit_spec_count()) return NULL;
    return &s_projectile_blits[index];
}

const CSB_V1_ViewportProjectileBlitSpec *csb_v1_viewport_get_projectile_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_projectile_blit_spec_count(); ++i) {
        if (s_projectile_blits[i].view_square == view_square) {
            return &s_projectile_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_projectile_blit_zone(const CSB_V1_ViewportProjectileBlitSpec *spec,
                                         unsigned char view_cell)
{
    if (!spec || view_cell > 4 || spec->projectile_visibility_row < 0) return -1;
    if (spec->view_depth == 3 && spec->suppresses_depth3_front_cells && view_cell <= 1) {
        return -1;
    }
    if (spec->view_depth == 0 && spec->suppresses_depth0_back_cells && view_cell >= 2) {
        return -1;
    }
    return spec->projectile_zone_base +
           (spec->projectile_visibility_row * spec->projectile_zone_cell_stride) +
           view_cell;
}

int csb_v1_viewport_projectile_blit_pixels(const CSB_V1_ViewportProjectileBlitSpec *spec,
                                           int flip_flags,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride,
                                           int width,
                                           int height)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride < width || destination_stride < width ||
        width <= 0 || height <= 0) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C F0115 lines 5755-5762/5791-5802 build
     * MASK0x0001/MASK0x0002 flip flags dynamically, then lines 5881-5882
     * send the scaled bitmap through F0791 with C10 transparency. */
    for (int y = 0; y < height; ++y) {
        int sy = (flip_flags & CSB_V1_FLIP_VERTICAL) ? (height - 1 - y) : y;
        for (int x = 0; x < width; ++x) {
            int sx = (flip_flags & CSB_V1_FLIP_HORIZONTAL) ? (width - 1 - x) : x;
            uint8_t pixel = source[(sy * source_stride) + sx];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }
    return copied;
}

size_t csb_v1_viewport_creature_visibility_spec_count(void)
{
    return sizeof(s_creature_visibility_routes) / sizeof(s_creature_visibility_routes[0]);
}

const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec(size_t index)
{
    if (index >= csb_v1_viewport_creature_visibility_spec_count()) return NULL;
    return &s_creature_visibility_routes[index];
}

const CSB_V1_ViewportCreatureVisibilitySpec *csb_v1_viewport_get_creature_visibility_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_creature_visibility_spec_count(); ++i) {
        if (s_creature_visibility_routes[i].view_square == view_square) {
            return &s_creature_visibility_routes[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_creature_visibility_zone(const CSB_V1_ViewportCreatureVisibilitySpec *spec,
                                             int coordinate_set,
                                             unsigned char view_cell)
{
    if (!spec || coordinate_set < 0 || view_cell > 4 || spec->creature_visibility_row < 0) {
        return -1;
    }
    return spec->creature_zone_base +
           (coordinate_set * spec->creature_coordinate_set_stride) +
           (spec->creature_visibility_row * spec->creature_zone_cell_stride) +
           view_cell;
}

size_t csb_v1_viewport_explosion_blit_spec_count(void)
{
    return sizeof(s_explosion_blits) / sizeof(s_explosion_blits[0]);
}

const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_explosion_blit_spec_count()) return NULL;
    return &s_explosion_blits[index];
}

const CSB_V1_ViewportExplosionBlitSpec *csb_v1_viewport_get_explosion_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_explosion_blit_spec_count(); ++i) {
        if (s_explosion_blits[i].view_square == view_square) {
            return &s_explosion_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_explosion_rebirth_step1_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    if (!spec || spec->explosion_row < 0) return -1;
    return spec->rebirth_step1_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_rebirth_step2_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    if (!spec || spec->explosion_row < 0) return -1;
    return spec->rebirth_step2_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_centered_zone(const CSB_V1_ViewportExplosionBlitSpec *spec)
{
    if (!spec || spec->explosion_row < 0) return -1;
    return spec->centered_zone_base + spec->explosion_row;
}

int csb_v1_viewport_explosion_side_zone(const CSB_V1_ViewportExplosionBlitSpec *spec,
                                        unsigned char view_cell)
{
    if (!spec || spec->explosion_row < 0 || view_cell > 1) return -1;
    return spec->side_zone_base +
           (spec->explosion_row * spec->side_zone_cell_stride) +
           view_cell;
}

size_t csb_v1_viewport_teleporter_field_spec_count(void)
{
    return sizeof(s_teleporter_fields) / sizeof(s_teleporter_fields[0]);
}

const CSB_V1_ViewportTeleporterFieldSpec *csb_v1_viewport_get_teleporter_field_spec(size_t index)
{
    if (index >= csb_v1_viewport_teleporter_field_spec_count()) return NULL;
    return &s_teleporter_fields[index];
}

const CSB_V1_ViewportTeleporterFieldSpec *csb_v1_viewport_get_teleporter_field_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_teleporter_field_spec_count(); ++i) {
        if (s_teleporter_fields[i].view_square == view_square) {
            return &s_teleporter_fields[i];
        }
    }
    return NULL;
}

size_t csb_v1_viewport_door_panel_blit_spec_count(void)
{
    return sizeof(s_door_panel_blits) / sizeof(s_door_panel_blits[0]);
}

const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec(size_t index)
{
    if (index >= csb_v1_viewport_door_panel_blit_spec_count()) return NULL;
    return &s_door_panel_blits[index];
}

const CSB_V1_ViewportDoorPanelBlitSpec *csb_v1_viewport_get_door_panel_blit_spec_for_square(int view_square)
{
    for (size_t i = 0; i < csb_v1_viewport_door_panel_blit_spec_count(); ++i) {
        if (s_door_panel_blits[i].view_square == view_square) {
            return &s_door_panel_blits[i];
        }
    }
    return NULL;
}

int csb_v1_viewport_door_panel_first_half_zone(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                               int door_state,
                                               int horizontal_door)
{
    if (!spec || door_state < 0) return -1;
    if (spec->skips_open_state && door_state == 0) return -1;
    if (!horizontal_door) return -1;
    if (door_state == 4 || door_state == spec->destroyed_state) return -1;

    /* ReDMCSB: DUNVIEW.C F0111 lines 4298-4311.  Partially-open
     * horizontal PC34/I34 doors blit their first half through
     * P2084_i_ZoneIndex + DoorState + C6_UNKNOWN before the final half. */
    return spec->door_zone_base + door_state + spec->horizontal_first_half_zone_offset;
}

int csb_v1_viewport_door_panel_final_zone(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                          int door_state,
                                          int horizontal_door)
{
    if (!spec || door_state < 0) return -1;
    if (spec->skips_open_state && door_state == 0) return -1;
    if (door_state == 4 || door_state == spec->destroyed_state) {
        return spec->door_zone_base;
    }

    /* ReDMCSB: DUNVIEW.C F0111 lines 4298-4321 shifts the zone by
     * DoorState for partially-open panels.  Horizontal doors add the second
     * half offset and MASK0x4000 before the final F0791 at line 4334. */
    int zone = spec->door_zone_base + door_state;
    if (horizontal_door) {
        zone += spec->horizontal_second_half_zone_offset;
        zone |= CSB_V1_DOOR_HORIZONTAL_FINAL_SHIFT_MASK;
    }
    return zone;
}

int csb_v1_viewport_door_panel_blit_pixels(const CSB_V1_ViewportDoorPanelBlitSpec *spec,
                                           int door_state,
                                           const uint8_t *source,
                                           int source_stride,
                                           uint8_t *destination,
                                           int destination_stride)
{
    int copied = 0;
    if (!spec || !source || !destination ||
        source_stride < spec->native_bitmap_width ||
        destination_stride < spec->clipped_width ||
        spec->clipped_width <= 0 || spec->clipped_height <= 0 ||
        spec->clipped_width > spec->native_bitmap_width ||
        spec->clipped_height > spec->native_bitmap_height) {
        return -1;
    }

    /* ReDMCSB: DUNVIEW.C F0111 lines 4248 and 4334 skip open doors, then
     * blit G0074 through F0791 with C10 transparency; COORD.C 1556-1560
     * clips the native D3 48x41 door bitmap to the 48x40 panel record. */
    if (spec->skips_open_state && door_state == 0) {
        return 0;
    }

    for (int y = 0; y < spec->clipped_height; ++y) {
        for (int x = 0; x < spec->clipped_width; ++x) {
            uint8_t pixel = source[(y * source_stride) + x];
            if (pixel == (uint8_t)spec->transparent_color) continue;
            destination[(y * destination_stride) + x] = pixel;
            ++copied;
        }
    }

    return copied;
}

const char *csb_v1_viewport_source_evidence(void) {
    return
        "ReDMCSB WIP20210206 Toolchains/Common/Source/DUNVIEW.C:\n"
        "  6226-6353 F0676/F0677 back-wall D3L2/D3R2 element routing\n"
        "  6254-6263 F0676 D3L2 wall panel then F0107 right-wall ornament route\n"
        "  6270-6286 F0676 D3L2 F0108 floor ornament, F0115 pass1/pass2, F0111 door panel route\n"
        "  6321-6330 F0677 D3R2 wall panel then F0107 left-wall ornament route\n"
        "  6337-6353 F0677 D3R2 F0108 floor ornament, F0115 pass1/pass2, F0111 door panel route\n"
        "  4567-4581 F0115 draws objects, then creatures, then projectiles per processed view cell\n"
        "  5668-5683 F0115 restarts for projectile things and uses C2900_ZONE_ + G2028 row * 4 + ViewCell\n"
        "  5881-5883 F0115 blits PC34/I34 projectile sprites through the computed C2900 zone\n"
        "  5710-5722 and 5755-5802 F0115 computes PC34/I34 projectile scale and dynamic MASK0x0001/MASK0x0002 flip flags before the C10 F0791 blit\n"
        "  4806-4811 F0115 maps PC34 view square to lane/depth/object visibility rows\n"
        "  4923 F0115 filters weapon..junk objects by visible row, matching cell, and D3/D0 cell gates\n"
        "  5030-5039 F0115 selects PC34/I34 object scale and shift set from depth/cell\n"
        "  5071-5110 F0115 blits objects through C2500_ZONE_ | MASK0x8000 plus G2028 row/cell and pile shifts\n"
        "  375, 5201-5214, 5615-5627 F0115 maps creatures through G2033 and C3200_ZONE_ with MASK0x8000 shifts\n"
        "  5915-5933 F0115 restarts for explosions after all processed view cells\n"
        "  5920-6219 F0115 maps PC34/I34 explosions through G2034/G2035, C3000/C3007/C3014/C3031 zones, F0791 C10 blits, and fluxcage field deferral\n"
        "  6288-6290 and 6355-6357 F0676/F0677 draw teleporter fields through G2035, F0113, and C702/C703 after the F0108/F0115 path\n"
        "  6863-6865 and 6894-6896 F0678/F0679 draw D2L2/D2R2 teleporter fields through G2035, F0113, and C707/C708 without F0108/F0115\n"
        "  6848-6862 and 6879-6893 F0678/F0679 D2L2/D2R2 wall branches swap C06/C05 wall bitmaps under G0076 and return before the teleporter field path\n"
        "  3502-3590, 3817-3829, 3921-3923 F0107 maps CSB/I34 far wall ornaments through C1004 + CoordinateSet*15 + ViewWall, C30/C14 scaling, D3 palette changes, optional D3R2 flip, and F0791 C10 blits\n"
        "  3589, 3608-3753, 3817-3829, 3923-3928 F0107 evaluates F0149 alcove status for C00/C01 but skips D1-only facing state, clickbox copy, and champion portrait overlay while using the I34 D3 CM1_DERIVED_BITMAP_NONE scaled path\n"
        "  6968-6969,7119-7120,7308,7459,7627,7842 F0119-F0124 call F0107 for D2/D1 wall ornaments; D2 uses C21/G0199 derived scaled bitmaps, D1 side uses native CM1_DERIVED_BITMAP_NONE, and only D1C front updates facing/clickbox/portrait state\n"
        "  3940-4008 F0108 floor ornament ordinal/index, G0191 native bitmap increment, C1500 zone, flip, C10 blit dispatch\n"
        "  4218-4337 F0111 door bitmap, ornament, state, zone shift, and C10 transparent blit dispatch\n"
        "  4301-4302 F0111 applies C15_DOOR_ORNAMENT_DESTROYED_MASK for C5_DOOR_STATE_DESTROYED\n"
        "  6837-6896 F0678/F0679 near-wall D2L2/D2R2 element routing\n"
        "  6848-6865 F0678 and 6877-6896 F0679 return for walls without F0107\n"
        "  8337-8339 F0128 draws the standard floor/ceiling baseline before square draws; F0098 2962-3002 draws G2109/G2108 and clears the request\n"
        "  8318-8542 F0128 shared viewport draw sequence\n"
        "  1008-1017 G0195 CSB/I34 floor ornament coordinate-set indices are all 0\n"
        "  DEFS.H:2750-2751 C00_VIEW_FLOOR_D3L2 / C01_VIEW_FLOOR_D3R2\n"
        "  DEFS.H:4250-4251 C3700_ZONE_DOOR_D3L2 / C3710_ZONE_DOOR_D3R2\n"
        "  DEFS.H:4223 C1500_ZONE_FLOOR_ORNAMENT; COORD.C:903-913 floor ornament zone records\n"
        "  DEFS.H:4222 C1004_ZONE_WALL_ORNAMENT; COORD.C:921-1025 wall ornament zone records\n"
        "  DEFS.H:2703-2710 M580..M587 D2/D1 view-wall indices\n"
        "  DEFS.H:4228 C2500_ZONE_; COORD.C:1129-1193 object zone records\n"
        "  DEFS.H:3517 MASK0x8000_SHIFT_OBJECTS_AND_CREATURES; 4236 C3200_ZONE_; COORD.C:1248-1251,2074-2075 creature zones\n"
        "  DEFS.H:4042-4048 C702/C703/C707/C708 field zones; 4232-4235 explosion zone bases; COORD.C:1058-1123,1194-1238 explosion zone records\n"
        "  DEFS.H:3428-3429 C05_WALL_D2R2 / C06_WALL_D2L2\n"
        "  COORD.C:1548-1565 D3 48x41 native door bitmap and 48x40 clip records; 788-807 far door zones\n"
        "  G0711/G0712 back-wall frame descriptors (lines 579-580)\n"
        "  G2107 WallSet bitmap indices (lines ~183)\n"
        "  G3048 WallSetFlipped (lines 277-295)\n"
        "ReDMCSB DEFS.H:2696-2697 C00_VIEW_WALL_D3L2_RIGHT / C01_VIEW_WALL_D3R2_LEFT\n"
        "ReDMCSB DUNGEON.C:1330-1347 F0149_DUNGEON_IsWallOrnamentAnAlcove scans C003_ALCOVE_ORNAMENT_COUNT\n"
        "CSBWin/Viewport.cpp: 7290 lines viewport rendering\n"
        "CSBWin/Viewport.cpp:5317-5325 relposSid/relposFwd; 5402-5412 GetBitmap CSD/CSD-I34 bitmap selection; 6444-6470 ApplyBackground masked composite; 6567-6615 CustomBackgrounds skin/mask/bitmap apply; 6919-7140 sixteen background room slots before cell draws\n"
        "CSBWin/Graphics.cpp: 3186 lines asset cache\n"
        "CSBWin/CSBCode.cpp:26 CustomBackgrounds\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack (prison door)\n";
}
