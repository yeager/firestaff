#ifndef FIRESTAFF_DM1_V1_VIEWPORT_3D_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_3D_PC34_COMPAT_H

/*
 * DM1 V1 Viewport 3D Wall Rendering Pipeline — pc34 compat layer.
 *
 * Source-locked to ReDMCSB WIP20210206, Toolchains/Common/Source/:
 *   VIEWPORT.C  (99 lines)  — F0564-F0566: bitplane init, palette, blit-to-screen
 *   DUNVIEW.C   (8619 lines) — F0096-F0128: wall/door/stair drawing, main draw loop
 *   DRAWVIEW.C  (1237 lines) — F0097, F0694, F0695: palette/viewport blit
 *
 * This module implements the core 3D dungeon viewport wall rendering:
 *   1. Viewport geometry (224×136 pixel viewport within 320×200 screen)
 *   2. Wall set bitmap management (native + flipped variants per depth)
 *   3. Depth-ordered square drawing (D3→D2→D1→D0, L→R→C per depth)
 *   4. Wall projection at each depth with correct clipping rectangles
 *   5. Door frame and door bitmap compositing
 *   6. Parity-based wall flip for visual variety
 *   7. PAL vblank timing integration
 *
 * ReDMCSB Function Map (DUNVIEW.C):
 *   F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF (line 2225):
 *     Loads wall set bitmaps, creates flipped variants, sets up door frames.
 *   F0098_DUNGEONVIEW_DrawFloorAndCeiling (line 2962):
 *     Clears viewport black area, copies floor/ceiling bitmaps.
 *   F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal (line 3018):
 *     Creates horizontally flipped copy of a wall bitmap.
 *   F0100_DUNGEONVIEW_DrawWallSetBitmap (line 3048):
 *     Blits a wall set bitmap into viewport with transparency.
 *   F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency (line 3065):
 *     Blits wall bitmap without transparency (center walls, perf opt).
 *   F0102_DUNGEONVIEW_DrawDoorBitmap (line 3082):
 *     Blits door bitmap from temporary buffer.
 *   F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally (line 3096):
 *     Flips door frame bitmap, then blits.
 *   F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap (line 3113):
 *     Generic depth-positioned bitmap draw.
 *   F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally (line 3185):
 *     Flipped variant of F0104.
 *   F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF (line 3502):
 *     Draws wall ornament, returns true if alcove.
 *   F0111_DUNGEONVIEW_DrawDoor (line 4218):
 *     Full door draw: state-dependent bitmap selection, ornament overlay.
 *   F0116_DUNGEONVIEW_DrawSquareD3L (line 6361):
 *     Draw square at depth 3, left lane.
 *   F0117_DUNGEONVIEW_DrawSquareD3R (line 6500):
 *     Draw square at depth 3, right lane.
 *   F0118_DUNGEONVIEW_DrawSquareD3C_CPSF (line 6642):
 *     Draw square at depth 3, center lane.
 *   F0119_DUNGEONVIEW_DrawSquareD2L (line 6900):
 *     Draw square at depth 2, left lane.
 *   F0120_DUNGEONVIEW_DrawSquareD2R_CPSF (line 7051):
 *     Draw square at depth 2, right lane.
 *   F0121_DUNGEONVIEW_DrawSquareD2C (line 7244):
 *     Draw square at depth 2, center lane.
 *   F0122_DUNGEONVIEW_DrawSquareD1L (line 7391):
 *     Draw square at depth 1, left lane.
 *   F0123_DUNGEONVIEW_DrawSquareD1R (line 7559):
 *     Draw square at depth 1, right lane.
 *   F0124_DUNGEONVIEW_DrawSquareD1C (line 7727):
 *     Draw square at depth 1, center lane.
 *   F0125_DUNGEONVIEW_DrawSquareD0L (line 7960):
 *     Draw square at depth 0, left lane.
 *   F0126_DUNGEONVIEW_DrawSquareD0R (line 8064):
 *     Draw square at depth 0, right lane.
 *   F0127_DUNGEONVIEW_DrawSquareD0C (line 8164):
 *     Draw square at depth 0, center lane.
 *   F0128_DUNGEONVIEW_Draw_CPSF (line 8318):
 *     Main entry: allocates temp bitmap, sets parity flip, iterates
 *     all visible squares back-to-front calling F0116-F0127, then
 *     restores wall pointers and triggers F0097 blit.
 *
 * ReDMCSB Function Map (VIEWPORT.C):
 *   F0564_VIEWPORT_InitializeBitPlanes (line 16):
 *     Sets up 4 source/dest bitplane pointers for 224×136 viewport.
 *   F0565_VIEWPORT_SetPalette (line 33):
 *     Waits for vblank, sets copper palette, triggers F0566 blit.
 *   F0566_VIEWPORT_BlitToScreen (line 56):
 *     Amiga blitter copy: 4 bitplanes, 224×136 → screen at line 33.
 *
 * ReDMCSB Function Map (DRAWVIEW.C):
 *   F0097_DUNGEONVIEW_DrawViewport (line varies per platform):
 *     Platform-specific viewport-to-screen transfer with palette switch.
 *   F0694_SetMultipleColorsInPalette (FM Towns, line ~1050):
 *     Sets palette colors from indexed palette table.
 *   F0695_SetCreatureReplacementColors (FM Towns, line ~1070):
 *     Patches creature colors into light-level palettes.
 *
 * Viewport Geometry (from VIEWPORT.C / DUNVIEW.C):
 *   Viewport pixel dimensions: 224 wide × 136 tall
 *   Screen position: starts at line 33 (below status bar)
 *   Byte width (Amiga bitplane): 224/8 = 28 bytes
 *   Viewport byte width (pc34 chunky): 224 bytes
 *   Black area: top 37 lines of viewport (above ceiling line 29)
 *   Floor area: bottom 70 lines (below midpoint at line 66)
 *   Ceiling area: top 29 lines
 *
 * Wall Set Layout (from DUNVIEW.C globals, Amiga A20E section):
 *   Each wall set provides bitmaps for these depth/lane positions:
 *     D3L2, D3R2, D3L, D3C, D3R
 *     D2L, D2C, D2R
 *     D1L, D1C, D1R
 *     D0L, D0R
 *   Door frames: Left/Right at D3L, D3C, D2C, D1C, D0C
 *   Door frame tops: D2L, D2C, D2R, D1L, D1C, D1R
 *
 * Parity Flip (DUNVIEW.C F0128, line 8357):
 *   G0076 = (mapX + mapY + direction) & 1
 *   When set: floor bitmap is flipped, wall pointers swap to
 *   pre-flipped variants (G0090-G0094 for Amiga, G3048 for I34E).
 *
 * PAL Timing (VIEWPORT.C):
 *   Amiga: VBeamPos() >= 200 triggers vblank wait
 *   PAL refresh: 50 Hz, ~20ms per frame
 *   Our implementation uses dm1_v1_vblank_timing for frame pacing.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ────────────────────────────────────────────────────────────────────────────
 * Viewport Constants (from ReDMCSB VIEWPORT.C / DUNVIEW.C)
 * ──────────────────────────────────────────────────────────────────────── */

/* Viewport pixel dimensions — VIEWPORT.C M091_BITPLANE_SIZE(224, 136) */
#define DM1_VIEWPORT_WIDTH          224
#define DM1_VIEWPORT_HEIGHT         136

/* Screen geometry — VIEWPORT.C line 27: viewport starts at line 33 */
#define DM1_VIEWPORT_SCREEN_X       0
#define DM1_VIEWPORT_SCREEN_Y       33

/* Viewport sub-regions — DUNVIEW.C F0098 */
#define DM1_VIEWPORT_BLACK_AREA_H   37   /* Lines 0..36: cleared to black */
#define DM1_VIEWPORT_CEILING_H      29   /* Lines 0..28: ceiling bitmap */
#define DM1_VIEWPORT_FLOOR_H        70   /* Lines 66..135: floor bitmap */
#define DM1_VIEWPORT_FLOOR_Y        66   /* Floor area starts at viewport line 66 */

/* Amiga byte widths — VIEWPORT.C */
#define DM1_VIEWPORT_BYTE_WIDTH     (DM1_VIEWPORT_WIDTH)  /* pc34 chunky: 1 bpp */
#define DM1_SCREEN_BYTE_WIDTH       320

/* ────────────────────────────────────────────────────────────────────────────
 * Depth/Lane Position Enumerations
 * (from DUNVIEW.C F0116-F0128 square naming, DEFS.H view square macros)
 * ──────────────────────────────────────────────────────────────────────── */

/* View square indices — correspond to ReDMCSB M597-M611 macros */
typedef enum {
    DM1_VIEW_SQUARE_D4C = 0,   /* M597 */
    DM1_VIEW_SQUARE_D4L,       /* M598 */
    DM1_VIEW_SQUARE_D4R,       /* M599 */
    DM1_VIEW_SQUARE_D3C,       /* M600 */
    DM1_VIEW_SQUARE_D3L,       /* M601 */
    DM1_VIEW_SQUARE_D3R,       /* M602 */
    DM1_VIEW_SQUARE_D2C,       /* M603 */
    DM1_VIEW_SQUARE_D2L,       /* M604 */
    DM1_VIEW_SQUARE_D2R,       /* M605 */
    DM1_VIEW_SQUARE_D1C,       /* M606 */
    DM1_VIEW_SQUARE_D1L,       /* M607 */
    DM1_VIEW_SQUARE_D1R,       /* M608 */
    DM1_VIEW_SQUARE_D0C,       /* M609 */
    DM1_VIEW_SQUARE_D0L,       /* M610 */
    DM1_VIEW_SQUARE_D0R,       /* M611 */
    DM1_VP_VIEW_SQUARE_COUNT
} DM1_ViewSquareIndex;

/* Wall set bitmap indices — from DUNVIEW.C G2107_WallSet[15] (I34E) */
typedef enum {
    DM1_WALL_D0R = 0,
    DM1_WALL_D0L,
    DM1_WALL_D1R,
    DM1_WALL_D1L,
    DM1_WALL_D1C,
    DM1_WALL_D2R2,
    DM1_WALL_D2L2,
    DM1_WALL_D2R,
    DM1_WALL_D2L,
    DM1_WALL_D2C,
    DM1_WALL_D3R2,
    DM1_WALL_D3L2,
    DM1_WALL_D3R,
    DM1_WALL_D3L,
    DM1_WALL_D3C,
    DM1_WALL_SET_COUNT           /* 15 entries */
} DM1_WallSetIndex;

/* Door frame indices — from DUNVIEW.C G2110-G2122 (I34E) */
typedef enum {
    DM1_DOOR_FRAME_TOP_D1R = 0,
    DM1_DOOR_FRAME_TOP_D1L,
    DM1_DOOR_FRAME_TOP_D1LCR,
    DM1_DOOR_FRAME_TOP_D2R,
    DM1_DOOR_FRAME_TOP_D2L,
    DM1_DOOR_FRAME_TOP_D2LCR,
    DM1_DOOR_FRAME_FRONT_D0C,
    DM1_DOOR_FRAME_RIGHT_D1C,
    DM1_DOOR_FRAME_LEFT_D1C,
    DM1_DOOR_FRAME_LEFT_D2C,
    DM1_DOOR_FRAME_LEFT_D3C,
    DM1_DOOR_FRAME_LEFT_D3L,
    DM1_DOOR_FRAME_COUNT
} DM1_DoorFrameIndex;

/* Square element types — from DUNVIEW.C switch cases.
 * Guarded: dm1_v1_dungeon_square_structs defines these as macros. */
typedef enum {
    DM1_VP_ELEMENT_WALL        = 0,
    DM1_VP_ELEMENT_CORRIDOR    = 1,
    DM1_VP_ELEMENT_PIT         = 2,
    DM1_VP_ELEMENT_TELEPORTER  = 5,
    DM1_VP_ELEMENT_DOOR_SIDE   = 16,
    DM1_VP_ELEMENT_DOOR_FRONT  = 17,
    DM1_VP_ELEMENT_STAIRS_SIDE = 18,
    DM1_VP_ELEMENT_STAIRS_FRONT = 19
} DM1_SquareElement;

/* ────────────────────────────────────────────────────────────────────────────
 * Wall Frame Descriptor
 * Encodes the viewport-relative clip rectangle for a wall bitmap at a
 * given depth/lane position. Mirrors the 8-byte frame arrays from
 * ReDMCSB DUNVIEW.C (G0163_aauc_Graphic558_Frame_Walls[12][8]).
 * ──────────────────────────────────────────────────────────────────────── */


/* F0115 thing-layer phases inside each processed view cell.
 * Source: ReDMCSB DUNVIEW.C:4561-4581, 4853-4860, 5195-5202,
 *         5681-5883, 5915-5933.
 */
typedef enum {
    DM1_VIEWPORT_THING_LAYER_OBJECTS = 0,
    DM1_VIEWPORT_THING_LAYER_CREATURES = 1,
    DM1_VIEWPORT_THING_LAYER_PROJECTILES = 2,
    DM1_VIEWPORT_THING_LAYER_EXPLOSIONS = 3
} DM1_ViewportThingLayer;

typedef struct {
    uint16_t cell_order;
    unsigned char cells[4];
    unsigned char cell_count;
    unsigned char door_pass; /* 0=no door pass, 1=behind door/frame, 2=in front */
    bool alcove;
} DM1_ViewportCellOrder;

typedef struct {
    DM1_ViewportThingLayer layer;
    const char *name;
    const char *source_lines;
    /* ReDMCSB F0115 executes object/creature/projectile phases for each
     * packed view cell, then restarts once after all cells for explosions.
     * Source: DUNVIEW.C:4567-4581, 5915-5933. */
    bool repeats_per_cell;
    bool after_all_cells;
} DM1_ViewportThingLayerSpec;

/* PC34/I34E projectile visibility and zone contract for F0115.  ReDMCSB
 * maps MEDIA720 view-square ids through G2028, clips D3 front cells and D0
 * back cells, then draws through C2900_ZONE_ + row*4 + viewCell.
 * Source: DUNVIEW.C:373, 5667-5683, 5710-5715, 5881-5883. */
typedef struct {
    DM1_ViewSquareIndex square;
    int8_t redmcsb_view_square_id;
    int8_t view_depth;
    int8_t g2028_row;
    const char *source_lines;
} DM1_ViewportProjectileOcclusionSpec;

/* Door-front occlusion contract for ReDMCSB F0116/F0118/F0121/F0124
 * front-door branches: rear cells are drawn with F0115 before the frame and
 * door bitmap, then front cells are drawn with a second F0115 pass after the
 * door. */
typedef struct {
    DM1_ViewSquareIndex square;
    uint16_t rear_cell_order;
    uint16_t front_cell_order;
    const char *floor_source_lines;
    const char *rear_pass_source_lines;
    const char *frame_source_lines;
    const char *button_source_lines;
    const char *door_source_lines;
    const char *front_pass_source_lines;
} DM1_ViewportDoorFrontOcclusionSpec;

/* Side-door/stairs-side branches do not draw a door bitmap in front of the
 * cell contents.  ReDMCSB still routes them through F0115 with a square-
 * specific packed cell order, which defines which side sub-cells are visible
 * around the side wall/stairs silhouette. */
typedef struct {
    DM1_ViewSquareIndex square;
    uint16_t cell_order;
    const char *function_name;
    const char *branch_source_lines;
    const char *f0115_source_lines;
} DM1_ViewportSideOcclusionSpec;

/* D0C door-side Thieves Eye occlusion contract.  ReDMCSB composes the
 * hole-in-wall graphic into a temporary D0C door-frame bitmap before blitting
 * that temporary frame, then falls through to the common D0C F0115 thing pass.
 */
typedef struct {
    DM1_ViewSquareIndex square;
    uint16_t cell_order;
    uint16_t door_frame_zone;
    uint16_t hole_zone;
    const char *branch_source_lines;
    const char *copy_source_lines;
    const char *hole_source_lines;
    const char *frame_blit_source_lines;
    const char *f0115_source_lines;
} DM1_ViewportThievesEyeDoorFrameOcclusionSpec;

typedef struct {
    bool command_mutates_before_draw;
    bool redraw_uses_party_tuple;
    bool present_waits_for_viewport;
    const char *command_source_lines;
    const char *main_loop_source_lines;
    const char *present_source_lines;
} DM1_ViewportPostCommandRedrawSpec;

typedef struct {
    DM1_ViewSquareIndex square;
    uint16_t cell_order;
    bool stairs_draw_before_floor_ornament;
    bool pit_draw_before_floor_ornament;
    bool floor_ornament_before_things;
    bool objects_creatures_projectiles_before_explosions;
    bool field_after_things;
    bool d0c_foreground_before_things;
    bool wall_case_returns_before_things;
    const char *function_name;
    const char *stairs_source_lines;
    const char *pit_source_lines;
    const char *floor_ornament_source_lines;
    const char *things_source_lines;
    const char *field_source_lines;
    const char *wall_return_source_lines;
} DM1_ViewportFloorFieldOrderSpec;

typedef struct {
    DM1_ViewSquareIndex square;
    int8_t rel_depth;
    int8_t rel_lateral;
    const char *redmcsb_function;
    const char *source_lines;
} DM1_ViewportDrawStep;

typedef struct {
    DM1_ViewSquareIndex square;
    int16_t map_x;
    int16_t map_y;
    int8_t rel_depth;
    int8_t rel_lateral;
    const char *redmcsb_function;
    const char *source_lines;
} DM1_ViewportResolvedDrawStep;

/* Source-locked PC34/I34E wall bitmap selection for a wall square.
 *
 * ReDMCSB PC34 selects the opposite left/right bitmap and requests
 * F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally when
 * G0076_B_UseFlippedWallAndFootprintsBitmaps is set.  Center walls keep
 * the same index and pass G0076 to F0792_DUNGEONVIEW_DrawBitmapYYY.
 */
typedef struct {
    DM1_ViewSquareIndex square;
    DM1_WallSetIndex native_wall;
    DM1_WallSetIndex parity_wall;
    bool parity_flips_horizontally;
    bool center_wall;
    uint16_t pc34_zone;
    bool wall_case_returns;
    bool front_alcove_reveals_contents;
    const char *redmcsb_function;
    const char *source_lines;
    const char *occlusion_source_lines;
} DM1_ViewportWallDrawSpec;

/* MEDIA720-only side-wall squares used by the PC34/I34E ReDMCSB draw path.
 * They are outside the original M597-M611 dense enum, so use stable negative
 * identifiers for metadata/probe reporting only. */
#define DM1_VIEW_SQUARE_D3L2 ((DM1_ViewSquareIndex)-101)
#define DM1_VIEW_SQUARE_D3R2 ((DM1_ViewSquareIndex)-102)
#define DM1_VIEW_SQUARE_D2L2 ((DM1_ViewSquareIndex)-103)
#define DM1_VIEW_SQUARE_D2R2 ((DM1_ViewSquareIndex)-104)

/* PC34/I34E viewport zone ids from ReDMCSB DEFS.H:4040-4057. */
#define DM1_PC34_ZONE_VIEWPORT_CEILING_AREA 700
#define DM1_PC34_ZONE_VIEWPORT_FLOOR_AREA   701
#define DM1_PC34_ZONE_WALL_D3L2             702
#define DM1_PC34_ZONE_WALL_D3R2             703
#define DM1_PC34_ZONE_WALL_D3C              704
#define DM1_PC34_ZONE_WALL_D3L              705
#define DM1_PC34_ZONE_WALL_D3R              706
#define DM1_PC34_ZONE_WALL_D2L2             707
#define DM1_PC34_ZONE_WALL_D2R2             708
#define DM1_PC34_ZONE_WALL_D2C              709
#define DM1_PC34_ZONE_WALL_D2L              710
#define DM1_PC34_ZONE_WALL_D2R              711
#define DM1_PC34_ZONE_WALL_D1C              712
#define DM1_PC34_ZONE_WALL_D1L              713
#define DM1_PC34_ZONE_WALL_D1R              714
#define DM1_PC34_ZONE_WALL_D0C              715
#define DM1_PC34_ZONE_WALL_D0L              716
#define DM1_PC34_ZONE_WALL_D0R              717

/* PC34/I34E D2C front-door frame zones from ReDMCSB DEFS.H:4082-4088. */
#define DM1_PC34_ZONE_DOOR_FRAME_LEFT_D2C   724
#define DM1_PC34_ZONE_DOOR_FRAME_RIGHT_D2C  725
#define DM1_PC34_ZONE_DOOR_FRAME_TOP_D2C    730

typedef struct {
    uint8_t left_x;     /* Viewport-relative clip left X (pixel) */
    uint8_t right_x;    /* Viewport-relative clip right boundary */
    uint8_t top_y;      /* Viewport-relative clip top Y */
    uint8_t bottom_y;   /* Viewport-relative clip bottom Y */
    uint8_t byte_width; /* Source bitmap row width in the active port */
    uint8_t height;     /* Source bitmap height in lines */
    uint8_t blit_x;     /* Source X offset passed to F0132/F0684 */
    uint8_t blit_y;     /* Source Y offset passed to F0132/F0684 */
} DM1_WallFrame;

/* Resolved source/destination rectangle for wall blits.  ReDMCSB passes the
 * frame/zone as the destination clip and frame[C6]/frame[C7] (or the bitmap
 * struct X/Y after F0635_) as the first source pixel.  This gate clips both
 * the viewport rectangle and the source rows, and marks fully occluded/empty
 * blits invisible before any pixel copy. */
typedef struct {
    bool visible;
    int16_t src_x;
    int16_t src_y;
    int16_t dst_x;
    int16_t dst_y;
    int16_t width;
    int16_t height;
    const char *source_lines;
} DM1_ViewportBlitClipGate;

/* ────────────────────────────────────────────────────────────────────────────
 * Viewport 3D Wall Rendering State
 * Aggregates all state needed for one frame of 3D dungeon rendering.
 * ──────────────────────────────────────────────────────────────────────── */

typedef struct {
    /* Viewport framebuffer (224×136, 8-bit indexed) */
    uint8_t *viewport_pixels;
    int      viewport_stride;

    /* Floor area sub-buffer pointer (viewport_pixels + FLOOR_Y * stride) */
    uint8_t *floor_area;

    /* Temporary bitmap for flipping operations.
     * Must be at least as large as the largest wall/door bitmap.
     * ReDMCSB F0128: allocates M075_BITMAP_BYTE_COUNT(160, 111). */
    uint8_t *temp_bitmap;
    int      temp_bitmap_size;

    /* Wall set bitmap indices for current map (15 entries).
     * Negative indices = GRAPHICS.DAT derived bitmap offsets.
     * From DUNVIEW.C G2107_WallSet[15]. */
    int16_t  wall_set[DM1_WALL_SET_COUNT];

    /* Flipped wall set (used when parity is set).
     * From DUNVIEW.C G3048_WallSetFlipped[15]. */
    int16_t  wall_set_flipped[DM1_WALL_SET_COUNT];

    /* Wall set not-flipped backup.
     * From DUNVIEW.C G3071_WallSetNotFlipped[15]. */
    int16_t  wall_set_native[DM1_WALL_SET_COUNT];

    /* Door frame bitmap indices (12 entries).
     * From DUNVIEW.C G2110-G2122. */
    int16_t  door_frames[DM1_DOOR_FRAME_COUNT];

    /* Door front bitmap indices per depth (2 entries per depth).
     * D3: G0693[2], D2: G0694[2], D1: G0695[2]. */
    int16_t  door_front_d3[2];
    int16_t  door_front_d2[2];
    int16_t  door_front_d1[2];

    /* Floor/ceiling graphic indices.
     * From DUNVIEW.C G2108_Floor, G2109_Ceiling. */
    int16_t  floor_graphic;
    int16_t  ceiling_graphic;

    /* Parity flip flag.
     * G0076_B_UseFlippedWallAndFootprintsBitmaps = (mapX+mapY+dir) & 1 */
    bool     parity_flip;

    /* Stairs bitmap indices (18 entries).
     * From DUNVIEW.C G0079_ai_StairsNativeBitmapIndices. */
    int16_t  stairs_indices[18];

    /* Current party position + direction for coordinate transforms */
    int16_t  party_map_x;
    int16_t  party_map_y;
    int16_t  party_direction;   /* 0=N, 1=E, 2=S, 3=W */
    int16_t  party_map_index;

    /* Wall ornament info (16 entries per type).
     * From DUNVIEW.C G0101-G0103. */
    /* (Ornament rendering is handled by separate ornament module) */

    /* Current dungeon view palette index (0-5 = light levels).
     * From DRAWVIEW.C G0304_i_DungeonViewPaletteIndex. */
    int16_t  palette_index;

    /* Whether floor and ceiling need redraw.
     * From DUNVIEW.C G0297_B_DrawFloorAndCeilingRequested. */
    bool     floor_ceiling_dirty;

} DM1_Viewport3DState;

/* ────────────────────────────────────────────────────────────────────────────
 * Public API
 * ──────────────────────────────────────────────────────────────────────── */

/*
 * Initialize the 3D viewport state.
 * Must be called once after allocating viewport framebuffer.
 *
 * viewport_pixels: pointer to 224×136 pixel buffer
 * viewport_stride: bytes per row (typically 224)
 *
 * Source: VIEWPORT.C F0564_VIEWPORT_InitializeBitPlanes (line 16)
 */
void dm1_viewport_3d_init(DM1_Viewport3DState *state,
                          uint8_t *viewport_pixels,
                          int viewport_stride);

/*
 * Load wall set graphics for the current map.
 * Populates wall_set[], wall_set_flipped[], door_frames[], etc.
 *
 * wall_set_index: the wall set number for the current dungeon level
 * floor_set_index: the floor set number
 *
 * Source: DUNVIEW.C F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF (line 2225)
 */
void dm1_viewport_3d_load_wall_set(DM1_Viewport3DState *state,
                                   int wall_set_index,
                                   int floor_set_index);

/*
 * Draw floor and ceiling into viewport framebuffer.
 * Clears black area, copies ceiling (top 29 lines), floor (bottom 70 lines).
 *
 * Source: DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling (line 2962)
 */
void dm1_viewport_3d_draw_floor_ceiling(DM1_Viewport3DState *state);

/*
 * Draw a wall bitmap at a given depth/lane position.
 * Uses transparency color 10 (C10_COLOR_FLESH).
 *
 * Source: DUNVIEW.C F0100_DUNGEONVIEW_DrawWallSetBitmap (line 3048)
 */
void dm1_viewport_3d_draw_wall(DM1_Viewport3DState *state,
                               const uint8_t *wall_bitmap,
                               const DM1_WallFrame *frame);

/*
 * Draw a wall bitmap without transparency (optimization for center walls).
 *
 * Source: DUNVIEW.C F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency (line 3065)
 */
void dm1_viewport_3d_draw_wall_opaque(DM1_Viewport3DState *state,
                                      const uint8_t *wall_bitmap,
                                      const DM1_WallFrame *frame);

/*
 * Draw a door bitmap from temporary buffer.
 *
 * Source: DUNVIEW.C F0102_DUNGEONVIEW_DrawDoorBitmap (line 3082)
 */
void dm1_viewport_3d_draw_door(DM1_Viewport3DState *state,
                               const DM1_WallFrame *frame);

/*
 * Draw a door frame bitmap, flipping it horizontally first.
 *
 * Source: DUNVIEW.C F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally (line 3096)
 */
void dm1_viewport_3d_draw_door_frame_flipped(DM1_Viewport3DState *state,
                                             const uint8_t *frame_bitmap,
                                             const DM1_WallFrame *frame);

/*
 * Execute one full frame of 3D viewport wall rendering.
 * This is the main entry point — calls all DrawSquare functions in
 * depth-order: D3L→D3R→D3C → D2L→D2R→D2C → D1L→D1R→D1C → D0L→D0R→D0C.
 *
 * direction: party facing direction (0-3)
 * map_x, map_y: party position on current map
 *
 * Source: DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF (line 8318)
 */
void dm1_viewport_3d_draw_frame(DM1_Viewport3DState *state,
                                int direction, int map_x, int map_y);

/*
 * Transfer viewport framebuffer to screen with palette switching.
 * Handles vblank synchronization for PAL timing.
 *
 * palette_switching: 0=inventory palette, 1=dungeon palette, 2=as-before
 *
 * Source: DRAWVIEW.C F0097_DUNGEONVIEW_DrawViewport (varies per platform)
 *         VIEWPORT.C F0565_VIEWPORT_SetPalette (line 33)
 *         VIEWPORT.C F0566_VIEWPORT_BlitToScreen (line 56)
 */
void dm1_viewport_3d_present(DM1_Viewport3DState *state,
                             uint8_t *screen_pixels,
                             int screen_stride,
                             int palette_switching);

/*
 * Copy bitmap and flip horizontally into destination buffer.
 * Used for creating mirror-image wall variants.
 *
 * Source: DUNVIEW.C F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal (line 3018)
 */
void dm1_viewport_3d_copy_and_flip_h(const uint8_t *src, uint8_t *dst,
                                     int width, int height);

/*
 * Get viewport wall frame data for a given view square.
 * Returns a pointer to the frame descriptor, or NULL if invalid.
 *
 * Source: DUNVIEW.C G0163_aauc_Graphic558_Frame_Walls[12][8]
 */
const DM1_WallFrame *dm1_viewport_3d_get_wall_frame(DM1_ViewSquareIndex square);
DM1_ViewportBlitClipGate dm1_viewport_3d_resolve_wall_blit_clip_gate(const DM1_WallFrame *frame,
                                                                      int source_width,
                                                                      int source_height);

/* Source-locked F0128 visible-square draw order metadata. */
size_t dm1_viewport_3d_draw_order_count(void);
const DM1_ViewportDrawStep *dm1_viewport_3d_get_draw_order_step(size_t index);
int dm1_viewport_3d_resolve_relative_map_xy(int direction,
                                            int rel_depth,
                                            int rel_lateral,
                                            int origin_x,
                                            int origin_y,
                                            int16_t *out_x,
                                            int16_t *out_y);
int dm1_viewport_3d_resolve_draw_order_step(size_t index,
                                            int direction,
                                            int origin_x,
                                            int origin_y,
                                            DM1_ViewportResolvedDrawStep *out_step);
size_t dm1_viewport_3d_wall_draw_spec_count(void);
const DM1_ViewportWallDrawSpec *dm1_viewport_3d_get_wall_draw_spec(size_t index);
const DM1_ViewportWallDrawSpec *dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_ViewSquareIndex square);
DM1_WallSetIndex dm1_viewport_3d_select_wall_bitmap(const DM1_ViewportWallDrawSpec *spec, bool parity_flip, bool *flip_horizontally);
bool dm1_viewport_3d_wall_occludes_floor_items(const DM1_ViewportWallDrawSpec *spec, bool front_alcove);
uint16_t dm1_viewport_3d_wall_item_cell_order(const DM1_ViewportWallDrawSpec *spec, bool front_alcove);

/* Decode F0115's packed cell-order nibbles (DUNVIEW.C:4561-4564). */
DM1_ViewportCellOrder dm1_viewport_3d_decode_cell_order(uint16_t order);
size_t dm1_viewport_3d_thing_layer_spec_count(void);
const DM1_ViewportThingLayerSpec *dm1_viewport_3d_get_thing_layer_spec(size_t index);
size_t dm1_viewport_3d_projectile_occlusion_spec_count(void);
const DM1_ViewportProjectileOcclusionSpec *dm1_viewport_3d_get_projectile_occlusion_spec(size_t index);
const DM1_ViewportProjectileOcclusionSpec *dm1_viewport_3d_get_projectile_occlusion_spec_for_square(DM1_ViewSquareIndex square);
int dm1_viewport_3d_projectile_zone_for_cell(const DM1_ViewportProjectileOcclusionSpec *spec, unsigned char view_cell);
int dm1_viewport_3d_projectile_scale_index_for_cell(const DM1_ViewportProjectileOcclusionSpec *spec, unsigned char view_cell);
size_t dm1_viewport_3d_door_front_occlusion_spec_count(void);
const DM1_ViewportDoorFrontOcclusionSpec *dm1_viewport_3d_get_door_front_occlusion_spec(size_t index);
const DM1_ViewportDoorFrontOcclusionSpec *dm1_viewport_3d_get_door_front_occlusion_spec_for_square(DM1_ViewSquareIndex square);
size_t dm1_viewport_3d_side_occlusion_spec_count(void);
const DM1_ViewportSideOcclusionSpec *dm1_viewport_3d_get_side_occlusion_spec(size_t index);
const DM1_ViewportSideOcclusionSpec *dm1_viewport_3d_get_side_occlusion_spec_for_square(DM1_ViewSquareIndex square);
size_t dm1_viewport_3d_thieves_eye_door_frame_occlusion_spec_count(void);
const DM1_ViewportThievesEyeDoorFrameOcclusionSpec *dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec(size_t index);
const DM1_ViewportThievesEyeDoorFrameOcclusionSpec *dm1_viewport_3d_get_thieves_eye_door_frame_occlusion_spec_for_square(DM1_ViewSquareIndex square);
size_t dm1_viewport_3d_floor_field_order_spec_count(void);
const DM1_ViewportFloorFieldOrderSpec *dm1_viewport_3d_get_floor_field_order_spec(size_t index);
const DM1_ViewportFloorFieldOrderSpec *dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_ViewSquareIndex square);

/* Source-locked post-command redraw contract: a completed command mutates the
 * party/world state first, GAMELOOP.C redraws F0128 from G0308/G0306/G0307,
 * and F0097 waits until the viewport buffer has been presented. */
const DM1_ViewportPostCommandRedrawSpec *dm1_viewport_3d_post_command_redraw_spec(void);

/*
 * Source evidence string for verification.
 */
const char *dm1_viewport_3d_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_3D_PC34_COMPAT_H */
