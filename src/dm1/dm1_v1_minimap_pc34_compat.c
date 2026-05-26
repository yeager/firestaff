/*
 * dm1_v1_minimap_pc34_compat — corner minimap overlay implementation.
 *
 * Reads the loaded DungeonDatState_Compat tile grid via the M11
 * game view, draws each visited cell into the framebuffer.
 * V1-only consumer; V2 launch path never calls this.
 */

#include "dm1_v1_minimap_pc34_compat.h"
#include "m11_qol_runtime.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stddef.h>
#include <stdint.h>

/* Palette indices (DM1 VGA palette) used for minimap drawing. */
#define MM_COL_BG       0   /* black */
#define MM_COL_BORDER   15  /* white */
#define MM_COL_WALL     0   /* black */
#define MM_COL_FLOOR    8   /* dark gray */
#define MM_COL_DOOR     6   /* brown */
#define MM_COL_STAIRS   14  /* yellow */
#define MM_COL_PARTY    15  /* white (blinks) */

static unsigned char mm_square_type(const struct DungeonDatState_Compat* dungeon,
                                    int mapIdx,
                                    int x,
                                    int y) {
    const struct DungeonMapTiles_Compat* tiles;
    const struct DungeonMapDesc_Compat* desc;
    unsigned char raw;
    if (!dungeon || !dungeon->tilesLoaded || !dungeon->tiles || !dungeon->maps) {
        return DUNGEON_ELEMENT_WALL;
    }
    if (mapIdx < 0 || mapIdx >= (int)dungeon->header.mapCount) {
        return DUNGEON_ELEMENT_WALL;
    }
    desc = &dungeon->maps[mapIdx];
    if (x < 0 || y < 0 || x >= (int)desc->width || y >= (int)desc->height) {
        return DUNGEON_ELEMENT_WALL;
    }
    tiles = &dungeon->tiles[mapIdx];
    if (!tiles->squareData) {
        return DUNGEON_ELEMENT_WALL;
    }
    /* column-major: [col*height + row] */
    raw = tiles->squareData[x * (int)desc->height + y];
    return (unsigned char)((raw & DUNGEON_SQUARE_MASK_TYPE) >> 5);
}

static void mm_fill_rect(unsigned char* fb,
                         int fbW, int fbH,
                         int x, int y, int w, int h,
                         unsigned char color) {
    int row, col;
    if (!fb || w <= 0 || h <= 0) return;
    for (row = 0; row < h; ++row) {
        int py = y + row;
        if (py < 0 || py >= fbH) continue;
        for (col = 0; col < w; ++col) {
            int px = x + col;
            if (px < 0 || px >= fbW) continue;
            fb[py * fbW + px] = color;
        }
    }
}

void DM1_Minimap_Render(M11_GameViewState* state,
                        unsigned char* fb,
                        int fbW,
                        int fbH) {
    const struct DungeonDatState_Compat* dungeon;
    const struct DungeonMapDesc_Compat* desc;
    int mapIdx;
    int mapW, mapH;
    int cellPx;
    int wantedSize;
    int mapPxW, mapPxH;
    int mmW, mmH;
    int originX, originY;
    int x, y;
    int corner;
    int partyVisible;

    if (!fb || fbW <= 0 || fbH <= 0) return;
    if (!M11_QolRuntime_GetMinimapEnabled()) return;
    if (!state || !state->active) return;
    dungeon = state->world.dungeon;
    if (!dungeon || !dungeon->loaded || !dungeon->tilesLoaded) return;
    mapIdx = state->world.party.mapIndex;
    if (mapIdx < 0 || mapIdx >= (int)dungeon->header.mapCount) return;
    desc = &dungeon->maps[mapIdx];
    mapW = (int)desc->width;
    mapH = (int)desc->height;
    if (mapW <= 0 || mapH <= 0) return;

    wantedSize = M11_QolRuntime_GetMinimapSize();
    if (wantedSize < 16) wantedSize = 16;
    /* Bound to a reasonable corner overlay on the 320x200 framebuffer. */
    if (wantedSize > fbW * 2 / 3) wantedSize = fbW * 2 / 3;
    if (wantedSize > fbH * 2 / 3) wantedSize = fbH * 2 / 3;

    cellPx = wantedSize / (mapW > mapH ? mapW : mapH);
    if (cellPx < 1) cellPx = 1;
    if (cellPx > 6) cellPx = 6;

    mapPxW = mapW * cellPx;
    mapPxH = mapH * cellPx;
    /* +2px border on each side. */
    mmW = mapPxW + 2;
    mmH = mapPxH + 2;

    corner = M11_QolRuntime_GetMinimapCorner();
    switch (corner) {
        case 1: /* top-left */
            originX = 2;
            originY = 2;
            break;
        case 2: /* bottom-right */
            originX = fbW - mmW - 2;
            originY = fbH - mmH - 2;
            break;
        case 3: /* bottom-left */
            originX = 2;
            originY = fbH - mmH - 2;
            break;
        case 0: /* top-right */
        default:
            originX = fbW - mmW - 2;
            originY = 2;
            break;
    }
    if (originX < 0) originX = 0;
    if (originY < 0) originY = 0;

    /* Background + border */
    mm_fill_rect(fb, fbW, fbH, originX, originY, mmW, mmH, MM_COL_BG);
    /* Top + bottom border */
    mm_fill_rect(fb, fbW, fbH, originX, originY, mmW, 1, MM_COL_BORDER);
    mm_fill_rect(fb, fbW, fbH, originX, originY + mmH - 1, mmW, 1, MM_COL_BORDER);
    /* Left + right border */
    mm_fill_rect(fb, fbW, fbH, originX, originY, 1, mmH, MM_COL_BORDER);
    mm_fill_rect(fb, fbW, fbH, originX + mmW - 1, originY, 1, mmH, MM_COL_BORDER);

    /* Cells (fog-of-war: only explored) */
    for (y = 0; y < mapH; ++y) {
        for (x = 0; x < mapW; ++x) {
            unsigned int cell;
            unsigned char etype;
            unsigned char color;
            cell = (unsigned int)(x * 32 + y);
            if (cell >= 1024U) continue;
            if (!(state->exploredBits[cell / 32U] & (1U << (cell % 32U)))) {
                continue;
            }
            etype = mm_square_type(dungeon, mapIdx, x, y);
            switch (etype) {
                case DUNGEON_ELEMENT_WALL:      color = MM_COL_WALL; break;
                case DUNGEON_ELEMENT_DOOR:      color = MM_COL_DOOR; break;
                case DUNGEON_ELEMENT_STAIRS:    color = MM_COL_STAIRS; break;
                case DUNGEON_ELEMENT_PIT:       color = MM_COL_FLOOR; break;
                case DUNGEON_ELEMENT_TELEPORTER:color = MM_COL_STAIRS; break;
                case DUNGEON_ELEMENT_FAKEWALL:  color = MM_COL_FLOOR; break;
                case DUNGEON_ELEMENT_CORRIDOR:
                default:                        color = MM_COL_FLOOR; break;
            }
            mm_fill_rect(fb, fbW, fbH,
                         originX + 1 + x * cellPx,
                         originY + 1 + y * cellPx,
                         cellPx, cellPx, color);
        }
    }

    /* Party marker (blinking). */
    partyVisible = ((state->world.gameTick / 2U) & 1U) == 0U;
    if (partyVisible) {
        int px = originX + 1 + state->world.party.mapX * cellPx;
        int py = originY + 1 + state->world.party.mapY * cellPx;
        mm_fill_rect(fb, fbW, fbH, px, py, cellPx, cellPx, MM_COL_PARTY);
    }
}
