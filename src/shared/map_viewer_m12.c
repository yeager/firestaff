#include "map_viewer_m12.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* ── Map Viewer Implementation ───────────────────────────────────
 *
 * Renders DM1 dungeon floors as a navigable top-down grid using
 * character symbols.  Tile data is read via the M10 compat layer
 * (DungeonDatState_Compat) which handles the column-major byte
 * layout of DUNGEON.DAT square data.
 *
 * Column-major indexing: squareData[col * height + row]
 * Tile type is the top 3 bits: (byte >> 5) & 0x07
 */

/* ── Visible tile counts per zoom level ──────────────────────────── */
static const int s_zoomVisibleW[M12_MAP_ZOOM_COUNT] = { 48, 32, 20 };
static const int s_zoomVisibleH[M12_MAP_ZOOM_COUNT] = { 36, 24, 15 };

/* ── Zoom names ──────────────────────────────────────────────────── */
static const char* s_zoomNames[M12_MAP_ZOOM_COUNT] = {
    "1x (OVERVIEW)",
    "2x (STANDARD)",
    "3x (DETAIL)"
};

/* ── Tile type names ─────────────────────────────────────────────── */
static const char* s_tileTypeNames[DUNGEON_ELEMENT_COUNT] = {
    "WALL",
    "CORRIDOR",
    "PIT",
    "STAIRS",
    "DOOR",
    "TELEPORTER",
    "FAKE WALL"
};

/* ── Forward declarations ────────────────────────────────────────── */
static void mv_update_floor_dimensions(M12_MapViewerState* mv);
static void mv_clamp_scroll(M12_MapViewerState* mv);

/* ── Public API ──────────────────────────────────────────────────── */

void M12_MapViewer_Init(M12_MapViewerState* mv) {
    if (!mv) return;
    memset(mv, 0, sizeof(*mv));
    mv->zoom = M12_MAP_ZOOM_MEDIUM;
    mv->currentFloor = 0;
}

int M12_MapViewer_LoadDungeon(M12_MapViewerState* mv, const char* path) {
    if (!mv || !path) return 0;

    /* Clean up any previous load */
    M12_MapViewer_Unload(mv);

    /* Load header + map descriptors */
    if (!F0500_DUNGEON_LoadDatHeader_Compat(path, &mv->dungeon)) {
        return 0;
    }

    /* Load tile data for all floors */
    if (!F0502_DUNGEON_LoadTileData_Compat(path, &mv->dungeon)) {
        F0500_DUNGEON_FreeDatHeader_Compat(&mv->dungeon);
        return 0;
    }

    mv->dungeonLoaded = 1;
    mv->floorCount = (int)mv->dungeon.header.mapCount;
    mv->currentFloor = 0;
    mv->scrollX = 0;
    mv->scrollY = 0;

    mv_update_floor_dimensions(mv);
    M12_MapViewer_RebuildGrid(mv);

    return 1;
}

void M12_MapViewer_Unload(M12_MapViewerState* mv) {
    if (!mv) return;
    if (mv->dungeonLoaded) {
        F0502_DUNGEON_FreeTileData_Compat(&mv->dungeon);
        F0500_DUNGEON_FreeDatHeader_Compat(&mv->dungeon);
    }
    mv->dungeonLoaded = 0;
    mv->floorCount = 0;
    mv->currentFloor = 0;
    mv->mapWidth = 0;
    mv->mapHeight = 0;
    mv->gridRows = 0;
    mv->gridCols = 0;
}

void M12_MapViewer_SetFloor(M12_MapViewerState* mv, int floor) {
    if (!mv || !mv->dungeonLoaded) return;

    if (floor < 0) floor = 0;
    if (floor >= mv->floorCount) floor = mv->floorCount - 1;

    mv->currentFloor = floor;
    mv->scrollX = 0;
    mv->scrollY = 0;

    mv_update_floor_dimensions(mv);
    M12_MapViewer_RebuildGrid(mv);
}

void M12_MapViewer_CycleZoom(M12_MapViewerState* mv, int direction) {
    int z;
    if (!mv) return;

    z = (int)mv->zoom + direction;
    if (z < 0) z = M12_MAP_ZOOM_COUNT - 1;
    if (z >= M12_MAP_ZOOM_COUNT) z = 0;
    mv->zoom = (M12_MapZoomLevel)z;

    /* Update visible dimensions for new zoom */
    mv->visibleW = s_zoomVisibleW[mv->zoom];
    mv->visibleH = s_zoomVisibleH[mv->zoom];
    if (mv->visibleW > M12_MAP_MAX_VISIBLE_W) {
        mv->visibleW = M12_MAP_MAX_VISIBLE_W;
    }
    if (mv->visibleH > M12_MAP_MAX_VISIBLE_H) {
        mv->visibleH = M12_MAP_MAX_VISIBLE_H;
    }

    mv_clamp_scroll(mv);
    M12_MapViewer_RebuildGrid(mv);
}

void M12_MapViewer_Scroll(M12_MapViewerState* mv, int dx, int dy) {
    if (!mv || !mv->dungeonLoaded) return;

    mv->scrollX += dx;
    mv->scrollY += dy;

    mv_clamp_scroll(mv);
    M12_MapViewer_RebuildGrid(mv);
}

int M12_MapViewer_HandleInput(M12_MapViewerState* mv, int input) {
    if (!mv) return 0;

    switch (input) {
        case M12_MAP_INPUT_UP:
            M12_MapViewer_Scroll(mv, 0, -1);
            break;
        case M12_MAP_INPUT_DOWN:
            M12_MapViewer_Scroll(mv, 0, 1);
            break;
        case M12_MAP_INPUT_LEFT:
            M12_MapViewer_Scroll(mv, -1, 0);
            break;
        case M12_MAP_INPUT_RIGHT:
            M12_MapViewer_Scroll(mv, 1, 0);
            break;
        case M12_MAP_INPUT_ZOOM_IN:
            M12_MapViewer_CycleZoom(mv, 1);
            break;
        case M12_MAP_INPUT_ZOOM_OUT:
            M12_MapViewer_CycleZoom(mv, -1);
            break;
        case M12_MAP_INPUT_FLOOR_UP:
            M12_MapViewer_SetFloor(mv, mv->currentFloor + 1);
            break;
        case M12_MAP_INPUT_FLOOR_DOWN:
            M12_MapViewer_SetFloor(mv, mv->currentFloor - 1);
            break;
        case M12_MAP_INPUT_BACK:
            return 1;
        default:
            break;
    }
    return 0;
}

void M12_MapViewer_RebuildGrid(M12_MapViewerState* mv) {
    int r, c;
    int mapCol, mapRow;
    struct DungeonMapTiles_Compat* tiles;
    struct DungeonMapDesc_Compat* mapDesc;
    unsigned char tileData;
    int tileType;

    if (!mv || !mv->dungeonLoaded) return;
    if (mv->currentFloor < 0 || mv->currentFloor >= mv->floorCount) return;
    if (!mv->dungeon.tiles) return;

    tiles = &mv->dungeon.tiles[mv->currentFloor];
    mapDesc = &mv->dungeon.maps[mv->currentFloor];

    if (!tiles->squareData) return;

    /* Determine actual grid size (clamp to map bounds and visible area) */
    mv->gridCols = mv->visibleW;
    if (mv->gridCols > mv->mapWidth) mv->gridCols = mv->mapWidth;
    if (mv->gridCols > M12_MAP_MAX_VISIBLE_W) {
        mv->gridCols = M12_MAP_MAX_VISIBLE_W;
    }

    mv->gridRows = mv->visibleH;
    if (mv->gridRows > mv->mapHeight) mv->gridRows = mv->mapHeight;
    if (mv->gridRows > M12_MAP_MAX_VISIBLE_H) {
        mv->gridRows = M12_MAP_MAX_VISIBLE_H;
    }

    for (r = 0; r < mv->gridRows; r++) {
        for (c = 0; c < mv->gridCols; c++) {
            M12_MapViewerCell* cell = &mv->grid[r][c];

            mapCol = mv->scrollX + c;
            mapRow = mv->scrollY + r;

            cell->col = mapCol;
            cell->row = mapRow;

            if (mapCol < 0 || mapCol >= (int)mapDesc->width ||
                mapRow < 0 || mapRow >= (int)mapDesc->height) {
                /* Out of bounds — show as void */
                cell->symbol = ' ';
                cell->tileType = -1;
                continue;
            }

            /* Column-major indexing: squareData[col * height + row] */
            tileData = tiles->squareData[mapCol * (int)mapDesc->height + mapRow];
            tileType = (tileData >> 5) & 0x07;

            cell->tileType = tileType;
            cell->symbol = M12_MapViewer_TileSymbol(tileData);
        }
    }

    /* Update floor label */
    snprintf(mv->floorLabel, sizeof(mv->floorLabel),
             "FLOOR %d / %d  [%d x %d]  ZOOM: %s",
             mv->currentFloor + 1, mv->floorCount,
             mv->mapWidth, mv->mapHeight,
             M12_MapViewer_ZoomName(mv->zoom));
}

char M12_MapViewer_TileSymbol(unsigned char tileData) {
    int tileType = (tileData >> 5) & 0x07;

    switch (tileType) {
        case DUNGEON_ELEMENT_WALL:       return M12_MAP_SYM_WALL;
        case DUNGEON_ELEMENT_CORRIDOR:   return M12_MAP_SYM_CORRIDOR;
        case DUNGEON_ELEMENT_PIT:        return M12_MAP_SYM_PIT;
        case DUNGEON_ELEMENT_STAIRS:     return M12_MAP_SYM_STAIRS;
        case DUNGEON_ELEMENT_DOOR:       return M12_MAP_SYM_DOOR;
        case DUNGEON_ELEMENT_TELEPORTER: return M12_MAP_SYM_TELEPORTER;
        case DUNGEON_ELEMENT_FAKEWALL:   return M12_MAP_SYM_FAKEWALL;
        default:                         return M12_MAP_SYM_UNKNOWN;
    }
}

const char* M12_MapViewer_TileTypeName(int tileType) {
    if (tileType >= 0 && tileType < DUNGEON_ELEMENT_COUNT) {
        return s_tileTypeNames[tileType];
    }
    return "UNKNOWN";
}

const char* M12_MapViewer_GetFloorLabel(const M12_MapViewerState* mv) {
    if (!mv) return "";
    return mv->floorLabel;
}

const char* M12_MapViewer_ZoomName(M12_MapZoomLevel zoom) {
    if (zoom >= 0 && zoom < M12_MAP_ZOOM_COUNT) {
        return s_zoomNames[zoom];
    }
    return "UNKNOWN";
}

/* ── Internal helpers ────────────────────────────────────────────── */

static void mv_update_floor_dimensions(M12_MapViewerState* mv) {
    struct DungeonMapDesc_Compat* mapDesc;

    if (!mv->dungeonLoaded || mv->currentFloor < 0 ||
        mv->currentFloor >= mv->floorCount) {
        mv->mapWidth = 0;
        mv->mapHeight = 0;
        mv->visibleW = 0;
        mv->visibleH = 0;
        return;
    }

    mapDesc = &mv->dungeon.maps[mv->currentFloor];
    mv->mapWidth = (int)mapDesc->width;
    mv->mapHeight = (int)mapDesc->height;

    mv->visibleW = s_zoomVisibleW[mv->zoom];
    mv->visibleH = s_zoomVisibleH[mv->zoom];
    if (mv->visibleW > M12_MAP_MAX_VISIBLE_W) {
        mv->visibleW = M12_MAP_MAX_VISIBLE_W;
    }
    if (mv->visibleH > M12_MAP_MAX_VISIBLE_H) {
        mv->visibleH = M12_MAP_MAX_VISIBLE_H;
    }
}

static void mv_clamp_scroll(M12_MapViewerState* mv) {
    int maxX, maxY;

    if (mv->scrollX < 0) mv->scrollX = 0;
    if (mv->scrollY < 0) mv->scrollY = 0;

    maxX = mv->mapWidth - mv->visibleW;
    if (maxX < 0) maxX = 0;
    if (mv->scrollX > maxX) mv->scrollX = maxX;

    maxY = mv->mapHeight - mv->visibleH;
    if (maxY < 0) maxY = 0;
    if (mv->scrollY > maxY) mv->scrollY = maxY;
}
