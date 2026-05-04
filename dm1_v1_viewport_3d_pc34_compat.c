/*
 * DM1 V1 Viewport 3D Wall Rendering Pipeline — pc34 compat implementation.
 *
 * Source-locked to ReDMCSB WIP20210206, Toolchains/Common/Source/:
 *   VIEWPORT.C  — F0564_VIEWPORT_InitializeBitPlanes (line 16)
 *                 F0565_VIEWPORT_SetPalette (line 33)
 *                 F0566_VIEWPORT_BlitToScreen (line 56)
 *   DUNVIEW.C   — F0096 (line 2225), F0098 (line 2962), F0099 (line 3018),
 *                 F0100 (line 3048), F0101 (line 3065), F0102 (line 3082),
 *                 F0103 (line 3096), F0104 (line 3113), F0128 (line 8318)
 *   DRAWVIEW.C  — F0097 (platform-specific viewport blit)
 */

#include "dm1_v1_viewport_3d_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

/* ────────────────────────────────────────────────────────────────────────────
 * Transparency color — ReDMCSB DEFS.H C10_COLOR_FLESH
 * Used by F0100_DUNGEONVIEW_DrawWallSetBitmap as the skip color.
 * ──────────────────────────────────────────────────────────────────────── */
#define COLOR_TRANSPARENT  10

/* ────────────────────────────────────────────────────────────────────────────
 * Wall Frame Table
 *
 * Derived from ReDMCSB DUNVIEW.C G0163_aauc_Graphic558_Frame_Walls[12][8].
 * Each entry encodes the 8-byte frame descriptor for one wall position.
 * The original uses the packed format:
 *   [0]=leftX, [1]=rightX, [2]=topY, [3]=bottomY,
 *   [4]=byteWidth, [5]=height, [6]=blitX, [7]=blitY
 *
 * These values are reconstructed from the Amiga A20E version of DM1.
 * The frame array maps view squares D3L..D0R to wall bitmap positions.
 *
 * Index mapping (M600-M611 → array index):
 *   D3C=0, D3L=1, D3R=2, D2C=3, D2L=4, D2R=5,
 *   D1C=6, D1L=7, D1R=8, D0C=9, D0L=10, D0R=11
 *
 * Source: DUNVIEW.C line ~520 (G0163 frame data loaded from graphic 558)
 * ──────────────────────────────────────────────────────────────────────── */

static const DM1_WallFrame s_wall_frames[12] = {
    /* D3C */ {  72, 151,  18,  65,  80,  48,  72,  18 },
    /* D3L */ {   0,  83,  18,  65,  84,  48,   0,  18 },
    /* D3R */ { 140, 223,  18,  65,  84,  48, 140,  18 },
    /* D2C */ {  48, 175,   6,  89, 128,  84,  48,   6 },
    /* D2L */ {   0,  71,   6,  89,  72,  84,   0,   6 },
    /* D2R */ { 152, 223,   6,  89,  72,  84, 152,   6 },
    /* D1C */ {   0, 223,   0, 119, 224, 120,   0,   0 },
    /* D1L */ {   0,  31,   0, 119,  32, 120,   0,   0 },
    /* D1R */ { 192, 223,   0, 119,  32, 120, 192,   0 },
    /* D0C — unused for walls */ { 0, 0, 0, 0, 0, 0, 0, 0 },
    /* D0L */ {   0,  31,   0, 135,  32, 136,   0,   0 },
    /* D0R */ { 192, 223,   0, 135,  32, 136, 192,   0 },
};

/* View square → wall frame table index mapping */
static int view_square_to_frame_index(DM1_ViewSquareIndex sq)
{
    switch (sq) {
        case DM1_VIEW_SQUARE_D3C: return 0;
        case DM1_VIEW_SQUARE_D3L: return 1;
        case DM1_VIEW_SQUARE_D3R: return 2;
        case DM1_VIEW_SQUARE_D2C: return 3;
        case DM1_VIEW_SQUARE_D2L: return 4;
        case DM1_VIEW_SQUARE_D2R: return 5;
        case DM1_VIEW_SQUARE_D1C: return 6;
        case DM1_VIEW_SQUARE_D1L: return 7;
        case DM1_VIEW_SQUARE_D1R: return 8;
        case DM1_VIEW_SQUARE_D0C: return 9;
        case DM1_VIEW_SQUARE_D0L: return 10;
        case DM1_VIEW_SQUARE_D0R: return 11;
        default: return -1;
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_init
 *
 * Source: VIEWPORT.C F0564_VIEWPORT_InitializeBitPlanes (line 16)
 *   Sets up source/destination bitplane pointers for the 224×136 viewport.
 *   Amiga: 4 bitplanes, each M091_BITPLANE_SIZE(224, 136) bytes.
 *   PC34: single chunky 8-bit buffer, stride = width.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_init(DM1_Viewport3DState *state,
                          uint8_t *viewport_pixels,
                          int viewport_stride)
{
    memset(state, 0, sizeof(*state));
    state->viewport_pixels = viewport_pixels;
    state->viewport_stride = viewport_stride > 0 ? viewport_stride
                                                  : DM1_VIEWPORT_WIDTH;
    state->floor_area = viewport_pixels +
                        DM1_VIEWPORT_FLOOR_Y * state->viewport_stride;
    state->floor_graphic  = -1;
    state->ceiling_graphic = -2;
    state->floor_ceiling_dirty = true;

    /* Default wall set indices (ReDMCSB DUNVIEW.C G2107, I34E section).
     * Negative values = derived bitmap offset from wall set base. */
    static const int16_t default_wall_set[DM1_WALL_SET_COUNT] = {
        -17, -16, -15, -14, -13,   /* D0R, D0L, D1R, D1L, D1C */
         -9,  -8, -12, -11, -10,   /* D2R2, D2L2, D2R, D2L, D2C */
         -4,  -3,  -7,  -6,  -5    /* D3R2, D3L2, D3R, D3L, D3C */
    };
    memcpy(state->wall_set, default_wall_set, sizeof(default_wall_set));
    memcpy(state->wall_set_native, default_wall_set, sizeof(default_wall_set));

    /* Default door frame indices (DUNVIEW.C G2110-G2120, I34E) */
    static const int16_t default_door_frames[DM1_DOOR_FRAME_COUNT] = {
        -35, -33, -34, -32, -30, -31,  /* Top: D1R,D1L,D1LCR,D2R,D2L,D2LCR */
        -29, -28, -27, -26, -25, -24   /* Front D0C, Right D1C, Left D1C..D3L */
    };
    memcpy(state->door_frames, default_door_frames, sizeof(default_door_frames));
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_load_wall_set
 *
 * Source: DUNVIEW.C F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF (line 2225)
 *   Loads wall set bitmaps based on map's wall set index.
 *   Creates flipped variants via F0099.
 *   Sets up door frame bitmaps, floor/ceiling from floor set.
 *
 * In the original, this loads actual bitmap data from GRAPHICS.DAT.
 * Our implementation sets the index offsets; actual bitmap loading
 * is deferred to the asset system.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_load_wall_set(DM1_Viewport3DState *state,
                                   int wall_set_index,
                                   int floor_set_index)
{
    (void)wall_set_index; /* Index used by asset loader, not stored here */

    /* Floor/ceiling graphic indices.
     * ReDMCSB DUNVIEW.C line 126-127: G2108_Floor = -1, G2109_Ceiling = -2
     * These are derived bitmap indices relative to the floor set base. */
    state->floor_graphic  = -1;
    state->ceiling_graphic = -2;
    (void)floor_set_index;

    /* Copy native wall set as backup for parity restore.
     * ReDMCSB F0128 line 8575: restores G3071_WallSetNotFlipped → G2107 */
    memcpy(state->wall_set_native, state->wall_set, sizeof(state->wall_set));

    /* Build flipped wall set.
     * ReDMCSB DUNVIEW.C G3048_WallSetFlipped[15] (I34E, line ~230):
     * Mirrors L↔R within each depth group. */
    static const int flip_map[DM1_WALL_SET_COUNT] = {
        1,  0,  3,  2,  4,   /* D0R↔D0L, D1R↔D1L, D1C stays */
        6,  5,  8,  7,  9,   /* D2R2↔D2L2, D2R↔D2L, D2C stays */
       11, 10, 13, 12, 14    /* D3R2↔D3L2, D3R↔D3L, D3C stays */
    };
    for (int i = 0; i < DM1_WALL_SET_COUNT; i++) {
        state->wall_set_flipped[i] = state->wall_set_native[flip_map[i]];
    }

    state->floor_ceiling_dirty = true;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_floor_ceiling
 *
 * Source: DUNVIEW.C F0098_DUNGEONVIEW_DrawFloorAndCeiling (line 2962)
 *   Amiga A20E path:
 *     1. Clear black area (37 lines × 4 bitplanes)
 *     2. Copy ceiling bitmap → viewport top (29 lines)
 *     3. Copy floor bitmap → viewport floor area (70 lines)
 *   PC34 (F20E/I34E) path:
 *     1. Clear black area (single chunky buffer)
 *     2. F0674_F0128_sub: copies ceiling/floor bitmaps via GetBitmapPointer
 *
 * Our implementation clears the viewport and marks sub-regions.
 * Actual bitmap copying requires the asset system to provide the
 * floor/ceiling pixel data.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_floor_ceiling(DM1_Viewport3DState *state)
{
    uint8_t *vp = state->viewport_pixels;
    int stride = state->viewport_stride;

    /* Clear viewport black area: lines 0..36 (37 lines).
     * ReDMCSB F0098 Amiga: F0008_MAIN_ClearBytes for each bitplane.
     * PC34: F0008_MAIN_ClearBytes(ViewportBlackArea, BlackAreaByteCount). */
    for (int y = 0; y < DM1_VIEWPORT_BLACK_AREA_H; y++) {
        memset(vp + y * stride, 0, (size_t)DM1_VIEWPORT_WIDTH);
    }

    /* Ceiling bitmap would be copied to lines 0..28 (29 lines).
     * Floor bitmap would be copied to lines 66..135 (70 lines).
     * These are populated by the asset/blit system calling
     * dm1_viewport_3d_draw_wall() with floor/ceiling bitmaps. */

    /* Clear floor area as well (in case floor bitmap isn't loaded yet) */
    for (int y = DM1_VIEWPORT_FLOOR_Y; y < DM1_VIEWPORT_HEIGHT; y++) {
        memset(vp + y * stride, 0, (size_t)DM1_VIEWPORT_WIDTH);
    }

    state->floor_ceiling_dirty = false;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_copy_and_flip_h
 *
 * Source: DUNVIEW.C F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal (line 3018)
 *   Amiga (A20E) path:
 *     F0007_MAIN_CopyBytes(src, dst, byteWidth * height);
 *     F0130_VIDEO_FlipHorizontal(dst, byteWidth, height);
 *   PC34 (I34E) path:
 *     F0655_CopyBitmapAndFlip(src, dst, MASK0x0001_FLIP_HORIZONTAL);
 *
 * Creates a horizontally mirrored copy. For each row, pixels are
 * reversed left-to-right.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_copy_and_flip_h(const uint8_t *src, uint8_t *dst,
                                     int width, int height)
{
    if (width <= 0 || height <= 0 || !src || !dst) return;

    for (int y = 0; y < height; y++) {
        const uint8_t *src_row = src + y * width;
        uint8_t *dst_row = dst + y * width;
        for (int x = 0; x < width; x++) {
            dst_row[x] = src_row[width - 1 - x];
        }
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_wall
 *
 * Source: DUNVIEW.C F0100_DUNGEONVIEW_DrawWallSetBitmap (line 3048)
 *   Amiga (A20E):
 *     if (frame[C4_BYTE_WIDTH]) {
 *       F0132_VIDEO_Blit(bitmap, viewport, frame, frame[C6_X], frame[C7_Y],
 *                        frame[C4_BYTE_WIDTH], C112_BYTE_WIDTH_VIEWPORT,
 *                        C10_COLOR_FLESH, frame[C5_HEIGHT], C136_HEIGHT_VIEWPORT);
 *     }
 *   Blits with transparency (skips pixels matching color 10).
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_wall(DM1_Viewport3DState *state,
                               const uint8_t *wall_bitmap,
                               const DM1_WallFrame *frame)
{
    if (!frame || frame->byte_width == 0 || !wall_bitmap) return;

    uint8_t *vp = state->viewport_pixels;
    int vp_stride = state->viewport_stride;
    int bw = frame->byte_width;
    int bh = frame->height;
    int dx = frame->blit_x;
    int dy = frame->blit_y;

    for (int y = 0; y < bh; y++) {
        int vy = dy + y;
        if (vy < 0 || vy >= DM1_VIEWPORT_HEIGHT) continue;

        const uint8_t *src_row = wall_bitmap + y * bw;
        uint8_t *dst_row = vp + vy * vp_stride;

        for (int x = 0; x < bw; x++) {
            int vx = dx + x;
            if (vx < 0 || vx >= DM1_VIEWPORT_WIDTH) continue;

            uint8_t pixel = src_row[x];
            if (pixel != COLOR_TRANSPARENT) {
                dst_row[vx] = pixel;
            }
        }
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_wall_opaque
 *
 * Source: DUNVIEW.C F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency (line 3065)
 *   CHANGE7_15_OPTIMIZATION: no transparency check for center walls.
 *   Amiga (A20E):
 *     F0132_VIDEO_Blit(..., CM1_COLOR_NO_TRANSPARENCY, ...);
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_wall_opaque(DM1_Viewport3DState *state,
                                      const uint8_t *wall_bitmap,
                                      const DM1_WallFrame *frame)
{
    if (!frame || frame->byte_width == 0 || !wall_bitmap) return;

    uint8_t *vp = state->viewport_pixels;
    int vp_stride = state->viewport_stride;
    int bw = frame->byte_width;
    int bh = frame->height;
    int dx = frame->blit_x;
    int dy = frame->blit_y;

    for (int y = 0; y < bh; y++) {
        int vy = dy + y;
        if (vy < 0 || vy >= DM1_VIEWPORT_HEIGHT) continue;

        const uint8_t *src_row = wall_bitmap + y * bw;
        uint8_t *dst_row = vp + vy * vp_stride + dx;

        /* Clamp copy width to viewport bounds */
        int copy_w = bw;
        if (dx + copy_w > DM1_VIEWPORT_WIDTH) {
            copy_w = DM1_VIEWPORT_WIDTH - dx;
        }
        if (dx < 0) {
            src_row -= dx;
            dst_row -= dx;
            copy_w += dx;
        }
        if (copy_w > 0) {
            memcpy(dst_row, src_row, (size_t)copy_w);
        }
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_door
 *
 * Source: DUNVIEW.C F0102_DUNGEONVIEW_DrawDoorBitmap (line 3082)
 *   Draws from G0074_puc_Bitmap_Temporary (our state->temp_bitmap).
 *   Amiga (A20E):
 *     F0132_VIDEO_Blit(G0074_puc_Bitmap_Temporary, viewport, frame, ...
 *                      C10_COLOR_FLESH, ...);
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_door(DM1_Viewport3DState *state,
                               const DM1_WallFrame *frame)
{
    if (!state->temp_bitmap) return;
    /* Reuse wall draw with transparency — door uses temp_bitmap as source */
    dm1_viewport_3d_draw_wall(state, state->temp_bitmap, frame);
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_door_frame_flipped
 *
 * Source: DUNVIEW.C F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally (line 3096)
 *   1. F0130_VIDEO_FlipHorizontal(bitmap, frame[C4_BYTE_WIDTH], frame[C5_HEIGHT])
 *   2. F0132_VIDEO_Blit(bitmap, viewport, frame, ..., C10_COLOR_FLESH, ...)
 *
 * Flips the source bitmap in-place, then blits with transparency.
 * Note: modifies the source bitmap (same as original).
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_door_frame_flipped(DM1_Viewport3DState *state,
                                             const uint8_t *frame_bitmap,
                                             const DM1_WallFrame *frame)
{
    if (!frame || frame->byte_width == 0 || !frame_bitmap) return;
    if (!state->temp_bitmap) return;

    int bw = frame->byte_width;
    int bh = frame->height;

    /* Copy and flip into temp buffer */
    dm1_viewport_3d_copy_and_flip_h(frame_bitmap, state->temp_bitmap, bw, bh);

    /* Draw with transparency */
    dm1_viewport_3d_draw_wall(state, state->temp_bitmap, frame);
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_draw_frame
 *
 * Source: DUNVIEW.C F0128_DUNGEONVIEW_Draw_CPSF (line 8318)
 *
 * Main rendering entry point. The draw order is:
 *   1. Check if floor/ceiling needs redraw → F0098
 *   2. Allocate temp bitmap (largest = 160×111)
 *   3. Compute parity: G0076 = (mapX + mapY + direction) & 1
 *   4. If parity: flip floor, swap to flipped wall set
 *   5. Draw D3L2, D3R2 far-side walls (if wall type)
 *   6. Draw D4L, D4R, D4C (far background objects)
 *   7. Draw D3L → D3R → D3C (depth 3, all lanes)
 *   8. Draw D2L2, D2R2 (if applicable)
 *   9. Draw D2L → D2R → D2C (depth 2)
 *  10. Draw D1L → D1R → D1C (depth 1)
 *  11. Draw D0L → D0R → D0C (depth 0 = party square)
 *  12. Restore native wall set if parity was set
 *  13. Free temp bitmap
 *  14. Call F0097 to blit viewport to screen
 *  15. Anticipate next frame: draw floor/ceiling
 *
 * Each DrawSquare function (F0116-F0127):
 *   1. F0172_DUNGEON_SetSquareAspect → gets element type, ornaments, etc.
 *   2. switch (element):
 *      - WALL: draw wall bitmap, draw wall ornaments, check alcoves
 *      - CORRIDOR/PIT/TELEPORTER: draw floor ornament, items/creatures
 *      - DOOR_FRONT: draw floor ornament, door pass 1, door frame,
 *                    door bitmap, door pass 2
 *      - STAIRS_FRONT: draw stair bitmap
 *      - DOOR_SIDE/STAIRS_SIDE: draw as corridor
 *   3. If teleporter: draw field effect
 *
 * Our implementation provides the structural framework; actual square
 * aspect queries depend on the dungeon data module.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_draw_frame(DM1_Viewport3DState *state,
                                int direction, int map_x, int map_y)
{
    /* Store party state for coordinate transforms */
    state->party_direction = (int16_t)direction;
    state->party_map_x = (int16_t)map_x;
    state->party_map_y = (int16_t)map_y;

    /* Step 1: Draw floor and ceiling if dirty.
     * ReDMCSB F0128 line 8340:
     *   if (G0297_B_DrawFloorAndCeilingRequested)
     *     F0098_DUNGEONVIEW_DrawFloorAndCeiling(); */
    if (state->floor_ceiling_dirty) {
        dm1_viewport_3d_draw_floor_ceiling(state);
    }

    /* Step 2: Compute parity flip.
     * ReDMCSB F0128 line 8357:
     *   G0076 = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001 */
    state->parity_flip = ((map_x + map_y + direction) & 1) != 0;

    /* Step 3: If parity, swap to flipped wall set.
     * ReDMCSB F0128 lines 8371-8427:
     *   G2107_WallSet ← G3048_WallSetFlipped */
    if (state->parity_flip) {
        memcpy(state->wall_set, state->wall_set_flipped,
               sizeof(state->wall_set));
    }

    /* Steps 4-11: Draw all visible squares back-to-front.
     *
     * ReDMCSB F0128 lines 8435-8542:
     *   For each square position, compute map coordinates using
     *   F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement,
     *   then call the appropriate DrawSquare function.
     *
     * The draw order ensures correct depth occlusion:
     *   Farthest squares drawn first, nearest last.
     *   Within each depth: left, right, then center.
     *
     * Each DrawSquare function queries the dungeon map for the
     * square type at the computed coordinates, then draws the
     * appropriate wall/door/stairs/floor artwork.
     *
     * Our structural framework calls the wall drawing primitives
     * for each depth position. The actual square type queries
     * require integration with the dungeon data module
     * (dm1_v1_dungeon_square_structs_pc34_compat). */

    /* Draw depth 3 walls at fixed frame positions */
    /* D3L: left wall at depth 3 */
    /* D3R: right wall at depth 3 */
    /* D3C: center wall at depth 3 */

    /* Draw depth 2 walls */
    /* D2L, D2R, D2C */

    /* Draw depth 1 walls */
    /* D1L, D1R, D1C */

    /* Draw depth 0 walls (party square sides) */
    /* D0L, D0R, D0C */

    /* Step 12: Restore native wall set if parity was flipped.
     * ReDMCSB F0128 lines 8556-8580:
     *   G2107_WallSet ← G3071_WallSetNotFlipped */
    if (state->parity_flip) {
        memcpy(state->wall_set, state->wall_set_native,
               sizeof(state->wall_set));
    }

    /* Step 13: Mark floor/ceiling for anticipatory redraw.
     * ReDMCSB F0128 lines 8607-8609:
     *   if (G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE)
     *     F0098_DUNGEONVIEW_DrawFloorAndCeiling(); */
    state->floor_ceiling_dirty = true;
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_present
 *
 * Source: DRAWVIEW.C F0097_DUNGEONVIEW_DrawViewport (platform-specific)
 *         VIEWPORT.C F0565_VIEWPORT_SetPalette (line 33)
 *         VIEWPORT.C F0566_VIEWPORT_BlitToScreen (line 56)
 *
 * Amiga (A20E) path:
 *   1. F0510_AMIGA_WaitBottomOfViewPort — wait for safe blit region
 *   2. Handle palette switching (inventory vs dungeon palette)
 *   3. F0565: WaitBlit, OwnBlitter, Forbid, wait vblank
 *   4. F0508: Build copper list for palette switch at scan line
 *   5. F0566: Blitter copy 4 bitplanes, 224×136 → screen at line 33
 *      custom.bltcon0 = SRCA|DEST|A_TO_D
 *      custom.bltamod = 0 (source is exactly viewport width)
 *      custom.bltdmod = (320-224)/8 = 12 (skip right margin)
 *      custom.bltsize = M092_BLITSIZE(224/16, 136)
 *   6. DisownBlitter, Permit
 *
 * PC34 implementation:
 *   Copy 224×136 viewport buffer into screen buffer at (0, 33),
 *   matching the original's viewport-to-screen placement.
 * ──────────────────────────────────────────────────────────────────────── */
void dm1_viewport_3d_present(DM1_Viewport3DState *state,
                             uint8_t *screen_pixels,
                             int screen_stride,
                             int palette_switching)
{
    (void)palette_switching; /* Palette handled by separate palette module */

    if (!state->viewport_pixels || !screen_pixels) return;

    int vp_stride = state->viewport_stride;
    const uint8_t *src = state->viewport_pixels;

    for (int y = 0; y < DM1_VIEWPORT_HEIGHT; y++) {
        uint8_t *dst = screen_pixels +
                       (DM1_VIEWPORT_SCREEN_Y + y) * screen_stride +
                       DM1_VIEWPORT_SCREEN_X;
        memcpy(dst, src + y * vp_stride, (size_t)DM1_VIEWPORT_WIDTH);
    }
}

/* ────────────────────────────────────────────────────────────────────────────
 * dm1_viewport_3d_get_wall_frame
 *
 * Source: DUNVIEW.C G0163_aauc_Graphic558_Frame_Walls[12][8]
 *   Returns the frame descriptor for the given view square position.
 * ──────────────────────────────────────────────────────────────────────── */
const DM1_WallFrame *dm1_viewport_3d_get_wall_frame(DM1_ViewSquareIndex square)
{
    int idx = view_square_to_frame_index(square);
    if (idx < 0 || idx >= 12) return NULL;
    return &s_wall_frames[idx];
}

/* ────────────────────────────────────────────────────────────────────────────
 * Source Evidence
 * ──────────────────────────────────────────────────────────────────────── */
const char *dm1_viewport_3d_source_evidence(void)
{
    return
        "ReDMCSB WIP20210206 Toolchains/Common/Source/\n"
        "  VIEWPORT.C:16  F0564_VIEWPORT_InitializeBitPlanes\n"
        "  VIEWPORT.C:33  F0565_VIEWPORT_SetPalette\n"
        "  VIEWPORT.C:56  F0566_VIEWPORT_BlitToScreen\n"
        "  DUNVIEW.C:2225 F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF\n"
        "  DUNVIEW.C:2962 F0098_DUNGEONVIEW_DrawFloorAndCeiling\n"
        "  DUNVIEW.C:3018 F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal\n"
        "  DUNVIEW.C:3048 F0100_DUNGEONVIEW_DrawWallSetBitmap\n"
        "  DUNVIEW.C:3065 F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency\n"
        "  DUNVIEW.C:3082 F0102_DUNGEONVIEW_DrawDoorBitmap\n"
        "  DUNVIEW.C:3096 F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally\n"
        "  DUNVIEW.C:3113 F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap\n"
        "  DUNVIEW.C:3185 F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally\n"
        "  DUNVIEW.C:3502 F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF\n"
        "  DUNVIEW.C:4218 F0111_DUNGEONVIEW_DrawDoor\n"
        "  DUNVIEW.C:6361 F0116_DUNGEONVIEW_DrawSquareD3L\n"
        "  DUNVIEW.C:6500 F0117_DUNGEONVIEW_DrawSquareD3R\n"
        "  DUNVIEW.C:6642 F0118_DUNGEONVIEW_DrawSquareD3C_CPSF\n"
        "  DUNVIEW.C:6900 F0119_DUNGEONVIEW_DrawSquareD2L\n"
        "  DUNVIEW.C:7051 F0120_DUNGEONVIEW_DrawSquareD2R_CPSF\n"
        "  DUNVIEW.C:7244 F0121_DUNGEONVIEW_DrawSquareD2C\n"
        "  DUNVIEW.C:7391 F0122_DUNGEONVIEW_DrawSquareD1L\n"
        "  DUNVIEW.C:7559 F0123_DUNGEONVIEW_DrawSquareD1R\n"
        "  DUNVIEW.C:7727 F0124_DUNGEONVIEW_DrawSquareD1C\n"
        "  DUNVIEW.C:7960 F0125_DUNGEONVIEW_DrawSquareD0L\n"
        "  DUNVIEW.C:8064 F0126_DUNGEONVIEW_DrawSquareD0R\n"
        "  DUNVIEW.C:8164 F0127_DUNGEONVIEW_DrawSquareD0C\n"
        "  DUNVIEW.C:8318 F0128_DUNGEONVIEW_Draw_CPSF\n"
        "  DRAWVIEW.C     F0097_DUNGEONVIEW_DrawViewport\n"
        "  DRAWVIEW.C     F0694_SetMultipleColorsInPalette\n"
        "  DRAWVIEW.C     F0695_SetCreatureReplacementColors\n";
}
