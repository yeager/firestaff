#include "m11_game_view.h"

#include "asset_status_m12.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    M11_COLOR_BLACK = 0,
    M11_COLOR_NAVY = 1,
    M11_COLOR_GREEN = 2,
    M11_COLOR_CYAN = 3,
    M11_COLOR_RED = 4,
    M11_COLOR_BROWN = 6,
    M11_COLOR_LIGHT_GRAY = 7,
    M11_COLOR_DARK_GRAY = 8,
    M11_COLOR_LIGHT_BLUE = 9,
    M11_COLOR_LIGHT_GREEN = 10,
    M11_COLOR_LIGHT_CYAN = 11,
    M11_COLOR_LIGHT_RED = 12,
    M11_COLOR_MAGENTA = 13,
    M11_COLOR_YELLOW = 14,
    M11_COLOR_WHITE = 15
};

enum {
    M11_VIEWPORT_X = 12,
    M11_VIEWPORT_Y = 24,
    M11_VIEWPORT_W = 196,
    M11_VIEWPORT_H = 118,
    M11_CONTROL_STRIP_X = 14,
    M11_CONTROL_STRIP_Y = 165,
    M11_CONTROL_STRIP_W = 88,
    M11_CONTROL_STRIP_H = 14,
    M11_PROMPT_STRIP_X = 104,
    M11_PROMPT_STRIP_Y = 165,
    M11_PROMPT_STRIP_W = 202,
    M11_PROMPT_STRIP_H = 14,
    M11_PARTY_PANEL_X = 12,
    M11_PARTY_PANEL_Y = 160,
    M11_PARTY_SLOT_W = 71,
    M11_PARTY_SLOT_H = 28,
    M11_PARTY_SLOT_STEP = 77,
    M11_UTILITY_PANEL_X = 218,
    M11_UTILITY_PANEL_Y = 28,
    M11_UTILITY_PANEL_W = 86,
    M11_UTILITY_PANEL_H = 42,
    M11_UTILITY_BUTTON_Y = 56,
    M11_UTILITY_BUTTON_H = 10,
    M11_UTILITY_INSPECT_X = 222,
    M11_UTILITY_INSPECT_W = 22,
    M11_UTILITY_SAVE_X = 250,
    M11_UTILITY_SAVE_W = 22,
    M11_UTILITY_LOAD_X = 278,
    M11_UTILITY_LOAD_W = 22
};

enum {
    M11_QUICKSAVE_HEADER_SIZE = 16
};

static const unsigned char g_m11_quicksave_magic[8] = {
    'F', 'S', 'M', '1', '1', 'Q', 'S', '1'
};

typedef struct {
    char ch;
    unsigned char rows[7];
} M11_Glyph;

typedef struct {
    int scale;
    int tracking;
    unsigned char color;
    int shadowDx;
    int shadowDy;
    unsigned char shadowColor;
} M11_TextStyle;

static const M11_Glyph g_font[] = {
    {' ', {0, 0, 0, 0, 0, 0, 0}},
    {'-', {0, 0, 0, 31, 0, 0, 0}},
    {'.', {0, 0, 0, 0, 0, 12, 12}},
    {':', {0, 12, 12, 0, 12, 12, 0}},
    {'/', {1, 1, 2, 4, 8, 16, 16}},
    {'>', {1, 2, 4, 8, 4, 2, 1}},
    {'0', {14, 17, 19, 21, 25, 17, 14}},
    {'1', {4, 12, 4, 4, 4, 4, 14}},
    {'2', {14, 17, 1, 2, 4, 8, 31}},
    {'3', {30, 1, 1, 14, 1, 1, 30}},
    {'4', {2, 6, 10, 18, 31, 2, 2}},
    {'5', {31, 16, 16, 30, 1, 1, 30}},
    {'6', {14, 16, 16, 30, 17, 17, 14}},
    {'7', {31, 1, 2, 4, 8, 8, 8}},
    {'8', {14, 17, 17, 14, 17, 17, 14}},
    {'9', {14, 17, 17, 15, 1, 1, 14}},
    {'A', {14, 17, 17, 31, 17, 17, 17}},
    {'B', {30, 17, 17, 30, 17, 17, 30}},
    {'C', {14, 17, 16, 16, 16, 17, 14}},
    {'D', {30, 17, 17, 17, 17, 17, 30}},
    {'E', {31, 16, 16, 30, 16, 16, 31}},
    {'F', {31, 16, 16, 30, 16, 16, 16}},
    {'G', {14, 17, 16, 23, 17, 17, 14}},
    {'H', {17, 17, 17, 31, 17, 17, 17}},
    {'I', {31, 4, 4, 4, 4, 4, 31}},
    {'J', {7, 2, 2, 2, 18, 18, 12}},
    {'K', {17, 18, 20, 24, 20, 18, 17}},
    {'L', {16, 16, 16, 16, 16, 16, 31}},
    {'M', {17, 27, 21, 17, 17, 17, 17}},
    {'N', {17, 25, 21, 19, 17, 17, 17}},
    {'O', {14, 17, 17, 17, 17, 17, 14}},
    {'P', {30, 17, 17, 30, 16, 16, 16}},
    {'Q', {14, 17, 17, 17, 21, 18, 13}},
    {'R', {30, 17, 17, 30, 20, 18, 17}},
    {'S', {15, 16, 16, 14, 1, 1, 30}},
    {'T', {31, 4, 4, 4, 4, 4, 4}},
    {'U', {17, 17, 17, 17, 17, 17, 14}},
    {'V', {17, 17, 17, 17, 17, 10, 4}},
    {'W', {17, 17, 17, 17, 21, 27, 17}},
    {'X', {17, 17, 10, 4, 10, 17, 17}},
    {'Y', {17, 17, 10, 4, 4, 4, 4}},
    {'Z', {31, 1, 2, 4, 8, 16, 31}}
};

static const M11_TextStyle g_text_small = {1, 1, M11_COLOR_WHITE, 0, 0, M11_COLOR_BLACK};
static const M11_TextStyle g_text_shadow = {1, 1, M11_COLOR_WHITE, 1, 1, M11_COLOR_BLACK};
static const M11_TextStyle g_text_title = {2, 1, M11_COLOR_YELLOW, 1, 1, M11_COLOR_BLACK};

static void m11_put_pixel(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          unsigned char color) {
    if (!framebuffer) {
        return;
    }
    if (x < 0 || y < 0 || x >= framebufferWidth || y >= framebufferHeight) {
        return;
    }
    framebuffer[(y * framebufferWidth) + x] = color;
}

static void m11_fill_rect(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char color) {
    int yy;
    int xx;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                          x + xx, y + yy, color);
        }
    }
}

static void m11_draw_rect(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          int w,
                          int h,
                          unsigned char color) {
    int i;
    for (i = 0; i < w; ++i) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + i, y, color);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + i, y + h - 1, color);
    }
    for (i = 0; i < h; ++i) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x, y + i, color);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + w - 1, y + i, color);
    }
}

static void m11_draw_hline(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x1,
                           int x2,
                           int y,
                           unsigned char color) {
    int x;
    if (x2 < x1) {
        int swap = x1;
        x1 = x2;
        x2 = swap;
    }
    for (x = x1; x <= x2; ++x) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x, y, color);
    }
}

static void m11_draw_vline(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x,
                           int y1,
                           int y2,
                           unsigned char color) {
    int y;
    if (y2 < y1) {
        int swap = y1;
        y1 = y2;
        y2 = swap;
    }
    for (y = y1; y <= y2; ++y) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x, y, color);
    }
}

static const M11_Glyph* m11_find_glyph(char ch) {
    size_t i;
    unsigned char uch = (unsigned char)ch;
    if (uch >= 'a' && uch <= 'z') {
        ch = (char)(uch - ('a' - 'A'));
    }
    for (i = 0; i < sizeof(g_font) / sizeof(g_font[0]); ++i) {
        if (g_font[i].ch == ch) {
            return &g_font[i];
        }
    }
    return &g_font[0];
}

static void m11_draw_glyph(unsigned char* framebuffer,
                           int framebufferWidth,
                           int framebufferHeight,
                           int x,
                           int y,
                           char ch,
                           const M11_TextStyle* style,
                           int drawShadow) {
    const M11_Glyph* glyph = m11_find_glyph(ch);
    int row;
    int col;
    int yy;
    int xx;
    const M11_TextStyle* s = style ? style : &g_text_small;
    for (row = 0; row < 7; ++row) {
        for (col = 0; col < 5; ++col) {
            if ((glyph->rows[row] & (1 << (4 - col))) == 0) {
                continue;
            }
            for (yy = 0; yy < s->scale; ++yy) {
                for (xx = 0; xx < s->scale; ++xx) {
                    m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                                  x + col * s->scale + xx + (drawShadow ? s->shadowDx : 0),
                                  y + row * s->scale + yy + (drawShadow ? s->shadowDy : 0),
                                  drawShadow ? s->shadowColor : s->color);
                }
            }
        }
    }
}

static void m11_draw_text(unsigned char* framebuffer,
                          int framebufferWidth,
                          int framebufferHeight,
                          int x,
                          int y,
                          const char* text,
                          const M11_TextStyle* style) {
    int cursor = x;
    size_t i;
    const M11_TextStyle* s = style ? style : &g_text_small;
    if (!text) {
        return;
    }
    for (i = 0; text[i] != '\0'; ++i) {
        m11_draw_glyph(framebuffer, framebufferWidth, framebufferHeight,
                       cursor, y, text[i], s, 1);
        m11_draw_glyph(framebuffer, framebufferWidth, framebufferHeight,
                       cursor, y, text[i], s, 0);
        cursor += (5 * s->scale) + s->tracking;
    }
}

static const char* m11_direction_name(int dir) {
    switch (dir) {
        case DIR_NORTH: return "NORTH";
        case DIR_EAST: return "EAST";
        case DIR_SOUTH: return "SOUTH";
        case DIR_WEST: return "WEST";
        default: return "UNKNOWN";
    }
}

static const char* m11_source_name(M11_GameSourceKind sourceKind) {
    switch (sourceKind) {
        case M11_GAME_SOURCE_CUSTOM_DUNGEON:
            return "CUSTOM";
        case M11_GAME_SOURCE_DIRECT_DUNGEON:
            return "DIRECT";
        default:
            return "BUILTIN";
    }
}

static int m11_join_path(char* out,
                         size_t outSize,
                         const char* left,
                         const char* right) {
    int rc;
    size_t leftLen;
    if (!out || outSize == 0U || !left || !right) {
        return 0;
    }
    leftLen = strlen(left);
    rc = snprintf(out, outSize, "%s%s%s",
                  left,
                  (leftLen > 0U && left[leftLen - 1U] == '/') ? "" : "/",
                  right);
    return rc > 0 && (size_t)rc < outSize;
}

static void m11_write_u32_le(unsigned char* dst, uint32_t value) {
    if (!dst) {
        return;
    }
    dst[0] = (unsigned char)(value & 0xFFu);
    dst[1] = (unsigned char)((value >> 8) & 0xFFu);
    dst[2] = (unsigned char)((value >> 16) & 0xFFu);
    dst[3] = (unsigned char)((value >> 24) & 0xFFu);
}

static uint32_t m11_read_u32_le(const unsigned char* src) {
    if (!src) {
        return 0;
    }
    return ((uint32_t)src[0]) |
           ((uint32_t)src[1] << 8) |
           ((uint32_t)src[2] << 16) |
           ((uint32_t)src[3] << 24);
}

int M11_GameView_GetQuickSavePath(const M11_GameViewState* state,
                                  char* out,
                                  size_t outSize) {
    const char* envPath;
    const char* sourceId = "dm1";
    int rc;

    if (!out || outSize == 0U) {
        return 0;
    }

    envPath = getenv("FIRESTAFF_QUICKSAVE_PATH");
    if (envPath && envPath[0] != '\0') {
        rc = snprintf(out, outSize, "%s", envPath);
        return rc > 0 && (size_t)rc < outSize;
    }

    if (state && state->sourceId[0] != '\0') {
        sourceId = state->sourceId;
    }

    rc = snprintf(out, outSize, "firestaff-%s-quicksave.sav", sourceId);
    return rc > 0 && (size_t)rc < outSize;
}

static int m11_resolve_builtin_dungeon_path(char* out,
                                            size_t outSize,
                                            const char* dataDir,
                                            const char* gameId) {
    if (!out || !dataDir || !gameId) {
        return 0;
    }
    if (strcmp(gameId, "dm1") == 0) {
        return m11_join_path(out, outSize, dataDir, "DUNGEON.DAT");
    }
    if (strcmp(gameId, "csb") == 0) {
        return m11_join_path(out, outSize, dataDir, "CSB.DAT");
    }
    if (strcmp(gameId, "dm2") == 0) {
        return m11_join_path(out, outSize, dataDir, "DM2DUNGEON.DAT");
    }
    return 0;
}

static uint8_t m11_forward_command_for_direction(int direction) {
    switch (direction) {
        case DIR_NORTH: return CMD_MOVE_NORTH;
        case DIR_EAST: return CMD_MOVE_EAST;
        case DIR_SOUTH: return CMD_MOVE_SOUTH;
        case DIR_WEST: return CMD_MOVE_WEST;
        default: return CMD_NONE;
    }
}

static uint8_t m11_backward_command_for_direction(int direction) {
    switch (direction) {
        case DIR_NORTH: return CMD_MOVE_SOUTH;
        case DIR_EAST: return CMD_MOVE_WEST;
        case DIR_SOUTH: return CMD_MOVE_NORTH;
        case DIR_WEST: return CMD_MOVE_EAST;
        default: return CMD_NONE;
    }
}

static uint8_t m11_strafe_left_command_for_direction(int direction) {
    switch (direction) {
        case DIR_NORTH: return CMD_MOVE_WEST;
        case DIR_EAST: return CMD_MOVE_NORTH;
        case DIR_SOUTH: return CMD_MOVE_EAST;
        case DIR_WEST: return CMD_MOVE_SOUTH;
        default: return CMD_NONE;
    }
}

static uint8_t m11_strafe_right_command_for_direction(int direction) {
    switch (direction) {
        case DIR_NORTH: return CMD_MOVE_EAST;
        case DIR_EAST: return CMD_MOVE_SOUTH;
        case DIR_SOUTH: return CMD_MOVE_WEST;
        case DIR_WEST: return CMD_MOVE_NORTH;
        default: return CMD_NONE;
    }
}

static unsigned char m11_tile_color(int elementType) {
    switch (elementType) {
        case DUNGEON_ELEMENT_WALL: return M11_COLOR_DARK_GRAY;
        case DUNGEON_ELEMENT_CORRIDOR: return M11_COLOR_LIGHT_GRAY;
        case DUNGEON_ELEMENT_PIT: return M11_COLOR_BROWN;
        case DUNGEON_ELEMENT_STAIRS: return M11_COLOR_YELLOW;
        case DUNGEON_ELEMENT_DOOR: return M11_COLOR_LIGHT_RED;
        case DUNGEON_ELEMENT_TELEPORTER: return M11_COLOR_LIGHT_CYAN;
        case DUNGEON_ELEMENT_FAKEWALL: return M11_COLOR_MAGENTA;
        default: return M11_COLOR_BLACK;
    }
}

static int m11_map_square_base(const struct DungeonDatState_Compat* dungeon,
                               int mapIndex) {
    int i;
    int base = 0;
    if (!dungeon || !dungeon->maps || mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) {
        return -1;
    }
    for (i = 0; i < mapIndex; ++i) {
        base += (int)dungeon->maps[i].width * (int)dungeon->maps[i].height;
    }
    return base;
}

static int m11_get_square_byte(const struct GameWorld_Compat* world,
                               int mapIndex,
                               int mapX,
                               int mapY,
                               unsigned char* outSquare) {
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int index;
    if (!world || !world->dungeon || !world->dungeon->tilesLoaded || !outSquare) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return 0;
    }
    tiles = &world->dungeon->tiles[mapIndex];
    index = mapX * (int)map->height + mapY;
    if (!tiles->squareData || index < 0 || index >= tiles->squareCount) {
        return 0;
    }
    *outSquare = tiles->squareData[index];
    return 1;
}

static unsigned char* m11_get_square_ptr(struct GameWorld_Compat* world,
                                         int mapIndex,
                                         int mapX,
                                         int mapY) {
    const struct DungeonMapDesc_Compat* map;
    struct DungeonMapTiles_Compat* tiles;
    int index;
    if (!world || !world->dungeon || !world->dungeon->tilesLoaded) {
        return NULL;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return NULL;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return NULL;
    }
    tiles = &world->dungeon->tiles[mapIndex];
    index = mapX * (int)map->height + mapY;
    if (!tiles->squareData || index < 0 || index >= tiles->squareCount) {
        return NULL;
    }
    return &tiles->squareData[index];
}

static unsigned short m11_raw_next_thing(const struct DungeonThings_Compat* things,
                                         unsigned short thing) {
    int type;
    int index;
    const unsigned char* raw;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    type = THING_GET_TYPE(thing);
    index = THING_GET_INDEX(thing);
    if (type < 0 || type >= 16 || !things->rawThingData[type] || index < 0 || index >= things->thingCounts[type]) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type] + (index * s_thingDataByteCount[type]);
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static unsigned short m11_get_first_square_thing(const struct GameWorld_Compat* world,
                                                 int mapIndex,
                                                 int mapX,
                                                 int mapY) {
    int base;
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    if (!world || !world->dungeon || !world->things || !world->things->squareFirstThings) {
        return THING_ENDOFLIST;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return THING_ENDOFLIST;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return THING_ENDOFLIST;
    }
    base = m11_map_square_base(world->dungeon, mapIndex);
    if (base < 0) {
        return THING_ENDOFLIST;
    }
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return THING_ENDOFLIST;
    }
    return world->things->squareFirstThings[squareIndex];
}

static int m11_count_square_things(const struct GameWorld_Compat* world,
                                   int mapIndex,
                                   int mapX,
                                   int mapY) {
    unsigned short thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);
    int count = 0;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && count < 32) {
        ++count;
        thing = m11_raw_next_thing(world->things, thing);
    }
    return count;
}

typedef struct {
    int total;
    int groups;
    int items;
    int sensors;
    int textStrings;
    int teleporters;
    int projectiles;
    int explosions;
    int doors;
} M11_SquareThingSummary;

struct M11_ViewportCell;

static int m11_thing_is_item(int thingType) {
    switch (thingType) {
        case THING_TYPE_WEAPON:
        case THING_TYPE_ARMOUR:
        case THING_TYPE_SCROLL:
        case THING_TYPE_POTION:
        case THING_TYPE_CONTAINER:
        case THING_TYPE_JUNK:
            return 1;
        default:
            return 0;
    }
}

static void m11_summarize_square_things(const struct GameWorld_Compat* world,
                                        int mapIndex,
                                        int mapX,
                                        int mapY,
                                        M11_SquareThingSummary* outSummary) {
    M11_SquareThingSummary summary;
    unsigned short thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);

    memset(&summary, 0, sizeof(summary));
    while (thing != THING_ENDOFLIST && thing != THING_NONE && summary.total < 32) {
        int thingType = THING_GET_TYPE(thing);
        ++summary.total;
        switch (thingType) {
            case THING_TYPE_DOOR:
                ++summary.doors;
                break;
            case THING_TYPE_TELEPORTER:
                ++summary.teleporters;
                break;
            case THING_TYPE_TEXTSTRING:
                ++summary.textStrings;
                break;
            case THING_TYPE_SENSOR:
                ++summary.sensors;
                break;
            case THING_TYPE_GROUP:
                ++summary.groups;
                break;
            case THING_TYPE_PROJECTILE:
                ++summary.projectiles;
                break;
            case THING_TYPE_EXPLOSION:
                ++summary.explosions;
                break;
            default:
                if (m11_thing_is_item(thingType)) {
                    ++summary.items;
                }
                break;
        }
        thing = m11_raw_next_thing(world->things, thing);
    }

    if (outSummary) {
        *outSummary = summary;
    }
}

static void m11_set_status(M11_GameViewState* state,
                           const char* action,
                           const char* outcome) {
    if (!state) {
        return;
    }
    snprintf(state->lastAction, sizeof(state->lastAction), "%s", action ? action : "NONE");
    snprintf(state->lastOutcome, sizeof(state->lastOutcome), "%s", outcome ? outcome : "");
}

static void m11_set_inspect_readout(M11_GameViewState* state,
                                    const char* title,
                                    const char* detail) {
    if (!state) {
        return;
    }
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s",
             title ? title : "NO FOCUS");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail), "%s",
             detail ? detail : "PRESS ENTER ON A REAL FRONT-CELL TARGET");
}

static void m11_refresh_hash(M11_GameViewState* state) {
    uint32_t hash = 0;
    if (!state) {
        return;
    }
    if (F0891_ORCH_WorldHash_Compat(&state->world, &hash) == 1) {
        state->lastWorldHash = hash;
    }
}

static int m11_inspect_front_cell(M11_GameViewState* state);
static int m11_front_cell_has_attack_target(const M11_GameViewState* state);
static int m11_front_cell_is_door(const M11_GameViewState* state);
static int m11_get_front_cell(const M11_GameViewState* state, struct M11_ViewportCell* outCell);
static int m11_toggle_front_door(M11_GameViewState* state);
static int m11_apply_tick(M11_GameViewState* state,
                          uint8_t command,
                          const char* actionLabel);
static void m11_check_party_death(M11_GameViewState* state);
static M12_MenuInput m11_pointer_viewport_input(const M11_GameViewState* state,
                                                int x,
                                                int y);
static void m11_get_active_champion_label(const M11_GameViewState* state,
                                          char* out,
                                          size_t outSize);
static void m11_format_champion_name(const unsigned char* raw,
                                     char* out,
                                     size_t outSize);
static const struct ChampionState_Compat* m11_get_active_champion(const M11_GameViewState* state);
static int m11_cycle_active_champion(M11_GameViewState* state);
static int m11_set_active_champion(M11_GameViewState* state, int championIndex);

static int m11_point_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
    return x >= rx && y >= ry && x < (rx + rw) && y < (ry + rh);
}

static int m11_point_in_utility_button(int x,
                                       int y,
                                       int buttonX,
                                       int buttonWidth) {
    return m11_point_in_rect(x, y,
                             buttonX,
                             M11_UTILITY_BUTTON_Y,
                             buttonWidth,
                             M11_UTILITY_BUTTON_H);
}

static const struct ChampionState_Compat* m11_get_active_champion(const M11_GameViewState* state) {
    int index;

    if (!state) {
        return NULL;
    }

    index = state->world.party.activeChampionIndex;
    if (index < 0 || index >= CHAMPION_MAX_PARTY) {
        return NULL;
    }
    if (index >= state->world.party.championCount || !state->world.party.champions[index].present) {
        return NULL;
    }

    return &state->world.party.champions[index];
}

/* ================================================================
 * Message log — ring buffer for event history
 * ================================================================ */

void M11_MessageLog_Push(M11_MessageLog* log, const char* text, unsigned char color) {
    M11_LogEntry* entry;
    if (!log || !text) {
        return;
    }
    entry = &log->entries[log->writeIndex];
    snprintf(entry->text, M11_MESSAGE_MAX_LENGTH, "%s", text);
    entry->color = color;
    log->writeIndex = (log->writeIndex + 1) % M11_MESSAGE_LOG_CAPACITY;
    if (log->count < M11_MESSAGE_LOG_CAPACITY) {
        ++log->count;
    }
}

int M11_GameView_GetMessageLogCount(const M11_GameViewState* state) {
    return state ? state->messageLog.count : 0;
}

const char* M11_GameView_GetMessageLogEntry(const M11_GameViewState* state, int reverseIndex) {
    int idx;
    if (!state || reverseIndex < 0 || reverseIndex >= state->messageLog.count) {
        return NULL;
    }
    idx = (state->messageLog.writeIndex - 1 - reverseIndex + M11_MESSAGE_LOG_CAPACITY) % M11_MESSAGE_LOG_CAPACITY;
    return state->messageLog.entries[idx].text;
}

static const M11_LogEntry* m11_log_entry_at(const M11_MessageLog* log, int reverseIndex) {
    int idx;
    if (!log || reverseIndex < 0 || reverseIndex >= log->count) {
        return NULL;
    }
    idx = (log->writeIndex - 1 - reverseIndex + M11_MESSAGE_LOG_CAPACITY) % M11_MESSAGE_LOG_CAPACITY;
    return &log->entries[idx];
}

static void m11_log_event(M11_GameViewState* state, unsigned char color, const char* fmt, ...) {
    char buf[M11_MESSAGE_MAX_LENGTH];
    va_list ap;
    if (!state) {
        return;
    }
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    M11_MessageLog_Push(&state->messageLog, buf, color);
}

/* ================================================================
 * Explored cell tracking
 * ================================================================ */

static void m11_mark_explored(M11_GameViewState* state) {
    unsigned int cell;
    if (!state || !state->active) {
        return;
    }
    cell = (unsigned int)(state->world.party.mapX * 32 + state->world.party.mapY);
    if (cell < 1024U) {
        state->exploredBits[cell / 32U] |= (1U << (cell % 32U));
    }
}

static int m11_is_explored(const M11_GameViewState* state, int mapX, int mapY) {
    unsigned int cell;
    if (!state) {
        return 0;
    }
    cell = (unsigned int)(mapX * 32 + mapY);
    if (cell >= 1024U) {
        return 0;
    }
    return (state->exploredBits[cell / 32U] & (1U << (cell % 32U))) != 0;
}

/* ================================================================
 * Level transitions
 * ================================================================ */

static int m11_try_stairs_transition(M11_GameViewState* state) {
    unsigned char square = 0;
    int elementType;
    int stairUp;
    int targetLevel;
    const struct DungeonMapDesc_Compat* targetMap;

    if (!state || !state->active || !state->world.dungeon) {
        return 0;
    }
    if (!m11_get_square_byte(&state->world,
                             state->world.party.mapIndex,
                             state->world.party.mapX,
                             state->world.party.mapY,
                             &square)) {
        return 0;
    }
    elementType = (square >> 5) & 7;
    if (elementType != DUNGEON_ELEMENT_STAIRS) {
        return 0;
    }

    /* Attribute bit 0 of the low nibble selects direction:
     *   0 = stairs down (mapIndex + 1)
     *   1 = stairs up   (mapIndex - 1)
     * This matches the Fontanel convention for DM1 stair encoding. */
    stairUp = (square & 0x01);
    if (stairUp) {
        targetLevel = state->world.party.mapIndex - 1;
    } else {
        targetLevel = state->world.party.mapIndex + 1;
    }
    if (targetLevel < 0 || targetLevel >= (int)state->world.dungeon->header.mapCount) {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: STAIRS LEAD NOWHERE",
                      (unsigned int)state->world.gameTick);
        return 0;
    }

    targetMap = &state->world.dungeon->maps[targetLevel];
    state->world.party.mapIndex = targetLevel;

    /* Clamp position to the new map bounds */
    if (state->world.party.mapX >= (int)targetMap->width) {
        state->world.party.mapX = (int)targetMap->width - 1;
    }
    if (state->world.party.mapY >= (int)targetMap->height) {
        state->world.party.mapY = (int)targetMap->height - 1;
    }

    memset(state->exploredBits, 0, sizeof(state->exploredBits));
    m11_mark_explored(state);
    m11_refresh_hash(state);
    if (stairUp) {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: ASCENDED TO LEVEL %d",
                      (unsigned int)state->world.gameTick,
                      targetLevel + 1);
        m11_set_status(state, "STAIRS", "ASCENDED TO PREVIOUS LEVEL");
    } else {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: DESCENDED TO LEVEL %d",
                      (unsigned int)state->world.gameTick,
                      targetLevel + 1);
        m11_set_status(state, "STAIRS", "DESCENDED TO NEXT LEVEL");
    }
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "LEVEL %d", targetLevel + 1);
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "MAP %dx%d, ENTERED FROM STAIRS",
             (int)targetMap->width, (int)targetMap->height);
    return 1;
}

/* ================================================================
 * Pit transition — party falls to the level below
 * ================================================================ */

static int m11_check_pit_fall(M11_GameViewState* state) {
    unsigned char square = 0;
    int elementType;
    int targetLevel;
    const struct DungeonMapDesc_Compat* targetMap;
    int i;
    int pitIsOpen;

    if (!state || !state->active || !state->world.dungeon) {
        return 0;
    }
    if (!m11_get_square_byte(&state->world,
                             state->world.party.mapIndex,
                             state->world.party.mapX,
                             state->world.party.mapY,
                             &square)) {
        return 0;
    }
    elementType = (square >> 5) & 7;
    if (elementType != DUNGEON_ELEMENT_PIT) {
        return 0;
    }

    /* Pit open state: bit 0 of square low nibble.
       In DM, pits have an "open" flag — an open pit is passable but
       the party falls through. A closed (imaginary) pit is just floor. */
    pitIsOpen = (square & 0x01) ? 1 : 1; /* In DM1, stepping on a pit
       square always triggers the fall. The element type itself means
       the pit exists. Bit 3 distinguishes open/closed in some versions
       but the conservative approach: pit element = fall. */
    if (!pitIsOpen) {
        return 0;
    }

    /* Fall to the level below */
    targetLevel = state->world.party.mapIndex + 1;
    if (targetLevel >= (int)state->world.dungeon->header.mapCount) {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: PIT LEADS NOWHERE",
                      (unsigned int)state->world.gameTick);
        return 0;
    }

    targetMap = &state->world.dungeon->maps[targetLevel];
    state->world.party.mapIndex = targetLevel;

    /* Keep X/Y the same — pit drops vertically. Clamp to map bounds. */
    if (state->world.party.mapX >= (int)targetMap->width) {
        state->world.party.mapX = (int)targetMap->width - 1;
    }
    if (state->world.party.mapY >= (int)targetMap->height) {
        state->world.party.mapY = (int)targetMap->height - 1;
    }

    /* Apply fall damage: 5-15 HP to each living champion */
    for (i = 0; i < state->world.party.championCount && i < CHAMPION_MAX_PARTY; ++i) {
        struct ChampionState_Compat* champ = &state->world.party.champions[i];
        if (champ->present && champ->hp.current > 0) {
            int fallDamage = 5 + (int)(state->world.gameTick % 11U);
            champ->hp.current -= (int16_t)fallDamage;
            if (champ->hp.current < 0) {
                champ->hp.current = 0;
            }
            m11_log_event(state, M11_COLOR_LIGHT_RED,
                          "T%u: %c%c TAKES %d FALL DAMAGE",
                          (unsigned int)state->world.gameTick,
                          champ->name[0] ? (char)champ->name[0] : '?',
                          champ->name[1] ? (char)champ->name[1] : '?',
                          fallDamage);
        }
    }

    memset(state->exploredBits, 0, sizeof(state->exploredBits));
    m11_mark_explored(state);
    m11_refresh_hash(state);
    m11_log_event(state, M11_COLOR_YELLOW, "T%u: FELL INTO PIT! LEVEL %d",
                  (unsigned int)state->world.gameTick,
                  targetLevel + 1);
    m11_set_status(state, "PIT", "FELL TO NEXT LEVEL");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "PIT FALL");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "DROPPED TO LEVEL %d, MAP %dx%d",
             targetLevel + 1,
             (int)targetMap->width, (int)targetMap->height);
    return 1;
}

/* ================================================================
 * Teleporter transition — find teleporter thing and transport party
 * ================================================================ */

static int m11_find_teleporter_on_square(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY,
    struct DungeonTeleporter_Compat* outTeleporter) {
    unsigned short thing;
    int safety = 0;

    if (!world || !world->things || !outTeleporter) {
        return 0;
    }
    thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 32) {
        int thingType = THING_GET_TYPE(thing);
        if (thingType == THING_TYPE_TELEPORTER) {
            int idx = THING_GET_INDEX(thing);
            if (idx >= 0 && idx < world->things->teleporterCount &&
                world->things->teleporters) {
                *outTeleporter = world->things->teleporters[idx];
                return 1;
            }
        }
        thing = m11_raw_next_thing(world->things, thing);
        ++safety;
    }
    return 0;
}

static int m11_check_teleporter(M11_GameViewState* state) {
    unsigned char square = 0;
    int elementType;
    struct DungeonTeleporter_Compat tp;
    int targetMapIndex;
    int targetX, targetY;
    const struct DungeonMapDesc_Compat* targetMap;

    if (!state || !state->active || !state->world.dungeon) {
        return 0;
    }
    if (!m11_get_square_byte(&state->world,
                             state->world.party.mapIndex,
                             state->world.party.mapX,
                             state->world.party.mapY,
                             &square)) {
        return 0;
    }
    elementType = (square >> 5) & 7;
    if (elementType != DUNGEON_ELEMENT_TELEPORTER) {
        return 0;
    }

    memset(&tp, 0, sizeof(tp));
    if (!m11_find_teleporter_on_square(&state->world,
                                       state->world.party.mapIndex,
                                       state->world.party.mapX,
                                       state->world.party.mapY,
                                       &tp)) {
        m11_log_event(state, M11_COLOR_LIGHT_CYAN,
                      "T%u: TELEPORTER HAS NO THING DATA",
                      (unsigned int)state->world.gameTick);
        return 0;
    }

    targetMapIndex = (int)tp.targetMapIndex;
    targetX = (int)tp.targetMapX;
    targetY = (int)tp.targetMapY;

    /* Validate target map */
    if (targetMapIndex < 0 || targetMapIndex >= (int)state->world.dungeon->header.mapCount) {
        m11_log_event(state, M11_COLOR_LIGHT_RED,
                      "T%u: TELEPORTER TARGET MAP %d OUT OF RANGE",
                      (unsigned int)state->world.gameTick, targetMapIndex);
        return 0;
    }
    targetMap = &state->world.dungeon->maps[targetMapIndex];

    /* Clamp target coordinates */
    if (targetX >= (int)targetMap->width) {
        targetX = (int)targetMap->width - 1;
    }
    if (targetY >= (int)targetMap->height) {
        targetY = (int)targetMap->height - 1;
    }
    if (targetX < 0) targetX = 0;
    if (targetY < 0) targetY = 0;

    /* Apply teleport */
    state->world.party.mapIndex = targetMapIndex;
    state->world.party.mapX = targetX;
    state->world.party.mapY = targetY;

    /* Apply rotation if specified */
    if (tp.absoluteRotation) {
        state->world.party.direction = (int)(tp.rotation & 3);
    } else if (tp.rotation != 0) {
        state->world.party.direction =
            (state->world.party.direction + (int)(tp.rotation & 3)) & 3;
    }

    /* Reset explored bits if we changed maps */
    memset(state->exploredBits, 0, sizeof(state->exploredBits));
    m11_mark_explored(state);
    m11_refresh_hash(state);

    if (tp.audible) {
        m11_log_event(state, M11_COLOR_LIGHT_CYAN,
                      "T%u: TELEPORTED TO MAP %d (%d,%d)",
                      (unsigned int)state->world.gameTick,
                      targetMapIndex + 1, targetX, targetY);
    } else {
        /* Silent teleporters — player might not even notice */
        m11_log_event(state, M11_COLOR_DARK_GRAY,
                      "T%u: SHIFTED TO (%d,%d)",
                      (unsigned int)state->world.gameTick,
                      targetX, targetY);
    }
    m11_set_status(state, "TELEPORT", "PARTY TRANSPORTED");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "TELEPORTER");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "ARRIVED AT LEVEL %d, POSITION (%d,%d)",
             targetMapIndex + 1, targetX, targetY);
    return 1;
}

/* Check for environmental transitions after party movement.
 * Handles pits (fall) and teleporters (transport).
 * Returns 1 if a transition occurred. */
static int m11_check_post_move_transitions(M11_GameViewState* state) {
    int transitioned = 0;
    int safetyLimit = 4; /* Prevent infinite teleporter chains */

    if (!state || !state->active) {
        return 0;
    }

    while (safetyLimit > 0) {
        if (m11_check_pit_fall(state)) {
            transitioned = 1;
            --safetyLimit;
            m11_check_party_death(state);
            continue;
        }
        if (m11_check_teleporter(state)) {
            transitioned = 1;
            --safetyLimit;
            continue;
        }
        break;
    }
    return transitioned;
}

int M11_GameView_CheckPostMoveTransitions(M11_GameViewState* state) {
    return m11_check_post_move_transitions(state);
}

/* ================================================================
 * Item name helpers
 * ================================================================ */

static const char* const s_weaponTypeNames[] = {
    "EYE OF TIME", "STORMRING", "TORCH", "FLAMITT",
    "STAFF OF CLAWS", "BOLT BLADE", "FURY", "THE FIRESTAFF",
    "DAGGER", "FALCHION", "SWORD", "RAPIER",
    "SABRE", "SAMURAI SWORD", "DELTA", "DIAMOND EDGE",
    "VORPAL BLADE", "THE INQUISITOR", "AXE", "HARDCLEAVE",
    "MACE", "MACE OF ORDER", "MORNING STAR", "CLUB",
    "STONE CLUB", "BOW", "CROSSBOW", "ARROW",
    "SLAYER", "SLING", "ROCK", "POISON DART",
    "THROWING STAR", "STICK", "STAFF", "WAND",
    "TEOWAND", "YEW STAFF", "STAFF OF MANAR", "SNAKE STAFF",
    "THE CONDUIT", "DRAGON SPIT", "SCEPTRE OF LYF", "HORN OF FEAR",
    "SPEED BOW", "THE FIRESTAFF"
};

static const char* const s_armourTypeNames[] = {
    "CAPE", "CLOAK OF NIGHT", "BARBARIAN HIDE", "SANDALS",
    "LEATHER BOOTS", "ELVEN BOOTS", "LEATHER JERKIN", "LEATHER PANTS",
    "SUEDE BOOTS", "BLUE PANTS", "GHI", "GHI TROUSERS",
    "CALISTA", "CROWN OF NERRA", "BEZERKER HELM", "HELMET",
    "BASINET", "NETA SHIRT", "CHAINMAIL", "PLATE MAIL",
    "MITHRAL MAIL", "MITHRAL HOSEN", "LEG MAIL", "FOOT PLATE",
    "SMALL SHIELD", "WOODEN SHIELD", "LARGE SHIELD", "SHIELD OF LYTE",
    "SHIELD OF DARC", "DEXHELM"
};

static const char* const s_potionTypeNames[] = {
    "MON POTION", "UM POTION", "DES POTION", "VEN POTION",
    "SAR POTION", "ZO POTION", "ROS POTION", "KU POTION",
    "DANE POTION", "NETA POTION", "BRO POTION", "MA POTION",
    "YA POTION", "EE POTION", "VI POTION", "WATER FLASK",
    "EMPTY FLASK"
};

static const char* const s_junkTypeNames[] = {
    "COMPASS", "TORCH", "WATERSKIN", "JEWEL SYMAL",
    "ILLUMULET", "ASHES", "BONES", "SAR COIN",
    "GOLD COIN", "IRON KEY", "KEY OF B", "SOLID KEY",
    "SQUARE KEY", "TOURQUOISE KEY", "CROSS KEY", "ONYX KEY",
    "SKELETON KEY", "GOLD KEY", "WINGED KEY", "TOPAZ KEY",
    "SAPPHIRE KEY", "EMERALD KEY", "RUBY KEY", "RA KEY",
    "MASTER KEY", "BOULDER", "BLUE GEM", "ORANGE GEM",
    "GREEN GEM", "APPLE", "CORN", "BREAD",
    "CHEESE", "SCREAMER SLICE", "WORM ROUND", "DRUMSTICK",
    "DRAGON STEAK", "GEM OF AGES", "EKKHARD CROSS", "MOONSTONE",
    "THE HELLION", "PENDANT FERAL", "MAGICAL BOX", "MIRROR OF DAWN",
    "ROPE", "RABBIT FOOT", "CORBAMITE", "CHOKER",
    "LOCK PICKS", "MAGNIFIER", "ZOKATHRA SPELL", "EMPTY FLASK"
};

/* Scroll names not needed yet — all scrolls are just "SCROLL" */

static const char* const s_containerTypeNames[] = {
    "CHEST", "OPEN CHEST", "OPEN CHEST"
};

/* Creature type name table (C00..C26, matching g_profiles order). */
static const char* const s_creatureTypeNames[27] = {
    "GIANT SCORPION",  /* C00 */
    "SWAMP SLIME",     /* C01 */
    "GIGGLER",         /* C02 */
    "WIZARD EYE",      /* C03 */
    "PAIN RAT",        /* C04 */
    "RUSTER",          /* C05 */
    "SCREAMER",        /* C06 */
    "ROCKPILE",        /* C07 */
    "GHOST",           /* C08 */
    "STONE GOLEM",     /* C09 */
    "MUMMY",           /* C10 */
    "BLACK FLAME",     /* C11 */
    "SKELETON",        /* C12 */
    "COUATL",          /* C13 */
    "VEXIRK",          /* C14 */
    "MAGENTA WORM",    /* C15 */
    "TROLIN",          /* C16 */
    "GIANT WASP",      /* C17 */
    "ANIMATED ARMOUR", /* C18 */
    "MATERIALIZER",    /* C19 */
    "WATER ELEMENTAL", /* C20 */
    "OITU",            /* C21 */
    "DEMON",           /* C22 */
    "LORD CHAOS",      /* C23 */
    "RED DRAGON",      /* C24 */
    "LORD ORDER",      /* C25 */
    "GREY LORD"        /* C26 */
};

static const char* m11_creature_name(int creatureType) {
    if (creatureType >= 0 && creatureType < 27) {
        return s_creatureTypeNames[creatureType];
    }
    return "UNKNOWN CREATURE";
}

static void m11_get_item_name(const struct DungeonThings_Compat* things,
                              unsigned short thingId,
                              char* out,
                              size_t outSize) {
    int thingType;
    int thingIndex;
    if (!out || outSize == 0U) {
        return;
    }
    if (!things || thingId == THING_NONE || thingId == THING_ENDOFLIST) {
        snprintf(out, outSize, "NOTHING");
        return;
    }
    thingType = THING_GET_TYPE(thingId);
    thingIndex = THING_GET_INDEX(thingId);
    switch (thingType) {
        case THING_TYPE_WEAPON:
            if (things->weapons && thingIndex >= 0 && thingIndex < things->weaponCount) {
                int subtype = things->weapons[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_weaponTypeNames) / sizeof(s_weaponTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_weaponTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "WEAPON %d", thingIndex);
            return;
        case THING_TYPE_ARMOUR:
            if (things->armours && thingIndex >= 0 && thingIndex < things->armourCount) {
                int subtype = things->armours[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_armourTypeNames) / sizeof(s_armourTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_armourTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "ARMOUR %d", thingIndex);
            return;
        case THING_TYPE_POTION:
            if (things->potions && thingIndex >= 0 && thingIndex < things->potionCount) {
                int subtype = things->potions[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_potionTypeNames) / sizeof(s_potionTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_potionTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "POTION %d", thingIndex);
            return;
        case THING_TYPE_JUNK:
            if (things->junks && thingIndex >= 0 && thingIndex < things->junkCount) {
                int subtype = things->junks[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_junkTypeNames) / sizeof(s_junkTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_junkTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "JUNK %d", thingIndex);
            return;
        case THING_TYPE_SCROLL:
            snprintf(out, outSize, "SCROLL");
            return;
        case THING_TYPE_CONTAINER:
            if (things->containers && thingIndex >= 0 && thingIndex < things->containerCount) {
                int subtype = things->containers[thingIndex].type;
                if (subtype >= 0 && subtype < (int)(sizeof(s_containerTypeNames) / sizeof(s_containerTypeNames[0]))) {
                    snprintf(out, outSize, "%s", s_containerTypeNames[subtype]);
                    return;
                }
            }
            snprintf(out, outSize, "CONTAINER %d", thingIndex);
            return;
        default:
            snprintf(out, outSize, "%s %d",
                     F0505_DUNGEON_GetThingTypeName_Compat(thingType), thingIndex);
            return;
    }
}

/* ================================================================
 * Thing chain manipulation: remove an item from a square,
 * prepend an item to a square
 * ================================================================ */

static void m11_set_raw_next_thing(struct DungeonThings_Compat* things,
                                   unsigned short thingId,
                                   unsigned short newNext) {
    int type;
    int index;
    unsigned char* raw;
    if (!things || thingId == THING_NONE || thingId == THING_ENDOFLIST) {
        return;
    }
    type = THING_GET_TYPE(thingId);
    index = THING_GET_INDEX(thingId);
    if (type < 0 || type >= 16 || !things->rawThingData[type] ||
        index < 0 || index >= things->thingCounts[type]) {
        return;
    }
    raw = things->rawThingData[type] + (index * s_thingDataByteCount[type]);
    raw[0] = (unsigned char)(newNext & 0xFFu);
    raw[1] = (unsigned char)((newNext >> 8) & 0xFFu);
}

/* Remove a specific thing from a square's chain. Returns 1 if removed. */
static int m11_unlink_thing_from_square(struct GameWorld_Compat* world,
                                        int mapIndex,
                                        int mapX,
                                        int mapY,
                                        unsigned short target) {
    int base;
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    unsigned short current;
    unsigned short prev;
    int safety = 0;

    if (!world || !world->dungeon || !world->things || !world->things->squareFirstThings) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return 0;
    }
    base = m11_map_square_base(world->dungeon, mapIndex);
    if (base < 0) {
        return 0;
    }
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    current = world->things->squareFirstThings[squareIndex];
    prev = THING_ENDOFLIST;

    while (current != THING_ENDOFLIST && current != THING_NONE && safety < 64) {
        if (current == target) {
            unsigned short next = m11_raw_next_thing(world->things, current);
            if (prev == THING_ENDOFLIST) {
                world->things->squareFirstThings[squareIndex] = next;
            } else {
                m11_set_raw_next_thing(world->things, prev, next);
            }
            m11_set_raw_next_thing(world->things, current, THING_ENDOFLIST);
            return 1;
        }
        prev = current;
        current = m11_raw_next_thing(world->things, current);
        ++safety;
    }
    return 0;
}

/* Prepend a thing to a square's chain. */
static int m11_prepend_thing_to_square(struct GameWorld_Compat* world,
                                       int mapIndex,
                                       int mapX,
                                       int mapY,
                                       unsigned short thingId) {
    int base;
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    unsigned short oldFirst;

    if (!world || !world->dungeon || !world->things || !world->things->squareFirstThings) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) {
        return 0;
    }
    base = m11_map_square_base(world->dungeon, mapIndex);
    if (base < 0) {
        return 0;
    }
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) {
        return 0;
    }

    oldFirst = world->things->squareFirstThings[squareIndex];
    m11_set_raw_next_thing(world->things, thingId, oldFirst);
    world->things->squareFirstThings[squareIndex] = thingId;
    return 1;
}

/* Find the first item-type thing on a square. */
static unsigned short m11_find_first_item_on_square(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY) {
    unsigned short thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);
    int safety = 0;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
        if (m11_thing_is_item(THING_GET_TYPE(thing))) {
            return thing;
        }
        thing = m11_raw_next_thing(world->things, thing);
        ++safety;
    }
    return THING_NONE;
}

/* Find the first empty backpack/pouch slot for a champion. */
static int m11_find_empty_slot(const struct ChampionState_Compat* champ) {
    int slot;
    if (!champ || !champ->present) {
        return -1;
    }
    /* Prefer hands first, then pouches, then backpack */
    if (champ->inventory[CHAMPION_SLOT_HAND_LEFT] == THING_NONE) {
        return CHAMPION_SLOT_HAND_LEFT;
    }
    if (champ->inventory[CHAMPION_SLOT_HAND_RIGHT] == THING_NONE) {
        return CHAMPION_SLOT_HAND_RIGHT;
    }
    for (slot = CHAMPION_SLOT_POUCH_1; slot <= CHAMPION_SLOT_POUCH_2; ++slot) {
        if (champ->inventory[slot] == THING_NONE) {
            return slot;
        }
    }
    for (slot = CHAMPION_SLOT_BACKPACK_1; slot <= CHAMPION_SLOT_BACKPACK_8; ++slot) {
        if (champ->inventory[slot] == THING_NONE) {
            return slot;
        }
    }
    return -1;
}

/* Count items in a champion's inventory. */
int M11_GameView_CountChampionItems(const M11_GameViewState* state, int championIndex) {
    int count = 0;
    int slot;
    const struct ChampionState_Compat* champ;
    if (!state || championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }
    champ = &state->world.party.champions[championIndex];
    if (!champ->present) {
        return 0;
    }
    for (slot = 0; slot < CHAMPION_SLOT_COUNT; ++slot) {
        if (champ->inventory[slot] != THING_NONE) {
            ++count;
        }
    }
    return count;
}

int M11_GameView_GetSkillLevel(const M11_GameViewState* state,
                               int championIndex,
                               int skillIndex) {
    if (!state || !state->active) return -1;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return -1;
    if (!state->world.party.champions[championIndex].present) return -1;
    if (skillIndex < 0 || skillIndex >= CHAMPION_SKILL_COUNT) return -1;
    return F0848_LIFECYCLE_ComputeSkillLevel_Compat(
        &state->world.lifecycle.champions[championIndex],
        skillIndex, 0);
}

/* ================================================================
 * Pickup: take first item from current cell -> champion inventory
 * ================================================================ */

int M11_GameView_PickupItem(M11_GameViewState* state) {
    unsigned short item;
    int targetSlot;
    struct ChampionState_Compat* champ;
    char itemName[48];
    char champName[16];

    if (!state || !state->active || state->partyDead) {
        return 0;
    }
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "PICKUP", "NO ACTIVE CHAMPION");
        return 0;
    }
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    if (!champ->present || champ->hp.current == 0) {
        m11_set_status(state, "PICKUP", "CHAMPION CANNOT ACT");
        return 0;
    }

    item = m11_find_first_item_on_square(
        &state->world,
        state->world.party.mapIndex,
        state->world.party.mapX,
        state->world.party.mapY);
    if (item == THING_NONE) {
        m11_set_status(state, "PICKUP", "NOTHING TO PICK UP");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "EMPTY FLOOR");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "NO ITEMS ON THIS CELL");
        return 0;
    }

    targetSlot = m11_find_empty_slot(champ);
    if (targetSlot < 0) {
        m11_set_status(state, "PICKUP", "INVENTORY FULL");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "HANDS FULL");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "DROP SOMETHING FIRST (P KEY)");
        return 0;
    }

    if (!m11_unlink_thing_from_square(&state->world,
                                      state->world.party.mapIndex,
                                      state->world.party.mapX,
                                      state->world.party.mapY,
                                      item)) {
        m11_set_status(state, "PICKUP", "CHAIN ERROR");
        return 0;
    }

    champ->inventory[targetSlot] = item;
    m11_get_item_name(state->world.things, item, itemName, sizeof(itemName));
    m11_format_champion_name(champ->name, champName, sizeof(champName));

    m11_log_event(state, M11_COLOR_LIGHT_GREEN, "T%u: %s TOOK %s",
                  (unsigned int)state->world.gameTick, champName, itemName);
    m11_set_status(state, "PICKUP", "ITEM TAKEN");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "PICKED UP");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "%s -> %s SLOT %d", itemName, champName, targetSlot);
    m11_refresh_hash(state);
    return 1;
}

/* ================================================================
 * Drop: take item from champion hand/slot -> current cell
 * ================================================================ */

int M11_GameView_DropItem(M11_GameViewState* state) {
    struct ChampionState_Compat* champ;
    unsigned short item = THING_NONE;
    int dropSlot = -1;
    int slot;
    char itemName[48];
    char champName[16];

    if (!state || !state->active || state->partyDead) {
        return 0;
    }
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "DROP", "NO ACTIVE CHAMPION");
        return 0;
    }
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    if (!champ->present || champ->hp.current == 0) {
        m11_set_status(state, "DROP", "CHAMPION CANNOT ACT");
        return 0;
    }

    /* Drop from hands first (right, then left), then last backpack item */
    if (champ->inventory[CHAMPION_SLOT_HAND_RIGHT] != THING_NONE) {
        dropSlot = CHAMPION_SLOT_HAND_RIGHT;
    } else if (champ->inventory[CHAMPION_SLOT_HAND_LEFT] != THING_NONE) {
        dropSlot = CHAMPION_SLOT_HAND_LEFT;
    } else {
        for (slot = CHAMPION_SLOT_BACKPACK_8; slot >= CHAMPION_SLOT_BACKPACK_1; --slot) {
            if (champ->inventory[slot] != THING_NONE) {
                dropSlot = slot;
                break;
            }
        }
        if (dropSlot < 0) {
            for (slot = CHAMPION_SLOT_POUCH_2; slot >= CHAMPION_SLOT_POUCH_1; --slot) {
                if (champ->inventory[slot] != THING_NONE) {
                    dropSlot = slot;
                    break;
                }
            }
        }
    }

    if (dropSlot < 0) {
        m11_set_status(state, "DROP", "NOTHING TO DROP");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "EMPTY HANDS");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "PICK SOMETHING UP FIRST (G KEY)");
        return 0;
    }

    item = champ->inventory[dropSlot];
    champ->inventory[dropSlot] = THING_NONE;

    if (!m11_prepend_thing_to_square(&state->world,
                                     state->world.party.mapIndex,
                                     state->world.party.mapX,
                                     state->world.party.mapY,
                                     item)) {
        /* Restore if chain operation fails */
        champ->inventory[dropSlot] = item;
        m11_set_status(state, "DROP", "CHAIN ERROR");
        return 0;
    }

    m11_get_item_name(state->world.things, item, itemName, sizeof(itemName));
    m11_format_champion_name(champ->name, champName, sizeof(champName));

    m11_log_event(state, M11_COLOR_YELLOW, "T%u: %s DROPPED %s",
                  (unsigned int)state->world.gameTick, champName, itemName);
    m11_set_status(state, "DROP", "ITEM DROPPED");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "DROPPED");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "%s FROM %s SLOT %d", itemName, champName, dropSlot);
    m11_refresh_hash(state);
    return 1;
}

/* Forward declarations for spell casting */
/* Forward declaration — public, see header */

/* ================================================================
 * Spell casting UI
 * ================================================================ */

/* DM1 rune names per row. Row 0 = power runes, rows 1-3 = element/form/class. */
static const char* const g_rune_names[4][6] = {
    { "LO",  "UM",  "ON",  "EE",  "PAL", "MON" },
    { "YA",  "VI",  "OH",  "FUL", "DES", "ZO"  },
    { "VEN", "EW",  "KATH","IR",  "BRO", "GOR" },
    { "KU",  "ROS", "DAIN","NETA","RA",  "SAR" }
};

/* Encode a rune symbol from (row, column) into the DM1 byte value.
 * Per SYMBOL.C:F0399: runeValue = 0x60 + 6*row + column. */
static int m11_encode_rune(int row, int col) {
    if (row < 0 || row > 3 || col < 0 || col > 5) return -1;
    return 0x60 + 6 * row + col;
}

int M11_GameView_OpenSpellPanel(M11_GameViewState* state) {
    if (!state || !state->active || state->partyDead) return 0;
    state->spellPanelOpen = 1;
    state->spellRuneRow = 0;
    memset(&state->spellBuffer, 0, sizeof(state->spellBuffer));
    m11_log_event(state, M11_COLOR_LIGHT_BLUE, "T%u: SPELL PANEL OPENED",
                  (unsigned int)state->world.gameTick);
    return 1;
}

int M11_GameView_CloseSpellPanel(M11_GameViewState* state) {
    if (!state) return 0;
    state->spellPanelOpen = 0;
    state->spellRuneRow = 0;
    memset(&state->spellBuffer, 0, sizeof(state->spellBuffer));
    return 1;
}

int M11_GameView_EnterRune(M11_GameViewState* state, int symbolIndex) {
    int runeValue;
    if (!state || !state->active || state->partyDead) return 0;
    if (!state->spellPanelOpen) return 0;
    if (symbolIndex < 0 || symbolIndex > 5) return 0;
    if (state->spellBuffer.runeCount >= 4) return 0;

    runeValue = m11_encode_rune(state->spellRuneRow, symbolIndex);
    if (runeValue < 0) return 0;

    state->spellBuffer.runes[state->spellBuffer.runeCount] = runeValue;
    state->spellBuffer.runeCount++;
    state->spellRuneRow++;

    m11_log_event(state, M11_COLOR_WHITE, "T%u: RUNE %s (%d)",
                  (unsigned int)state->world.gameTick,
                  g_rune_names[state->spellRuneRow - 1][symbolIndex],
                  runeValue);

    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "RUNE ENTERED");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "%s — %d OF 4 SYMBOLS",
             g_rune_names[state->spellRuneRow - 1][symbolIndex],
             state->spellBuffer.runeCount);

    /* Auto-close panel after 4 runes (full sequence) */
    if (state->spellRuneRow >= 4) {
        state->spellRuneRow = 3; /* clamp display row */
    }
    return 1;
}

int M11_GameView_ClearSpell(M11_GameViewState* state) {
    if (!state || !state->active) return 0;
    state->spellRuneRow = 0;
    memset(&state->spellBuffer, 0, sizeof(state->spellBuffer));
    if (state->spellPanelOpen) {
        m11_log_event(state, M11_COLOR_YELLOW, "T%u: SPELL CLEARED",
                      (unsigned int)state->world.gameTick);
    }
    return 1;
}

int M11_GameView_CastSpell(M11_GameViewState* state) {
    struct ChampionState_Compat* champ;
    struct SpellCastRequest_Compat req;
    struct SpellDefinition_Compat spell;
    struct SpellEffect_Compat effect;
    uint32_t packed = 0;
    int tableIndex = -1;
    int manaCost = 0;
    int failureReason = 0;
    char champName[16];

    if (!state || !state->active || state->partyDead) return 0;
    if (state->spellBuffer.runeCount < 2) {
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: NEED AT LEAST 2 RUNES",
                      (unsigned int)state->world.gameTick);
        m11_set_status(state, "CAST", "NOT ENOUGH RUNES");
        return 0;
    }
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "CAST", "NO ACTIVE CHAMPION");
        return 0;
    }
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    if (!champ->present || champ->hp.current == 0) {
        m11_set_status(state, "CAST", "CHAMPION CANNOT ACT");
        return 0;
    }

    m11_format_champion_name(champ->name, champName, sizeof(champName));

    /* Encode the rune sequence */
    memset(&spell, 0, sizeof(spell));
    if (!F0750_MAGIC_EncodeRuneSequence_Compat(&state->spellBuffer, &packed)) {
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: %s — INVALID RUNES",
                      (unsigned int)state->world.gameTick, champName);
        m11_set_status(state, "CAST", "INVALID RUNE SEQUENCE");
        M11_GameView_ClearSpell(state);
        state->spellPanelOpen = 0;
        return 0;
    }

    /* Look up spell in table */
    if (!F0752_MAGIC_LookupSpellInTable_Compat(packed, &tableIndex, &spell)) {
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: %s — MEANINGLESS SPELL",
                      (unsigned int)state->world.gameTick, champName);
        m11_set_status(state, "CAST", "UNKNOWN SPELL");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "SPELL FAILED");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "NO MATCHING SPELL IN TABLE");
        M11_GameView_ClearSpell(state);
        state->spellPanelOpen = 0;
        return 1; /* consumed the input even though spell failed */
    }

    /* Compute mana cost */
    F0753_MAGIC_ComputeManaCost_Compat(&state->spellBuffer, &manaCost);
    if ((int)champ->mana.current < manaCost) {
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: %s — NOT ENOUGH MANA (%d/%d)",
                      (unsigned int)state->world.gameTick, champName,
                      (int)champ->mana.current, manaCost);
        m11_set_status(state, "CAST", "OUT OF MANA");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "NEED MANA");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "COST %d, HAVE %d", manaCost, (int)champ->mana.current);
        M11_GameView_ClearSpell(state);
        state->spellPanelOpen = 0;
        return 1;
    }

    /* Build a cast request */
    memset(&req, 0, sizeof(req));
    req.championIndex = state->world.party.activeChampionIndex;
    req.currentMana = (int)champ->mana.current;
    req.maximumMana = (int)champ->mana.maximum;
    req.skillLevelForSpell = (spell.skillIndex >= 0 && spell.skillIndex < CHAMPION_SKILL_COUNT)
                                  ? champ->skillLevels[spell.skillIndex] : 0;
    req.statisticWisdom = champ->attributes[CHAMPION_ATTR_WISDOM];
    req.luckCurrent = state->world.magic.luckCurrent;
    req.partyDirection = state->world.party.direction;
    req.partyMapIndex = state->world.party.mapIndex;
    req.partyMapX = state->world.party.mapX;
    req.partyMapY = state->world.party.mapY;
    req.hasEmptyFlaskInHand = 0; /* TODO: check inventory */
    req.hasMagicMapInHand = 0;
    req.gameTimeTicksLow = (int)(state->world.gameTick & 0x7FFFFFFF);
    req.spellTableIndex = tableIndex;
    req.rawSymbolsPacked = (int)packed;

    /* Validate the cast */
    memset(&effect, 0, sizeof(effect));
    if (!F0754_MAGIC_ValidateCastRequest_Compat(&req, &spell,
                                                 state->spellBuffer.runes[0] > 0
                                                     ? (state->spellBuffer.runes[0] - 0x60) / 6 + 1
                                                     : 1,
                                                 &state->world.masterRng,
                                                 &failureReason)) {
        const char* failMsg = "NEEDS MORE PRACTICE";
        if (failureReason == SPELL_FAILURE_MEANINGLESS_SPELL) failMsg = "MEANINGLESS SPELL";
        else if (failureReason == SPELL_FAILURE_NEEDS_FLASK_IN_HAND) failMsg = "NEED FLASK";
        else if (failureReason == SPELL_FAILURE_NEEDS_MAGIC_MAP) failMsg = "NEED MAGIC MAP";
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: %s — %s",
                      (unsigned int)state->world.gameTick, champName, failMsg);
        m11_set_status(state, "CAST", failMsg);
    }

    /* Deduct mana */
    champ->mana.current = (uint16_t)((int)champ->mana.current - manaCost);

    /* Issue CMD_CAST_SPELL through the tick system */
    {
        struct TickInput_Compat input;
        memset(&input, 0, sizeof(input));
        input.tick = state->world.gameTick;
        input.command = CMD_CAST_SPELL;
        input.commandArg1 = (uint8_t)state->world.party.activeChampionIndex;
        input.commandArg2 = (uint8_t)tableIndex;
        input.reserved = (uint8_t)(state->spellBuffer.runes[0] > 0
                                       ? (state->spellBuffer.runes[0] - 0x60) / 6 + 1
                                       : 1);
        memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
        F0884_ORCH_AdvanceOneTick_Compat(&state->world, &input, &state->lastTickResult);
        state->lastWorldHash = state->lastTickResult.worldHashPost;
        M11_GameView_ProcessTickEmissions(state);
    }

    m11_log_event(state, M11_COLOR_GREEN, "T%u: %s CAST SPELL #%d (COST %d MANA)",
                  (unsigned int)state->world.gameTick, champName, tableIndex, manaCost);
    m11_set_status(state, "CAST", "SPELL COMMITTED");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s CASTS", champName);
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "SPELL #%d, COST %d MANA, %d REMAINING",
             tableIndex, manaCost, (int)champ->mana.current);

    M11_GameView_ClearSpell(state);
    state->spellPanelOpen = 0;
    m11_refresh_hash(state);
    return 1;
}

/* ================================================================
 * Food / water drain, rest recovery, champion death
 * ================================================================ */

static void m11_apply_survival_drain(M11_GameViewState* state) {
    int i;
    if (!state || !state->active) {
        return;
    }
    /* Every 6 ticks, drain 1 food and 1 water from each champion */
    if ((state->world.gameTick % 6U) != 0U) {
        return;
    }
    for (i = 0; i < state->world.party.championCount; ++i) {
        struct ChampionState_Compat* champ = &state->world.party.champions[i];
        if (!champ->present) {
            continue;
        }
        if (champ->hp.current == 0) {
            continue; /* dead */
        }
        if (champ->food > 0) {
            --champ->food;
        }
        if (champ->water > 0) {
            --champ->water;
        }
        /* Starvation/dehydration damage */
        if (champ->food == 0 && champ->water == 0 && champ->hp.current > 0) {
            if (champ->hp.current > 1) {
                --champ->hp.current;
            }
        }
    }
}

static void m11_apply_rest_recovery(M11_GameViewState* state) {
    int i;
    if (!state || !state->active || !state->resting) {
        return;
    }
    /* Every 4 ticks while resting, recover 1 HP, 2 stamina, 1 mana */
    if ((state->world.gameTick % 4U) != 0U) {
        return;
    }
    for (i = 0; i < state->world.party.championCount; ++i) {
        struct ChampionState_Compat* champ = &state->world.party.champions[i];
        if (!champ->present || champ->hp.current == 0) {
            continue;
        }
        if (champ->hp.current < champ->hp.maximum) {
            ++champ->hp.current;
        }
        if (champ->stamina.current + 2 <= champ->stamina.maximum) {
            champ->stamina.current += 2;
        } else {
            champ->stamina.current = champ->stamina.maximum;
        }
        if (champ->mana.current < champ->mana.maximum) {
            ++champ->mana.current;
        }
    }
}

static void m11_check_party_death(M11_GameViewState* state) {
    int i;
    int anyAlive = 0;
    if (!state || !state->active) {
        return;
    }
    for (i = 0; i < state->world.party.championCount; ++i) {
        if (state->world.party.champions[i].present &&
            state->world.party.champions[i].hp.current > 0) {
            anyAlive = 1;
            break;
        }
    }
    if (!anyAlive && state->world.party.championCount > 0) {
        state->partyDead = 1;
        m11_log_event(state, M11_COLOR_LIGHT_RED, "T%u: ALL CHAMPIONS HAVE FALLEN",
                      (unsigned int)state->world.gameTick);
        m11_set_status(state, "DEATH", "PARTY WIPED");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "GAME OVER");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "THE DUNGEON CLAIMS YOUR SOULS. LOAD A SAVE OR RETURN TO MENU.");
    }
}

/* ================================================================
 * Creature AI simulation (M11-layer)
 *
 * The M10 tick orchestrator treats TIMELINE_EVENT_CREATURE_TICK as a
 * no-op (v1 stub). This M11-layer simulation provides simple creature
 * behavior so the game feels alive:
 *   - Creatures within sight range move toward the party (one step per
 *     movementTicks interval).
 *   - Creatures on the party square deal autonomous damage.
 *   - Creature death removes the group from the map.
 * All manipulation happens on M11-owned world state — no M10 files
 * are modified.
 * ================================================================ */

/* Find a creature group thing on a specific square. */
static unsigned short m11_find_group_on_square(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY) {
    unsigned short thing = m11_get_first_square_thing(world, mapIndex, mapX, mapY);
    int safety = 0;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_GROUP) {
            return thing;
        }
        thing = m11_raw_next_thing(world->things, thing);
        ++safety;
    }
    return THING_NONE;
}

/* Locate a group by thing-id: scan all squares on the given map.
 * Returns 1 if found, filling outX/outY. */
static int m11_find_group_position(
    const struct GameWorld_Compat* world,
    int mapIndex,
    unsigned short groupThing,
    int* outX,
    int* outY) {
    const struct DungeonMapDesc_Compat* map;
    int mx, my, base, idx;
    unsigned short thing;
    int safety;
    if (!world || !world->dungeon || !world->things ||
        !world->things->squareFirstThings) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    base = m11_map_square_base(world->dungeon, mapIndex);
    if (base < 0) return 0;
    for (mx = 0; mx < (int)map->width; ++mx) {
        for (my = 0; my < (int)map->height; ++my) {
            idx = base + mx * (int)map->height + my;
            if (idx < 0 || idx >= world->things->squareFirstThingCount) continue;
            thing = world->things->squareFirstThings[idx];
            safety = 0;
            while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
                if (thing == groupThing) {
                    *outX = mx;
                    *outY = my;
                    return 1;
                }
                thing = m11_raw_next_thing(world->things, thing);
                ++safety;
            }
        }
    }
    return 0;
}

/* Check whether a square is walkable for a creature. */
static int m11_square_walkable_for_creature(
    const struct GameWorld_Compat* world,
    int mapIndex,
    int mapX,
    int mapY) {
    unsigned char square = 0;
    int elementType;
    if (!m11_get_square_byte(world, mapIndex, mapX, mapY, &square)) {
        return 0;
    }
    elementType = (square >> 5) & 7;
    switch (elementType) {
        case DUNGEON_ELEMENT_CORRIDOR:
        case DUNGEON_ELEMENT_PIT:
        case DUNGEON_ELEMENT_TELEPORTER:
        case DUNGEON_ELEMENT_FAKEWALL:
            return 1;
        case DUNGEON_ELEMENT_DOOR: {
            /* Open doors (low bits == 0) are walkable */
            int doorState = square & 0x07;
            return (doorState == 0) ? 1 : 0;
        }
        default:
            return 0;
    }
}

/* Move a creature one step toward the party. Returns 1 if moved. */
static int m11_creature_try_move(
    M11_GameViewState* state,
    unsigned short groupThing,
    int groupX,
    int groupY,
    int* outNewX,
    int* outNewY) {
    int partyX = state->world.party.mapX;
    int partyY = state->world.party.mapY;
    int mapIdx = state->world.party.mapIndex;
    int dx = 0, dy = 0;
    int bestX = groupX, bestY = groupY;
    int bestDist;
    int candidates[4][2];
    int candidateCount = 0;
    int i;

    /* Compute desired direction */
    if (partyX > groupX) dx = 1;
    else if (partyX < groupX) dx = -1;
    if (partyY > groupY) dy = 1;
    else if (partyY < groupY) dy = -1;

    /* Already on party square? Don't move. */
    if (groupX == partyX && groupY == partyY) {
        *outNewX = groupX;
        *outNewY = groupY;
        return 0;
    }

    /* Try primary direction first, then lateral, then diagonal */
    if (dx != 0) {
        candidates[candidateCount][0] = groupX + dx;
        candidates[candidateCount][1] = groupY;
        candidateCount++;
    }
    if (dy != 0) {
        candidates[candidateCount][0] = groupX;
        candidates[candidateCount][1] = groupY + dy;
        candidateCount++;
    }
    if (dx != 0 && dy != 0) {
        candidates[candidateCount][0] = groupX + dx;
        candidates[candidateCount][1] = groupY + dy;
        candidateCount++;
    }
    /* Lateral fallback: if only one axis differs, try the other axis */
    if (dx == 0 && dy != 0) {
        candidates[candidateCount][0] = groupX + 1;
        candidates[candidateCount][1] = groupY;
        candidateCount++;
    }
    if (dy == 0 && dx != 0) {
        candidates[candidateCount][0] = groupX;
        candidates[candidateCount][1] = groupY + 1;
        candidateCount++;
    }

    bestDist = abs(partyX - groupX) + abs(partyY - groupY);
    for (i = 0; i < candidateCount; ++i) {
        int cx = candidates[i][0];
        int cy = candidates[i][1];
        int dist;
        if (!m11_square_walkable_for_creature(&state->world, mapIdx, cx, cy)) {
            continue;
        }
        /* Don't step onto a square that already has another group */
        if (m11_find_group_on_square(&state->world, mapIdx, cx, cy) != THING_NONE) {
            /* Allow stepping onto the party square even if another group is there */
            if (cx != partyX || cy != partyY) continue;
        }
        dist = abs(partyX - cx) + abs(partyY - cy);
        if (dist < bestDist) {
            bestDist = dist;
            bestX = cx;
            bestY = cy;
        }
    }

    if (bestX == groupX && bestY == groupY) {
        *outNewX = groupX;
        *outNewY = groupY;
        return 0; /* couldn't move */
    }

    /* Move: unlink from old square, prepend to new square */
    if (!m11_unlink_thing_from_square(&state->world, mapIdx, groupX, groupY, groupThing)) {
        *outNewX = groupX;
        *outNewY = groupY;
        return 0;
    }
    if (!m11_prepend_thing_to_square(&state->world, mapIdx, bestX, bestY, groupThing)) {
        /* Failed to place — put it back */
        m11_prepend_thing_to_square(&state->world, mapIdx, groupX, groupY, groupThing);
        *outNewX = groupX;
        *outNewY = groupY;
        return 0;
    }
    *outNewX = bestX;
    *outNewY = bestY;
    return 1;
}

/* Deal autonomous creature damage to the party. */
static void m11_creature_attack_party(
    M11_GameViewState* state,
    const struct DungeonGroup_Compat* group) {
    const struct CreatureBehaviorProfile_Compat* profile;
    int creatureCount;
    int totalDamage;
    int targetChamp;
    int i;
    struct ChampionState_Compat* champ;
    int baseDmg;

    if (!state || !group) return;

    profile = CREATURE_GetProfile_Compat(group->creatureType);
    creatureCount = (int)group->count + 1;  /* count field is 0-based */
    baseDmg = profile ? profile->baseAttack : 10;

    /* Each creature in the group attacks for baseDmg / 3 (simplified) */
    totalDamage = 0;
    for (i = 0; i < creatureCount; ++i) {
        if (group->health[i] > 0) {
            /* Simple damage: baseAttack / 3, minimum 1 */
            int dmg = baseDmg / 3;
            if (dmg < 1) dmg = 1;
            totalDamage += dmg;
        }
    }

    if (totalDamage <= 0) return;

    /* Target the active champion, or first alive champion */
    targetChamp = state->world.party.activeChampionIndex;
    if (targetChamp < 0 || targetChamp >= state->world.party.championCount ||
        !state->world.party.champions[targetChamp].present ||
        state->world.party.champions[targetChamp].hp.current == 0) {
        targetChamp = -1;
        for (i = 0; i < state->world.party.championCount; ++i) {
            if (state->world.party.champions[i].present &&
                state->world.party.champions[i].hp.current > 0) {
                targetChamp = i;
                break;
            }
        }
    }
    if (targetChamp < 0) return;

    champ = &state->world.party.champions[targetChamp];
    if ((int)champ->hp.current > totalDamage) {
        champ->hp.current -= (unsigned short)totalDamage;
    } else {
        champ->hp.current = 0;
    }

    {
        char champName[16];
        m11_format_champion_name(champ->name, champName, sizeof(champName));
        m11_log_event(state, M11_COLOR_LIGHT_RED,
                      "T%u: %s HIT BY %s FOR %d",
                      (unsigned int)state->world.gameTick,
                      champName,
                      m11_creature_name(group->creatureType),
                      totalDamage);
    }
}

/* Process one creature group: check distance, maybe move, maybe attack. */
static void m11_process_one_creature_group(
    M11_GameViewState* state,
    unsigned short groupThing,
    int groupIndex) {
    const struct CreatureBehaviorProfile_Compat* profile;
    struct DungeonGroup_Compat* group;
    int groupX, groupY;
    int dist;
    int anyAlive;
    int i;

    if (!state || !state->world.things ||
        groupIndex < 0 || groupIndex >= state->world.things->groupCount) {
        return;
    }
    group = &state->world.things->groups[groupIndex];
    profile = CREATURE_GetProfile_Compat(group->creatureType);
    if (!profile) return;

    /* Check if any creature in the group is alive */
    anyAlive = 0;
    for (i = 0; i < (int)group->count + 1; ++i) {
        if (group->health[i] > 0) {
            anyAlive = 1;
            break;
        }
    }
    if (!anyAlive) return;

    /* Find where this group is on the map */
    if (!m11_find_group_position(&state->world,
                                 state->world.party.mapIndex,
                                 groupThing,
                                 &groupX, &groupY)) {
        return; /* not on current map */
    }

    dist = abs(state->world.party.mapX - groupX) +
           abs(state->world.party.mapY - groupY);

    /* If on same square as party: attack */
    if (dist == 0) {
        /* Attack cadence: only attack every attackTicks ticks */
        if (profile->attackTicks > 0 &&
            (state->world.gameTick % (uint32_t)profile->attackTicks) == 0u) {
            m11_creature_attack_party(state, group);
        }
        return;
    }

    /* Movement check: within sight range? */
    if (dist > profile->sightRange && dist > profile->smellRange) {
        return; /* too far to detect party */
    }

    /* Movement cadence: only move every movementTicks ticks */
    if (profile->movementTicks > 0 &&
        (state->world.gameTick % (uint32_t)profile->movementTicks) != 0u) {
        return;
    }

    /* Try to move toward the party */
    {
        int newX, newY;
        if (m11_creature_try_move(state, groupThing, groupX, groupY, &newX, &newY)) {
            /* Check if creature arrived at party square */
            if (newX == state->world.party.mapX &&
                newY == state->world.party.mapY) {
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s REACHES THE PARTY!",
                              (unsigned int)state->world.gameTick,
                              m11_creature_name(group->creatureType));
            }
        }
    }
}

/* Scan all groups on the current map and process their AI. */
static void m11_process_creature_ticks(M11_GameViewState* state) {
    int mapIdx;
    int i;
    const struct DungeonMapDesc_Compat* map;
    int base, mx, my, idx;
    unsigned short thing;
    int safety;

    if (!state || !state->active || state->partyDead) return;
    if (!state->world.dungeon || !state->world.things ||
        !state->world.things->squareFirstThings) return;

    mapIdx = state->world.party.mapIndex;
    if (mapIdx < 0 || mapIdx >= (int)state->world.dungeon->header.mapCount) return;
    map = &state->world.dungeon->maps[mapIdx];
    base = m11_map_square_base(state->world.dungeon, mapIdx);
    if (base < 0) return;

    /* Scan all squares on the current map for groups */
    for (mx = 0; mx < (int)map->width; ++mx) {
        for (my = 0; my < (int)map->height; ++my) {
            idx = base + mx * (int)map->height + my;
            if (idx < 0 || idx >= state->world.things->squareFirstThingCount) continue;
            thing = state->world.things->squareFirstThings[idx];
            safety = 0;
            while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_GROUP) {
                    int groupIdx = THING_GET_INDEX(thing);
                    m11_process_one_creature_group(state, thing, groupIdx);
                    break; /* one group per square for simplicity */
                }
                thing = m11_raw_next_thing(state->world.things, thing);
                ++safety;
            }
        }
    }

    (void)i; /* suppress unused warning */
}

/* ================================================================
 * XP award and level-up processing
 *
 * When EMIT_DAMAGE_DEALT fires, the active champion earns combat XP
 * through the M10 lifecycle layer.  If the accumulated experience
 * crosses a level threshold, F0850 applies stat boosts and we log
 * the level-up.
 * ================================================================ */
static void m11_award_combat_xp(M11_GameViewState* state,
                                int championIndex,
                                int damage) {
    struct ChampionLifecycleState_Compat* lc;
    struct LevelUpMarker_Compat marker;
    int xpAmount;
    int skillIndex;
    char name[16];

    if (!state) return;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return;
    if (!state->world.party.champions[championIndex].present) return;

    lc = &state->world.lifecycle.champions[championIndex];
    memset(&marker, 0, sizeof(marker));

    /* XP proportional to damage, minimum 1.
     * The lifecycle layer uses 20-skill indices: LIFECYCLE_SKILL_SWING
     * is the default melee sub-skill under Fighter. */
    xpAmount = damage;
    if (xpAmount < 1) xpAmount = 1;
    skillIndex = LIFECYCLE_SKILL_SWING;

    if (F0851_LIFECYCLE_AwardCombatXP_Compat(
            lc, championIndex, skillIndex, xpAmount,
            state->world.party.mapIndex, /* map difficulty */
            state->world.gameTick,
            state->world.lifecycle.lastCreatureAttackTime,
            &state->world.masterRng,
            &marker)) {
        /* Sync base skill level back to Phase 10 party state.
         * The lifecycle uses 20 sub-skills; Phase 10 only tracks the
         * 4 base classes (Fighter/Ninja/Priest/Wizard). */
        int baseIdx = (skillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        if (baseIdx >= 0 && baseIdx < CHAMPION_SKILL_COUNT) {
            int newLevel = F0848_LIFECYCLE_ComputeSkillLevel_Compat(
                lc, baseIdx, 0);
            if (newLevel > 0) {
                state->world.party.champions[championIndex].skillLevels[baseIdx] =
                    (unsigned short)newLevel;
            }
        }

        /* Check for level-up */
        if (marker.newLevel > marker.previousLevel && marker.newLevel > 0) {
            m11_format_champion_name(
                state->world.party.champions[championIndex].name,
                name, sizeof(name));
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s LEVELED UP! (%s %d -> %d)",
                          (unsigned int)state->world.gameTick,
                          name,
                          (baseIdx == LIFECYCLE_SKILL_FIGHTER)  ? "FIGHTER" :
                          (baseIdx == LIFECYCLE_SKILL_NINJA)    ? "NINJA"   :
                          (baseIdx == LIFECYCLE_SKILL_PRIEST)   ? "PRIEST"  :
                          (baseIdx == LIFECYCLE_SKILL_WIZARD)   ? "WIZARD"  :
                          "SKILL",
                          marker.previousLevel, marker.newLevel);
        }
    }
}

static void m11_award_magic_xp(M11_GameViewState* state,
                               int championIndex,
                               int spellKind,
                               int power) {
    struct ChampionLifecycleState_Compat* lc;
    struct LevelUpMarker_Compat marker;
    int skillIndex;
    char name[16];

    if (!state) return;
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return;
    if (!state->world.party.champions[championIndex].present) return;

    lc = &state->world.lifecycle.champions[championIndex];
    memset(&marker, 0, sizeof(marker));

    /* Map spell kind to lifecycle sub-skill: Heal (13) for potions,
     * Fire (16) for projectile/combat magic. */
    if (spellKind == C1_SPELL_KIND_POTION_COMPAT) {
        skillIndex = LIFECYCLE_SKILL_HEAL;
    } else {
        skillIndex = LIFECYCLE_SKILL_FIRE;
    }

    if (F0852_LIFECYCLE_AwardMagicXP_Compat(
            lc, championIndex, skillIndex, power > 0 ? power : 1,
            state->world.party.mapIndex,
            state->world.gameTick,
            state->world.lifecycle.lastCreatureAttackTime,
            &state->world.masterRng,
            &marker)) {
        int baseIdx = (skillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        if (baseIdx >= 0 && baseIdx < CHAMPION_SKILL_COUNT) {
            int newLevel = F0848_LIFECYCLE_ComputeSkillLevel_Compat(
                lc, baseIdx, 0);
            if (newLevel > 0) {
                state->world.party.champions[championIndex].skillLevels[baseIdx] =
                    (unsigned short)newLevel;
            }
        }
        if (marker.newLevel > marker.previousLevel && marker.newLevel > 0) {
            m11_format_champion_name(
                state->world.party.champions[championIndex].name,
                name, sizeof(name));
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s LEVELED UP! (%s %d -> %d)",
                          (unsigned int)state->world.gameTick,
                          name,
                          (baseIdx == LIFECYCLE_SKILL_PRIEST)  ? "PRIEST"  :
                          (baseIdx == LIFECYCLE_SKILL_WIZARD)  ? "WIZARD"  :
                          "SKILL",
                          marker.previousLevel, marker.newLevel);
        }
    }
}

/* ================================================================
 * Kill XP bonus
 *
 * When a creature is killed (EMIT_KILL_NOTIFY), the active champion
 * receives a bonus XP award proportional to the slain creature's
 * baseHealth.  This rewards actually defeating enemies, not just
 * scratching them.
 * ================================================================ */
static void m11_award_kill_xp(M11_GameViewState* state,
                              int creatureType) {
    const struct CreatureBehaviorProfile_Compat* profile;
    int champIdx;
    int xpBonus;

    if (!state) return;
    profile = CREATURE_GetProfile_Compat(creatureType);
    champIdx = state->world.party.activeChampionIndex;
    if (champIdx < 0 || champIdx >= CHAMPION_MAX_PARTY) return;
    if (!state->world.party.champions[champIdx].present) return;

    /* XP bonus = baseHealth / 2, minimum 5 */
    xpBonus = profile ? profile->baseHealth / 2 : 10;
    if (xpBonus < 5) xpBonus = 5;

    m11_award_combat_xp(state, champIdx, xpBonus);

    {
        char name[16];
        m11_format_champion_name(
            state->world.party.champions[champIdx].name,
            name, sizeof(name));
        m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                      "T%u: %s EARNS %d KILL XP (%s)",
                      (unsigned int)state->world.gameTick,
                      name, xpBonus,
                      m11_creature_name(creatureType));
    }
}

/* ================================================================
 * Potion / flask use
 *
 * The active champion drinks a potion or water flask from a hand
 * slot.  Potions apply stat effects based on their DM1 type:
 *   - VEN (type 3): antidote / restore stamina
 *   - KU  (type 7): heal HP
 *   - ZO  (type 5): restore mana
 *   - MON (type 0): shield / temporary defense (logged only)
 *   - DES (type 2): poison (damage)
 *   - VI  (type 14): restore all stats partially
 *   - WATER FLASK (type 15): restore water
 *   - EMPTY FLASK (type 16): no effect
 *   - Others: generic small heal
 * The potion thing is consumed (replaced by EMPTY FLASK subtype 16
 * if doNotDiscard is set, or removed from inventory otherwise).
 * ================================================================ */

/* DM1 potion type indices (match s_potionTypeNames order) */
enum {
    M11_POTION_MON  = 0,
    M11_POTION_UM   = 1,
    M11_POTION_DES  = 2,
    M11_POTION_VEN  = 3,
    M11_POTION_SAR  = 4,
    M11_POTION_ZO   = 5,
    M11_POTION_ROS  = 6,
    M11_POTION_KU   = 7,
    M11_POTION_DANE = 8,
    M11_POTION_NETA = 9,
    M11_POTION_BRO  = 10,
    M11_POTION_MA   = 11,
    M11_POTION_YA   = 12,
    M11_POTION_EE   = 13,
    M11_POTION_VI   = 14,
    M11_POTION_WATER_FLASK = 15,
    M11_POTION_EMPTY_FLASK = 16
};

static void m11_apply_potion_effect(M11_GameViewState* state,
                                    struct ChampionState_Compat* champ,
                                    int potionType,
                                    int power,
                                    const char* champName) {
    int amount;
    if (!state || !champ) return;

    /* Power scales the effect: 1..255, typical potion power is 20-80 */
    amount = (power > 0) ? (int)power / 4 + 1 : 5;

    switch (potionType) {
        case M11_POTION_KU: /* Heal HP */
            if ((int)champ->hp.current + amount > (int)champ->hp.maximum) {
                champ->hp.current = champ->hp.maximum;
            } else {
                champ->hp.current += (uint16_t)amount;
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s HEALED %d HP",
                          (unsigned int)state->world.gameTick,
                          champName, amount);
            break;

        case M11_POTION_VEN: /* Antidote / restore stamina */
            if ((int)champ->stamina.current + amount * 2 > (int)champ->stamina.maximum) {
                champ->stamina.current = champ->stamina.maximum;
            } else {
                champ->stamina.current += (uint16_t)(amount * 2);
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s STAMINA RESTORED +%d",
                          (unsigned int)state->world.gameTick,
                          champName, amount * 2);
            break;

        case M11_POTION_ZO: /* Restore mana */
            if ((int)champ->mana.current + amount > (int)champ->mana.maximum) {
                champ->mana.current = champ->mana.maximum;
            } else {
                champ->mana.current += (uint16_t)amount;
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s MANA RESTORED +%d",
                          (unsigned int)state->world.gameTick,
                          champName, amount);
            break;

        case M11_POTION_DES: /* Poison — damages the drinker */
            if ((int)champ->hp.current > amount) {
                champ->hp.current -= (uint16_t)amount;
            } else {
                champ->hp.current = 0;
            }
            m11_log_event(state, M11_COLOR_LIGHT_RED,
                          "T%u: %s POISONED! -%d HP",
                          (unsigned int)state->world.gameTick,
                          champName, amount);
            break;

        case M11_POTION_VI: /* Restore all stats partially */
            if ((int)champ->hp.current + amount / 2 <= (int)champ->hp.maximum) {
                champ->hp.current += (uint16_t)(amount / 2);
            } else {
                champ->hp.current = champ->hp.maximum;
            }
            if ((int)champ->stamina.current + amount <= (int)champ->stamina.maximum) {
                champ->stamina.current += (uint16_t)amount;
            } else {
                champ->stamina.current = champ->stamina.maximum;
            }
            if ((int)champ->mana.current + amount / 2 <= (int)champ->mana.maximum) {
                champ->mana.current += (uint16_t)(amount / 2);
            } else {
                champ->mana.current = champ->mana.maximum;
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s VITALITY RESTORED",
                          (unsigned int)state->world.gameTick,
                          champName);
            break;

        case M11_POTION_WATER_FLASK: /* Restore water */
            if (champ->water + amount * 8 > 255) {
                champ->water = 255;
            } else {
                champ->water += (unsigned char)(amount * 8);
            }
            m11_log_event(state, M11_COLOR_LIGHT_BLUE,
                          "T%u: %s DRINKS WATER",
                          (unsigned int)state->world.gameTick,
                          champName);
            break;

        case M11_POTION_EMPTY_FLASK:
            m11_log_event(state, M11_COLOR_YELLOW,
                          "T%u: %s — FLASK IS EMPTY",
                          (unsigned int)state->world.gameTick,
                          champName);
            break;

        case M11_POTION_MON: /* Shield — log only for now */
            m11_log_event(state, M11_COLOR_CYAN,
                          "T%u: %s FEELS SHIELDED",
                          (unsigned int)state->world.gameTick,
                          champName);
            break;

        default: /* Generic small heal for unhandled types */
            if ((int)champ->hp.current + amount / 2 <= (int)champ->hp.maximum) {
                champ->hp.current += (uint16_t)(amount / 2);
            } else {
                champ->hp.current = champ->hp.maximum;
            }
            m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                          "T%u: %s DRINKS POTION (+%d HP)",
                          (unsigned int)state->world.gameTick,
                          champName, amount / 2);
            break;
    }
}

int M11_GameView_UseItem(M11_GameViewState* state) {
    struct ChampionState_Compat* champ;
    unsigned short item = THING_NONE;
    int useSlot = -1;
    int thingType;
    int thingIndex;
    char itemName[48];
    char champName[16];

    if (!state || !state->active || state->partyDead) {
        return 0;
    }
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) {
        m11_set_status(state, "USE", "NO ACTIVE CHAMPION");
        return 0;
    }
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    if (!champ->present || champ->hp.current == 0) {
        m11_set_status(state, "USE", "CHAMPION CANNOT ACT");
        return 0;
    }

    /* Check right hand first, then left hand */
    if (champ->inventory[CHAMPION_SLOT_HAND_RIGHT] != THING_NONE) {
        useSlot = CHAMPION_SLOT_HAND_RIGHT;
    } else if (champ->inventory[CHAMPION_SLOT_HAND_LEFT] != THING_NONE) {
        useSlot = CHAMPION_SLOT_HAND_LEFT;
    }
    if (useSlot < 0) {
        m11_set_status(state, "USE", "HANDS ARE EMPTY");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "NOTHING TO USE");
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "PICK UP A POTION OR FLASK FIRST (G KEY)");
        return 0;
    }

    item = champ->inventory[useSlot];
    thingType = THING_GET_TYPE(item);
    thingIndex = THING_GET_INDEX(item);
    m11_get_item_name(state->world.things, item, itemName, sizeof(itemName));
    m11_format_champion_name(champ->name, champName, sizeof(champName));

    if (thingType == THING_TYPE_POTION) {
        /* Drink the potion */
        if (state->world.things->potions &&
            thingIndex >= 0 && thingIndex < state->world.things->potionCount) {
            struct DungeonPotion_Compat* pot =
                &state->world.things->potions[thingIndex];
            int potType = (int)pot->type;
            int potPower = (int)pot->power;

            if (potType == M11_POTION_EMPTY_FLASK) {
                m11_set_status(state, "USE", "FLASK IS EMPTY");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                         "EMPTY FLASK");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "NOTHING TO DRINK");
                return 0;
            }

            m11_apply_potion_effect(state, champ, potType, potPower, champName);

            /* Award priest XP for drinking a potion */
            m11_award_magic_xp(state,
                               state->world.party.activeChampionIndex,
                               C1_SPELL_KIND_POTION_COMPAT,
                               potPower > 0 ? potPower / 10 + 1 : 1);

            /* Consume: if doNotDiscard, convert to empty flask;
             * otherwise remove from inventory */
            if (pot->doNotDiscard) {
                pot->type = M11_POTION_EMPTY_FLASK;
                pot->power = 0;
                m11_log_event(state, M11_COLOR_YELLOW,
                              "T%u: %s NOW HOLDS AN EMPTY FLASK",
                              (unsigned int)state->world.gameTick,
                              champName);
            } else {
                champ->inventory[useSlot] = THING_NONE;
            }

            m11_set_status(state, "USE", "POTION CONSUMED");
            snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                     "DRANK %s", itemName);
            snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                     "%s USED %s (POWER %d)",
                     champName, itemName, potPower);
            m11_check_party_death(state);
            m11_refresh_hash(state);
            return 1;
        }
        m11_set_status(state, "USE", "POTION DATA ERROR");
        return 0;
    }

    /* Junk type: waterskin (type 2) can be drunk */
    if (thingType == THING_TYPE_JUNK) {
        if (state->world.things->junks &&
            thingIndex >= 0 && thingIndex < state->world.things->junkCount) {
            int junkType = (int)state->world.things->junks[thingIndex].type;
            if (junkType == 2) { /* WATERSKIN */
                if (champ->water + 40 > 255) {
                    champ->water = 255;
                } else {
                    champ->water += 40;
                }
                m11_log_event(state, M11_COLOR_LIGHT_BLUE,
                              "T%u: %s DRINKS FROM WATERSKIN",
                              (unsigned int)state->world.gameTick,
                              champName);
                m11_set_status(state, "USE", "DRANK WATER");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                         "WATERSKIN");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "%s QUENCHED THIRST", champName);
                m11_refresh_hash(state);
                return 1;
            }
        }
    }

    /* Food items: apple(29), corn(30), bread(31), cheese(32),
     * screamer slice(33), worm round(34), drumstick(35), dragon steak(36) */
    if (thingType == THING_TYPE_JUNK) {
        if (state->world.things->junks &&
            thingIndex >= 0 && thingIndex < state->world.things->junkCount) {
            int junkType = (int)state->world.things->junks[thingIndex].type;
            if (junkType >= 29 && junkType <= 36) {
                /* Eat food: restore food stat */
                int foodAmount = 20 + (junkType - 29) * 5;
                if (champ->food + foodAmount > 255) {
                    champ->food = 255;
                } else {
                    champ->food += (unsigned char)foodAmount;
                }
                /* Remove from hand */
                if (state->world.things->junks[thingIndex].doNotDiscard) {
                    m11_log_event(state, M11_COLOR_YELLOW,
                                  "T%u: %s ATE %s (KEEPS ITEM)",
                                  (unsigned int)state->world.gameTick,
                                  champName, itemName);
                } else {
                    champ->inventory[useSlot] = THING_NONE;
                }
                m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                              "T%u: %s ATE %s (+%d FOOD)",
                              (unsigned int)state->world.gameTick,
                              champName, itemName, foodAmount);
                m11_set_status(state, "USE", "FOOD CONSUMED");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle),
                         "ATE %s", itemName);
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "%s FEELS LESS HUNGRY (+%d)",
                         champName, foodAmount);
                m11_refresh_hash(state);
                return 1;
            }
        }
    }

    m11_set_status(state, "USE", "CANNOT USE THIS ITEM");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s", itemName);
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "THIS ITEM CANNOT BE USED DIRECTLY");
    return 0;
}

void M11_GameView_ProcessTickEmissions(M11_GameViewState* state) {
    int i;
    if (!state) {
        return;
    }
    for (i = 0; i < state->lastTickResult.emissionCount; ++i) {
        const struct TickEmission_Compat* e = &state->lastTickResult.emissions[i];
        switch (e->kind) {
            case EMIT_DAMAGE_DEALT: {
                int champIdx = state->world.party.activeChampionIndex;
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: DAMAGE %d DEALT",
                              (unsigned int)state->world.gameTick,
                              (int)e->payload[2]);
                /* Award combat XP to the active champion */
                m11_award_combat_xp(state, champIdx, (int)e->payload[2]);
                break;
            }
            case EMIT_KILL_NOTIFY: {
                /* payload[0] = creature type (if available), else -1 */
                int cType = (int)e->payload[0];
                m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                              "T%u: %s DEFEATED",
                              (unsigned int)state->world.gameTick,
                              (cType >= 0 && cType < 27)
                                  ? m11_creature_name(cType)
                                  : "ENEMY");
                /* Award kill XP bonus */
                m11_award_kill_xp(state, cType >= 0 ? cType : 0);
                break;
            }
            case EMIT_XP_AWARD:
                m11_log_event(state, M11_COLOR_YELLOW,
                              "T%u: EXPERIENCE GAINED",
                              (unsigned int)state->world.gameTick);
                break;
            case EMIT_CHAMPION_DOWN: {
                int champIdx = (int)e->payload[0];
                char name[16];
                if (champIdx >= 0 && champIdx < CHAMPION_MAX_PARTY &&
                    state->world.party.champions[champIdx].present) {
                    m11_format_champion_name(
                        state->world.party.champions[champIdx].name,
                        name, sizeof(name));
                } else {
                    snprintf(name, sizeof(name), "CHAMPION");
                }
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: %s HAS FALLEN",
                              (unsigned int)state->world.gameTick, name);
                break;
            }
            case EMIT_PARTY_MOVED:
                m11_log_event(state, M11_COLOR_LIGHT_GRAY,
                              "T%u: PARTY MOVED TO %d,%d",
                              (unsigned int)state->world.gameTick,
                              state->world.party.mapX,
                              state->world.party.mapY);
                break;
            case EMIT_DOOR_STATE:
                m11_log_event(state, M11_COLOR_YELLOW,
                              "T%u: DOOR STATE CHANGED",
                              (unsigned int)state->world.gameTick);
                break;
            case EMIT_SPELL_EFFECT: {
                /* payload[0]=champIdx, [1]=spellKind, [2]=spellType, [3]=power */
                int sChamp = (int)e->payload[0];
                int sKind = (int)e->payload[1];
                int sType = (int)e->payload[2];
                int sPow  = (int)e->payload[3];
                const char* kindStr = "SPELL";
                if (sKind == C2_SPELL_KIND_PROJECTILE_COMPAT) kindStr = "PROJECTILE";
                else if (sKind == C3_SPELL_KIND_OTHER_COMPAT) kindStr = "ENCHANTMENT";
                else if (sKind == C1_SPELL_KIND_POTION_COMPAT) kindStr = "POTION";
                m11_log_event(state, M11_COLOR_CYAN,
                              "T%u: %s EFFECT APPLIED (TYPE %d, POWER %d)",
                              (unsigned int)state->world.gameTick,
                              kindStr, sType, sPow);
                /* Award magic XP to the casting champion */
                m11_award_magic_xp(state, sChamp, sKind, sPow);
                break;
            }
            default:
                break;
        }
    }
}

void M11_GameView_Init(M11_GameViewState* state) {
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    m11_set_status(state, "BOOT", "GAME VIEW NOT STARTED");
    m11_set_inspect_readout(state, "NO FOCUS", "PRESS ENTER OR CLICK THE VIEW TO READ THE FRONT CELL");
}

void M11_GameView_Shutdown(M11_GameViewState* state) {
    if (!state) {
        return;
    }
    if (state->assetsAvailable) {
        M11_AssetLoader_Shutdown(&state->assetLoader);
        state->assetsAvailable = 0;
    }
    if (state->active) {
        F0883_WORLD_Free_Compat(&state->world);
    }
    memset(state, 0, sizeof(*state));
}

int M11_GameView_Start(M11_GameViewState* state, const M11_GameLaunchSpec* spec) {
    char dungeonPath[M11_GAME_VIEW_PATH_CAPACITY];
    if (!state || !spec || !spec->title) {
        return 0;
    }
    if (spec->sourceKind == M11_GAME_SOURCE_DIRECT_DUNGEON) {
        if (!spec->dungeonPath || spec->dungeonPath[0] == '\0') {
            return 0;
        }
        snprintf(dungeonPath, sizeof(dungeonPath), "%s", spec->dungeonPath);
    } else if (spec->dungeonPath && spec->dungeonPath[0] != '\0') {
        snprintf(dungeonPath, sizeof(dungeonPath), "%s", spec->dungeonPath);
    } else if (!spec->dataDir || spec->dataDir[0] == '\0' ||
               !m11_resolve_builtin_dungeon_path(dungeonPath,
                                                sizeof(dungeonPath),
                                                spec->dataDir,
                                                spec->gameId)) {
        return 0;
    }
    M11_GameView_Shutdown(state);
    M11_GameView_Init(state);
    if (F0882_WORLD_InitFromDungeonDat_Compat(dungeonPath, 0xF1A5U, &state->world) != 1) {
        m11_set_status(state, "BOOT", "FAILED TO LOAD DUNGEON.DAT");
        return 0;
    }
    state->active = 1;
    state->startedFromLauncher = 1;
    state->sourceKind = spec->sourceKind;
    snprintf(state->title, sizeof(state->title), "%s", spec->title);
    snprintf(state->sourceId, sizeof(state->sourceId), "%s",
             spec->sourceId ? spec->sourceId : "launcher");
    snprintf(state->dungeonPath, sizeof(state->dungeonPath), "%s", dungeonPath);

    /* Try to open GRAPHICS.DAT from the same directory as the dungeon file */
    {
        char graphicsDatPath[M11_GAME_VIEW_PATH_CAPACITY];
        size_t dungeonLen = strlen(dungeonPath);
        size_t slashPos = dungeonLen;
        while (slashPos > 0 && dungeonPath[slashPos - 1] != '/' && dungeonPath[slashPos - 1] != '\\') {
            --slashPos;
        }
        if (slashPos > 0 && slashPos + 13 < sizeof(graphicsDatPath)) {
            memcpy(graphicsDatPath, dungeonPath, slashPos);
            memcpy(graphicsDatPath + slashPos, "GRAPHICS.DAT", 13);
            if (M11_AssetLoader_Init(&state->assetLoader, graphicsDatPath)) {
                state->assetsAvailable = 1;
            }
        }
    }

    m11_refresh_hash(state);
    m11_mark_explored(state);
    m11_log_event(state, M11_COLOR_YELLOW, "T0: %s LOADED", spec->title);
    m11_set_status(state, "BOOT", "GAME DATA LOADED");
    m11_set_inspect_readout(state, "READY", "CLICK CENTER TO ADVANCE OR READ, CLICK SIDES TO TURN, TAB PICKS THE FRONT CHAMPION");
    return 1;
}

int M11_GameView_OpenSelectedMenuEntry(M11_GameViewState* state,
                                       const M12_StartupMenuState* menuState) {
    const M12_MenuEntry* entry;
    M11_GameLaunchSpec spec;
    if (!state || !menuState) {
        return 0;
    }
    entry = M12_StartupMenu_GetEntry(menuState, menuState->selectedIndex);
    if (!entry || entry->kind != M12_MENU_ENTRY_GAME || !entry->available) {
        return 0;
    }
    memset(&spec, 0, sizeof(spec));
    spec.title = entry->title;
    spec.gameId = entry->gameId;
    spec.dataDir = M12_AssetStatus_GetDataDir(&menuState->assetStatus);
    spec.sourceId = entry->gameId;
    spec.sourceKind = (entry->sourceKind == M12_MENU_SOURCE_CUSTOM_DUNGEON)
                          ? M11_GAME_SOURCE_CUSTOM_DUNGEON
                          : M11_GAME_SOURCE_BUILTIN_CATALOG;
    return M11_GameView_Start(state, &spec);
}

int M11_GameView_StartDm1(M11_GameViewState* state, const char* dataDir) {
    M11_GameLaunchSpec spec;
    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER";
    spec.gameId = "dm1";
    spec.dataDir = dataDir;
    spec.sourceId = "dm1";
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    return M11_GameView_Start(state, &spec);
}

int M11_GameView_QuickSave(M11_GameViewState* state) {
    char path[M11_GAME_VIEW_PATH_CAPACITY];
    FILE* file = NULL;
    unsigned char* blob = NULL;
    int blobSize;
    int bytesWritten = 0;
    unsigned char header[M11_QUICKSAVE_HEADER_SIZE];

    if (!state || !state->active) {
        return 0;
    }
    if (!M11_GameView_GetQuickSavePath(state, path, sizeof(path))) {
        m11_set_status(state, "SAVE", "SAVE PATH TOO LONG");
        return 0;
    }

    blobSize = F0899_WORLD_SerializedSize_Compat(&state->world);
    if (blobSize <= 0) {
        m11_set_status(state, "SAVE", "SERIALISE SIZE FAILED");
        return 0;
    }

    blob = (unsigned char*)malloc((size_t)blobSize);
    if (!blob) {
        m11_set_status(state, "SAVE", "OUT OF MEMORY");
        return 0;
    }
    if (!F0897_WORLD_Serialize_Compat(&state->world, blob, blobSize, &bytesWritten) ||
        bytesWritten != blobSize) {
        free(blob);
        m11_set_status(state, "SAVE", "SERIALISE FAILED");
        return 0;
    }

    memcpy(header, g_m11_quicksave_magic, sizeof(g_m11_quicksave_magic));
    m11_write_u32_le(header + 8, (uint32_t)blobSize);
    m11_write_u32_le(header + 12, state->lastWorldHash);

    file = fopen(path, "wb");
    if (!file) {
        free(blob);
        m11_set_status(state, "SAVE", "OPEN FAILED");
        return 0;
    }
    if (fwrite(header, 1U, sizeof(header), file) != sizeof(header) ||
        fwrite(blob, 1U, (size_t)blobSize, file) != (size_t)blobSize ||
        fclose(file) != 0) {
        free(blob);
        m11_set_status(state, "SAVE", "WRITE FAILED");
        return 0;
    }
    free(blob);

    m11_set_status(state, "SAVE", "QUICKSAVE WRITTEN");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "SAVE SLOT READY");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "F9 RESTORES TICK %u FROM %s",
             (unsigned int)state->world.gameTick,
             path);
    return 1;
}

int M11_GameView_QuickLoad(M11_GameViewState* state) {
    char path[M11_GAME_VIEW_PATH_CAPACITY];
    FILE* file = NULL;
    long fileSizeLong;
    int blobSize;
    unsigned char header[M11_QUICKSAVE_HEADER_SIZE];
    unsigned char* blob = NULL;
    struct GameWorld_Compat loadedWorld;
    uint32_t expectedHash;
    uint32_t loadedHash = 0;

    if (!state || !state->active) {
        return 0;
    }
    if (!M11_GameView_GetQuickSavePath(state, path, sizeof(path))) {
        m11_set_status(state, "LOAD", "SAVE PATH TOO LONG");
        return 0;
    }

    file = fopen(path, "rb");
    if (!file) {
        m11_set_status(state, "LOAD", "NO QUICKSAVE FOUND");
        return 0;
    }
    if (fread(header, 1U, sizeof(header), file) != sizeof(header) ||
        memcmp(header, g_m11_quicksave_magic, sizeof(g_m11_quicksave_magic)) != 0) {
        fclose(file);
        m11_set_status(state, "LOAD", "SAVE HEADER INVALID");
        return 0;
    }

    blobSize = (int)m11_read_u32_le(header + 8);
    expectedHash = m11_read_u32_le(header + 12);
    if (blobSize <= 0) {
        fclose(file);
        m11_set_status(state, "LOAD", "SAVE SIZE INVALID");
        return 0;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        m11_set_status(state, "LOAD", "SAVE SEEK FAILED");
        return 0;
    }
    fileSizeLong = ftell(file);
    if (fileSizeLong < (long)M11_QUICKSAVE_HEADER_SIZE ||
        fileSizeLong != (long)(M11_QUICKSAVE_HEADER_SIZE + blobSize) ||
        fseek(file, (long)M11_QUICKSAVE_HEADER_SIZE, SEEK_SET) != 0) {
        fclose(file);
        m11_set_status(state, "LOAD", "SAVE LENGTH MISMATCH");
        return 0;
    }

    blob = (unsigned char*)malloc((size_t)blobSize);
    if (!blob) {
        fclose(file);
        m11_set_status(state, "LOAD", "OUT OF MEMORY");
        return 0;
    }
    if (fread(blob, 1U, (size_t)blobSize, file) != (size_t)blobSize || fclose(file) != 0) {
        free(blob);
        m11_set_status(state, "LOAD", "READ FAILED");
        return 0;
    }

    memset(&loadedWorld, 0, sizeof(loadedWorld));
    if (!F0898_WORLD_Deserialize_Compat(&loadedWorld, blob, blobSize, NULL) ||
        !F0891_ORCH_WorldHash_Compat(&loadedWorld, &loadedHash) ||
        loadedHash != expectedHash) {
        F0883_WORLD_Free_Compat(&loadedWorld);
        free(blob);
        m11_set_status(state, "LOAD", "SAVE VERIFY FAILED");
        return 0;
    }
    free(blob);

    F0883_WORLD_Free_Compat(&state->world);
    state->world = loadedWorld;
    memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
    m11_refresh_hash(state);
    m11_set_status(state, "LOAD", "QUICKSAVE RESTORED");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "RESTORED");
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "TICK %u HASH %08X RELOADED FROM %s",
             (unsigned int)state->world.gameTick,
             (unsigned int)state->lastWorldHash,
             path);
    return 1;
}

M11_GameInputResult M11_GameView_AdvanceIdleTick(M11_GameViewState* state) {
    if (!state || !state->active) {
        return M11_GAME_INPUT_IGNORED;
    }
    if (!m11_apply_tick(state, CMD_NONE, "WAIT")) {
        return M11_GAME_INPUT_IGNORED;
    }
    return M11_GAME_INPUT_REDRAW;
}

static int m11_apply_tick(M11_GameViewState* state,
                          uint8_t command,
                          const char* actionLabel) {
    struct TickInput_Compat input;
    struct PartyState_Compat beforeParty;
    uint32_t beforeTick;
    uint32_t beforeHash;
    if (!state || !state->active) {
        return 0;
    }
    memset(&input, 0, sizeof(input));
    beforeParty = state->world.party;
    beforeTick = state->world.gameTick;
    beforeHash = state->lastWorldHash;
    input.tick = state->world.gameTick;
    input.command = command;
    if (command == CMD_ATTACK) {
        input.commandArg1 = (uint8_t)(state->world.party.activeChampionIndex >= 0
                                          ? state->world.party.activeChampionIndex
                                          : 0);
        input.commandArg2 = (uint8_t)state->world.party.direction;
    }
    memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
    if (F0884_ORCH_AdvanceOneTick_Compat(&state->world, &input, &state->lastTickResult) == 0) {
        m11_set_status(state, actionLabel, "TICK REJECTED");
        return 0;
    }
    state->lastWorldHash = state->lastTickResult.worldHashPost;

    /* Process emissions into the message log */
    M11_GameView_ProcessTickEmissions(state);

    /* Apply survival mechanics */
    m11_apply_survival_drain(state);
    m11_apply_rest_recovery(state);

    /* Creature AI: movement and autonomous damage */
    m11_process_creature_ticks(state);

    m11_check_party_death(state);

    /* Mark current cell as explored */
    m11_mark_explored(state);

    /* Check for environmental transitions (pits, teleporters) after movement */
    if (state->world.party.mapX != beforeParty.mapX ||
        state->world.party.mapY != beforeParty.mapY ||
        state->world.party.mapIndex != beforeParty.mapIndex) {
        m11_check_post_move_transitions(state);
    }

    if (command == CMD_ATTACK) {
        char champion[16];
        int attackRoll = 0;
        int i;
        m11_get_active_champion_label(state, champion, sizeof(champion));
        for (i = 0; i < state->lastTickResult.emissionCount; ++i) {
            const struct TickEmission_Compat* emission = &state->lastTickResult.emissions[i];
            if (emission->kind == EMIT_DAMAGE_DEALT) {
                attackRoll = emission->payload[2];
                break;
            }
        }
        m11_set_status(state, actionLabel, "STRIKE COMMITTED");
        snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s ATTACKS", champion);
        snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                 "FRONT CONTACT, ROLL %d, TICK %u",
                 attackRoll,
                 (unsigned int)state->world.gameTick);
    } else if (command == CMD_TURN_LEFT || command == CMD_TURN_RIGHT) {
        if (state->world.party.direction != beforeParty.direction) {
            m11_set_status(state, actionLabel, "FACING UPDATED");
        } else {
            m11_set_status(state, actionLabel, "TURN IGNORED");
        }
    } else if (command == CMD_NONE) {
        if (state->world.gameTick != beforeTick || state->lastWorldHash != beforeHash) {
            m11_set_status(state, actionLabel, "IDLE TICK ADVANCED");
        } else {
            m11_set_status(state, actionLabel, "IDLE TICK HELD");
        }
    } else if (state->world.party.mapIndex != beforeParty.mapIndex) {
        /* Map changed via pit or teleporter — status already set by transition handler */
        (void)0;
    } else if (state->world.party.mapX != beforeParty.mapX ||
               state->world.party.mapY != beforeParty.mapY) {
        m11_set_status(state, actionLabel, "PARTY MOVED");
    } else {
        m11_set_status(state, actionLabel, "MOVEMENT BLOCKED");
    }
    return 1;
}

M11_GameInputResult M11_GameView_HandleInput(M11_GameViewState* state,
                                             M12_MenuInput input) {
    uint8_t command = CMD_NONE;
    const char* label = "NONE";
    if (!state || !state->active) {
        return M11_GAME_INPUT_IGNORED;
    }
    switch (input) {
        case M12_MENU_INPUT_UP:
            command = m11_forward_command_for_direction(state->world.party.direction);
            label = "FORWARD";
            break;
        case M12_MENU_INPUT_DOWN:
            command = m11_backward_command_for_direction(state->world.party.direction);
            label = "BACKSTEP";
            break;
        case M12_MENU_INPUT_LEFT:
            command = CMD_TURN_LEFT;
            label = "TURN LEFT";
            break;
        case M12_MENU_INPUT_RIGHT:
            command = CMD_TURN_RIGHT;
            label = "TURN RIGHT";
            break;
        case M12_MENU_INPUT_STRAFE_LEFT:
            command = m11_strafe_left_command_for_direction(state->world.party.direction);
            label = "STRAFE LEFT";
            break;
        case M12_MENU_INPUT_STRAFE_RIGHT:
            command = m11_strafe_right_command_for_direction(state->world.party.direction);
            label = "STRAFE RIGHT";
            break;
        case M12_MENU_INPUT_ACCEPT:
            if (m11_inspect_front_cell(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        case M12_MENU_INPUT_ACTION:
            if (m11_front_cell_has_attack_target(state)) {
                command = CMD_ATTACK;
                label = "ATTACK";
                break;
            }
            if (m11_front_cell_is_door(state)) {
                if (m11_toggle_front_door(state)) {
                    return M11_GAME_INPUT_REDRAW;
                }
                return M11_GAME_INPUT_IGNORED;
            }
            command = CMD_NONE;
            label = "WAIT";
            break;
        case M12_MENU_INPUT_CYCLE_CHAMPION:
            if (m11_cycle_active_champion(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        case M12_MENU_INPUT_REST_TOGGLE:
            state->resting = !state->resting;
            if (state->resting) {
                m11_log_event(state, M11_COLOR_LIGHT_BLUE, "T%u: RESTING",
                              (unsigned int)state->world.gameTick);
                m11_set_status(state, "REST", "PARTY IS RESTING");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle), "RESTING");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "HP AND STAMINA RECOVER SLOWLY. PRESS R AGAIN TO WAKE.");
            } else {
                m11_log_event(state, M11_COLOR_LIGHT_BLUE, "T%u: WOKE UP",
                              (unsigned int)state->world.gameTick);
                m11_set_status(state, "REST", "PARTY AWAKE");
                snprintf(state->inspectTitle, sizeof(state->inspectTitle), "AWAKE");
                snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                         "REST ENDED. RESUME EXPLORING.");
            }
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_USE_STAIRS:
            if (m11_try_stairs_transition(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            m11_set_status(state, "STAIRS", "NO STAIRS HERE");
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_PICKUP_ITEM:
            if (M11_GameView_PickupItem(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_DROP_ITEM:
            if (M11_GameView_DropItem(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_SPELL_RUNE_1:
        case M12_MENU_INPUT_SPELL_RUNE_2:
        case M12_MENU_INPUT_SPELL_RUNE_3:
        case M12_MENU_INPUT_SPELL_RUNE_4:
        case M12_MENU_INPUT_SPELL_RUNE_5:
        case M12_MENU_INPUT_SPELL_RUNE_6: {
            int runeIdx = (int)(input - M12_MENU_INPUT_SPELL_RUNE_1);
            if (!state->spellPanelOpen) {
                M11_GameView_OpenSpellPanel(state);
            }
            if (M11_GameView_EnterRune(state, runeIdx)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        }
        case M12_MENU_INPUT_SPELL_CAST:
            if (state->spellPanelOpen && state->spellBuffer.runeCount >= 2) {
                M11_GameView_CastSpell(state);
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        case M12_MENU_INPUT_SPELL_CLEAR:
            if (state->spellPanelOpen) {
                M11_GameView_ClearSpell(state);
                M11_GameView_CloseSpellPanel(state);
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        case M12_MENU_INPUT_USE_ITEM:
            if (M11_GameView_UseItem(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_REDRAW;
        case M12_MENU_INPUT_BACK:
            m11_set_status(state, "RETURN", "BACK TO LAUNCHER");
            return M11_GAME_INPUT_RETURN_TO_MENU;
        case M12_MENU_INPUT_NONE:
        default:
            return M11_GAME_INPUT_IGNORED;
    }
    if (!m11_apply_tick(state, command, label)) {
        return M11_GAME_INPUT_IGNORED;
    }
    return M11_GAME_INPUT_REDRAW;
}

M11_GameInputResult M11_GameView_HandlePointer(M11_GameViewState* state,
                                               int x,
                                               int y,
                                               int primaryButton) {
    int slot;

    if (!state || !state->active || !primaryButton) {
        return M11_GAME_INPUT_IGNORED;
    }

    if (m11_point_in_utility_button(x, y, M11_UTILITY_INSPECT_X, M11_UTILITY_INSPECT_W)) {
        return M11_GameView_HandleInput(state, M12_MENU_INPUT_ACCEPT);
    }
    if (m11_point_in_utility_button(x, y, M11_UTILITY_SAVE_X, M11_UTILITY_SAVE_W)) {
        if (M11_GameView_QuickSave(state)) {
            return M11_GAME_INPUT_REDRAW;
        }
        return M11_GAME_INPUT_IGNORED;
    }
    if (m11_point_in_utility_button(x, y, M11_UTILITY_LOAD_X, M11_UTILITY_LOAD_W)) {
        if (M11_GameView_QuickLoad(state)) {
            return M11_GAME_INPUT_REDRAW;
        }
        return M11_GAME_INPUT_IGNORED;
    }
    if (m11_point_in_rect(x, y,
                          M11_UTILITY_PANEL_X,
                          M11_UTILITY_PANEL_Y,
                          M11_UTILITY_PANEL_W,
                          12)) {
        return M11_GameView_HandleInput(state, M12_MENU_INPUT_BACK);
    }

    if (m11_point_in_rect(x, y,
                          M11_PROMPT_STRIP_X,
                          M11_PROMPT_STRIP_Y,
                          M11_PROMPT_STRIP_W,
                          M11_PROMPT_STRIP_H) ||
        m11_point_in_rect(x, y, 218, 106, 86, 34)) {
        return M11_GameView_HandleInput(state, M12_MENU_INPUT_ACCEPT);
    }

    if (m11_point_in_rect(x, y,
                          M11_VIEWPORT_X,
                          M11_VIEWPORT_Y,
                          M11_VIEWPORT_W,
                          M11_VIEWPORT_H)) {
        return M11_GameView_HandleInput(state, m11_pointer_viewport_input(state, x, y));
    }

    if (m11_point_in_rect(x, y,
                          M11_CONTROL_STRIP_X,
                          M11_CONTROL_STRIP_Y,
                          M11_CONTROL_STRIP_W,
                          M11_CONTROL_STRIP_H)) {
        if (m11_point_in_rect(x, y, 18, 167, 15, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_LEFT);
        }
        if (m11_point_in_rect(x, y, 35, 167, 15, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_UP);
        }
        if (m11_point_in_rect(x, y, 52, 167, 15, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_RIGHT);
        }
        if (m11_point_in_rect(x, y, 69, 167, 15, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_DOWN);
        }
        if (m11_point_in_rect(x, y, 86, 167, 12, 10)) {
            return M11_GameView_HandleInput(state, M12_MENU_INPUT_ACTION);
        }
    }

    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        int slotX = M11_PARTY_PANEL_X + slot * M11_PARTY_SLOT_STEP;
        if (m11_point_in_rect(x, y,
                              slotX,
                              M11_PARTY_PANEL_Y + 20,
                              M11_PARTY_SLOT_W,
                              M11_PARTY_SLOT_H - 20)) {
            if (m11_set_active_champion(state, slot)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
        }
    }

    return M11_GAME_INPUT_IGNORED;
}

static void m11_draw_party_arrow(unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int size,
                                 int direction,
                                 unsigned char color) {
    int i;
    int cx = x + size / 2;
    int cy = y + size / 2;
    for (i = 0; i < size / 3; ++i) {
        switch (direction) {
            case DIR_NORTH:
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx, y + 2 + i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx - i, y + 3 + i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx + i, y + 3 + i, color);
                break;
            case DIR_EAST:
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + size - 3 - i, cy, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + size - 4 - i, cy - i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + size - 4 - i, cy + i, color);
                break;
            case DIR_SOUTH:
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx, y + size - 3 - i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx - i, y + size - 4 - i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx + i, y + size - 4 - i, color);
                break;
            case DIR_WEST:
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 2 + i, cy, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 3 + i, cy - i, color);
                m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 3 + i, cy + i, color);
                break;
            default:
                break;
        }
    }
}

typedef struct {
    int x;
    int y;
    int w;
    int h;
} M11_ViewRect;

typedef struct M11_ViewportCell {
    int valid;
    int mapX;
    int mapY;
    int relForward;
    int relSide;
    unsigned char square;
    int elementType;
    int thingCount;
    unsigned short firstThing;
    int doorState;
    int doorVertical;
    int hasDoorThing;
    int creatureType; /* -1 if no creature, else creature type index (0-26) */
    /* First floor item info for sprite rendering */
    int firstItemThingType;    /* THING_TYPE_WEAPON..JUNK, or -1 if no item */
    int firstItemSubtype;      /* weapon/armour/potion/junk subtype, or -1 */
    /* Wall/door ornament ordinal from thing data (0-15, -1 if none) */
    int wallOrnamentOrdinal;
    int doorOrnamentOrdinal;
    M11_SquareThingSummary summary;
} M11_ViewportCell;

static void m11_draw_creature_cue(unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  int x,
                                  int y,
                                  int w,
                                  int h,
                                  int depthIndex) {
    int bodyW = w / 3;
    int bodyH = h / 3;
    int bodyX;
    int bodyY;
    int eyeY;
    unsigned char color = depthIndex == 0 ? M11_COLOR_LIGHT_GREEN : M11_COLOR_GREEN;

    if (bodyW < 4 || bodyH < 5) {
        return;
    }

    bodyX = x + (w - bodyW) / 2;
    bodyY = y + (h - bodyH) / 2;
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  bodyX + bodyW / 4, bodyY - 2, bodyW / 2, 3, color);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  bodyX, bodyY, bodyW, bodyH, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                   bodyX + 1, bodyY + bodyH, bodyY + bodyH + 2, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                   bodyX + bodyW - 2, bodyY + bodyH, bodyY + bodyH + 2, color);
    eyeY = bodyY + 2;
    m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                  bodyX + bodyW / 3, eyeY, M11_COLOR_BLACK);
    m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                  bodyX + (bodyW * 2) / 3, eyeY, M11_COLOR_BLACK);
}

static void m11_draw_item_cue(unsigned char* framebuffer,
                              int framebufferWidth,
                              int framebufferHeight,
                              int x,
                              int y,
                              int w,
                              int h,
                              int count) {
    int items = count > 3 ? 3 : count;
    int i;
    int baseY = y + h - 6;

    for (i = 0; i < items; ++i) {
        int cx = x + w / 2 - 8 + i * 7;
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx, baseY - 2, M11_COLOR_WHITE);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx - 1, baseY - 1, M11_COLOR_WHITE);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, cx + 1, baseY - 1, M11_COLOR_WHITE);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      cx - 1, baseY, 3, 2, M11_COLOR_YELLOW);
    }
}

static void m11_draw_effect_cue(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                int x,
                                int y,
                                int w,
                                int h,
                                const M11_ViewportCell* cell) {
    int cx = x + w / 2;
    int cy = y + h / 2;
    if (!cell) {
        return;
    }
    if (cell->summary.projectiles > 0 || cell->summary.teleporters > 0) {
        m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                       cx - 3, cx + 3, cy, M11_COLOR_LIGHT_CYAN);
        m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                       cx, cy - 3, cy + 3, M11_COLOR_LIGHT_CYAN);
    }
    if (cell->summary.explosions > 0 || cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      cx - 4, cy - 4, 9, 9, M11_COLOR_MAGENTA);
    }
}

static void m11_append_phrase(char* out,
                              size_t outSize,
                              const char* phrase) {
    size_t used;
    if (!out || outSize == 0U || !phrase || phrase[0] == '\0') {
        return;
    }
    used = strlen(out);
    if (used > 0U && used + 2U < outSize) {
        snprintf(out + used, outSize - used, ", ");
        used = strlen(out);
    }
    if (used + 1U < outSize) {
        snprintf(out + used, outSize - used, "%s", phrase);
    }
}

static void m11_append_count_phrase(char* out,
                                    size_t outSize,
                                    int count,
                                    const char* singular,
                                    const char* plural) {
    char chunk[32];
    if (count <= 0 || !singular || !plural) {
        return;
    }
    snprintf(chunk, sizeof(chunk), "%d %s", count, count == 1 ? singular : plural);
    m11_append_phrase(out, outSize, chunk);
}

static void m11_format_square_summary(const M11_ViewportCell* cell,
                                      char* out,
                                      size_t outSize) {
    char extras[96];
    const char* base;

    if (!out || outSize == 0U) {
        return;
    }
    if (!cell || !cell->valid) {
        snprintf(out, outSize, "VOID");
        return;
    }

    extras[0] = '\0';
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        m11_append_phrase(extras, sizeof(extras),
                          (cell->doorState == 0 || cell->doorState == 5) ? "OPEN" : "SHUT");
    }
    m11_append_count_phrase(extras, sizeof(extras), cell->summary.groups, "FOE", "FOES");
    m11_append_count_phrase(extras, sizeof(extras), cell->summary.items, "ITEM", "ITEMS");
    m11_append_count_phrase(extras, sizeof(extras),
                            cell->summary.projectiles + cell->summary.explosions,
                            "EFFECT", "EFFECTS");
    m11_append_count_phrase(extras, sizeof(extras), cell->summary.sensors, "SENSOR", "SENSORS");
    m11_append_count_phrase(extras, sizeof(extras),
                            cell->summary.textStrings + cell->summary.teleporters,
                            "MARK", "MARKS");

    base = F0503_DUNGEON_GetElementName_Compat(cell->elementType);
    if (extras[0] != '\0') {
        snprintf(out, outSize, "%s, %s", base, extras);
    } else {
        snprintf(out, outSize, "%s", base);
    }
}

static void m11_format_square_things(unsigned short firstThing,
                                     int squareThingCount,
                                     char* out,
                                     size_t outSize) {
    int thingType;
    if (!out || outSize == 0U) {
        return;
    }
    if (firstThing == THING_ENDOFLIST || firstThing == THING_NONE || squareThingCount <= 0) {
        snprintf(out, outSize, "QUIET FLOOR");
        return;
    }
    thingType = THING_GET_TYPE(firstThing);
    switch (thingType) {
        case THING_TYPE_GROUP:
            snprintf(out, outSize, "%d CREATURE %s",
                     squareThingCount,
                     squareThingCount == 1 ? "CONTACT" : "CONTACTS");
            break;
        case THING_TYPE_PROJECTILE:
        case THING_TYPE_EXPLOSION:
            snprintf(out, outSize, "%d LIVE EFFECT %s",
                     squareThingCount,
                     squareThingCount == 1 ? "AHEAD" : "STACKED");
            break;
        case THING_TYPE_TEXTSTRING:
            snprintf(out, outSize, "TEXT PLAQUE");
            break;
        case THING_TYPE_SENSOR:
            snprintf(out, outSize, "TRIGGER TILE");
            break;
        case THING_TYPE_TELEPORTER:
            snprintf(out, outSize, "TELEPORT FIELD");
            break;
        default:
            snprintf(out, outSize, "%d FLOOR %s",
                     squareThingCount,
                     squareThingCount == 1 ? "ITEM" : "ITEMS");
            break;
    }
}

static void m11_get_food_water_average(const struct PartyState_Compat* party,
                                       int* outFood,
                                       int* outWater) {
    int slot;
    int foodTotal = 0;
    int waterTotal = 0;
    int count = 0;
    if (!party) {
        if (outFood) {
            *outFood = 0;
        }
        if (outWater) {
            *outWater = 0;
        }
        return;
    }
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        if (!party->champions[slot].present) {
            continue;
        }
        foodTotal += party->champions[slot].food;
        waterTotal += party->champions[slot].water;
        ++count;
    }
    if (outFood) {
        *outFood = count > 0 ? foodTotal / count : 0;
    }
    if (outWater) {
        *outWater = count > 0 ? waterTotal / count : 0;
    }
}

static void m11_direction_vectors(int direction,
                                  int* outForwardX,
                                  int* outForwardY,
                                  int* outRightX,
                                  int* outRightY) {
    if (!outForwardX || !outForwardY || !outRightX || !outRightY) {
        return;
    }
    switch (direction) {
        case DIR_NORTH:
            *outForwardX = 0;
            *outForwardY = -1;
            *outRightX = 1;
            *outRightY = 0;
            break;
        case DIR_EAST:
            *outForwardX = 1;
            *outForwardY = 0;
            *outRightX = 0;
            *outRightY = 1;
            break;
        case DIR_SOUTH:
            *outForwardX = 0;
            *outForwardY = 1;
            *outRightX = -1;
            *outRightY = 0;
            break;
        case DIR_WEST:
        default:
            *outForwardX = -1;
            *outForwardY = 0;
            *outRightX = 0;
            *outRightY = -1;
            break;
    }
}

static int m11_sample_viewport_cell(const M11_GameViewState* state,
                                    int relForward,
                                    int relSide,
                                    M11_ViewportCell* outCell) {
    int fx;
    int fy;
    int rx;
    int ry;
    int mapX;
    int mapY;
    unsigned char square = 0;
    unsigned short firstThing = THING_ENDOFLIST;
    M11_ViewportCell cell;

    memset(&cell, 0, sizeof(cell));
    cell.relForward = relForward;
    cell.relSide = relSide;
    cell.elementType = DUNGEON_ELEMENT_WALL;
    cell.firstThing = THING_ENDOFLIST;
    cell.doorState = -1;
    cell.creatureType = -1;
    cell.firstItemThingType = -1;
    cell.firstItemSubtype = -1;
    cell.wallOrnamentOrdinal = -1;
    cell.doorOrnamentOrdinal = -1;

    if (!state || !state->active) {
        if (outCell) {
            *outCell = cell;
        }
        return 0;
    }

    m11_direction_vectors(state->world.party.direction, &fx, &fy, &rx, &ry);
    mapX = state->world.party.mapX + relForward * fx + relSide * rx;
    mapY = state->world.party.mapY + relForward * fy + relSide * ry;
    cell.mapX = mapX;
    cell.mapY = mapY;

    if (!m11_get_square_byte(&state->world,
                             state->world.party.mapIndex,
                             mapX,
                             mapY,
                             &square)) {
        if (outCell) {
            *outCell = cell;
        }
        return 0;
    }

    firstThing = m11_get_first_square_thing(&state->world,
                                            state->world.party.mapIndex,
                                            mapX,
                                            mapY);
    cell.valid = 1;
    cell.square = square;
    cell.elementType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    cell.thingCount = m11_count_square_things(&state->world,
                                              state->world.party.mapIndex,
                                              mapX,
                                              mapY);
    cell.firstThing = firstThing;
    m11_summarize_square_things(&state->world,
                                state->world.party.mapIndex,
                                mapX,
                                mapY,
                                &cell.summary);

    if (cell.elementType == DUNGEON_ELEMENT_DOOR) {
        cell.doorState = square & 0x07;
        cell.doorVertical = (square & 0x08) != 0;
        if (firstThing != THING_ENDOFLIST && firstThing != THING_NONE &&
            THING_GET_TYPE(firstThing) == THING_TYPE_DOOR &&
            state->world.things && state->world.things->doors) {
            int doorIndex = THING_GET_INDEX(firstThing);
            if (doorIndex >= 0 && doorIndex < state->world.things->doorCount) {
                cell.hasDoorThing = 1;
                cell.doorVertical = state->world.things->doors[doorIndex].vertical;
            }
        }
    }

    /* Extract creature type from the first group thing on this square */
    if (cell.summary.groups > 0 && state->world.things && state->world.things->groups) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_GROUP) {
                int gIdx = THING_GET_INDEX(scanThing);
                if (gIdx >= 0 && gIdx < state->world.things->groupCount) {
                    cell.creatureType = (int)state->world.things->groups[gIdx].creatureType;
                }
                break;
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Extract first floor item type and subtype for sprite rendering */
    if (cell.summary.items > 0 && state->world.things) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            int tType = THING_GET_TYPE(scanThing);
            int tIdx = THING_GET_INDEX(scanThing);
            if (m11_thing_is_item(tType)) {
                cell.firstItemThingType = tType;
                switch (tType) {
                    case THING_TYPE_WEAPON:
                        if (state->world.things->weapons && tIdx >= 0 && tIdx < state->world.things->weaponCount)
                            cell.firstItemSubtype = (int)state->world.things->weapons[tIdx].type;
                        break;
                    case THING_TYPE_ARMOUR:
                        if (state->world.things->armours && tIdx >= 0 && tIdx < state->world.things->armourCount)
                            cell.firstItemSubtype = (int)state->world.things->armours[tIdx].type;
                        break;
                    case THING_TYPE_POTION:
                        if (state->world.things->potions && tIdx >= 0 && tIdx < state->world.things->potionCount)
                            cell.firstItemSubtype = (int)state->world.things->potions[tIdx].type;
                        break;
                    case THING_TYPE_JUNK:
                        if (state->world.things->junks && tIdx >= 0 && tIdx < state->world.things->junkCount)
                            cell.firstItemSubtype = (int)state->world.things->junks[tIdx].type;
                        break;
                    case THING_TYPE_SCROLL:
                        cell.firstItemSubtype = 0;
                        break;
                    case THING_TYPE_CONTAINER:
                        if (state->world.things->containers && tIdx >= 0 && tIdx < state->world.things->containerCount)
                            cell.firstItemSubtype = (int)state->world.things->containers[tIdx].type;
                        break;
                    default:
                        cell.firstItemSubtype = 0;
                        break;
                }
                break;
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Extract door ornament ordinal */
    if (cell.elementType == DUNGEON_ELEMENT_DOOR && cell.hasDoorThing &&
        state->world.things && state->world.things->doors) {
        int doorIdx = THING_GET_INDEX(firstThing);
        if (doorIdx >= 0 && doorIdx < state->world.things->doorCount) {
            int ord = (int)state->world.things->doors[doorIdx].ornamentOrdinal;
            if (ord > 0) cell.doorOrnamentOrdinal = ord;
        }
    }

    /* Extract wall ornament ordinal from sensors on this square.
     * In DM, wall ornaments are placed via sensor things whose
     * ornamentOrdinal field specifies the graphic to display. */
    if (state->world.things && state->world.things->sensors) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_SENSOR) {
                int sIdx = THING_GET_INDEX(scanThing);
                if (sIdx >= 0 && sIdx < state->world.things->sensorCount) {
                    int ord = (int)state->world.things->sensors[sIdx].ornamentOrdinal;
                    if (ord > 0) {
                        cell.wallOrnamentOrdinal = ord;
                        break;
                    }
                }
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    if (outCell) {
        *outCell = cell;
    }
    return 1;
}

static int m11_viewport_cell_is_open(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return 0;
    }
    if (cell->elementType == DUNGEON_ELEMENT_WALL ||
        cell->elementType == DUNGEON_ELEMENT_FAKEWALL) {
        return 0;
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        return cell->doorState == 0 || cell->doorState == 5;
    }
    return 1;
}

static int m11_build_front_text_readout(const M11_GameViewState* state,
                                        const M11_ViewportCell* cell,
                                        char* outTitle,
                                        size_t outTitleSize,
                                        char* outDetail,
                                        size_t outDetailSize) {
    unsigned short thing;

    if (!state || !cell || !cell->valid || !outTitle || !outDetail) {
        return 0;
    }

    thing = cell->firstThing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE) {
        int thingType = THING_GET_TYPE(thing);
        int thingIndex = THING_GET_INDEX(thing);
        if (thingType == THING_TYPE_TEXTSTRING && state->world.things &&
            state->world.things->textStrings && thingIndex >= 0 &&
            thingIndex < state->world.things->textStringCount) {
            const struct DungeonTextString_Compat* textThing = &state->world.things->textStrings[thingIndex];
            char decoded[96];
            if (state->world.things->textData && state->world.things->textDataWordCount > 0 &&
                F0507_DUNGEON_DecodeTextAtOffset_Compat(state->world.things->textData,
                                                        state->world.things->textDataWordCount,
                                                        textThing->textDataWordOffset,
                                                        decoded,
                                                        sizeof(decoded)) >= 0 &&
                decoded[0] != '\0') {
                snprintf(outTitle, outTitleSize, "TEXT PLAQUE");
                snprintf(outDetail, outDetailSize, "%s", decoded);
                return 1;
            }
            snprintf(outTitle, outTitleSize, "TEXT PLAQUE");
            snprintf(outDetail, outDetailSize, "TEXT OFFSET %u (DECODE UNAVAILABLE)",
                     (unsigned int)textThing->textDataWordOffset);
            return 1;
        }
        thing = m11_raw_next_thing(state->world.things, thing);
    }

    return 0;
}

static int m11_inspect_front_cell(M11_GameViewState* state) {
    M11_ViewportCell frontCell;
    char title[64];
    char detail[128];

    if (!state || !state->active) {
        return 0;
    }

    memset(&frontCell, 0, sizeof(frontCell));
    if (!m11_sample_viewport_cell(state, 1, 0, &frontCell) || !frontCell.valid) {
        m11_set_status(state, "INSPECT", "VOID AHEAD");
        m11_set_inspect_readout(state, "VOID", "TURN TO RE-ENTER THE MAP");
        return 1;
    }

    if (m11_build_front_text_readout(state, &frontCell,
                                     title, sizeof(title),
                                     detail, sizeof(detail))) {
        m11_set_status(state, "INSPECT", "READ TEXT");
        m11_set_inspect_readout(state, title, detail);
        return 1;
    }

    if (frontCell.elementType == DUNGEON_ELEMENT_DOOR) {
        snprintf(title, sizeof(title), "FRONT DOOR");
        snprintf(detail, sizeof(detail), "%s, STATE %d, %s",
                 frontCell.doorVertical ? "VERTICAL" : "HORIZONTAL",
                 frontCell.doorState,
                 m11_viewport_cell_is_open(&frontCell) ? "OPEN ENOUGH TO CROSS" : "STILL CLOSED");
        m11_set_status(state, "INSPECT", "DOOR CHECK");
        m11_set_inspect_readout(state, title, detail);
        return 1;
    }

    if (frontCell.summary.groups > 0) {
        snprintf(title, sizeof(title), "CREATURE CONTACT");
        snprintf(detail, sizeof(detail), "%d GROUP TARGET%s IN FRONT CELL",
                 frontCell.summary.groups,
                 frontCell.summary.groups == 1 ? "" : "S");
        m11_set_status(state, "INSPECT", "ENEMY SPOTTED");
        m11_set_inspect_readout(state, title, detail);
        return 1;
    }

    if (frontCell.summary.items > 0 || frontCell.summary.projectiles > 0 ||
        frontCell.summary.explosions > 0 || frontCell.summary.sensors > 0 ||
        frontCell.summary.teleporters > 0 || frontCell.summary.textStrings > 0 ||
        frontCell.elementType == DUNGEON_ELEMENT_PIT ||
        frontCell.elementType == DUNGEON_ELEMENT_STAIRS ||
        frontCell.elementType == DUNGEON_ELEMENT_TELEPORTER) {
        snprintf(title, sizeof(title), "%s",
                 F0503_DUNGEON_GetElementName_Compat(frontCell.elementType));
        m11_format_square_summary(&frontCell, detail, sizeof(detail));
        m11_set_status(state, "INSPECT", "FRONT CELL READOUT");
        m11_set_inspect_readout(state, title, detail);
        return 1;
    }

    return 0;
}

static int m11_get_front_cell(const M11_GameViewState* state, M11_ViewportCell* outCell) {
    M11_ViewportCell frontCell;

    memset(&frontCell, 0, sizeof(frontCell));
    if (!state || !state->active || !m11_sample_viewport_cell(state, 1, 0, &frontCell) || !frontCell.valid) {
        if (outCell) {
            *outCell = frontCell;
        }
        return 0;
    }
    if (outCell) {
        *outCell = frontCell;
    }
    return 1;
}

static int m11_front_cell_has_attack_target(const M11_GameViewState* state) {
    M11_ViewportCell frontCell;

    if (!m11_get_front_cell(state, &frontCell)) {
        return 0;
    }
    return frontCell.summary.groups > 0;
}

static int m11_front_cell_is_door(const M11_GameViewState* state) {
    M11_ViewportCell frontCell;

    if (!m11_get_front_cell(state, &frontCell)) {
        return 0;
    }
    return frontCell.valid && frontCell.elementType == DUNGEON_ELEMENT_DOOR;
}

static int m11_toggle_front_door(M11_GameViewState* state) {
    M11_ViewportCell frontCell;
    unsigned char* squarePtr;
    int newDoorState;
    uint32_t preTick;

    if (!state || !state->active || !m11_get_front_cell(state, &frontCell) ||
        !frontCell.valid || frontCell.elementType != DUNGEON_ELEMENT_DOOR) {
        return 0;
    }

    squarePtr = m11_get_square_ptr(&state->world,
                                   state->world.party.mapIndex,
                                   frontCell.mapX,
                                   frontCell.mapY);
    if (!squarePtr) {
        return 0;
    }

    if (frontCell.doorState == 5) {
        m11_set_status(state, "DOOR", "DOOR DESTROYED");
        m11_set_inspect_readout(state, "FRONT DOOR", "DESTROYED, NO LONGER BLOCKING THE PASSAGE");
        return 1;
    }

    newDoorState = (frontCell.doorState == 0) ? 4 : 0;
    *squarePtr = (unsigned char)((*squarePtr & ~0x07U) | (unsigned char)newDoorState);

    preTick = state->world.gameTick;
    memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
    state->world.gameTick += 1;
    state->world.timeline.nowTick = state->world.gameTick;
    state->lastTickResult.preTick = preTick;
    state->lastTickResult.postTick = state->world.gameTick;
    state->lastTickResult.emissionCount = 2;
    state->lastTickResult.emissions[0].kind = EMIT_DOOR_STATE;
    state->lastTickResult.emissions[0].payloadSize = 4;
    state->lastTickResult.emissions[0].payload[0] = frontCell.mapX;
    state->lastTickResult.emissions[0].payload[1] = frontCell.mapY;
    state->lastTickResult.emissions[0].payload[2] = newDoorState;
    state->lastTickResult.emissions[0].payload[3] = state->world.party.mapIndex;
    state->lastTickResult.emissions[1].kind = EMIT_SOUND_REQUEST;
    state->lastTickResult.emissions[1].payloadSize = 4;
    state->lastTickResult.emissions[1].payload[0] = newDoorState == 0 ? 1 : 2;
    state->lastTickResult.emissions[1].payload[1] = frontCell.mapX;
    state->lastTickResult.emissions[1].payload[2] = frontCell.mapY;
    state->lastTickResult.emissions[1].payload[3] = state->world.party.mapIndex;
    m11_refresh_hash(state);
    state->lastTickResult.worldHashPost = state->lastWorldHash;

    if (newDoorState == 0) {
        m11_set_status(state, "DOOR", "DOOR OPENED");
        m11_set_inspect_readout(state, "FRONT DOOR", "OPEN, PASSAGE CLEAR, CLICK CENTER OR PRESS UP TO CROSS");
    } else {
        m11_set_status(state, "DOOR", "DOOR CLOSED");
        m11_set_inspect_readout(state, "FRONT DOOR", "SHUT, ENTER INSPECTS, SPACE TOGGLES AGAIN");
    }
    return 1;
}

static M12_MenuInput m11_pointer_viewport_input(const M11_GameViewState* state,
                                                int x,
                                                int y) {
    M11_ViewportCell frontCell;
    int localX;
    int localY;

    if (!state || !state->active) {
        return M12_MENU_INPUT_NONE;
    }

    localX = x - M11_VIEWPORT_X;
    localY = y - M11_VIEWPORT_Y;

    if (localX < M11_VIEWPORT_W / 3) {
        if (localY >= (M11_VIEWPORT_H * 2) / 3) {
            return M12_MENU_INPUT_STRAFE_LEFT;
        }
        return M12_MENU_INPUT_LEFT;
    }
    if (localX >= (M11_VIEWPORT_W * 2) / 3) {
        if (localY >= (M11_VIEWPORT_H * 2) / 3) {
            return M12_MENU_INPUT_STRAFE_RIGHT;
        }
        return M12_MENU_INPUT_RIGHT;
    }
    if (!m11_get_front_cell(state, &frontCell)) {
        return M12_MENU_INPUT_ACCEPT;
    }
    if (localY >= (M11_VIEWPORT_H * 2) / 3) {
        if (frontCell.summary.groups > 0) {
            return M12_MENU_INPUT_ACTION;
        }
        if (m11_viewport_cell_is_open(&frontCell) &&
            frontCell.summary.items == 0 &&
            frontCell.summary.sensors == 0 &&
            frontCell.summary.textStrings == 0 &&
            frontCell.summary.projectiles == 0 &&
            frontCell.summary.explosions == 0 &&
            frontCell.elementType != DUNGEON_ELEMENT_PIT) {
            return M12_MENU_INPUT_UP;
        }
    }
    return M12_MENU_INPUT_ACCEPT;
}

static unsigned char m11_viewport_fill_color(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return M11_COLOR_DARK_GRAY;
    }
    switch (cell->elementType) {
        case DUNGEON_ELEMENT_WALL:
            return M11_COLOR_DARK_GRAY;
        case DUNGEON_ELEMENT_CORRIDOR:
            return M11_COLOR_BLACK;
        case DUNGEON_ELEMENT_PIT:
            return M11_COLOR_RED;
        case DUNGEON_ELEMENT_STAIRS:
            return M11_COLOR_GREEN;
        case DUNGEON_ELEMENT_DOOR:
            return M11_COLOR_BROWN;
        case DUNGEON_ELEMENT_TELEPORTER:
            return M11_COLOR_MAGENTA;
        case DUNGEON_ELEMENT_FAKEWALL:
            return M11_COLOR_LIGHT_GRAY;
        default:
            return M11_COLOR_BLACK;
    }
}

static unsigned char m11_feature_accent_color(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return M11_COLOR_DARK_GRAY;
    }
    switch (cell->elementType) {
        case DUNGEON_ELEMENT_DOOR: return M11_COLOR_LIGHT_RED;
        case DUNGEON_ELEMENT_STAIRS: return M11_COLOR_YELLOW;
        case DUNGEON_ELEMENT_PIT: return M11_COLOR_BROWN;
        case DUNGEON_ELEMENT_TELEPORTER: return M11_COLOR_LIGHT_CYAN;
        case DUNGEON_ELEMENT_FAKEWALL: return M11_COLOR_MAGENTA;
        case DUNGEON_ELEMENT_CORRIDOR: return M11_COLOR_LIGHT_GRAY;
        default: return M11_COLOR_WHITE;
    }
}

static void m11_draw_wall_contents(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const M11_ViewRect* rect,
                                   const M11_ViewportCell* cell,
                                   int depthIndex);

/* Forward declaration — m11_draw_wall_face needs access to the game state
 * for asset-backed rendering, but the state pointer is threaded through
 * the Draw call chain. We use a file-scope pointer set during Draw. */
static const M11_GameViewState* g_drawState = NULL;

/* Forward declarations for asset-backed rendering helpers */
static int m11_draw_door_frame_asset(const M11_GameViewState* state,
                                     unsigned char* framebuffer,
                                     int fbW, int fbH,
                                     const M11_ViewRect* rect,
                                     int depthIndex);
static int m11_draw_door_side_asset(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW, int fbH,
                                    int x, int y, int w, int h,
                                    int depthIndex);
static int m11_draw_stairs_asset(const M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 int fbW, int fbH,
                                 const M11_ViewRect* rect,
                                 int depthIndex, int stairUp);
static int m11_draw_creature_sprite(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW, int fbH,
                                    int x, int y, int w, int h,
                                    int creatureType, int depthIndex);
static int m11_draw_item_sprite(const M11_GameViewState* state,
                                unsigned char* framebuffer,
                                int fbW, int fbH,
                                int x, int y, int w, int h,
                                int thingType, int subtype,
                                int depthIndex);
static int m11_draw_wall_ornament(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW, int fbH,
                                  const M11_ViewRect* rect,
                                  int ornamentOrdinal,
                                  int depthIndex);
static int m11_draw_door_ornament(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW, int fbH,
                                  const M11_ViewRect* rect,
                                  int ornamentOrdinal,
                                  int depthIndex);

static void m11_draw_wall_face(unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight,
                               const M11_ViewRect* rect,
                               const M11_ViewportCell* cell,
                               int depthIndex) {
    int inset = 6 + depthIndex * 4;
    int faceX = rect->x + inset;
    int faceY = rect->y + inset / 2;
    int faceW = rect->w - inset * 2;
    int faceH = rect->h - inset;
    unsigned char accent = m11_feature_accent_color(cell);

    if (faceW < 8 || faceH < 8) {
        return;
    }

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  faceX, faceY, faceW, faceH,
                  cell->elementType == DUNGEON_ELEMENT_WALL ? M11_COLOR_DARK_GRAY : m11_viewport_fill_color(cell));
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  faceX, faceY, faceW, faceH, accent);

    switch (cell->elementType) {
        case DUNGEON_ELEMENT_DOOR:
            /* Try real door frame graphics first */
            if (g_drawState &&
                m11_draw_door_frame_asset(g_drawState, framebuffer,
                                          framebufferWidth, framebufferHeight,
                                          rect, depthIndex)) {
                break; /* Real asset drawn successfully */
            }
            /* Fallback: primitive door lines */
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                           faceX + faceW / 2, faceY + 3, faceY + faceH - 4, M11_COLOR_YELLOW);
            if (cell->doorState > 0 && cell->doorState < 5) {
                int shutHeight = (faceH - 8) * cell->doorState / 5;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              faceX + 3, faceY + faceH - 4 - shutHeight,
                              faceW - 6, shutHeight, M11_COLOR_BLACK);
            }
            break;
        case DUNGEON_ELEMENT_STAIRS:
            /* Try real stair graphics first */
            if (g_drawState) {
                int stairUp = (cell->square & 0x01);
                if (m11_draw_stairs_asset(g_drawState, framebuffer,
                                          framebufferWidth, framebufferHeight,
                                          rect, depthIndex, stairUp)) {
                    break;
                }
            }
            /* Fallback: primitive stair lines */
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           faceX + 4, faceX + faceW - 5, faceY + faceH - 5, M11_COLOR_YELLOW);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           faceX + 8, faceX + faceW - 9, faceY + faceH - 10, M11_COLOR_YELLOW);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           faceX + 12, faceX + faceW - 13, faceY + faceH - 15, M11_COLOR_YELLOW);
            break;
        case DUNGEON_ELEMENT_PIT:
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 5, faceY + faceH / 2,
                          faceW - 10, faceH / 3, M11_COLOR_BLACK);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 5, faceY + faceH / 2,
                          faceW - 10, faceH / 3, M11_COLOR_BROWN);
            break;
        case DUNGEON_ELEMENT_TELEPORTER:
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 5, faceY + 4, faceW - 10, faceH - 8, M11_COLOR_LIGHT_CYAN);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 10, faceY + 8, faceW - 20, faceH - 16, M11_COLOR_WHITE);
            break;
        case DUNGEON_ELEMENT_FAKEWALL:
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 5, faceY + 4, faceW - 10, faceH - 8, M11_COLOR_MAGENTA);
            break;
        case DUNGEON_ELEMENT_CORRIDOR:
        default:
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 4, faceY + 4, faceW - 8, faceH - 8, M11_COLOR_DARK_GRAY);
            break;
    }

    /* Draw wall ornament on wall-type cells (sensor-placed ornaments) */
    if (cell->elementType == DUNGEON_ELEMENT_WALL && cell->wallOrnamentOrdinal >= 0) {
        M11_ViewRect ornRect;
        ornRect.x = faceX;
        ornRect.y = faceY;
        ornRect.w = faceW;
        ornRect.h = faceH;
        if (g_drawState) {
            m11_draw_wall_ornament(g_drawState, framebuffer,
                                   framebufferWidth, framebufferHeight,
                                   &ornRect, cell->wallOrnamentOrdinal, depthIndex);
        }
    }

    /* Draw door ornament on door-type cells */
    if (cell->elementType == DUNGEON_ELEMENT_DOOR && cell->doorOrnamentOrdinal >= 0) {
        M11_ViewRect ornRect;
        ornRect.x = faceX;
        ornRect.y = faceY;
        ornRect.w = faceW;
        ornRect.h = faceH;
        if (g_drawState) {
            m11_draw_door_ornament(g_drawState, framebuffer,
                                   framebufferWidth, framebufferHeight,
                                   &ornRect, cell->doorOrnamentOrdinal, depthIndex);
        }
    }

    if (m11_viewport_cell_is_open(cell)) {
        m11_draw_wall_contents(framebuffer, framebufferWidth, framebufferHeight,
                               rect, cell, depthIndex);
    } else if (cell->thingCount > 0) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      faceX + faceW / 2 - 1, faceY + faceH / 2 - 1,
                      3, 3, M11_COLOR_WHITE);
    }
}

static void m11_draw_wall_contents(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const M11_ViewRect* rect,
                                   const M11_ViewportCell* cell,
                                   int depthIndex) {
    int inset = 6 + depthIndex * 4;
    int faceX = rect->x + inset;
    int faceY = rect->y + inset / 2;
    int faceW = rect->w - inset * 2;
    int faceH = rect->h - inset;

    if (!cell || !m11_viewport_cell_is_open(cell) || faceW < 8 || faceH < 8) {
        return;
    }
    if (cell->summary.groups > 0) {
        /* Try real creature sprite first, fall back to primitive cue */
        if (!g_drawState ||
            !m11_draw_creature_sprite(g_drawState, framebuffer,
                                      framebufferWidth, framebufferHeight,
                                      faceX + 4, faceY + 5,
                                      faceW - 8, faceH - 10,
                                      cell->creatureType, depthIndex)) {
            m11_draw_creature_cue(framebuffer, framebufferWidth, framebufferHeight,
                                  faceX + 4, faceY + 5, faceW - 8, faceH - 10, depthIndex);
        }
    }
    if (cell->summary.items > 0) {
        /* Try real item sprite first, fall back to primitive cue */
        if (!g_drawState ||
            !m11_draw_item_sprite(g_drawState, framebuffer,
                                  framebufferWidth, framebufferHeight,
                                  faceX + 2, faceY + 2, faceW - 4, faceH - 4,
                                  cell->firstItemThingType, cell->firstItemSubtype,
                                  depthIndex)) {
            m11_draw_item_cue(framebuffer, framebufferWidth, framebufferHeight,
                              faceX + 2, faceY + 2, faceW - 4, faceH - 4,
                              cell->summary.items);
        }
    }
    m11_draw_effect_cue(framebuffer, framebufferWidth, framebufferHeight,
                        faceX + 3, faceY + 3, faceW - 6, faceH - 6, cell);
}

/* Known GRAPHICS.DAT indices for rendering assets in CSB PC 3.4. */
enum {
    /* Wall texture sets (256x32) */
    M11_GFX_WALL_SET_0 = 42,
    M11_GFX_WALL_SET_1 = 43,
    M11_GFX_WALL_SET_2 = 44,
    M11_GFX_WALL_SET_3 = 45,

    /* Floor tiles (32x32) */
    M11_GFX_FLOOR_SET_0 = 76,
    M11_GFX_FLOOR_SET_1 = 77,

    /* Viewport background (224x136) */
    M11_GFX_VIEWPORT_BG = 0,
    M11_GFX_VIEWPORT_BG_ALT = 17,

    /* Viewport ceiling/floor panels */
    M11_GFX_CEILING_PANEL = 78,  /* 224x97 */
    M11_GFX_FLOOR_PANEL   = 79,  /* 224x39 */

    /* Door frame graphics at depth (perspective-scaled sizes) */
    M11_GFX_DOOR_FRAME_D3   = 70,  /*  36x49  — far depth */
    M11_GFX_DOOR_FRAME_D3W  = 71,  /*  83x49  — far depth wide */
    M11_GFX_DOOR_PILLAR_D3  = 72,  /*   8x52  — thin pillar */
    M11_GFX_DOOR_FRAME_D2   = 73,  /*  78x74  — mid depth */
    M11_GFX_DOOR_FRAME_D1   = 74,  /*  60x111 — near depth */
    M11_GFX_DOOR_FRAME_D0   = 75,  /*  33x136 — nearest side strip */
    M11_GFX_DOOR_SIDE_D0    = 86,  /*  32x123 */
    M11_GFX_DOOR_SIDE_D1    = 87,  /*  25x94  */
    M11_GFX_DOOR_SIDE_D2    = 88,  /*  18x65  */
    M11_GFX_DOOR_SIDE_D3    = 89,  /*  10x42  */

    /* Creature front-view sprites (groups of 3: small/mid/large).
     * Each group covers one creature graphic set:
     *   set 0 = 246..248, set 1 = 249..251, etc. */
    M11_GFX_CREATURE_BASE   = 246,
    M11_GFX_CREATURE_SETS_A = 4,   /* 4 sets at 246-257 */
    M11_GFX_CREATURE_BASE_B = 439, /* second batch */
    M11_GFX_CREATURE_SETS_B = 4,   /* 4 more sets at 439+ (439,443,448,451 are 96x88) */

    /* Item viewport sprites (small icons shown in corridor).
     * In CSB/DM, object list entries map thing types to graphic indices.
     * The mapping uses a base index per thing type; subtypes offset from
     * there.  Graphic indices 267+ hold the floor item icons. */
    M11_GFX_ITEM_SPRITE_BASE = 267, /* pairs of 14x19/16x19 icons */
    M11_GFX_ITEM_WEAPON_BASE = 267, /* 46 weapon subtypes */
    M11_GFX_ITEM_ARMOUR_BASE = 313, /* 30 armour subtypes */
    M11_GFX_ITEM_SCROLL_BASE = 343, /* 1 generic scroll icon */
    M11_GFX_ITEM_POTION_BASE = 344, /* 17 potion subtypes */
    M11_GFX_ITEM_CONTAINER_BASE = 361, /* 3 container subtypes */
    M11_GFX_ITEM_JUNK_BASE   = 364, /* 52 junk subtypes */
    M11_GFX_ITEM_SPRITE_END  = 416, /* safety cap */

    /* Wall ornament graphics (per wall set, 16 ornaments each).
     * In CSB/DM, wall ornaments start at graphic index 321 and are
     * organized as 16 ornaments per wall set.  We map using the map's
     * wallSet + ornament ordinal. */
    M11_GFX_WALL_ORNAMENT_BASE = 101, /* first wall ornament graphic */
    M11_GFX_WALL_ORNAMENTS_PER_SET = 16,
    M11_GFX_DOOR_ORNAMENT_BASE = 165, /* first door ornament graphic */
    M11_GFX_DOOR_ORNAMENTS_PER_SET = 16,

    /* Stair graphics */
    M11_GFX_STAIRS_DOWN_D2 = 93,  /* 33x136 */
    M11_GFX_STAIRS_DOWN_D1 = 95,  /* 60x111 */
    M11_GFX_STAIRS_UP_D2   = 94,  /* 33x136 */
    M11_GFX_STAIRS_UP_D1   = 96   /* 60x111 */
};

/* ================================================================
 * Asset-backed rendering helpers for GRAPHICS.DAT integration.
 *
 * These functions load specific graphics from the asset loader and
 * blit them into the framebuffer at the correct viewport positions.
 * Each function gracefully falls back to the existing primitive
 * rendering when assets are unavailable.
 * ================================================================ */

/* Apply depth-based light dimming to a rectangular region.
 * depthIndex 0 = nearest (bright), 2 = far (dim).
 * Works by shifting high-value palette indices down toward darker
 * counterparts in the 16-color VGA palette. */
static void m11_apply_depth_dimming(unsigned char* framebuffer,
                                    int fbW,
                                    int fbH,
                                    int rx,
                                    int ry,
                                    int rw,
                                    int rh,
                                    int depthIndex) {
    int yy, xx;
    /* Dimming table: maps palette index to a darker equivalent.
     * Applied once per depth level (depth 0 = no change). */
    static const unsigned char g_dim_table[16] = {
        0,  /* 0  BLACK       -> BLACK */
        0,  /* 1  NAVY        -> BLACK */
        0,  /* 2  GREEN       -> BLACK */
        1,  /* 3  CYAN        -> NAVY  */
        0,  /* 4  RED         -> BLACK */
        0,  /* 5  (unused)    -> BLACK */
        4,  /* 6  BROWN       -> RED   */
        8,  /* 7  LIGHT_GRAY  -> DARK_GRAY */
        0,  /* 8  DARK_GRAY   -> BLACK */
        1,  /* 9  LIGHT_BLUE  -> NAVY  */
        2,  /* 10 LIGHT_GREEN -> GREEN */
        3,  /* 11 LIGHT_CYAN  -> CYAN  */
        4,  /* 12 LIGHT_RED   -> RED   */
        1,  /* 13 MAGENTA     -> NAVY  */
        6,  /* 14 YELLOW      -> BROWN */
        7   /* 15 WHITE       -> LIGHT_GRAY */
    };
    int passes;
    if (depthIndex <= 0) return;
    passes = depthIndex; /* depth 1 = dim once, depth 2 = dim twice */
    for (yy = ry; yy < ry + rh && yy < fbH; ++yy) {
        if (yy < 0) continue;
        for (xx = rx; xx < rx + rw && xx < fbW; ++xx) {
            int p;
            unsigned char px;
            if (xx < 0) continue;
            px = framebuffer[yy * fbW + xx];
            for (p = 0; p < passes; ++p) {
                px = g_dim_table[px & 0x0F];
            }
            framebuffer[yy * fbW + xx] = px;
        }
    }
}

/* Draw viewport background from GRAPHICS.DAT graphic 0 (224x136).
 * Falls back to solid NAVY if asset is unavailable. */
static void m11_draw_viewport_background(const M11_GameViewState* state,
                                         unsigned char* framebuffer,
                                         int fbW,
                                         int fbH,
                                         int vpX,
                                         int vpY,
                                         int vpW,
                                         int vpH) {
    if (state->assetsAvailable) {
        const M11_AssetSlot* bgSlot = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader, M11_GFX_VIEWPORT_BG);
        if (bgSlot && bgSlot->width > 0 && bgSlot->height > 0) {
            M11_AssetLoader_BlitScaled(bgSlot, framebuffer, fbW, fbH,
                                       vpX, vpY, vpW, vpH, -1);
            return;
        }
    }
    /* Fallback: solid fills */
    m11_fill_rect(framebuffer, fbW, fbH,
                  vpX + 2, vpY + 2, vpW - 4, vpH / 2, M11_COLOR_NAVY);
    m11_fill_rect(framebuffer, fbW, fbH,
                  vpX + 2, vpY + vpH / 2, vpW - 4, vpH / 2 - 2,
                  M11_COLOR_DARK_GRAY);
}

/* Draw a real door frame from GRAPHICS.DAT at the given depth.
 * depthIndex 0 = nearest, 2 = farthest.
 * Falls back to the primitive line-based door rendering if unavailable. */
static int m11_draw_door_frame_asset(const M11_GameViewState* state,
                                     unsigned char* framebuffer,
                                     int fbW,
                                     int fbH,
                                     const M11_ViewRect* rect,
                                     int depthIndex) {
    unsigned int doorIdx;
    const M11_AssetSlot* slot;
    if (!state->assetsAvailable) return 0;
    /* Select the depth-appropriate door frame graphic */
    switch (depthIndex) {
        case 0: doorIdx = M11_GFX_DOOR_FRAME_D1; break;
        case 1: doorIdx = M11_GFX_DOOR_FRAME_D2; break;
        case 2: doorIdx = M11_GFX_DOOR_FRAME_D3; break;
        default: return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, doorIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;
    /* Center the door frame graphic in the face rect */
    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               rect->x + 2, rect->y + 2,
                               rect->w - 4, rect->h - 4, 0);
    return 1;
}

/* Draw door side pillars from GRAPHICS.DAT. */
static int m11_draw_door_side_asset(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH,
                                    int x,
                                    int y,
                                    int w,
                                    int h,
                                    int depthIndex) {
    unsigned int sideIdx;
    const M11_AssetSlot* slot;
    if (!state->assetsAvailable || w < 4 || h < 8) return 0;
    switch (depthIndex) {
        case 0: sideIdx = M11_GFX_DOOR_SIDE_D0; break;
        case 1: sideIdx = M11_GFX_DOOR_SIDE_D1; break;
        case 2: sideIdx = M11_GFX_DOOR_SIDE_D2; break;
        default: return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, sideIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;
    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               x, y, w, h, 0);
    return 1;
}

/* Map an item thing type + subtype to a GRAPHICS.DAT graphic index.
 * Returns the graphic index, or 0 if no mapping exists. */
static unsigned int m11_item_sprite_index(int thingType, int subtype) {
    unsigned int base;
    unsigned int maxSubtype;
    if (subtype < 0) subtype = 0;
    switch (thingType) {
        case THING_TYPE_WEAPON:
            base = M11_GFX_ITEM_WEAPON_BASE;
            maxSubtype = 45;
            break;
        case THING_TYPE_ARMOUR:
            base = M11_GFX_ITEM_ARMOUR_BASE;
            maxSubtype = 29;
            break;
        case THING_TYPE_SCROLL:
            return M11_GFX_ITEM_SCROLL_BASE;
        case THING_TYPE_POTION:
            base = M11_GFX_ITEM_POTION_BASE;
            maxSubtype = 16;
            break;
        case THING_TYPE_CONTAINER:
            base = M11_GFX_ITEM_CONTAINER_BASE;
            maxSubtype = 2;
            break;
        case THING_TYPE_JUNK:
            base = M11_GFX_ITEM_JUNK_BASE;
            maxSubtype = 51;
            break;
        default:
            return 0;
    }
    if ((unsigned int)subtype > maxSubtype) subtype = 0;
    return base + (unsigned int)subtype;
}

/* Draw an item sprite from GRAPHICS.DAT at the given viewport position.
 * Falls back to the primitive item cue if the asset is unavailable.
 * Returns 1 if a real sprite was drawn. */
static int m11_draw_item_sprite(const M11_GameViewState* state,
                                unsigned char* framebuffer,
                                int fbW,
                                int fbH,
                                int x,
                                int y,
                                int w,
                                int h,
                                int thingType,
                                int subtype,
                                int depthIndex) {
    unsigned int gfxIdx;
    const M11_AssetSlot* slot;
    int spriteW, spriteH;
    int drawW, drawH, drawX, drawY;

    if (!state || !state->assetsAvailable || thingType < 0) return 0;
    gfxIdx = m11_item_sprite_index(thingType, subtype);
    if (gfxIdx == 0 || gfxIdx >= M11_GFX_ITEM_SPRITE_END) return 0;

    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    spriteW = (int)slot->width;
    spriteH = (int)slot->height;

    /* Scale item sprite to fit within the floor area, keeping aspect ratio.
     * Reduce size at greater depths for perspective. */
    drawW = w / 2;
    drawH = (drawW * spriteH) / spriteW;
    if (drawH > h / 2) {
        drawH = h / 2;
        drawW = (drawH * spriteW) / spriteH;
    }
    /* Shrink at depth */
    if (depthIndex >= 1) {
        drawW = drawW * 2 / 3;
        drawH = drawH * 2 / 3;
    }
    if (depthIndex >= 2) {
        drawW = drawW * 2 / 3;
        drawH = drawH * 2 / 3;
    }
    if (drawW < 3 || drawH < 3) return 0;

    /* Position item on the floor area (bottom-center of the face rect) */
    drawX = x + (w - drawW) / 2;
    drawY = y + h - drawH - 2;

    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               drawX, drawY, drawW, drawH, 0);
    return 1;
}

/* Draw a wall ornament from GRAPHICS.DAT on the wall face.
 * Uses the map's wall set and the cell's ornament ordinal.
 * Returns 1 if a real ornament was drawn. */
static int m11_draw_wall_ornament(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW,
                                  int fbH,
                                  const M11_ViewRect* rect,
                                  int ornamentOrdinal,
                                  int depthIndex) {
    unsigned int gfxIdx;
    const M11_AssetSlot* slot;
    int ornW, ornH, drawX, drawY;
    int wallSet;

    if (!state || !state->assetsAvailable || ornamentOrdinal < 0) return 0;
    if (!state->world.dungeon) return 0;

    /* Get the current map's wall set */
    wallSet = 0;
    if (state->world.party.mapIndex >= 0 &&
        state->world.party.mapIndex < (int)state->world.dungeon->header.mapCount) {
        wallSet = (int)state->world.dungeon->maps[state->world.party.mapIndex].wallSet;
    }

    gfxIdx = (unsigned int)(M11_GFX_WALL_ORNAMENT_BASE +
              wallSet * M11_GFX_WALL_ORNAMENTS_PER_SET +
              ornamentOrdinal);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    /* Scale ornament to fit in the center of the wall face, at ~40% of face size */
    ornW = rect->w * 2 / 5;
    ornH = (ornW * (int)slot->height) / (int)slot->width;
    if (ornH > rect->h * 2 / 5) {
        ornH = rect->h * 2 / 5;
        ornW = (ornH * (int)slot->width) / (int)slot->height;
    }
    /* Shrink at depth */
    if (depthIndex >= 1) {
        ornW = ornW * 3 / 4;
        ornH = ornH * 3 / 4;
    }
    if (ornW < 3 || ornH < 3) return 0;

    drawX = rect->x + (rect->w - ornW) / 2;
    drawY = rect->y + (rect->h - ornH) / 2;

    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               drawX, drawY, ornW, ornH, 0);
    return 1;
}

/* Draw a door ornament from GRAPHICS.DAT on the door face.
 * Returns 1 if a real ornament was drawn. */
static int m11_draw_door_ornament(const M11_GameViewState* state,
                                  unsigned char* framebuffer,
                                  int fbW,
                                  int fbH,
                                  const M11_ViewRect* rect,
                                  int ornamentOrdinal,
                                  int depthIndex) {
    unsigned int gfxIdx;
    const M11_AssetSlot* slot;
    int ornW, ornH, drawX, drawY;
    int doorSet;

    if (!state || !state->assetsAvailable || ornamentOrdinal < 0) return 0;
    if (!state->world.dungeon) return 0;

    /* Get the current map's door set (use doorSet0 by default) */
    doorSet = 0;
    if (state->world.party.mapIndex >= 0 &&
        state->world.party.mapIndex < (int)state->world.dungeon->header.mapCount) {
        doorSet = (int)state->world.dungeon->maps[state->world.party.mapIndex].doorSet0;
    }

    gfxIdx = (unsigned int)(M11_GFX_DOOR_ORNAMENT_BASE +
              doorSet * M11_GFX_DOOR_ORNAMENTS_PER_SET +
              ornamentOrdinal);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    ornW = rect->w / 3;
    ornH = (ornW * (int)slot->height) / (int)slot->width;
    if (ornH > rect->h / 3) {
        ornH = rect->h / 3;
        ornW = (ornH * (int)slot->width) / (int)slot->height;
    }
    if (depthIndex >= 1) {
        ornW = ornW * 3 / 4;
        ornH = ornH * 3 / 4;
    }
    if (ornW < 3 || ornH < 3) return 0;

    drawX = rect->x + (rect->w - ornW) / 2;
    drawY = rect->y + (rect->h - ornH) / 2;

    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               drawX, drawY, ornW, ornH, 0);
    return 1;
}

/* Get the current map's wall set index for wall texture selection.
 * Returns 0 (default set) if dungeon data is unavailable. */
static int m11_current_map_wall_set(const M11_GameViewState* state) {
    if (!state || !state->world.dungeon ||
        state->world.party.mapIndex < 0 ||
        state->world.party.mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    return (int)state->world.dungeon->maps[state->world.party.mapIndex].wallSet;
}

/* Get the current map's floor set index for floor texture selection. */
static int m11_current_map_floor_set(const M11_GameViewState* state) {
    if (!state || !state->world.dungeon ||
        state->world.party.mapIndex < 0 ||
        state->world.party.mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    return (int)state->world.dungeon->maps[state->world.party.mapIndex].floorSet;
}

/* Draw stair graphics from GRAPHICS.DAT at the given depth. */
static int m11_draw_stairs_asset(const M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 int fbW,
                                 int fbH,
                                 const M11_ViewRect* rect,
                                 int depthIndex,
                                 int stairUp) {
    unsigned int stairIdx;
    const M11_AssetSlot* slot;
    if (!state->assetsAvailable) return 0;
    if (depthIndex == 0 || depthIndex == 1) {
        stairIdx = stairUp ? M11_GFX_STAIRS_UP_D1 : M11_GFX_STAIRS_DOWN_D1;
    } else {
        stairIdx = stairUp ? M11_GFX_STAIRS_UP_D2 : M11_GFX_STAIRS_DOWN_D2;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, stairIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;
    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               rect->x + 2, rect->y + 2,
                               rect->w - 4, rect->h - 4, 0);
    return 1;
}

/* Map a creature type index (0-26) to a GRAPHICS.DAT sprite set.
 * Returns the base index for the 3-sprite set (small/mid/large),
 * or 0 if no sprite is available for this type.
 * The mapping covers 8 creature graphic sets: 4 at base 246, 4 at base 439.
 * Creature types are mapped round-robin to the available sets. */
static unsigned int m11_creature_sprite_base(int creatureType) {
    /* CSB has 8 creature graphic sets.
     * Sets 0-3: indices 246, 249, 252, 255 (each set = 3 consecutive indices)
     * Sets 4-7: indices 439, 443, 448, 451 (irregular spacing) */
    static const unsigned int s_set_bases[8] = {
        246, 249, 252, 255, 439, 443, 448, 451
    };
    int setIdx;
    if (creatureType < 0 || creatureType > 26) return 0;
    setIdx = creatureType % 8;
    return s_set_bases[setIdx];
}

/* Draw a creature sprite from GRAPHICS.DAT at the given viewport depth.
 * depthIndex 0 = near (large sprite), 1 = mid, 2 = far (small sprite).
 * Returns 1 if a real sprite was drawn, 0 if fallback needed. */
static int m11_draw_creature_sprite(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH,
                                    int x,
                                    int y,
                                    int w,
                                    int h,
                                    int creatureType,
                                    int depthIndex) {
    unsigned int base;
    unsigned int spriteIdx;
    const M11_AssetSlot* slot;
    int spriteW, spriteH;
    int drawW, drawH, drawX, drawY;

    if (!state->assetsAvailable || creatureType < 0) return 0;
    base = m11_creature_sprite_base(creatureType);
    if (base == 0) return 0;

    /* Depth 0 = large (offset +2), depth 1 = mid (+1), depth 2 = small (+0) */
    switch (depthIndex) {
        case 0: spriteIdx = base + 2; break; /* 96x88 */
        case 1: spriteIdx = base + 1; break; /* 64x61 */
        default: spriteIdx = base;    break; /* 44x38 */
    }

    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, spriteIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    spriteW = (int)slot->width;
    spriteH = (int)slot->height;

    /* Scale to fit within the face rect while preserving aspect ratio */
    drawW = w - 4;
    drawH = (drawW * spriteH) / spriteW;
    if (drawH > h - 4) {
        drawH = h - 4;
        drawW = (drawH * spriteW) / spriteH;
    }
    if (drawW < 4 || drawH < 4) return 0;

    drawX = x + (w - drawW) / 2;
    drawY = y + (h - drawH) / 2;

    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               drawX, drawY, drawW, drawH, 0);
    return 1;
}

/* Draw the outer UI frame border using ceiling/floor panels from
 * GRAPHICS.DAT. Returns 1 if real assets were used. */
static int m11_draw_ui_frame_assets(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int fbW,
                                    int fbH) {
    const M11_AssetSlot* ceilSlot;
    const M11_AssetSlot* floorSlot;
    if (!state->assetsAvailable) return 0;

    ceilSlot = M11_AssetLoader_Load(
        (M11_AssetLoader*)&state->assetLoader, M11_GFX_CEILING_PANEL);
    floorSlot = M11_AssetLoader_Load(
        (M11_AssetLoader*)&state->assetLoader, M11_GFX_FLOOR_PANEL);

    if (ceilSlot && ceilSlot->width > 0 && ceilSlot->height > 0) {
        /* Blit the ceiling panel at the top of the viewport area */
        M11_AssetLoader_BlitScaled(ceilSlot, framebuffer, fbW, fbH,
                                   12, 24, 196, 14, -1);
    }
    if (floorSlot && floorSlot->width > 0 && floorSlot->height > 0) {
        /* Blit the floor panel below the viewport */
        M11_AssetLoader_BlitScaled(floorSlot, framebuffer, fbW, fbH,
                                   12, 142, 196, 6, -1);
    }
    return (ceilSlot != NULL || floorSlot != NULL) ? 1 : 0;
}

static void m11_draw_corridor_frame(unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    const M11_ViewRect* outer,
                                    const M11_ViewRect* inner,
                                    int depthIndex) {
    unsigned char wallShade = depthIndex == 0 ? M11_COLOR_DARK_GRAY : M11_COLOR_BLACK;
    unsigned char edge = depthIndex == 0 ? M11_COLOR_LIGHT_GRAY : M11_COLOR_DARK_GRAY;
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  outer->x, outer->y, outer->w, inner->y - outer->y, wallShade);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  outer->x, inner->y + inner->h, outer->w,
                  (outer->y + outer->h) - (inner->y + inner->h), M11_COLOR_BROWN);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  outer->x, inner->y, inner->x - outer->x, inner->h, wallShade);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  inner->x + inner->w, inner->y,
                  (outer->x + outer->w) - (inner->x + inner->w), inner->h, wallShade);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  outer->x, outer->y, outer->w, outer->h, edge);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  inner->x, inner->y, inner->w, inner->h, M11_COLOR_LIGHT_GRAY);
}

static void m11_draw_side_feature(unsigned char* framebuffer,
                                  int framebufferWidth,
                                  int framebufferHeight,
                                  const M11_ViewRect* outer,
                                  const M11_ViewRect* inner,
                                  const M11_ViewportCell* cell,
                                  int side,
                                  int depthIndex) {
    int paneX;
    int paneW;
    int paneY = inner->y + 3;
    int paneH = inner->h - 6;
    unsigned char accent;

    if (!cell || !cell->valid || paneH <= 4) {
        return;
    }

    if (side < 0) {
        paneX = outer->x + 4;
        paneW = (inner->x - outer->x) - 8;
    } else {
        paneX = inner->x + inner->w + 4;
        paneW = (outer->x + outer->w) - paneX - 4;
    }
    if (paneW <= 4) {
        return;
    }

    accent = m11_feature_accent_color(cell);
    if (m11_viewport_cell_is_open(cell)) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH, M11_COLOR_BLACK);
        if (cell->elementType == DUNGEON_ELEMENT_TELEPORTER) {
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + 1, paneY + 1, paneW - 2, paneH - 2, M11_COLOR_LIGHT_CYAN);
        } else if (cell->elementType == DUNGEON_ELEMENT_PIT) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + 1, paneY + paneH / 2, paneW - 2, paneH / 3, M11_COLOR_BROWN);
        }
    } else {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH,
                      depthIndex == 0 ? M11_COLOR_DARK_GRAY : M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH, accent);
    }

    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        /* Try real door side pillar graphic first */
        if (!g_drawState ||
            !m11_draw_door_side_asset(g_drawState, framebuffer,
                                      framebufferWidth, framebufferHeight,
                                      paneX, paneY, paneW, paneH, depthIndex)) {
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                           paneX + paneW / 2, paneY + 2, paneY + paneH - 3, M11_COLOR_YELLOW);
        }
        /* Re-draw the accent border on top so the door element
         * accent colour stays visible in probe invariants. */
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH, accent);
    }

    if (m11_viewport_cell_is_open(cell)) {
        if (cell->summary.groups > 0) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + paneW / 2 - 1, paneY + paneH / 2 - 2,
                          3, 5, depthIndex == 0 ? M11_COLOR_LIGHT_GREEN : M11_COLOR_GREEN);
        }
        if (cell->summary.items > 0) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + paneW / 2 - 2, paneY + paneH - 4,
                          5, 2, M11_COLOR_YELLOW);
        }
        if (cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + paneW / 2, paneY + paneH / 2, M11_COLOR_LIGHT_CYAN);
        }
    }
}

static unsigned char m11_focus_color(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return M11_COLOR_DARK_GRAY;
    }
    if (cell->summary.groups > 0 || cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
        return M11_COLOR_LIGHT_RED;
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR && !m11_viewport_cell_is_open(cell)) {
        return M11_COLOR_YELLOW;
    }
    if (cell->summary.items > 0 || cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
        return M11_COLOR_LIGHT_CYAN;
    }
    if (cell->elementType == DUNGEON_ELEMENT_PIT ||
        cell->elementType == DUNGEON_ELEMENT_TELEPORTER ||
        cell->elementType == DUNGEON_ELEMENT_STAIRS) {
        return M11_COLOR_MAGENTA;
    }
    return M11_COLOR_LIGHT_GREEN;
}

static void m11_format_lane_label(const M11_ViewportCell* cell,
                                  const char* prefix,
                                  char* out,
                                  size_t outSize) {
    if (!out || outSize == 0U) {
        return;
    }
    if (!cell || !cell->valid) {
        snprintf(out, outSize, "%s VOID", prefix ? prefix : "?");
        return;
    }
    if (cell->summary.groups > 0) {
        snprintf(out, outSize, "%s %dG", prefix ? prefix : "?", cell->summary.groups);
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        snprintf(out, outSize, "%s DOOR", prefix ? prefix : "?");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_PIT) {
        snprintf(out, outSize, "%s PIT", prefix ? prefix : "?");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_STAIRS) {
        snprintf(out, outSize, "%s STEP", prefix ? prefix : "?");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_TELEPORTER) {
        snprintf(out, outSize, "%s TELE", prefix ? prefix : "?");
        return;
    }
    if (cell->summary.items > 0) {
        snprintf(out, outSize, "%s ITEM", prefix ? prefix : "?");
        return;
    }
    if (cell->summary.sensors > 0 || cell->summary.textStrings > 0 ||
        cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
        snprintf(out, outSize, "%s MARK", prefix ? prefix : "?");
        return;
    }
    if (m11_viewport_cell_is_open(cell)) {
        snprintf(out, outSize, "%s OPEN", prefix ? prefix : "?");
        return;
    }
    snprintf(out, outSize, "%s WALL", prefix ? prefix : "?");
}

static void m11_draw_lane_chip(unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight,
                               int x,
                               int y,
                               int w,
                               int h,
                               const M11_ViewportCell* cell,
                               const char* prefix) {
    char label[16];
    unsigned char border = m11_focus_color(cell);
    unsigned char fill = (!cell || !cell->valid || !m11_viewport_cell_is_open(cell))
                             ? M11_COLOR_DARK_GRAY
                             : M11_COLOR_BLACK;

    m11_format_lane_label(cell, prefix, label, sizeof(label));
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x, y, w, h, fill);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x, y, w, h, border);
    if (cell && cell->valid) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      x + 2, y + 2, 4, h - 4, border);
    }
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  x + 9, y + 2, label, &g_text_small);
}

static void m11_format_depth_label(const M11_ViewportCell* cell,
                                   int depth,
                                   char* out,
                                   size_t outSize) {
    char lane[16];
    char depthPrefix[2];

    if (!out || outSize == 0U) {
        return;
    }
    depthPrefix[0] = (char)('1' + depth);
    depthPrefix[1] = '\0';
    m11_format_lane_label(cell, depthPrefix, lane, sizeof(lane));
    snprintf(out, outSize, "%s", lane);
}

static void m11_draw_depth_chip(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                int x,
                                int y,
                                int w,
                                int h,
                                const M11_ViewportCell* cell,
                                int depth) {
    char label[16];
    unsigned char border = m11_focus_color(cell);
    unsigned char fill = (!cell || !cell->valid || !m11_viewport_cell_is_open(cell))
                             ? M11_COLOR_DARK_GRAY
                             : M11_COLOR_BLACK;

    m11_format_depth_label(cell, depth, label, sizeof(label));
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x, y, w, h, fill);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x, y, w, h, border);
    if (cell && cell->valid) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      x + 2, y + 2, w - 4, 2, border);
    }
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  x + 4, y + 2, label, &g_text_small);
}

static void m11_draw_focus_brackets(unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    const M11_ViewRect* rect,
                                    const M11_ViewportCell* cell) {
    int x1;
    int x2;
    int y1;
    int y2;
    int arm = 10;
    unsigned char color;

    if (!rect || !cell || !cell->valid) {
        return;
    }

    x1 = rect->x + 8;
    y1 = rect->y + 7;
    x2 = rect->x + rect->w - 9;
    y2 = rect->y + rect->h - 8;
    if (x2 - x1 < 12 || y2 - y1 < 12) {
        return;
    }

    color = m11_focus_color(cell);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x1, x1 + arm, y1, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x1, y1, y1 + arm, color);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x2 - arm, x2, y1, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x2, y1, y1 + arm, color);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x1, x1 + arm, y2, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x1, y2 - arm, y2, color);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x2 - arm, x2, y2, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x2, y2 - arm, y2, color);

    m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                  rect->x + rect->w / 2, rect->y + rect->h / 2, color);
}

static int m11_tick_has_emission_kind(const struct TickResult_Compat* tick,
                                      uint8_t kind) {
    int i;
    if (!tick) {
        return 0;
    }
    for (i = 0; i < tick->emissionCount; ++i) {
        if (tick->emissions[i].kind == kind) {
            return 1;
        }
    }
    return 0;
}

static unsigned char m11_feedback_color(const M11_GameViewState* state,
                                        const M11_ViewportCell* aheadCell) {
    if (!state) {
        return M11_COLOR_LIGHT_CYAN;
    }
    if (m11_tick_has_emission_kind(&state->lastTickResult, EMIT_DAMAGE_DEALT)) {
        return M11_COLOR_LIGHT_RED;
    }
    if (m11_tick_has_emission_kind(&state->lastTickResult, EMIT_DOOR_STATE)) {
        return M11_COLOR_YELLOW;
    }
    if (m11_tick_has_emission_kind(&state->lastTickResult, EMIT_SOUND_REQUEST)) {
        return M11_COLOR_MAGENTA;
    }
    if (m11_tick_has_emission_kind(&state->lastTickResult, EMIT_PARTY_MOVED)) {
        if (state->lastAction[0] != '\0' && strstr(state->lastAction, "TURN") != NULL) {
            return M11_COLOR_YELLOW;
        }
        return M11_COLOR_LIGHT_CYAN;
    }
    if (state->lastOutcome[0] != '\0' && strstr(state->lastOutcome, "BLOCKED") != NULL) {
        return M11_COLOR_YELLOW;
    }
    return m11_focus_color(aheadCell);
}

static void m11_build_feedback_summary(const M11_GameViewState* state,
                                       char* out,
                                       size_t outSize) {
    const struct TickResult_Compat* tick;
    const struct TickEmission_Compat* emission;
    int i;
    int first = 1;

    if (!out || outSize == 0U) {
        return;
    }
    out[0] = '\0';
    if (!state) {
        return;
    }

    tick = &state->lastTickResult;
    for (i = 0; i < tick->emissionCount; ++i) {
        char chunk[32];
        emission = &tick->emissions[i];
        chunk[0] = '\0';
        switch (emission->kind) {
            case EMIT_DAMAGE_DEALT:
                snprintf(chunk, sizeof(chunk), "HIT %d", (int)emission->payload[2]);
                break;
            case EMIT_PARTY_MOVED:
                if (state->lastAction[0] != '\0' && strstr(state->lastAction, "TURN") != NULL) {
                    snprintf(chunk, sizeof(chunk), "TURN %s",
                             m11_direction_name(state->world.party.direction));
                } else {
                    snprintf(chunk, sizeof(chunk), "STEP %d:%d",
                             state->world.party.mapX,
                             state->world.party.mapY);
                }
                break;
            case EMIT_DOOR_STATE:
                snprintf(chunk, sizeof(chunk), "DOOR %d:%d",
                         (int)emission->payload[0],
                         (int)emission->payload[1]);
                break;
            case EMIT_SOUND_REQUEST:
                snprintf(chunk, sizeof(chunk), "FX %d", (int)emission->payload[0]);
                break;
            case EMIT_CHAMPION_DOWN:
                snprintf(chunk, sizeof(chunk), "DOWN %d", (int)emission->payload[0] + 1);
                break;
            case EMIT_KILL_NOTIFY:
                snprintf(chunk, sizeof(chunk), "KILL");
                break;
            default:
                break;
        }
        if (chunk[0] == '\0') {
            continue;
        }
        if (!first) {
            snprintf(out + strlen(out), outSize - strlen(out), " | ");
        }
        snprintf(out + strlen(out), outSize - strlen(out), "%s", chunk);
        first = 0;
    }

    if (out[0] == '\0') {
        if (state->lastOutcome[0] != '\0') {
            snprintf(out, outSize, "%s", state->lastOutcome);
        } else {
            snprintf(out, outSize, "READY");
        }
    }
}

static void m11_draw_feedback_strip(unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    const M11_GameViewState* state,
                                    const M11_ViewportCell* aheadCell) {
    char summary[96];
    unsigned char color;

    if (!state) {
        return;
    }

    color = m11_feedback_color(state, aheadCell);
    m11_build_feedback_summary(state, summary, sizeof(summary));

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  24, 130, 172, 10, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  24, 130, 172, 10, color);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  26, 132, 6, 6, color);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  36, 131, summary, &g_text_small);
}

static void m11_draw_viewport_feedback_frame(unsigned char* framebuffer,
                                             int framebufferWidth,
                                             int framebufferHeight,
                                             const M11_GameViewState* state,
                                             const M11_ViewportCell* aheadCell) {
    unsigned char color = m11_feedback_color(state, aheadCell);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  10, 22, 200, 122, color);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   82, 138, 145, color);
}

static void m11_draw_arrow_glyph(unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight,
                                 int x,
                                 int y,
                                 int direction,
                                 unsigned char color) {
    switch (direction) {
        case DIR_NORTH:
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x, x + 6, y + 1, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 1, x + 5, y + 2, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 2, x + 4, y + 3, color);
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x + 3, y + 3, y + 8, color);
            break;
        case DIR_SOUTH:
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x + 3, y, y + 5, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 2, x + 4, y + 5, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 1, x + 5, y + 6, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x, x + 6, y + 7, color);
            break;
        case DIR_EAST:
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x + 5, y, y + 6, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x, x + 5, y + 3, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + 2, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 4, y + 4, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 6, y + 3, color);
            break;
        case DIR_WEST:
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight, x + 1, y, y + 6, color);
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight, x + 1, x + 6, y + 3, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 2, y + 2, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x + 2, y + 4, color);
            m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight, x, y + 3, color);
            break;
        default:
            break;
    }
}

static void m11_draw_control_button(unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight,
                                    int x,
                                    int y,
                                    int w,
                                    int h,
                                    const char* label,
                                    int iconDirection,
                                    unsigned char border,
                                    unsigned char fill) {
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, fill);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight, x, y, w, h, border);
    if (iconDirection >= 0) {
        m11_draw_arrow_glyph(framebuffer, framebufferWidth, framebufferHeight,
                             x + 2, y + 2, iconDirection, border);
    }
    if (label && label[0] != '\0') {
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      x + 11, y + 2, label, &g_text_small);
    }
}

static void m11_draw_control_strip(unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const M11_ViewportCell* aheadCell) {
    unsigned char accent = m11_focus_color(aheadCell);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_CONTROL_STRIP_X, M11_CONTROL_STRIP_Y,
                  M11_CONTROL_STRIP_W, M11_CONTROL_STRIP_H, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_CONTROL_STRIP_X, M11_CONTROL_STRIP_Y,
                  M11_CONTROL_STRIP_W, M11_CONTROL_STRIP_H, accent);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            18, 167, 15, 10, "", DIR_WEST,
                            M11_COLOR_YELLOW, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            35, 167, 15, 10, "", DIR_NORTH,
                            M11_COLOR_LIGHT_CYAN, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            52, 167, 15, 10, "", DIR_EAST,
                            M11_COLOR_YELLOW, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            69, 167, 15, 10, "", DIR_SOUTH,
                            M11_COLOR_LIGHT_CYAN, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            86, 167, 12, 10, "", -1,
                            accent, M11_COLOR_BLACK);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  88, 169, "A", &g_text_small);
}

static const char* m11_focus_badge_label(const M11_ViewportCell* cell) {
    if (!cell || !cell->valid) {
        return "VOID";
    }
    if (cell->summary.groups > 0) {
        return "CONTACT";
    }
    if (cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
        return "DANGER";
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        return m11_viewport_cell_is_open(cell) ? "ENTRY" : "BARRIER";
    }
    if (cell->elementType == DUNGEON_ELEMENT_PIT ||
        cell->elementType == DUNGEON_ELEMENT_TELEPORTER ||
        cell->elementType == DUNGEON_ELEMENT_STAIRS) {
        return "HAZARD";
    }
    if (cell->summary.items > 0 || cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
        return "INTERACT";
    }
    if (m11_viewport_cell_is_open(cell)) {
        return "CLEAR";
    }
    return "STONE";
}

static void m11_draw_focus_card(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                const M11_GameViewState* state,
                                const M11_ViewportCell* aheadCell) {
    unsigned char accent = m11_focus_color(aheadCell);
    const char* badge = m11_focus_badge_label(aheadCell);

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  218, 106, 86, 34, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  218, 106, 86, 34, accent);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  222, 110, 26, 8, accent);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  224, 111, badge, &g_text_small);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  222, 121, state->inspectTitle, &g_text_small);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  222, 129, state->inspectDetail, &g_text_small);
}

static void m11_draw_utility_panel(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight,
                                   const struct DungeonMapDesc_Compat* mapDesc) {
    char line[32];
    char champion[16];
    const struct ChampionState_Compat* activeChampion = m11_get_active_champion(state);
    unsigned char accent = activeChampion ? M11_COLOR_LIGHT_GREEN : M11_COLOR_LIGHT_CYAN;

    if (!state) {
        return;
    }

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_UTILITY_PANEL_X, M11_UTILITY_PANEL_Y,
                  M11_UTILITY_PANEL_W, M11_UTILITY_PANEL_H, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_UTILITY_PANEL_X, M11_UTILITY_PANEL_Y,
                  M11_UTILITY_PANEL_W, M11_UTILITY_PANEL_H, M11_COLOR_LIGHT_CYAN);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  222, 34, "MENU", &g_text_small);

    m11_get_active_champion_label(state, champion, sizeof(champion));
    snprintf(line, sizeof(line), "%s %s", m11_source_name(state->sourceKind), champion);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  250, 34, line, &g_text_small);

    if (activeChampion) {
        snprintf(line, sizeof(line), "L%d HP%u ST%u",
                 mapDesc ? (int)mapDesc->level : 0,
                 (unsigned int)activeChampion->hp.current,
                 (unsigned int)activeChampion->stamina.current);
    } else {
        snprintf(line, sizeof(line), "%s", state->lastOutcome[0] != '\0' ? state->lastOutcome : "READY");
    }
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  222, 44, line, &g_text_small);

    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            M11_UTILITY_INSPECT_X, M11_UTILITY_BUTTON_Y,
                            M11_UTILITY_INSPECT_W, M11_UTILITY_BUTTON_H,
                            "I", -1,
                            accent, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            M11_UTILITY_SAVE_X, M11_UTILITY_BUTTON_Y,
                            M11_UTILITY_SAVE_W, M11_UTILITY_BUTTON_H,
                            "S", -1,
                            M11_COLOR_YELLOW, M11_COLOR_BLACK);
    m11_draw_control_button(framebuffer, framebufferWidth, framebufferHeight,
                            M11_UTILITY_LOAD_X, M11_UTILITY_BUTTON_Y,
                            M11_UTILITY_LOAD_W, M11_UTILITY_BUTTON_H,
                            "L", -1,
                            M11_COLOR_LIGHT_BLUE, M11_COLOR_BLACK);
}

static void m11_draw_viewport(const M11_GameViewState* state,
                              unsigned char* framebuffer,
                              int framebufferWidth,
                              int framebufferHeight) {
    static const M11_ViewRect viewport = {M11_VIEWPORT_X, M11_VIEWPORT_Y, M11_VIEWPORT_W, M11_VIEWPORT_H};
    static const M11_ViewRect frames[4] = {
        {20, 32, 180, 102},
        {40, 44, 140, 78},
        {58, 56, 104, 54},
        {74, 66, 72, 34}
    };
    M11_ViewportCell cells[3][3];
    int depth;
    int occluded = 0;

    memset(cells, 0, sizeof(cells));

    for (depth = 0; depth < 3; ++depth) {
        int side;
        for (side = 0; side < 3; ++side) {
            (void)m11_sample_viewport_cell(state, depth + 1, side - 1, &cells[depth][side]);
        }
    }

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  viewport.x, viewport.y, viewport.w, viewport.h, M11_COLOR_BLACK);
    /* Real viewport background from GRAPHICS.DAT, or solid fallback */
    m11_draw_viewport_background(state, framebuffer, framebufferWidth, framebufferHeight,
                                 viewport.x, viewport.y, viewport.w, viewport.h);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  viewport.x - 2, viewport.y - 2, viewport.w + 4, viewport.h + 4, M11_COLOR_YELLOW);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  viewport.x, viewport.y, viewport.w, viewport.h, M11_COLOR_LIGHT_CYAN);

    for (depth = 0; depth < 3; ++depth) {
        m11_draw_corridor_frame(framebuffer, framebufferWidth, framebufferHeight,
                                &frames[depth], &frames[depth + 1], depth);
    }

    for (depth = 0; depth < 3; ++depth) {
        m11_draw_side_feature(framebuffer, framebufferWidth, framebufferHeight,
                              &frames[depth], &frames[depth + 1], &cells[depth][0], -1, depth);
        m11_draw_side_feature(framebuffer, framebufferWidth, framebufferHeight,
                              &frames[depth], &frames[depth + 1], &cells[depth][2], 1, depth);
        if (!occluded) {
            m11_draw_wall_face(framebuffer, framebufferWidth, framebufferHeight,
                               &frames[depth + 1], &cells[depth][1], depth);
            if (!m11_viewport_cell_is_open(&cells[depth][1])) {
                occluded = 1;
            }
        }
    }

    occluded = 0;
    for (depth = 0; depth < 3; ++depth) {
        if (!occluded) {
            m11_draw_wall_contents(framebuffer, framebufferWidth, framebufferHeight,
                                   &frames[depth + 1], &cells[depth][1], depth);
            if (!m11_viewport_cell_is_open(&cells[depth][1])) {
                occluded = 1;
            }
        }
    }

    m11_draw_lane_chip(framebuffer, framebufferWidth, framebufferHeight,
                       viewport.x + 26, viewport.y + 6, 42, 11,
                       &cells[0][0], "L");
    m11_draw_lane_chip(framebuffer, framebufferWidth, framebufferHeight,
                       viewport.x + 77, viewport.y + 6, 42, 11,
                       &cells[0][1], "F");
    m11_draw_lane_chip(framebuffer, framebufferWidth, framebufferHeight,
                       viewport.x + 128, viewport.y + 6, 42, 11,
                       &cells[0][2], "R");

    m11_draw_depth_chip(framebuffer, framebufferWidth, framebufferHeight,
                        viewport.x + 40, viewport.y + viewport.h - 24, 34, 10,
                        &cells[0][1], 0);
    m11_draw_depth_chip(framebuffer, framebufferWidth, framebufferHeight,
                        viewport.x + 88, viewport.y + viewport.h - 24, 34, 10,
                        &cells[1][1], 1);
    m11_draw_depth_chip(framebuffer, framebufferWidth, framebufferHeight,
                        viewport.x + 136, viewport.y + viewport.h - 24, 34, 10,
                        &cells[2][1], 2);

    m11_draw_focus_brackets(framebuffer, framebufferWidth, framebufferHeight,
                            &frames[1], &cells[0][1]);
    m11_draw_viewport_feedback_frame(framebuffer, framebufferWidth, framebufferHeight,
                                     state, &cells[0][1]);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   viewport.x + 6, viewport.x + viewport.w - 7,
                   viewport.y + viewport.h / 2 + 8, M11_COLOR_LIGHT_GRAY);

    /* Overlay real GRAPHICS.DAT wall textures when available.
     * Tiles wall-set strips into the corridor frame regions using
     * scaled blits. Draws back-to-front so nearer strips occlude.
     * Uses the current map's wallSet for texture selection. */
    if (state->assetsAvailable) {
        int d;
        int mapWallSet = m11_current_map_wall_set(state);
        int mapFloorSet = m11_current_map_floor_set(state);
        for (d = 2; d >= 0; --d) {
            const M11_AssetSlot* wallSlot;
            unsigned int wallIdx = (unsigned int)(M11_GFX_WALL_SET_0 + (mapWallSet & 3));
            wallSlot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, wallIdx);
            if (wallSlot && wallSlot->width > 0 && wallSlot->height > 0) {
                int ceilH = frames[d + 1].y - frames[d].y;
                int floorY = frames[d + 1].y + frames[d + 1].h;
                int floorH = (frames[d].y + frames[d].h) - floorY;
                int leftW = frames[d + 1].x - frames[d].x;
                int rightX = frames[d + 1].x + frames[d + 1].w;
                int rightW = (frames[d].x + frames[d].w) - rightX;
                /* Ceiling strip */
                if (ceilH > 2) {
                    M11_AssetLoader_BlitScaled(wallSlot, framebuffer,
                        framebufferWidth, framebufferHeight,
                        frames[d].x + 1, frames[d].y + 1,
                        frames[d].w - 2, ceilH - 1, -1);
                }
                /* Left wall strip */
                if (leftW > 2) {
                    M11_AssetLoader_BlitScaled(wallSlot, framebuffer,
                        framebufferWidth, framebufferHeight,
                        frames[d].x + 1, frames[d + 1].y,
                        leftW - 1, frames[d + 1].h, -1);
                }
                /* Right wall strip */
                if (rightW > 2) {
                    M11_AssetLoader_BlitScaled(wallSlot, framebuffer,
                        framebufferWidth, framebufferHeight,
                        rightX + 1, frames[d + 1].y,
                        rightW - 1, frames[d + 1].h, -1);
                }
                /* Floor strip — use the current map's floor set */
                if (floorH > 2) {
                    const M11_AssetSlot* floorSlot = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader,
                        (unsigned int)(M11_GFX_FLOOR_SET_0 + (mapFloorSet & 1)));
                    if (floorSlot && floorSlot->width > 0 && floorSlot->height > 0) {
                        M11_AssetLoader_BlitScaled(floorSlot, framebuffer,
                            framebufferWidth, framebufferHeight,
                            frames[d].x + 1, floorY + 1,
                            frames[d].w - 2, floorH - 1, -1);
                    }
                }
            }
        }
    }

    /* Apply depth-based light dimming to the far corridor band only.
     * Depth 2 gets dimmed to simulate torch light falloff in the
     * dungeon.  Depth 1 is left bright so near-side accents (door
     * LIGHT_RED, stair YELLOW) remain clearly visible. */
    m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                            frames[2].x, frames[2].y,
                            frames[2].w, frames[2].h, 1);
}

static void m11_format_champion_name(const unsigned char* raw,
                                     char* out,
                                     size_t outSize) {
    size_t i;
    size_t end = 0;
    if (!out || outSize == 0U) {
        return;
    }
    if (!raw) {
        snprintf(out, outSize, "EMPTY");
        return;
    }
    for (i = 0; i + 1 < outSize && i < CHAMPION_NAME_LENGTH; ++i) {
        unsigned char ch = raw[i];
        if (ch == 0U) {
            break;
        }
        out[i] = isprint(ch) ? (char)ch : ' ';
        if (out[i] != ' ') {
            end = i + 1;
        }
    }
    out[i < outSize ? i : outSize - 1] = '\0';
    out[end] = '\0';
    if (out[0] == '\0') {
        snprintf(out, outSize, "EMPTY");
    }
}

static void m11_get_active_champion_label(const M11_GameViewState* state,
                                          char* out,
                                          size_t outSize) {
    if (!out || outSize == 0U) {
        return;
    }
    if (!state || !state->active ||
        state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY ||
        !state->world.party.champions[state->world.party.activeChampionIndex].present) {
        snprintf(out, outSize, "LEADER");
        return;
    }
    m11_format_champion_name(state->world.party.champions[state->world.party.activeChampionIndex].name,
                             out,
                             outSize);
}

static int m11_cycle_active_champion(M11_GameViewState* state) {
    int start;
    int step;
    char champion[16];

    if (!state || !state->active || state->world.party.championCount <= 0) {
        return 0;
    }

    start = state->world.party.activeChampionIndex;
    if (start < 0 || start >= CHAMPION_MAX_PARTY ||
        !state->world.party.champions[start].present) {
        start = 0;
    }

    for (step = 1; step <= CHAMPION_MAX_PARTY; ++step) {
        int candidate = (start + step) % CHAMPION_MAX_PARTY;
        if (candidate < state->world.party.championCount &&
            state->world.party.champions[candidate].present) {
            state->world.party.activeChampionIndex = candidate;
            m11_get_active_champion_label(state, champion, sizeof(champion));
            m11_set_status(state, "CHAMP", "ACTIVE CHAMPION READY");
            snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s READY", champion);
            snprintf(state->inspectDetail, sizeof(state->inspectDetail),
                     "SPACE ACTS, ENTER INSPECTS, CLICK CENTER TO COMMIT, TAB CYCLES THE FRONT CHAMPION");
            return 1;
        }
    }

    return 0;
}

static int m11_set_active_champion(M11_GameViewState* state, int championIndex) {
    char champion[16];

    if (!state || !state->active || championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) {
        return 0;
    }
    if (championIndex >= state->world.party.championCount ||
        !state->world.party.champions[championIndex].present) {
        return 0;
    }
    if (state->world.party.activeChampionIndex == championIndex) {
        return 1;
    }

    state->world.party.activeChampionIndex = championIndex;
    m11_get_active_champion_label(state, champion, sizeof(champion));
    m11_set_status(state, "CHAMP", "ACTIVE CHAMPION READY");
    snprintf(state->inspectTitle, sizeof(state->inspectTitle), "%s READY", champion);
    snprintf(state->inspectDetail, sizeof(state->inspectDetail),
             "CLICK OR TAB TO SWAP, CLICK CENTER TO COMMIT, SPACE ACTS, ENTER INSPECTS");
    return 1;
}

static void m11_draw_party_panel(const M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight) {
    int slot;
    int activeIndex = -1;
    if (state) {
        activeIndex = state->world.party.activeChampionIndex;
    }
    for (slot = 0; slot < CHAMPION_MAX_PARTY; ++slot) {
        int x = M11_PARTY_PANEL_X + slot * M11_PARTY_SLOT_STEP;
        int y = M11_PARTY_PANEL_Y;
        char line[48];
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      x, y, 71, 28, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      x, y, 71, 28,
                      slot < state->world.party.championCount ? M11_COLOR_LIGHT_CYAN : M11_COLOR_DARK_GRAY);
        if (slot < state->world.party.championCount && state->world.party.champions[slot].present) {
            char name[16];
            const struct ChampionState_Compat* champ = &state->world.party.champions[slot];
            m11_format_champion_name(champ->name, name, sizeof(name));
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 4, name, &g_text_small);
            if (slot == activeIndex) {
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 1, y + 1, 69, 26, M11_COLOR_YELLOW);
            }
            {
            int hpWidth = champ->hp.maximum > 0 ? (int)(champ->hp.current * 59) / (int)champ->hp.maximum : 0;
            int staminaWidth = champ->stamina.maximum > 0 ? (int)(champ->stamina.current * 59) / (int)champ->stamina.maximum : 0;
            int manaWidth = champ->mana.maximum > 0 ? (int)(champ->mana.current * 59) / (int)champ->mana.maximum : 0;
            int isDead = (champ->hp.current == 0);
            if (isDead) {
                snprintf(line, sizeof(line), "DEAD");
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 12, line, &g_text_small);
            } else {
                int itemCount = M11_GameView_CountChampionItems(state, slot);
                if (itemCount > 0) {
                    snprintf(line, sizeof(line), "HP%u I%d", (unsigned int)champ->hp.current, itemCount);
                } else {
                    snprintf(line, sizeof(line), "HP%u ST%u", (unsigned int)champ->hp.current, (unsigned int)champ->stamina.current);
                }
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 12, line, &g_text_small);
            }
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 20, 59, 2, M11_COLOR_DARK_GRAY);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 20, hpWidth, 2, isDead ? M11_COLOR_DARK_GRAY : M11_COLOR_LIGHT_RED);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 23, 59, 1, M11_COLOR_DARK_GRAY);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 23, staminaWidth, 1, M11_COLOR_LIGHT_GREEN);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 25, 59, 1, M11_COLOR_DARK_GRAY);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 25, manaWidth, 1, M11_COLOR_LIGHT_BLUE);
            }
        } else {
            snprintf(line, sizeof(line), "SLOT %d", slot + 1);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 6, line, &g_text_small);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 16, "EMPTY", &g_text_small);
        }
    }
}

static void m11_format_front_cell_prompt(const M11_GameViewState* state,
                                         const M11_ViewportCell* cell,
                                         char* outAction,
                                         size_t outActionSize,
                                         char* outHint,
                                         size_t outHintSize) {
    char champion[16];
    if (outAction && outActionSize > 0U) {
        outAction[0] = '\0';
    }
    if (outHint && outHintSize > 0U) {
        outHint[0] = '\0';
    }
    if (!cell || !cell->valid) {
        snprintf(outAction, outActionSize, "FOCUS VOID");
        snprintf(outHint, outHintSize, "TURN TO RE-ENTER THE MAP");
        return;
    }
    if (cell->summary.groups > 0) {
        m11_get_active_champion_label(state, champion, sizeof(champion));
        snprintf(outAction, outActionSize, "ATTACK WITH %s", champion);
        snprintf(outHint, outHintSize, "SPACE OR CLICK CENTER STRIKES, A/D OR LOWER CORNERS STRAFE, TAB SWAPS CHAMPION");
        return;
    }
    if (cell->summary.projectiles > 0 || cell->summary.explosions > 0) {
        snprintf(outAction, outActionSize, "FOCUS ACTIVE EFFECT");
        snprintf(outHint, outHintSize, "SPACE WAITS, ENTER INSPECTS, TURN IF THE CELL STAYS HOT");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_DOOR) {
        if (m11_viewport_cell_is_open(cell)) {
            snprintf(outAction, outActionSize, "FOCUS OPEN DOOR");
            snprintf(outHint, outHintSize, "UP OR CLICK CENTER CROSSES, A/D STRAFES, ENTER INSPECTS, SPACE CLOSES THE DOOR");
        } else {
            snprintf(outAction, outActionSize, "FOCUS CLOSED DOOR");
            snprintf(outHint, outHintSize, "SPACE OPENS THE DOOR, ENTER INSPECTS, A/D STRAFES, TURN TO SEARCH");
        }
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_PIT) {
        snprintf(outAction, outActionSize, "FOCUS PIT");
        snprintf(outHint, outHintSize, "UP RISKS A DROP, ENTER INSPECTS BEFORE COMMITTING");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_STAIRS) {
        snprintf(outAction, outActionSize, "FOCUS STAIRS");
        snprintf(outHint, outHintSize, "UP CLIMBS, ENTER INSPECTS, SPACE WAITS");
        return;
    }
    if (cell->elementType == DUNGEON_ELEMENT_TELEPORTER) {
        snprintf(outAction, outActionSize, "FOCUS TELEPORTER");
        snprintf(outHint, outHintSize, "UP TESTS THE RIFT, ENTER INSPECTS FIRST");
        return;
    }
    if (cell->summary.items > 0 || cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
        snprintf(outAction, outActionSize, "FOCUS INTERACTABLE");
        snprintf(outHint, outHintSize, "ENTER INSPECTS, G GRABS FLOOR ITEMS, A/D STRAFES, SPACE WAITS");
        return;
    }
    if (m11_viewport_cell_is_open(cell)) {
        snprintf(outAction, outActionSize, "FOCUS CLEAR PASSAGE");
        snprintf(outHint, outHintSize, "UP ADVANCES, A/D OR LOWER CORNERS STRAFE, ENTER INSPECTS, TAB SWAPS CHAMPION");
        return;
    }
    snprintf(outAction, outActionSize, "FOCUS BLOCKED");
    snprintf(outHint, outHintSize, "ENTER INSPECTS, SPACE WAITS, TURN TO SEARCH");
}

static void m11_draw_map_panel(const M11_GameViewState* state,
                               unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight) {
    static const int radius = 2;
    static const int cell = 11;
    int cx;
    int cy;
    int gx;
    int gy;
    int baseX = 228;
    int baseY = 78;
    if (!state || !state->active) {
        return;
    }
    for (gy = -radius; gy <= radius; ++gy) {
        for (gx = -radius; gx <= radius; ++gx) {
            unsigned char square = 0;
            int drawX = baseX + (gx + radius) * cell;
            int drawY = baseY + (gy + radius) * cell;
            unsigned char fill = M11_COLOR_BLACK;
            int ok = m11_get_square_byte(&state->world,
                                         state->world.party.mapIndex,
                                         state->world.party.mapX + gx,
                                         state->world.party.mapY + gy,
                                         &square);
            if (ok) {
                fill = m11_tile_color(square >> 5);
                /* Dim unexplored cells */
                if (!m11_is_explored(state,
                                     state->world.party.mapX + gx,
                                     state->world.party.mapY + gy)) {
                    fill = M11_COLOR_NAVY;
                }
            }
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          drawX, drawY, cell - 1, cell - 1, fill);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          drawX, drawY, cell - 1, cell - 1, M11_COLOR_BLACK);
            if (gx == 0 && gy == 0) {
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              drawX + 2, drawY + 2, cell - 5, cell - 5, M11_COLOR_LIGHT_GREEN);
                m11_draw_party_arrow(framebuffer, framebufferWidth, framebufferHeight,
                                     drawX + 2, drawY + 2, cell - 5,
                                     state->world.party.direction, M11_COLOR_BLACK);
            }
        }
    }
    cx = baseX + radius * cell;
    cy = baseY + radius * cell;
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  cx - cell, cy - cell, cell * 3 - 1, cell * 3 - 1, M11_COLOR_YELLOW);
}

void M11_GameView_Draw(const M11_GameViewState* state,
                       unsigned char* framebuffer,
                       int framebufferWidth,
                       int framebufferHeight) {
    char line[96];
    char line2[96];
    char line3[96];
    char focusAction[96];
    char focusHint[96];
    unsigned short firstThing = THING_ENDOFLIST;
    int squareThingCount = 0;
    M11_ViewportCell currentCell;
    M11_ViewportCell aheadCell;
    const struct DungeonMapDesc_Compat* mapDesc = NULL;
    int avgFood = 0;
    int avgWater = 0;
    char champion[24];
    if (!framebuffer || framebufferWidth <= 0 || framebufferHeight <= 0) {
        return;
    }
    /* Set file-scope draw state for asset-backed rendering helpers */
    g_drawState = state;
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  0, 0, framebufferWidth, framebufferHeight, M11_COLOR_NAVY);
    if (!state || !state->active) {
        g_drawState = NULL;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      18, 18, "NO GAME VIEW", &g_text_title);
        return;
    }
    firstThing = m11_get_first_square_thing(&state->world,
                                            state->world.party.mapIndex,
                                            state->world.party.mapX,
                                            state->world.party.mapY);
    squareThingCount = m11_count_square_things(&state->world,
                                               state->world.party.mapIndex,
                                               state->world.party.mapX,
                                               state->world.party.mapY);
    memset(&currentCell, 0, sizeof(currentCell));
    memset(&aheadCell, 0, sizeof(aheadCell));
    (void)m11_sample_viewport_cell(state, 0, 0, &currentCell);
    (void)m11_sample_viewport_cell(state, 1, 0, &aheadCell);
    if (state->world.dungeon && state->world.party.mapIndex >= 0 &&
        state->world.party.mapIndex < (int)state->world.dungeon->header.mapCount) {
        mapDesc = &state->world.dungeon->maps[state->world.party.mapIndex];
    }
    m11_get_food_water_average(&state->world.party, &avgFood, &avgWater);
    m11_get_active_champion_label(state, champion, sizeof(champion));
    m11_format_front_cell_prompt(state,
                                 &aheadCell,
                                 focusAction,
                                 sizeof(focusAction),
                                 focusHint,
                                 sizeof(focusHint));

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  8, 8, framebufferWidth - 16, framebufferHeight - 16, M11_COLOR_DARK_GRAY);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  8, 8, framebufferWidth - 16, framebufferHeight - 16, M11_COLOR_YELLOW);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  12, 12, framebufferWidth - 24, 12, M11_COLOR_BLACK);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  12, 24, 196, 120, M11_COLOR_BLACK);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  214, 24, 94, 120, M11_COLOR_BLACK);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  12, 146, 296, 46, M11_COLOR_BLACK);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  18, 13, state->title[0] != '\0' ? state->title : "GAME VIEW", &g_text_shadow);

    snprintf(line, sizeof(line), "L%d %s %s%s",
             state->world.party.mapIndex + 1,
             m11_direction_name(state->world.party.direction),
             state->lastAction,
             state->resting ? " ZZZ" : "");
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  140, 13, line, &g_text_small);

    snprintf(line, sizeof(line), "G GRAB P DROP R REST 1-6 RUNE C CAST");
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  240, 13, line, &g_text_small);

    m11_draw_viewport(state, framebuffer, framebufferWidth, framebufferHeight);

    /* Overlay UI frame border elements from GRAPHICS.DAT */
    m11_draw_ui_frame_assets(state, framebuffer, framebufferWidth, framebufferHeight);

    m11_draw_utility_panel(state, framebuffer, framebufferWidth, framebufferHeight, mapDesc);

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  218, 74, 86, 68, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  218, 74, 86, 68, M11_COLOR_LIGHT_BLUE);
    m11_draw_map_panel(state, framebuffer, framebufferWidth, framebufferHeight);

    m11_format_square_summary(&currentCell, line, sizeof(line));
    m11_format_square_things(firstThing, squareThingCount, line3, sizeof(line3));
    snprintf(line2, sizeof(line2), "HERE  %s", line);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 16, 149, line2, &g_text_small);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 122, 149, line3, &g_text_small);
    snprintf(line, sizeof(line), "TICK %u", (unsigned int)state->world.gameTick);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 240, 149, line, &g_text_small);

    m11_format_square_summary(&aheadCell, line, sizeof(line));
    snprintf(line2, sizeof(line2), "AHEAD %s", line);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 16, 157, line2, &g_text_small);
    snprintf(line, sizeof(line), "%s F%03d W%03d",
             champion,
             avgFood,
             avgWater);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 154, 157, line, &g_text_small);

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_PROMPT_STRIP_X, M11_PROMPT_STRIP_Y,
                  M11_PROMPT_STRIP_W, M11_PROMPT_STRIP_H, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  M11_PROMPT_STRIP_X, M11_PROMPT_STRIP_Y,
                  M11_PROMPT_STRIP_W, M11_PROMPT_STRIP_H, m11_focus_color(&aheadCell));
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  108, 168, focusAction, &g_text_small);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  197, 168, focusHint, &g_text_small);

    m11_draw_control_strip(framebuffer, framebufferWidth, framebufferHeight, &aheadCell);
    m11_draw_focus_card(framebuffer, framebufferWidth, framebufferHeight, state, &aheadCell);

    m11_draw_party_panel(state, framebuffer, framebufferWidth, framebufferHeight);
    m11_draw_feedback_strip(framebuffer, framebufferWidth, framebufferHeight,
                            state, &aheadCell);

    /* Message log overlay — draw the last few events in the sidebar */
    {
        int logI;
        int logY = 108;
        int logCount = state->messageLog.count;
        int logMax = 4;
        if (logCount > logMax) {
            logCount = logMax;
        }
        for (logI = 0; logI < logCount; ++logI) {
            const M11_LogEntry* entry = m11_log_entry_at(&state->messageLog, logI);
            if (entry && entry->text[0] != '\0') {
                M11_TextStyle logStyle = g_text_small;
                logStyle.color = entry->color;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              222, logY + logI * 8, entry->text, &logStyle);
            }
        }
    }

    /* Spell panel overlay */
    if (state->spellPanelOpen) {
        int spI;
        int spX = 40;
        int spY = 52;
        int row = state->spellRuneRow < 4 ? state->spellRuneRow : 3;
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      spX - 4, spY - 4, 240, 82, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      spX - 4, spY - 4, 240, 82, M11_COLOR_LIGHT_BLUE);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      spX, spY, "CAST SPELL (1-6 RUNE, C CAST, V CLEAR)", &g_text_small);
        /* Show current rune buffer */
        {
            char buf[64];
            int bI;
            buf[0] = '\0';
            for (bI = 0; bI < state->spellBuffer.runeCount; ++bI) {
                int rv = state->spellBuffer.runes[bI];
                int rr = (rv - 0x60) / 6;
                int rc = (rv - 0x60) % 6;
                if (rr >= 0 && rr < 4 && rc >= 0 && rc < 6) {
                    if (bI > 0) strncat(buf, " ", sizeof(buf) - strlen(buf) - 1);
                    strncat(buf, g_rune_names[rr][rc], sizeof(buf) - strlen(buf) - 1);
                }
            }
            if (buf[0] != '\0') {
                M11_TextStyle spStyle = g_text_small;
                spStyle.color = M11_COLOR_GREEN;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              spX, spY + 10, buf, &spStyle);
            }
        }
        /* Show the active rune row */
        if (state->spellBuffer.runeCount < 4) {
            char rowLabel[64];
            const char* rowNames[4] = { "POWER", "ELEMENT", "FORM", "CLASS" };
            snprintf(rowLabel, sizeof(rowLabel), "ROW %d: %s", row + 1, rowNames[row]);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          spX, spY + 22, rowLabel, &g_text_small);
            for (spI = 0; spI < 6; ++spI) {
                char label[12];
                int bx = spX + spI * 38;
                int by = spY + 32;
                snprintf(label, sizeof(label), "%d:%s", spI + 1, g_rune_names[row][spI]);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              bx, by, 36, 14, M11_COLOR_DARK_GRAY);
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              bx, by, 36, 14, M11_COLOR_WHITE);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              bx + 2, by + 3, label, &g_text_small);
            }
        } else {
            M11_TextStyle readyStyle = g_text_small;
            readyStyle.color = M11_COLOR_GREEN;
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          spX, spY + 22, "READY TO CAST (C) OR CLEAR (V)", &readyStyle);
        }
        /* Show mana of active champion */
        if (state->world.party.activeChampionIndex >= 0 &&
            state->world.party.activeChampionIndex < CHAMPION_MAX_PARTY) {
            const struct ChampionState_Compat* sc =
                &state->world.party.champions[state->world.party.activeChampionIndex];
            char manaLine[48];
            snprintf(manaLine, sizeof(manaLine), "MANA: %d/%d",
                     (int)sc->mana.current, (int)sc->mana.maximum);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          spX, spY + 58, manaLine, &g_text_small);
        }
    }

    /* Rest / death overlay */
    if (state->partyDead) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      60, 70, 200, 60, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      60, 70, 200, 60, M11_COLOR_LIGHT_RED);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      80, 80, "GAME OVER", &g_text_title);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      72, 108, "LOAD SAVE OR ESC TO MENU", &g_text_shadow);
    } else if (state->resting) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      100, 70, 120, 30, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      100, 70, 120, 30, M11_COLOR_LIGHT_BLUE);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      112, 78, "RESTING...", &g_text_shadow);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      112, 88, "R TO WAKE", &g_text_small);
    }
    g_drawState = NULL;
}
