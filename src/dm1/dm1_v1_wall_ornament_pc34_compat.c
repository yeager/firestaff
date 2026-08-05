/* DM1 V1 Wall Ornament Rendering — source-locked from ReDMCSB
 * DUNVIEW.C F0107_IsDrawnWallOrnamentAnAlcove_CPSF: alcove detection
 * DUNVIEW.C: ornament coordinate sets loaded from graphic 558
 * DRAWVIEW.C: ornament blit overlay on wall zones at correct depth/side */

#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "dm1_v1_wall_ornament_alcove_f0149_pc34_compat.h"
#include "firestaff/dm1/v1/G0192_pc34_compat.h"
#include "firestaff/dm1/v1/G0194_pc34_compat.h"
#include "firestaff/dm1/v1/G0205_pc34_compat.h"
#include <string.h>

enum {
    DM1_GFX_WALL_ORNAMENT_BASE_PC34 = 259,
    DM1_WALL_ORNAMENT_TRANSPARENT_COLOR_PC34 = 10,
    DM1_GFX_CHAMPION_PORTRAITS_PC34 = 26,
    DM1_FRONT_MIRROR_GLOBAL_ORNAMENT_PC34 = 43,
    DM1_FRONT_MIRROR_VIEW_WALL_INDEX_PC34 = 12,
    DM1_PORTRAIT_WIDTH_PC34 = 32,
    DM1_PORTRAIT_HEIGHT_PC34 = 29,
    DM1_PORTRAIT_ATLAS_COLS_PC34 = 8,
    DM1_PORTRAIT_ATLAS_ROWS_PC34 = 3,
    DM1_PORTRAIT_ATLAS_COUNT_PC34 =
        DM1_PORTRAIT_ATLAS_COLS_PC34 * DM1_PORTRAIT_ATLAS_ROWS_PC34,
    DM1_PORTRAIT_DST_X_PC34 = 96,
    DM1_PORTRAIT_DST_Y_PC34 = 35,
    DM1_PORTRAIT_TRANSPARENT_COLOR_PC34 = 1,
    DM1_FRONT_MIRROR_BACKING_SOURCE_WIDTH_PC34 = 48,
    DM1_FRONT_MIRROR_BACKING_SOURCE_HEIGHT_PC34 = 43
};

static const DM1_WallOrnamentViewSpecPc34 s_wallOrnamentViewSpecs[] = {
    {3, -2,  0, 1},
    {3,  2,  1, 1},
    {3, -1,  2, 0},
    {3,  1,  4, 0},
    {3, -1,  2, 0},
    {3,  0,  3, 0},
    {3,  1,  4, 0},
    {2, -1,  5, 0},
    {2,  1,  6, 0},
    {2, -1,  7, 0},
    {2,  0,  8, 0},
    {2,  1,  9, 0},
    {1, -1, 10, 0},
    {1,  1, 11, 0},
    {1,  0, 12, 0}
};

static const unsigned char s_wallOrnamentPaletteD3[16] = {
    0, 0, 12, 3, 4, 3, 0, 6, 3, 9, 10, 11, 0, 1, 0, 2
};

static const unsigned char s_wallOrnamentPaletteD2[16] = {
    0, 12, 1, 3, 4, 3, 6, 7, 5, 9, 10, 11, 0, 2, 14, 13
};

static void copy_palette(unsigned char dst[16], const unsigned char src[16])
{
    int i;
    for (i = 0; i < 16; ++i) {
        dst[i] = src[i];
    }
}

void DM1_V1_WallOrnament_InitPc34Compat(DM1_V1_WallOrnamentStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_WallOrnamentStatePc34));
}

void DM1_V1_WallOrnament_SetLevelOrnamentsPc34Compat(DM1_V1_WallOrnamentStatePc34* state,
                                 uint8_t wall_count, uint8_t floor_count,
                                 uint8_t door_count) {
    if (!state) return;
    state->wall_count = (wall_count > DM1_WALL_ORN_MAX) ? DM1_WALL_ORN_MAX : wall_count;
    state->floor_count = (floor_count > DM1_FLOOR_ORN_MAX) ? DM1_FLOOR_ORN_MAX : floor_count;
    state->door_count = (door_count > DM1_DOOR_ORN_MAX) ? DM1_DOOR_ORN_MAX : door_count;
}

/* F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF
 * Alcoves are wall ornaments where items can be placed/retrieved */
bool DM1_V1_WallOrnament_IsAlcovePc34Compat(const DM1_V1_WallOrnamentDefPc34* orn) {
    if (!orn) return false;
    return orn->is_alcove || orn->kind == DM1_V1_WALL_ORN_ALCOVE;
}

const DM1_V1_WallOrnamentCoordPc34* DM1_V1_WallOrnament_GetCoordPc34Compat(const DM1_V1_WallOrnamentDefPc34* orn,
                                          int16_t depth, int16_t side) {
    if (!orn) return NULL;
    if (depth < 0 || depth > 3 || side < 0 || side > 2) return NULL;

    /* Coordinate set index: depth * 3 + side (max 12 entries) */
    int idx = depth * 3 + side;
    if (idx >= DM1_ORN_COORD_SETS) return NULL;
    return &orn->coords[idx];
}

/* Setup default ornament coordinate positions based on DM1 perspective geometry.
 * These approximate the ReDMCSB coordinate tables from graphic 558.
 * Depth 0 = closest (largest), depth 3 = farthest (smallest) */
void DM1_V1_WallOrnament_SetupDefaultCoordsPc34Compat(DM1_V1_WallOrnamentDefPc34* orn) {
    if (!orn) return;

    /* Depth scaling factors: approximate DM1 perspective projection
     * Based on DUNVIEW.C wall zone geometry */
    static const struct { int16_t base_w, base_h, base_x_left, base_x_center, base_x_right, base_y; } depth_params[4] = {
        /* D0: closest — ornament fills most of wall */
        { 64, 48, 16, 80, 144, 44 },
        /* D1: medium close */
        { 40, 30, 32, 92, 152, 52 },
        /* D2: medium far */
        { 24, 18, 48, 100, 152, 58 },
        /* D3: farthest — small */
        { 16, 12, 56, 104, 152, 62 }
    };

    for (int d = 0; d < 4; d++) {
        for (int s = 0; s < 3; s++) {
            int idx = d * 3 + s;
            if (idx >= DM1_ORN_COORD_SETS) break;
            orn->coords[idx].w = depth_params[d].base_w;
            orn->coords[idx].h = depth_params[d].base_h;
            orn->coords[idx].depth = (int16_t)d;
            orn->coords[idx].side = (int16_t)s;
            orn->coords[idx].y = depth_params[d].base_y;

            switch (s) {
                case 0: /* left */
                    orn->coords[idx].x = depth_params[d].base_x_left;
                    break;
                case 1: /* center */
                    orn->coords[idx].x = depth_params[d].base_x_center;
                    break;
                case 2: /* right */
                    orn->coords[idx].x = depth_params[d].base_x_right;
                    break;
            }
        }
    }
}

int dm1_v1_wall_ornament_coord_set_index_pc34(int globalIndex) {
    int coordSet = dm1_v1_g0194_get_pc34(globalIndex);
    if (coordSet < 0) {
        return 0;
    }
    return coordSet;
}

int dm1_v1_wall_ornament_zone_pc34(int coordSet,
                                   int viewWallIndex,
                                   DM1_WallOrnamentZoneBlitPc34* outBlit) {
    /* ReDMCSB DUNVIEW.C G0205_aaauc_Graphic558_WallOrnamentCoordinateSets:
     * { X1, X2, Y1, Y2, ByteWidth, Height }.  The viewport destination is
     * the inclusive X/Y box; ByteWidth describes source-byte storage and is
     * not a destination width. */
    int x1;
    int x2;
    int y1;
    int y2;
    if (!outBlit) {
        return 0;
    }
    x1 = dm1_v1_g0205_get_pc34(coordSet, viewWallIndex, 0);
    x2 = dm1_v1_g0205_get_pc34(coordSet, viewWallIndex, 1);
    y1 = dm1_v1_g0205_get_pc34(coordSet, viewWallIndex, 2);
    y2 = dm1_v1_g0205_get_pc34(coordSet, viewWallIndex, 3);
    if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0) {
        return 0;
    }
    outBlit->srcX = 0;
    outBlit->srcY = 0;
    outBlit->dstX = x1;
    outBlit->dstY = y1;
    outBlit->width = x2 - x1 + 1;
    outBlit->height = y2 - y1 + 1;
    return outBlit->width > 0 && outBlit->height > 0;
}

int dm1_v1_wall_ornament_flip_horizontal_pc34(int viewWallIndex) {
    /* ReDMCSB DUNVIEW.C F0107: for PC34/I34E the wall-ornament bitmap is
     * flipped only for right-side left-wall projections:
     * M576_VIEW_WALL_D3R_LEFT, M581_VIEW_WALL_D2R_LEFT, and
     * M586_VIEW_WALL_D1R_LEFT. */
    return viewWallIndex == 1 || viewWallIndex == 6 || viewWallIndex == 11;
}

/* ReDMCSB F0174 current-map alcove list (G0267 equivalent).  DUNVIEW.C
 * lines 2672-2690 clears G0267 to -1 on every current-map change, then
 * scans the map's G0261 wall-ornament table: local slot i joins the alcove
 * list when its global ornament index matches one of the G0192 source-table
 * entries.  The engine wires the loaded DUNGEON.DAT table here; until then
 * the legacy global entry point keeps failing closed (no synthetic
 * classification). */
static int s_f0174AlcoveOrnamentIndices[DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34];
static int s_f0174AlcoveOrnamentGlobals[DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34];
static int s_f0174MapIndex = -1;
static int s_f0174Wired = 0;

static void f0174_fill_minus_one(void) {
    int i;
    for (i = 0; i < DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34; ++i) {
        s_f0174AlcoveOrnamentIndices[i] = -1;
        s_f0174AlcoveOrnamentGlobals[i] = -1;
    }
}

int dm1_v1_wall_ornament_wire_current_map_alcove_list_pc34(
    int mapIndex,
    const int* wallOrnamentGlobalIndices,
    int wallOrnamentSlotCount) {
    /* DUNVIEW.C:2672 F0010_MAIN_WriteSpacedWords(G0267, 3, -1) then the
     * G0261 scan at lines 2676-2684.  The inscription slot's global index
     * (C0_WALL_ORNAMENT_INSCRIPTION == 0) never matches G0192. */
    int i;
    int k;
    int count = 0;
    f0174_fill_minus_one();
    s_f0174Wired = 0;
    s_f0174MapIndex = -1;
    if (!wallOrnamentGlobalIndices || wallOrnamentSlotCount <= 0) {
        return 0;
    }
    for (i = 0; i < wallOrnamentSlotCount; ++i) {
        const int globalIndex = wallOrnamentGlobalIndices[i];
        for (k = 0; k < dm1_v1_g0192_size_pc34(); ++k) {
            if (globalIndex == dm1_v1_g0192_get_pc34(k)) {
                if (count < DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34) {
                    s_f0174AlcoveOrnamentIndices[count] = i;
                    s_f0174AlcoveOrnamentGlobals[count] = globalIndex;
                    ++count;
                }
                break;
            }
        }
    }
    s_f0174Wired = 1;
    s_f0174MapIndex = mapIndex;
    return count;
}

void dm1_v1_wall_ornament_clear_current_map_alcove_list_pc34(void) {
    f0174_fill_minus_one();
    s_f0174Wired = 0;
    s_f0174MapIndex = -1;
}

int dm1_v1_wall_ornament_current_map_alcove_list_map_pc34(void) {
    return s_f0174Wired ? s_f0174MapIndex : -1;
}

int dm1_v1_wall_ornament_is_alcove_global_pc34(int globalIndex) {
    /* F0149 via the F0174-wired current-map list: a global ornament index
     * drawn on this map is an alcove only when the map's DUNGEON.DAT wall
     * ornament table slots it into the G0267-equivalent list.  Without the
     * wiring this legacy call site has no F0174 current-map alcove list
     * and fails closed instead of inventing source metadata. */
    int i;
    if (!s_f0174Wired || globalIndex < 0) {
        return 0;
    }
    for (i = 0; i < DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34; ++i) {
        if (s_f0174AlcoveOrnamentGlobals[i] == globalIndex) {
            return 1;
        }
    }
    return 0;
}

int dm1_v1_wall_ornament_is_alcove_local_ordinal_pc34(int wallOrnamentOrdinal) {
    /* F0107 entry decrements the ordinal (P0116_i_WallOrnamentOrdinal--),
     * then F0149 tests the 0-based local index against G0267. */
    if (!s_f0174Wired) {
        return 0;
    }
    return F0149_DUNGEON_IsWallOrnamentAnAlcove(
        s_f0174AlcoveOrnamentIndices,
        DM1_V1_F0149_ALCOVE_ORNAMENT_COUNT_PC34,
        wallOrnamentOrdinal - 1);
}

int dm1_v1_wall_ornament_view_spec_count_pc34(void) {
    return (int)(sizeof(s_wallOrnamentViewSpecs) /
                 sizeof(s_wallOrnamentViewSpecs[0]));
}

int dm1_v1_wall_ornament_view_spec_pc34(
    int index,
    DM1_WallOrnamentViewSpecPc34* outSpec) {
    if (!outSpec || index < 0 ||
        index >= dm1_v1_wall_ornament_view_spec_count_pc34()) {
        return 0;
    }
    *outSpec = s_wallOrnamentViewSpecs[index];
    return 1;
}

int dm1_v1_wall_ornament_render_plan_pc34(
    int globalIndex,
    int viewWallIndex,
    int maxHeight,
    DM1_WallOrnamentRenderPlanPc34* outPlan) {
    DM1_WallOrnamentZoneBlitPc34 blit;
    int coordSet;
    int nativeOffset;
    if (!outPlan || globalIndex < 0) {
        return 0;
    }
    coordSet = dm1_v1_wall_ornament_coord_set_index_pc34(globalIndex);
    if (!dm1_v1_wall_ornament_zone_pc34(coordSet, viewWallIndex, &blit)) {
        return 0;
    }
    if (maxHeight > 0 && maxHeight < blit.height) {
        blit.height = maxHeight;
    }
    if (blit.width <= 0 || blit.height <= 0) {
        return 0;
    }

    /* ReDMCSB DUNVIEW.C F0107 increments the native wall-ornament bitmap
     * for front-facing projections and D1 side views, but not for
     * D2L_RIGHT/D2R_LEFT side projections. */
    nativeOffset = (viewWallIndex >= 2 &&
                    viewWallIndex != 5 &&
                    viewWallIndex != 6) ? 1 : 0;

    outPlan->graphicIndex =
        DM1_GFX_WALL_ORNAMENT_BASE_PC34 + globalIndex * 2 + nativeOffset;
    outPlan->srcX = blit.srcX;
    outPlan->srcY = blit.srcY;
    outPlan->dstX = blit.dstX;
    outPlan->dstY = blit.dstY;
    outPlan->width = blit.width;
    outPlan->height = blit.height;
    outPlan->transparentColor = DM1_WALL_ORNAMENT_TRANSPARENT_COLOR_PC34;
    outPlan->flipHorizontal =
        dm1_v1_wall_ornament_flip_horizontal_pc34(viewWallIndex);
    outPlan->paletteMapValid = 1;
    if (viewWallIndex <= 4) {
        copy_palette(outPlan->paletteMap, s_wallOrnamentPaletteD3);
    } else if (viewWallIndex <= 9) {
        copy_palette(outPlan->paletteMap, s_wallOrnamentPaletteD2);
    } else {
        /* ReDMCSB F0107 still applies the D2 wall palette to the D1/D0
         * ornament pass.  Keeping the native indices here turns the real
         * torch-holder/material pixels into black silhouettes on HoC walls. */
        copy_palette(outPlan->paletteMap, s_wallOrnamentPaletteD2);
        outPlan->paletteMapValid = 1;
    }
    outPlan->isAlcove =
        dm1_v1_wall_ornament_is_alcove_global_pc34(globalIndex);
    return 1;
}

int dm1_v1_front_mirror_wall_ornament_zone_xywh_pc34(
    int* outX, int* outY, int* outW, int* outH) {
    /* ReDMCSB DUNVIEW.C:3913-3928 routes the C346 Hall-mirror frame through
     * F0791 into G0205's D1C zone. G0194 maps global ornament 43 (mirror)
     * to its coordinate set and G0205 row 12 is the D1C wall, giving the
     * 80,29..143,71 (64x43) box the C026 portrait is centered in. */
    DM1_WallOrnamentZoneBlitPc34 blit;
    int coordSet = dm1_v1_wall_ornament_coord_set_index_pc34(
        DM1_FRONT_MIRROR_GLOBAL_ORNAMENT_PC34);
    if (!dm1_v1_wall_ornament_zone_pc34(
            coordSet, DM1_FRONT_MIRROR_VIEW_WALL_INDEX_PC34, &blit)) {
        return 0;
    }
    if (outX) *outX = blit.dstX;
    if (outY) *outY = blit.dstY;
    if (outW) *outW = blit.width;
    if (outH) *outH = blit.height;
    return 1;
}

int dm1_v1_front_mirror_render_plan_pc34(
    int portraitOrdinal,
    DM1_FrontMirrorRenderPlanPc34* outPlan) {
    if (!outPlan || portraitOrdinal < 0 ||
        portraitOrdinal >= DM1_PORTRAIT_ATLAS_COUNT_PC34) {
        return 0;
    }
    if (!dm1_v1_wall_ornament_render_plan_pc34(
            DM1_FRONT_MIRROR_GLOBAL_ORNAMENT_PC34,
            DM1_FRONT_MIRROR_VIEW_WALL_INDEX_PC34,
            0,
            &outPlan->ornament)) {
        return 0;
    }

    /* ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData as G0289.
     * DUNVIEW.C:3913-3928 sends C346 through F0791: its native 48x43
     * raster goes into G0205's 64x43 D1C zone before C026 at G0109. */
    outPlan->portraitGraphicIndex = DM1_GFX_CHAMPION_PORTRAITS_PC34;
    outPlan->portraitSrcX =
        (portraitOrdinal % DM1_PORTRAIT_ATLAS_COLS_PC34) *
        DM1_PORTRAIT_WIDTH_PC34;
    outPlan->portraitSrcY =
        (portraitOrdinal / DM1_PORTRAIT_ATLAS_COLS_PC34) *
        DM1_PORTRAIT_HEIGHT_PC34;
    outPlan->portraitDstX = DM1_PORTRAIT_DST_X_PC34;
    outPlan->portraitDstY = DM1_PORTRAIT_DST_Y_PC34;
    outPlan->portraitWidth = DM1_PORTRAIT_WIDTH_PC34;
    outPlan->portraitHeight = DM1_PORTRAIT_HEIGHT_PC34;
    outPlan->portraitTransparentColor = DM1_PORTRAIT_TRANSPARENT_COLOR_PC34;
    outPlan->backingDstX = outPlan->ornament.dstX;
    outPlan->backingDstY = outPlan->ornament.dstY;
    outPlan->backingSourceWidth = DM1_FRONT_MIRROR_BACKING_SOURCE_WIDTH_PC34;
    outPlan->backingSourceHeight = DM1_FRONT_MIRROR_BACKING_SOURCE_HEIGHT_PC34;
    outPlan->backingWidth = outPlan->ornament.width;
    outPlan->backingHeight = outPlan->ornament.height;
    return 1;
}

int dm1_v1_wall_ornament_host_material_receipt_pc34(
    int globalIndex,
    int viewWallIndex,
    int maxHeight,
    DM1_WallOrnamentHostMaterialReceiptPc34* outReceipt) {
    if (!outReceipt) {
        return 0;
    }
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!dm1_v1_wall_ornament_render_plan_pc34(
            globalIndex, viewWallIndex, maxHeight, &outReceipt->plan)) {
        return 0;
    }
    outReceipt->valid = 1;
    outReceipt->globalOrnamentIndex = globalIndex;
    outReceipt->viewWallIndex = viewWallIndex;
    return 1;
}
