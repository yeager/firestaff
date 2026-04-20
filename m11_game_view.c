#include "m11_game_view.h"

#include "asset_status_m12.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <ctype.h>
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
    M11_PARTY_SLOT_STEP = 77
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
static int m11_get_front_cell(const M11_GameViewState* state, struct M11_ViewportCell* outCell);
static M12_MenuInput m11_pointer_viewport_input(const M11_GameViewState* state,
                                                int x,
                                                int y);
static void m11_get_active_champion_label(const M11_GameViewState* state,
                                          char* out,
                                          size_t outSize);
static int m11_cycle_active_champion(M11_GameViewState* state);
static int m11_set_active_champion(M11_GameViewState* state, int championIndex);

static int m11_point_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
    return x >= rx && y >= ry && x < (rx + rw) && y < (ry + rh);
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
    m11_refresh_hash(state);
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
            command = CMD_NONE;
            label = "WAIT";
            break;
        case M12_MENU_INPUT_CYCLE_CHAMPION:
            if (m11_cycle_active_champion(state)) {
                return M11_GAME_INPUT_REDRAW;
            }
            return M11_GAME_INPUT_IGNORED;
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
        return M12_MENU_INPUT_LEFT;
    }
    if (localX >= (M11_VIEWPORT_W * 2) / 3) {
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
        m11_draw_creature_cue(framebuffer, framebufferWidth, framebufferHeight,
                              faceX + 4, faceY + 5, faceW - 8, faceH - 10, depthIndex);
    }
    if (cell->summary.items > 0) {
        m11_draw_item_cue(framebuffer, framebufferWidth, framebufferHeight,
                          faceX + 2, faceY + 2, faceW - 4, faceH - 4,
                          cell->summary.items);
    }
    m11_draw_effect_cue(framebuffer, framebufferWidth, framebufferHeight,
                        faceX + 3, faceY + 3, faceW - 6, faceH - 6, cell);
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
        m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                       paneX + paneW / 2, paneY + 2, paneY + paneH - 3, M11_COLOR_YELLOW);
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
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  viewport.x + 2, viewport.y + 2, viewport.w - 4, viewport.h / 2, M11_COLOR_NAVY);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  viewport.x + 2, viewport.y + viewport.h / 2, viewport.w - 4, viewport.h / 2 - 2,
                  M11_COLOR_DARK_GRAY);
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
            int hpWidth;
            int staminaWidth;
            m11_format_champion_name(champ->name, name, sizeof(name));
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 4, name, &g_text_small);
            if (slot == activeIndex) {
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 1, y + 1, 69, 26, M11_COLOR_YELLOW);
            }
            hpWidth = champ->hp.maximum > 0 ? (champ->hp.current * 59) / champ->hp.maximum : 0;
            staminaWidth = champ->stamina.maximum > 0 ? (champ->stamina.current * 59) / champ->stamina.maximum : 0;
            snprintf(line, sizeof(line), "HP %u", (unsigned int)champ->hp.current);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 12, line, &g_text_small);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 20, 59, 2, M11_COLOR_DARK_GRAY);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 20, hpWidth, 2, M11_COLOR_LIGHT_RED);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 24, 59, 2, M11_COLOR_DARK_GRAY);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x + 4, y + 24, staminaWidth, 2, M11_COLOR_LIGHT_GREEN);
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
        snprintf(outHint, outHintSize, "SPACE OR CLICK CENTER STRIKES, ENTER INSPECTS, TAB SWAPS CHAMPION");
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
            snprintf(outHint, outHintSize, "UP OR CLICK CENTER CROSSES, ENTER INSPECTS, SPACE WAITS");
        } else {
            snprintf(outAction, outActionSize, "FOCUS CLOSED DOOR");
            snprintf(outHint, outHintSize, "ENTER INSPECTS, SPACE WAITS, TURN TO SEARCH");
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
        snprintf(outHint, outHintSize, "ENTER INSPECTS, UP OR CLICK CENTER CLOSES DISTANCE, SPACE WAITS");
        return;
    }
    if (m11_viewport_cell_is_open(cell)) {
        snprintf(outAction, outActionSize, "FOCUS CLEAR PASSAGE");
        snprintf(outHint, outHintSize, "UP OR CLICK CENTER ADVANCES, ENTER INSPECTS, TAB SWAPS CHAMPION");
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
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  0, 0, framebufferWidth, framebufferHeight, M11_COLOR_NAVY);
    if (!state || !state->active) {
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

    snprintf(line, sizeof(line), "%s %s", m11_direction_name(state->world.party.direction), state->lastAction);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  150, 13, line, &g_text_small);

    snprintf(line, sizeof(line), "ENTER INSPECT SPACE ACT TAB CHAMP");
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  214, 13, line, &g_text_small);

    m11_draw_viewport(state, framebuffer, framebufferWidth, framebufferHeight);

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  218, 28, 86, 42, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  218, 28, 86, 42, M11_COLOR_LIGHT_CYAN);

    snprintf(line, sizeof(line), "%s %s",
             m11_source_name(state->sourceKind),
             state->sourceId[0] != '\0' ? state->sourceId : "launcher");
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 224, 34, line, &g_text_small);
    snprintf(line, sizeof(line), "LEVEL %d  FLOOR %d/%d",
             mapDesc ? (int)mapDesc->level : 0,
             state->world.party.mapIndex + 1,
             (int)state->world.dungeon->header.mapCount);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 224, 46, line, &g_text_small);
    snprintf(line, sizeof(line), "X%d Y%d %s C%d",
             state->world.party.mapX,
             state->world.party.mapY,
             m11_direction_name(state->world.party.direction),
             state->world.party.championCount);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 224, 58, line, &g_text_small);

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
}
