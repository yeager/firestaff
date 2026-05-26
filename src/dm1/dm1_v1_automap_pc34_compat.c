/*
 * dm1_v1_automap_pc34_compat — auto-map logger + BMP exporter.
 *
 * Visit tracking lives in a per-level bitset that mirrors what the
 * minimap already records for the active level; on level transitions
 * the existing exploredBits[] is reset, so this module keeps an extra
 * persistent copy that survives transitions.  BMP export writes a
 * minimap-style 24-bit Windows bitmap (no external lib).
 */

#include "dm1_v1_automap_pc34_compat.h"
#include "m11_qol_runtime.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <unistd.h>
#define MKDIR(p) mkdir((p), 0755)
#endif
#include <sys/types.h>

/* DM1 dungeons are bounded; we mirror the 32x32 cap the runtime
 * already uses for exploredBits.  Per-level grid keyed by mapIndex
 * (DM1 has up to 16 levels in practice; we size for 32 to be safe). */
#define AUTOMAP_MAX_LEVELS 32
#define AUTOMAP_MAX_DIM    32

typedef struct {
    uint32_t bits[AUTOMAP_MAX_LEVELS][AUTOMAP_MAX_DIM]; /* 32 cols * 32 rows */
} AutoMapStore;

static AutoMapStore g_store; /* zero-init by default */

static void automap_mark(int mapIdx, int x, int y) {
    if (mapIdx < 0 || mapIdx >= AUTOMAP_MAX_LEVELS) return;
    if (x < 0 || x >= AUTOMAP_MAX_DIM) return;
    if (y < 0 || y >= AUTOMAP_MAX_DIM) return;
    g_store.bits[mapIdx][x] |= (1U << y);
}

static int automap_get(int mapIdx, int x, int y) {
    if (mapIdx < 0 || mapIdx >= AUTOMAP_MAX_LEVELS) return 0;
    if (x < 0 || x >= AUTOMAP_MAX_DIM) return 0;
    if (y < 0 || y >= AUTOMAP_MAX_DIM) return 0;
    return (g_store.bits[mapIdx][x] & (1U << y)) ? 1 : 0;
}

void DM1_AutoMap_RecordVisit(M11_GameViewState* state) {
    if (!state || !state->active) return;
    if (!M11_QolRuntime_GetAutoMapEnabled()) return;
    automap_mark(state->world.party.mapIndex,
                 state->world.party.mapX,
                 state->world.party.mapY);
}

/* Pull the same square-type info the minimap renders so the BMP looks
 * like a top-down level layout. */
static unsigned char am_square_type(const struct DungeonDatState_Compat* d,
                                    int mapIdx, int x, int y) {
    const struct DungeonMapTiles_Compat* tiles;
    const struct DungeonMapDesc_Compat* desc;
    unsigned char raw;
    if (!d || !d->tilesLoaded || !d->tiles || !d->maps) return DUNGEON_ELEMENT_WALL;
    if (mapIdx < 0 || mapIdx >= (int)d->header.mapCount) return DUNGEON_ELEMENT_WALL;
    desc = &d->maps[mapIdx];
    if (x < 0 || y < 0 || x >= (int)desc->width || y >= (int)desc->height) return DUNGEON_ELEMENT_WALL;
    tiles = &d->tiles[mapIdx];
    if (!tiles->squareData) return DUNGEON_ELEMENT_WALL;
    raw = tiles->squareData[x * (int)desc->height + y];
    return (unsigned char)((raw & DUNGEON_SQUARE_MASK_TYPE) >> 5);
}

static void am_color_for(unsigned char etype,
                         unsigned char* r,
                         unsigned char* g,
                         unsigned char* b) {
    switch (etype) {
        case DUNGEON_ELEMENT_WALL:       *r = 30;  *g = 30;  *b = 30;  break;
        case DUNGEON_ELEMENT_DOOR:       *r = 160; *g = 80;  *b = 20;  break;
        case DUNGEON_ELEMENT_STAIRS:     *r = 240; *g = 220; *b = 60;  break;
        case DUNGEON_ELEMENT_PIT:        *r = 80;  *g = 80;  *b = 80;  break;
        case DUNGEON_ELEMENT_TELEPORTER: *r = 200; *g = 60;  *b = 200; break;
        case DUNGEON_ELEMENT_FAKEWALL:   *r = 120; *g = 120; *b = 120; break;
        case DUNGEON_ELEMENT_CORRIDOR:
        default:                         *r = 180; *g = 180; *b = 180; break;
    }
}

/* Write a uint32 little-endian to FILE*. */
static void put_u32_le(FILE* fp, uint32_t v) {
    unsigned char b[4];
    b[0] = (unsigned char)(v & 0xFFu);
    b[1] = (unsigned char)((v >> 8) & 0xFFu);
    b[2] = (unsigned char)((v >> 16) & 0xFFu);
    b[3] = (unsigned char)((v >> 24) & 0xFFu);
    fwrite(b, 1, 4, fp);
}
static void put_u16_le(FILE* fp, uint16_t v) {
    unsigned char b[2];
    b[0] = (unsigned char)(v & 0xFFu);
    b[1] = (unsigned char)((v >> 8) & 0xFFu);
    fwrite(b, 1, 2, fp);
}

static int am_mkdir_p(const char* path) {
    char buf[1024];
    size_t i, n;
    if (!path) return 0;
    n = strlen(path);
    if (n >= sizeof(buf)) return 0;
    memcpy(buf, path, n + 1);
    for (i = 1; i < n; ++i) {
        if (buf[i] == '/') {
            buf[i] = '\0';
            MKDIR(buf);
            buf[i] = '/';
        }
    }
    MKDIR(buf);
    return 1;
}

int DM1_AutoMap_ExportPNG(M11_GameViewState* state,
                          int mapIndex,
                          const char* outputPath) {
    const struct DungeonDatState_Compat* dungeon;
    const struct DungeonMapDesc_Compat* desc;
    int mapW, mapH;
    int cellPx = 8;
    int margin = 4;
    int imgW, imgH;
    int rowBytes, padBytes;
    int x, y;
    FILE* fp;
    unsigned char* rowBuf;

    if (!state || !state->active || !outputPath) return 0;
    dungeon = state->world.dungeon;
    if (!dungeon || !dungeon->loaded || !dungeon->tilesLoaded) return 0;
    if (mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return 0;
    desc = &dungeon->maps[mapIndex];
    mapW = (int)desc->width;
    mapH = (int)desc->height;
    if (mapW <= 0 || mapH <= 0) return 0;

    imgW = mapW * cellPx + margin * 2;
    imgH = mapH * cellPx + margin * 2;
    rowBytes = imgW * 3;
    padBytes = (4 - (rowBytes % 4)) % 4;

    /* Make parent directory tree, then open the file. */
    {
        char dirPath[1024];
        size_t i, n = strlen(outputPath);
        if (n >= sizeof(dirPath)) return 0;
        memcpy(dirPath, outputPath, n + 1);
        for (i = n; i > 0; --i) {
            if (dirPath[i - 1] == '/') {
                dirPath[i - 1] = '\0';
                am_mkdir_p(dirPath);
                break;
            }
        }
    }

    fp = fopen(outputPath, "wb");
    if (!fp) return 0;

    /* BMP file header (14 bytes) */
    {
        uint32_t pixelDataSize = (uint32_t)((rowBytes + padBytes) * imgH);
        uint32_t fileSize = 14 + 40 + pixelDataSize;
        fputc('B', fp);
        fputc('M', fp);
        put_u32_le(fp, fileSize);
        put_u16_le(fp, 0);
        put_u16_le(fp, 0);
        put_u32_le(fp, 14 + 40);
    }
    /* DIB header BITMAPINFOHEADER (40 bytes) */
    put_u32_le(fp, 40);
    put_u32_le(fp, (uint32_t)imgW);
    put_u32_le(fp, (uint32_t)imgH); /* positive => bottom-up */
    put_u16_le(fp, 1);
    put_u16_le(fp, 24);
    put_u32_le(fp, 0); /* BI_RGB */
    put_u32_le(fp, 0); /* image size (0 ok for BI_RGB) */
    put_u32_le(fp, 2835); /* x ppm (~72 dpi) */
    put_u32_le(fp, 2835);
    put_u32_le(fp, 0);
    put_u32_le(fp, 0);

    rowBuf = (unsigned char*)calloc((size_t)(rowBytes + padBytes), 1);
    if (!rowBuf) { fclose(fp); return 0; }

    /* BMP is bottom-up: write image row imgH-1 down to row 0 (in screen
     * coordinates), but in our map we treat y=0 as the top of the map. */
    for (y = imgH - 1; y >= 0; --y) {
        int mapY = (y - margin) / cellPx;
        for (x = 0; x < imgW; ++x) {
            int mapX = (x - margin) / cellPx;
            unsigned char r, g, b;
            int visited;
            int withinMap = (mapX >= 0 && mapX < mapW &&
                              mapY >= 0 && mapY < mapH);
            if (!withinMap) {
                r = 0; g = 0; b = 0;
            } else {
                visited = automap_get(mapIndex, mapX, mapY);
                if (!visited &&
                    !(state->world.party.mapIndex == mapIndex &&
                      ((state->exploredBits[((unsigned int)(mapX * 32 + mapY)) / 32U]
                        >> (((unsigned int)(mapX * 32 + mapY)) % 32U)) & 1U))) {
                    /* unexplored cell: render dark */
                    r = 10; g = 10; b = 18;
                } else {
                    am_color_for(am_square_type(dungeon, mapIndex, mapX, mapY),
                                 &r, &g, &b);
                }
                /* Party marker for current level: bright red. */
                if (state->world.party.mapIndex == mapIndex &&
                    mapX == state->world.party.mapX &&
                    mapY == state->world.party.mapY) {
                    r = 255; g = 40; b = 40;
                }
            }
            rowBuf[x * 3 + 0] = b;
            rowBuf[x * 3 + 1] = g;
            rowBuf[x * 3 + 2] = r;
        }
        fwrite(rowBuf, 1, (size_t)(rowBytes + padBytes), fp);
    }
    free(rowBuf);
    fclose(fp);
    return 1;
}

int DM1_AutoMap_ExportCurrentLevel(M11_GameViewState* state) {
    char path[1024];
    const char* home;
    int idx;
    if (!state || !state->active) return 0;
    idx = state->world.party.mapIndex;
    home = getenv("HOME");
    if (!home || !*home) home = ".";
    snprintf(path, sizeof(path), "%s/.firestaff/maps/dm1-level-%02d.bmp",
             home, idx);
    return DM1_AutoMap_ExportPNG(state, idx, path);
}
