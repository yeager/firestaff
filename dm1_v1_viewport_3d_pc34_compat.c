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
 * These values are copied directly from ReDMCSB DUNVIEW.C lines 581-594.
 * The frame array maps view squares D3L..D0R to wall bitmap positions.
 *
 * Index mapping (M600-M611 → array index):
 *   D3C=0, D3L=1, D3R=2, D2C=3, D2L=4, D2R=5,
 *   D1C=6, D1L=7, D1R=8, D0C=9, D0L=10, D0R=11
 *
 * Source: DUNVIEW.C lines 581-594 (G0163_aauc_Graphic558_Frame_Walls)
 * ──────────────────────────────────────────────────────────────────────── */

static const DM1_WallFrame s_wall_frames[12] = {
    /* D3C */ {  74, 149, 25,  75,  64,  51,  18, 0 },
    /* D3L */ {   0,  83, 25,  75,  64,  51,  32, 0 },
    /* D3R */ { 139, 223, 25,  75,  64,  51,   0, 0 },
    /* D2C */ {  60, 163, 20,  90,  72,  71,  16, 0 },
    /* D2L */ {   0,  74, 20,  90,  72,  71,  61, 0 },
    /* D2R */ { 149, 223, 20,  90,  72,  71,   0, 0 },
    /* D1C */ {  32, 191,  9, 119, 128, 111,  48, 0 },
    /* D1L */ {   0,  63,  9, 119, 128, 111, 192, 0 },
    /* D1R */ { 160, 223,  9, 119, 128, 111,   0, 0 },
    /* D0C — unused for walls */ { 0, 223, 0, 135, 0, 0, 0, 0 },
    /* D0L */ {   0,  31,  0, 135,  16, 136,   0, 0 },
    /* D0R */ { 192, 223,  0, 135,  16, 136,   0, 0 },
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

/* ReDMCSB DUNVIEW.C F0128 draw sequence, lines 8446-8542.
 * rel_depth/rel_lateral are the F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement
 * arguments immediately before each draw call; D0C is the party square and does not
 * call F0150. */
static const DM1_ViewportDrawStep s_draw_order[] = {
    { DM1_VIEW_SQUARE_D4L, 4, -1, "F0115:D4L objects", "DUNVIEW.C:8466-8469" },
    { DM1_VIEW_SQUARE_D4R, 4,  1, "F0115:D4R objects", "DUNVIEW.C:8470-8473" },
    { DM1_VIEW_SQUARE_D4C, 4,  0, "F0115:D4C objects", "DUNVIEW.C:8474-8477" },
    { DM1_VIEW_SQUARE_D3L2, 3, -2, "F0676_DrawD3L2", "DUNVIEW.C:8478-8482" },
    { DM1_VIEW_SQUARE_D3R2, 3,  2, "F0677_DrawD3R2", "DUNVIEW.C:8483-8486" },
    { DM1_VIEW_SQUARE_D3L, 3, -1, "F0116_DUNGEONVIEW_DrawSquareD3L", "DUNVIEW.C:8488-8491" },
    { DM1_VIEW_SQUARE_D3R, 3,  1, "F0117_DUNGEONVIEW_DrawSquareD3R", "DUNVIEW.C:8492-8495" },
    { DM1_VIEW_SQUARE_D3C, 3,  0, "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", "DUNVIEW.C:8496-8499" },
    { DM1_VIEW_SQUARE_D2L2, 2, -2, "F0678_DrawD2L2", "DUNVIEW.C:8500-8504" },
    { DM1_VIEW_SQUARE_D2R2, 2,  2, "F0679_DrawD2R2", "DUNVIEW.C:8505-8508" },
    { DM1_VIEW_SQUARE_D2L, 2, -1, "F0119_DUNGEONVIEW_DrawSquareD2L", "DUNVIEW.C:8510-8513" },
    { DM1_VIEW_SQUARE_D2R, 2,  1, "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "DUNVIEW.C:8514-8517" },
    { DM1_VIEW_SQUARE_D2C, 2,  0, "F0121_DUNGEONVIEW_DrawSquareD2C", "DUNVIEW.C:8518-8521" },
    { DM1_VIEW_SQUARE_D1L, 1, -1, "F0122_DUNGEONVIEW_DrawSquareD1L", "DUNVIEW.C:8522-8525" },
    { DM1_VIEW_SQUARE_D1R, 1,  1, "F0123_DUNGEONVIEW_DrawSquareD1R", "DUNVIEW.C:8526-8529" },
    { DM1_VIEW_SQUARE_D1C, 1,  0, "F0124_DUNGEONVIEW_DrawSquareD1C", "DUNVIEW.C:8530-8533" },
    { DM1_VIEW_SQUARE_D0L, 0, -1, "F0125_DUNGEONVIEW_DrawSquareD0L", "DUNVIEW.C:8534-8537" },
    { DM1_VIEW_SQUARE_D0R, 0,  1, "F0126_DUNGEONVIEW_DrawSquareD0R", "DUNVIEW.C:8538-8541" },
    { DM1_VIEW_SQUARE_D0C, 0,  0, "F0127_DUNGEONVIEW_DrawSquareD0C", "DUNVIEW.C:8542" },
};


/* PC34/I34E wall bitmap selection table, source-locked to the MEDIA709/720
 * draw calls in ReDMCSB DUNVIEW.C.  These entries encode the native draw
 * bitmap and the parity draw bitmap used with F0105/F0792. */

/* F0115 per-cell z-order.  ReDMCSB explicitly scans the thing list multiple
 * times for each cell: objects first, then creatures, then projectiles; after
 * all packed cells are processed it restarts once more for explosions/fluxcage. */
static const DM1_ViewportThingLayerSpec s_thing_layers[] = {
    { DM1_VIEWPORT_THING_LAYER_OBJECTS,     "objects",     "DUNVIEW.C:4567-4571,4853-4860", true,  false },
    { DM1_VIEWPORT_THING_LAYER_CREATURES,   "creatures",   "DUNVIEW.C:4573,5195-5202",      true,  false },
    { DM1_VIEWPORT_THING_LAYER_PROJECTILES, "projectiles", "DUNVIEW.C:4575-4577,5681-5883", true,  false },
    { DM1_VIEWPORT_THING_LAYER_EXPLOSIONS,  "explosions",  "DUNVIEW.C:4579-4581,5915-5933", false, true  },
};


static const DM1_ViewportProjectileOcclusionSpec s_projectile_occlusion_specs[] = {
    { DM1_VIEW_SQUARE_D0C,   0, 0, 11, "DEFS.H:2596; DUNVIEW.C:373,5675-5676,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D1C,   3, 1,  8, "DEFS.H:2599; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D1L,   4, 1,  9, "DEFS.H:2600; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D1R,   5, 1, 10, "DEFS.H:2601; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D2C,   6, 2,  5, "DEFS.H:2602; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D2L,   7, 2,  6, "DEFS.H:2603; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D2R,   8, 2,  7, "DEFS.H:2604; DUNVIEW.C:373,5667-5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3C,  11, 3,  0, "DEFS.H:2607; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3L,  12, 3,  1, "DEFS.H:2608; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3R,  13, 3,  2, "DEFS.H:2609; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3L2, 14, 3,  3, "DEFS.H:2610; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
    { DM1_VIEW_SQUARE_D3R2, 15, 3,  4, "DEFS.H:2611; DUNVIEW.C:373,5672-5673,5683,5710-5715,5881-5883" },
};

static const DM1_ViewportDoorFrontOcclusionSpec s_door_front_occlusion_specs[] = {
    { DM1_VIEW_SQUARE_D3L, 0x0218, 0x0349, "DUNVIEW.C:6443-6444 pass1 rear cells before left frame", "DUNVIEW.C:6446-6454 left/right frame draw", "DUNVIEW.C:6457 F0111 door bitmap/ornament", "DUNVIEW.C:6459 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D3C, 0x0218, 0x0349, "DUNVIEW.C:6722-6723 pass1 rear cells before frame", "DUNVIEW.C:6725-6739 top/side frame and button draw", "DUNVIEW.C:6744 F0111 door bitmap/ornament", "DUNVIEW.C:6746 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D2C, 0x0218, 0x0349, "DUNVIEW.C:7314-7315 pass1 rear cells before frame", "DUNVIEW.C:7317-7333 top/side frame and button draw", "DUNVIEW.C:7339 F0111 door bitmap/ornament", "DUNVIEW.C:7341 pass2 front cells after door" },
    { DM1_VIEW_SQUARE_D1C, 0x0218, 0x0349, "DUNVIEW.C:7874-7875 pass1 rear cells before frame", "DUNVIEW.C:7877-7902 top/side frame and button draw", "DUNVIEW.C:7905-7908 F0111 door bitmap/ornament", "DUNVIEW.C:7910-7937 pass2 front cells after door" },
};

static const DM1_ViewportPostCommandRedrawSpec s_post_command_redraw = {
    true,
    true,
    true,
    "COMMAND.C:2045-2156/F0380 pops a queued command; lines 2118-2127 pop/unlock, 2150-2156 dispatch turn/move mutations",
    "GAMELOOP.C:55-90 next loop iteration redraws F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)",
    "DRAWVIEW.C:709-722 F0097_DUNGEONVIEW_DrawViewport requests the G0296 viewport blit and waits for vertical blank",
};

static const DM1_ViewportFloorFieldOrderSpec s_floor_field_order_specs[] = {
    { DM1_VIEW_SQUARE_D3L2, 0x3421, true, true, true, true, true, true,
      "F0676_DrawD3L2",
      "DUNVIEW.C:6237-6252 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:6275-6278 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:6282-6284 order then F0108 floor ornament",
      "DUNVIEW.C:6286 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:6288-6289 teleporter field after F0115",
      "DUNVIEW.C:6253-6264 wall bitmap/ornament then return before F0115" },
    { DM1_VIEW_SQUARE_D3L, 0x3421, true, true, true, true, true, true,
      "F0116_DUNGEONVIEW_DrawSquareD3L",
      "DUNVIEW.C:6375-6405 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:6461-6472 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:6475-6478 order then F0108 floor ornament",
      "DUNVIEW.C:6480 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:6482-6495 teleporter field after F0115",
      "DUNVIEW.C:6406-6437 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D3C, 0x3421, true, true, true, true, true, true,
      "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
      "DUNVIEW.C:6666-6696 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:6748-6762 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:6811-6814 order then F0108 floor ornament",
      "DUNVIEW.C:6816 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:6818-6831 teleporter field after F0115",
      "DUNVIEW.C:6697-6720 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D2C, 0x3421, true, true, true, true, true, true,
      "F0121_DUNGEONVIEW_DrawSquareD2C",
      "DUNVIEW.C:7260-7288 stairs front bitmap before common floor/thing path",
      "DUNVIEW.C:7343-7353 pit bitmap before floor ornament; BUG0_64 floor ornaments can draw over open pits",
      "DUNVIEW.C:7355-7357 order then F0108 floor ornament; ceiling pit follows before F0115",
      "DUNVIEW.C:7367-7368 F0115 object/creature/projectile/explosion handoff",
      "DUNVIEW.C:7370-7388 teleporter field after F0115",
      "DUNVIEW.C:7289-7312 wall bitmap/ornament then return unless front alcove branches to F0115" },
    { DM1_VIEW_SQUARE_D0C, 0x0021, true, true, false, true, true, false,
      "F0127_DUNGEONVIEW_DrawSquareD0C",
      "DUNVIEW.C:8241-8273 stairs front bitmap draws and breaks before F0115",
      "DUNVIEW.C:8274-8292 pit floor/ceiling bitmap before F0115",
      "DUNVIEW.C:8284-8294 no D0C floor-ornament call in this branch; ceiling pit precedes F0115",
      "DUNVIEW.C:8294 F0115 object/creature/projectile/explosion handoff with C0x0021",
      "DUNVIEW.C:8295-8308 teleporter field after F0115",
      "DUNVIEW.C:8185-8240 door-side case breaks before common F0115; no wall case in D0C" },
};

static const DM1_ViewportWallDrawSpec s_wall_draw_specs[] = {
    { DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2, true,  false, DM1_PC34_ZONE_WALL_D3L2, true,  false, "F0676_DrawD3L2",                  "DUNVIEW.C:6254-6260", "DUNVIEW.C:6263-6264 wall ornament then return" },
    { DM1_VIEW_SQUARE_D3R2, DM1_WALL_D3R2, DM1_WALL_D3L2, true,  false, DM1_PC34_ZONE_WALL_D3R2, true,  false, "F0677_DrawD3R2",                  "DUNVIEW.C:6321-6327", "DUNVIEW.C:6330-6331 wall ornament then return" },
    { DM1_VIEW_SQUARE_D3L,  DM1_WALL_D3L,  DM1_WALL_D3R,  true,  false, DM1_PC34_ZONE_WALL_D3L,  true,  true,  "F0116_DUNGEONVIEW_DrawSquareD3L", "DUNVIEW.C:6421-6427", "DUNVIEW.C:6432-6437 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D3R,  DM1_WALL_D3R,  DM1_WALL_D3L,  true,  false, DM1_PC34_ZONE_WALL_D3R,  true,  true,  "F0117_DUNGEONVIEW_DrawSquareD3R", "DUNVIEW.C:6554-6564", "DUNVIEW.C:6568-6573 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D3C,  DM1_WALL_D3C,  DM1_WALL_D3C,  true,  true,  DM1_PC34_ZONE_WALL_D3C,  true,  true,  "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", "DUNVIEW.C:6707-6714", "DUNVIEW.C:6716-6720 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2, true,  false, DM1_PC34_ZONE_WALL_D2L2, true,  false, "F0678_DrawD2L2",                  "DUNVIEW.C:6849-6858", "DUNVIEW.C:6848-6862 wall case returns" },
    { DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2, true,  false, DM1_PC34_ZONE_WALL_D2R2, true,  false, "F0679_DrawD2R2",                  "DUNVIEW.C:6880-6889", "DUNVIEW.C:6882-6893 wall case returns" },
    { DM1_VIEW_SQUARE_D2L,  DM1_WALL_D2L,  DM1_WALL_D2R,  true,  false, DM1_PC34_ZONE_WALL_D2L,  true,  true,  "F0119_DUNGEONVIEW_DrawSquareD2L", "DUNVIEW.C:6954-6964", "DUNVIEW.C:6968-6973 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D2R,  DM1_WALL_D2R,  DM1_WALL_D2L,  true,  false, DM1_PC34_ZONE_WALL_D2R,  true,  true,  "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF", "DUNVIEW.C:7105-7115", "DUNVIEW.C:7119-7123 front alcove branch; DUNVIEW.C:7166 blocker return" },
    { DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C,  true,  true,  DM1_PC34_ZONE_WALL_D2C,  true,  true,  "F0121_DUNGEONVIEW_DrawSquareD2C", "DUNVIEW.C:7299-7306", "DUNVIEW.C:7308-7312 front alcove branches to F0115, else return" },
    { DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R,  true,  false, DM1_PC34_ZONE_WALL_D1L,  true,  false, "F0122_DUNGEONVIEW_DrawSquareD1L", "DUNVIEW.C:7445-7455", "DUNVIEW.C:7459-7460 side ornament then return" },
    { DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L,  true,  false, DM1_PC34_ZONE_WALL_D1R,  true,  false, "F0123_DUNGEONVIEW_DrawSquareD1R", "DUNVIEW.C:7613-7623", "DUNVIEW.C:7627-7628 side ornament then return" },
    { DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C,  true,  true,  DM1_PC34_ZONE_WALL_D1C,  false, true,  "F0124_DUNGEONVIEW_DrawSquareD1C", "DUNVIEW.C:7833-7840", "DUNVIEW.C:7842-7843 front alcove draws F0115; no side cells behind D1C" },
    { DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R,  true,  false, DM1_PC34_ZONE_WALL_D0L,  true,  false, "F0125_DUNGEONVIEW_DrawSquareD0L", "DUNVIEW.C:8016-8033", "DUNVIEW.C:8036-8038 wall case returns" },
    { DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L,  true,  false, DM1_PC34_ZONE_WALL_D0R,  true,  false, "F0126_DUNGEONVIEW_DrawSquareD0R", "DUNVIEW.C:8126-8139", "DUNVIEW.C:8142-8144 wall case returns" },
};

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
 *   5. Draw D4L, D4R, D4C far background objects
 *   6. Draw D3L2, D3R2 far-side PC34/I34E wall lanes
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

    /* Step 3: PC34/I34E parity is applied per draw call.
     * ReDMCSB DUNVIEW.C lines 6254-6260, 6321-6327, 6421-6427, etc.:
     *   side walls select the opposite G2107_WallSet[] entry and call F0105
     *   to flip horizontally; center walls pass G0076 to F0792.  Keep the
     *   native G2107 order stable in state->wall_set and use
     *   dm1_viewport_3d_select_wall_bitmap() at integration points. */

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

    /* Step 12: Native wall set remains stable on PC34/I34E; other platform
     * paths restore at DUNVIEW.C:8553-8579 after temporary flipped pointers. */

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

size_t dm1_viewport_3d_draw_order_count(void)
{
    return sizeof(s_draw_order) / sizeof(s_draw_order[0]);
}

const DM1_ViewportDrawStep *dm1_viewport_3d_get_draw_order_step(size_t index)
{
    if (index >= dm1_viewport_3d_draw_order_count()) return NULL;
    return &s_draw_order[index];
}

size_t dm1_viewport_3d_wall_draw_spec_count(void)
{
    return sizeof(s_wall_draw_specs) / sizeof(s_wall_draw_specs[0]);
}

const DM1_ViewportWallDrawSpec *dm1_viewport_3d_get_wall_draw_spec(size_t index)
{
    if (index >= dm1_viewport_3d_wall_draw_spec_count()) return NULL;
    return &s_wall_draw_specs[index];
}

const DM1_ViewportWallDrawSpec *dm1_viewport_3d_get_wall_draw_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_wall_draw_spec_count(); ++i) {
        if (s_wall_draw_specs[i].square == square) return &s_wall_draw_specs[i];
    }
    return NULL;
}

DM1_WallSetIndex dm1_viewport_3d_select_wall_bitmap(const DM1_ViewportWallDrawSpec *spec,
                                                    bool parity_flip,
                                                    bool *flip_horizontally)
{
    if (flip_horizontally) *flip_horizontally = false;
    if (!spec) return DM1_WALL_SET_COUNT;
    if (parity_flip) {
        if (flip_horizontally) *flip_horizontally = spec->parity_flips_horizontally;
        return spec->parity_wall;
    }
    return spec->native_wall;
}

DM1_ViewportCellOrder dm1_viewport_3d_decode_cell_order(uint16_t order)
{
    DM1_ViewportCellOrder out;
    memset(&out, 0, sizeof(out));
    out.cell_order = order;

    uint16_t remaining = order;
    uint16_t first = remaining & 0x000f;
    if (first == 0) {
        out.alcove = true;
        return out;
    }
    if (first & 0x0008) {
        out.door_pass = (unsigned char)((first & 0x0007) + 1);
        remaining >>= 4;
    }
    for (int i = 0; i < 4; ++i) {
        uint16_t cell = remaining & 0x000f;
        if (cell == 0) break;
        out.cells[out.cell_count++] = (unsigned char)cell;
        remaining >>= 4;
    }
    return out;
}

size_t dm1_viewport_3d_thing_layer_spec_count(void)
{
    return sizeof(s_thing_layers) / sizeof(s_thing_layers[0]);
}

const DM1_ViewportThingLayerSpec *dm1_viewport_3d_get_thing_layer_spec(size_t index)
{
    if (index >= dm1_viewport_3d_thing_layer_spec_count()) return NULL;
    return &s_thing_layers[index];
}


size_t dm1_viewport_3d_projectile_occlusion_spec_count(void)
{
    return sizeof(s_projectile_occlusion_specs) / sizeof(s_projectile_occlusion_specs[0]);
}

const DM1_ViewportProjectileOcclusionSpec *dm1_viewport_3d_get_projectile_occlusion_spec(size_t index)
{
    if (index >= dm1_viewport_3d_projectile_occlusion_spec_count()) return NULL;
    return &s_projectile_occlusion_specs[index];
}

const DM1_ViewportProjectileOcclusionSpec *dm1_viewport_3d_get_projectile_occlusion_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_projectile_occlusion_spec_count(); ++i) {
        if (s_projectile_occlusion_specs[i].square == square) return &s_projectile_occlusion_specs[i];
    }
    return NULL;
}

int dm1_viewport_3d_projectile_zone_for_cell(const DM1_ViewportProjectileOcclusionSpec *spec, unsigned char view_cell)
{
    if (!spec || view_cell > 3 || spec->g2028_row < 0) return -1;
    if ((spec->view_depth == 3) && (view_cell <= 1)) return -1;
    if ((spec->view_depth == 0) && (view_cell >= 2)) return -1;
    return 2900 + ((int)spec->g2028_row * 4) + view_cell;
}

int dm1_viewport_3d_projectile_scale_index_for_cell(const DM1_ViewportProjectileOcclusionSpec *spec, unsigned char view_cell)
{
    if (!spec || view_cell > 3) return -1;
    if (dm1_viewport_3d_projectile_zone_for_cell(spec, view_cell) < 0) return -1;
    return (spec->view_depth << 1) - (view_cell >> 1);
}

size_t dm1_viewport_3d_door_front_occlusion_spec_count(void)
{
    return sizeof(s_door_front_occlusion_specs) / sizeof(s_door_front_occlusion_specs[0]);
}

const DM1_ViewportDoorFrontOcclusionSpec *dm1_viewport_3d_get_door_front_occlusion_spec(size_t index)
{
    if (index >= dm1_viewport_3d_door_front_occlusion_spec_count()) return NULL;
    return &s_door_front_occlusion_specs[index];
}

const DM1_ViewportDoorFrontOcclusionSpec *dm1_viewport_3d_get_door_front_occlusion_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_door_front_occlusion_spec_count(); ++i) {
        if (s_door_front_occlusion_specs[i].square == square) return &s_door_front_occlusion_specs[i];
    }
    return NULL;
}

size_t dm1_viewport_3d_floor_field_order_spec_count(void)
{
    return sizeof(s_floor_field_order_specs) / sizeof(s_floor_field_order_specs[0]);
}

const DM1_ViewportFloorFieldOrderSpec *dm1_viewport_3d_get_floor_field_order_spec(size_t index)
{
    if (index >= dm1_viewport_3d_floor_field_order_spec_count()) return NULL;
    return &s_floor_field_order_specs[index];
}

const DM1_ViewportFloorFieldOrderSpec *dm1_viewport_3d_get_floor_field_order_spec_for_square(DM1_ViewSquareIndex square)
{
    for (size_t i = 0; i < dm1_viewport_3d_floor_field_order_spec_count(); ++i) {
        if (s_floor_field_order_specs[i].square == square) return &s_floor_field_order_specs[i];
    }
    return NULL;
}

const DM1_ViewportPostCommandRedrawSpec *dm1_viewport_3d_post_command_redraw_spec(void)
{
    return &s_post_command_redraw;
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
        "  DUNVIEW.C:183  G2107_WallSet[15] PC34/I34E wall bitmap order\n"
        "  DUNVIEW.C:2225 F0096_DUNGEONVIEW_LoadCurrentMapGraphics_CPSDF\n"
        "  DUNVIEW.C:2427-2443 G3048_WallSetFlipped pair generation for flipped-capable ports\n"
        "  DUNVIEW.C:581  G0163_aauc_Graphic558_Frame_Walls[12][8]\n"
        "  DEFS.H:4040-4057 PC34 viewport zone constants for ceiling/floor/wall areas\n"
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
        "  DUNVIEW.C:4547 F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF\n"
        "  DUNVIEW.C:4561-4581 F0115 packed cell-order and object/creature/projectile/explosion z-order\n"
        "  DUNVIEW.C:5681-5883 F0115 projectile draw pass and PC34 zone draw\n"
        "  DUNVIEW.C:373,5667-5683 projectile occlusion via G2028 row and C2900 zone mapping; D3 front cells/D0 back cells clipped\n"
        "  DUNVIEW.C:5915-5933 F0115 explosion pass after all ordered cells\n"
        "  DUNVIEW.C:6237-6289 D3L2 stairs/pit/floor-ornament/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:6375-6495 D3L stairs/pit/floor-ornament/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:6666-6831 D3C stairs/pit/floor-ornament/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:7260-7388 D2C stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115\n"
        "  DUNVIEW.C:8241-8308 D0C stairs/pit/ceiling-pit/F0115/teleporter-field order\n"
        "  DUNVIEW.C:6443-6459 D3L door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:6722-6746 D3C door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:7314-7341 D2C door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:7874-7937 D1C door-front occlusion: rear pass, frame/door, front pass\n"
        "  DUNVIEW.C:6254-6327 F0676/F0677 PC34 parity side-wall selection; wall case returns / front alcove occlusion boundaries\n"
        "  DUNVIEW.C:6849-6893 F0678/F0679 PC34 D2L2/D2R2 side-wall zones and wall-case returns\n"
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
        "  COMMAND.C:2045-2156 F0380 command dispatch before next main-loop redraw\n"
        "  GAMELOOP.C:55-90 next loop iteration redraws F0128 from post-command G0308/G0306/G0307 party tuple\n"
        "  DRAWVIEW.C:709-722 F0097 requests viewport blit and waits for vertical blank\n"
        "  DUNVIEW.C:8446-8542 F0128 back-to-front viewport wall/object draw order\n"
        "  DUNVIEW.C:8577-8579 F0128 restores G3071_WallSetNotFlipped to G2107_WallSet\n"
        "  DUNVIEW.C:8609-8610 F0128 calls F0097_DUNGEONVIEW_DrawViewport after wall order completes\n"
        "  DRAWVIEW.C:721-722 F0097 viewport blit request/vblank wait\n"
        "  DRAWVIEW.C     F0694_SetMultipleColorsInPalette\n"
        "  DRAWVIEW.C     F0695_SetCreatureReplacementColors\n";
}
