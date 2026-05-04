#ifndef FIRESTAFF_MAP_VIEWER_M12_H
#define FIRESTAFF_MAP_VIEWER_M12_H

/*
 * Map Viewer — M12 launcher feature.
 *
 * Renders a top-down grid view of dungeon levels parsed from
 * DUNGEON.DAT.  Each tile is displayed as a character symbol
 * indicating its type (wall, corridor, pit, stairs, door,
 * teleporter, fake wall).  The viewer supports floor navigation,
 * zoom in/out, and scroll panning across large maps.
 *
 * Data flow:
 *   1. M12_MapViewer_Init()         — reset state
 *   2. M12_MapViewer_LoadDungeon()  — parse DUNGEON.DAT via the M10
 *                                     compat layer and cache tile data
 *   3. M12_MapViewer_SetFloor()     — select a floor to display
 *   4. M12_MapViewer_Draw()         — render the grid into a buffer
 *   5. M12_MapViewer_HandleInput()  — process navigation / zoom keys
 *   6. M12_MapViewer_Unload()       — free resources
 */

#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Zoom levels ─────────────────────────────────────────────────── */
typedef enum {
    M12_MAP_ZOOM_SMALL = 0,   /* 1 char per tile  (dense overview)    */
    M12_MAP_ZOOM_MEDIUM,      /* 2×1 chars per tile (balanced)        */
    M12_MAP_ZOOM_LARGE,       /* 3×2 chars per tile (detailed)        */
    M12_MAP_ZOOM_COUNT
} M12_MapZoomLevel;

/* ── Tile symbol characters ──────────────────────────────────────── */
#define M12_MAP_SYM_WALL        '#'
#define M12_MAP_SYM_CORRIDOR    '.'
#define M12_MAP_SYM_PIT         'O'
#define M12_MAP_SYM_STAIRS      '>'
#define M12_MAP_SYM_DOOR        '+'
#define M12_MAP_SYM_TELEPORTER  'T'
#define M12_MAP_SYM_FAKEWALL    '~'
#define M12_MAP_SYM_UNKNOWN     '?'

/* ── Grid cell for rendered output ───────────────────────────────── */
typedef struct {
    char symbol;       /* Display character  */
    int  tileType;     /* DUNGEON_ELEMENT_*  */
    int  col;          /* Map column         */
    int  row;          /* Map row            */
} M12_MapViewerCell;

/* Maximum visible grid (in tiles) at any zoom */
#define M12_MAP_MAX_VISIBLE_W   64
#define M12_MAP_MAX_VISIBLE_H   48

/* ── Viewer state ────────────────────────────────────────────────── */
typedef struct {
    /* Dungeon data (owned; freed on Unload) */
    struct DungeonDatState_Compat  dungeon;
    int                            dungeonLoaded;

    /* Current view */
    int                currentFloor;   /* 0-based map index           */
    int                floorCount;     /* == header.mapCount          */
    M12_MapZoomLevel   zoom;

    /* Scroll position (tile coordinates of top-left visible cell) */
    int                scrollX;
    int                scrollY;

    /* Cached floor dimensions (from current map descriptor) */
    int                mapWidth;       /* tiles                       */
    int                mapHeight;      /* tiles                       */

    /* Visible grid after zoom calculation */
    int                visibleW;       /* tiles visible horizontally  */
    int                visibleH;       /* tiles visible vertically    */

    /* Rendered grid cells */
    M12_MapViewerCell  grid[M12_MAP_MAX_VISIBLE_H][M12_MAP_MAX_VISIBLE_W];
    int                gridRows;       /* actual rows filled          */
    int                gridCols;       /* actual cols filled          */

    /* Floor label cache */
    char               floorLabel[64];
} M12_MapViewerState;

/* ── Input commands ──────────────────────────────────────────────── */
#define M12_MAP_INPUT_UP       1
#define M12_MAP_INPUT_DOWN     2
#define M12_MAP_INPUT_LEFT     3
#define M12_MAP_INPUT_RIGHT    4
#define M12_MAP_INPUT_ZOOM_IN  5
#define M12_MAP_INPUT_ZOOM_OUT 6
#define M12_MAP_INPUT_FLOOR_UP   7   /* Next deeper floor  */
#define M12_MAP_INPUT_FLOOR_DOWN 8   /* Previous floor     */
#define M12_MAP_INPUT_BACK     9     /* Return to menu     */

/**
 * Initialize viewer state to defaults.
 */
void M12_MapViewer_Init(M12_MapViewerState* mv);

/**
 * Load a DUNGEON.DAT file and parse header + tile data.
 * Returns 1 on success, 0 on failure.
 */
int M12_MapViewer_LoadDungeon(M12_MapViewerState* mv, const char* path);

/**
 * Free loaded dungeon data.
 */
void M12_MapViewer_Unload(M12_MapViewerState* mv);

/**
 * Set the active floor (0-based map index). Clamps to valid range.
 * Resets scroll position and rebuilds the visible grid.
 */
void M12_MapViewer_SetFloor(M12_MapViewerState* mv, int floor);

/**
 * Cycle zoom level: +1 = zoom in, -1 = zoom out. Wraps around.
 * Rebuilds the visible grid.
 */
void M12_MapViewer_CycleZoom(M12_MapViewerState* mv, int direction);

/**
 * Scroll the view by (dx, dy) tiles. Clamps to map bounds.
 * Rebuilds the visible grid.
 */
void M12_MapViewer_Scroll(M12_MapViewerState* mv, int dx, int dy);

/**
 * Process an input command. Returns 1 if the viewer should close
 * (M12_MAP_INPUT_BACK), 0 otherwise.
 */
int M12_MapViewer_HandleInput(M12_MapViewerState* mv, int input);

/**
 * Rebuild the visible grid cells from current floor, scroll, and zoom.
 * Called automatically by SetFloor / CycleZoom / Scroll, but can be
 * invoked manually after direct state changes.
 */
void M12_MapViewer_RebuildGrid(M12_MapViewerState* mv);

/**
 * Return the display symbol for a raw tile byte.
 */
char M12_MapViewer_TileSymbol(unsigned char tileData);

/**
 * Return a human-readable name for a tile type.
 */
const char* M12_MapViewer_TileTypeName(int tileType);

/**
 * Return the floor label string for the current floor
 * (e.g. "FLOOR 1 / 14  [32 x 32]").
 */
const char* M12_MapViewer_GetFloorLabel(const M12_MapViewerState* mv);

/**
 * Return a zoom level name string (e.g. "1x", "2x", "3x").
 */
const char* M12_MapViewer_ZoomName(M12_MapZoomLevel zoom);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MAP_VIEWER_M12_H */
