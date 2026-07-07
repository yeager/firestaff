/* DM1 V1 Wall Ornament Rendering — source-locked from ReDMCSB
 * DUNVIEW.C F0107_IsDrawnWallOrnamentAnAlcove_CPSF: alcove detection
 * DUNVIEW.C: ornament coordinate sets loaded from graphic 558
 * DRAWVIEW.C: ornament blit overlay on wall zones at correct depth/side */

#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "firestaff/dm1/v1/G0194_pc34_compat.h"
#include "firestaff/dm1/v1/G0205_pc34_compat.h"
#include <string.h>

void m11_wo_init(M11_WO_OrnamentState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_WO_OrnamentState));
}

void m11_wo_set_level_ornaments(M11_WO_OrnamentState* state,
                                 uint8_t wall_count, uint8_t floor_count,
                                 uint8_t door_count) {
    if (!state) return;
    state->wall_count = (wall_count > DM1_WALL_ORN_MAX) ? DM1_WALL_ORN_MAX : wall_count;
    state->floor_count = (floor_count > DM1_FLOOR_ORN_MAX) ? DM1_FLOOR_ORN_MAX : floor_count;
    state->door_count = (door_count > DM1_DOOR_ORN_MAX) ? DM1_DOOR_ORN_MAX : door_count;
}

/* F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF
 * Alcoves are wall ornaments where items can be placed/retrieved */
bool m11_wo_is_alcove(const M11_WO_OrnamentDef* orn) {
    if (!orn) return false;
    return orn->is_alcove || orn->kind == M11_ORN_ALCOVE;
}

const M11_WO_OrnCoord* m11_wo_get_coord(const M11_WO_OrnamentDef* orn,
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
void m11_wo_setup_default_coords(M11_WO_OrnamentDef* orn) {
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

int dm1_v1_wall_ornament_is_alcove_global_pc34(int globalIndex) {
    /* ReDMCSB DUNVIEW.C G0192_auc_Graphic558_AlcoveOrnamentIndices and
     * DUNGEON.C F0149: current-map global wall ornaments 1, 2, and 3 are
     * alcoves for wall-cell object visibility. */
    return globalIndex == 1 || globalIndex == 2 || globalIndex == 3;
}
