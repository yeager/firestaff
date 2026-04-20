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

static void m11_set_status(M11_GameViewState* state,
                           const char* action,
                           const char* outcome) {
    if (!state) {
        return;
    }
    snprintf(state->lastAction, sizeof(state->lastAction), "%s", action ? action : "NONE");
    snprintf(state->lastOutcome, sizeof(state->lastOutcome), "%s", outcome ? outcome : "");
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

void M11_GameView_Init(M11_GameViewState* state) {
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    m11_set_status(state, "BOOT", "GAME VIEW NOT STARTED");
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
    memset(&state->lastTickResult, 0, sizeof(state->lastTickResult));
    if (F0884_ORCH_AdvanceOneTick_Compat(&state->world, &input, &state->lastTickResult) == 0) {
        m11_set_status(state, actionLabel, "TICK REJECTED");
        return 0;
    }
    state->lastWorldHash = state->lastTickResult.worldHashPost;
    if (command == CMD_TURN_LEFT || command == CMD_TURN_RIGHT) {
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
            command = CMD_NONE;
            label = "WAIT";
            break;
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

static void m11_draw_map_panel(const M11_GameViewState* state,
                               unsigned char* framebuffer,
                               int framebufferWidth,
                               int framebufferHeight) {
    static const int radius = 4;
    static const int cell = 14;
    int cx;
    int cy;
    int gx;
    int gy;
    int baseX = 16;
    int baseY = 30;
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
    unsigned char currentSquare = 0;
    unsigned short firstThing = THING_ENDOFLIST;
    int squareThingCount = 0;
    const struct DungeonMapDesc_Compat* map = NULL;
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
    map = &state->world.dungeon->maps[state->world.party.mapIndex];
    (void)m11_get_square_byte(&state->world,
                              state->world.party.mapIndex,
                              state->world.party.mapX,
                              state->world.party.mapY,
                              &currentSquare);
    firstThing = m11_get_first_square_thing(&state->world,
                                            state->world.party.mapIndex,
                                            state->world.party.mapX,
                                            state->world.party.mapY);
    squareThingCount = m11_count_square_things(&state->world,
                                               state->world.party.mapIndex,
                                               state->world.party.mapX,
                                               state->world.party.mapY);

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  8, 8, framebufferWidth - 16, framebufferHeight - 16, M11_COLOR_DARK_GRAY);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  8, 8, framebufferWidth - 16, framebufferHeight - 16, M11_COLOR_YELLOW);
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  12, 12, framebufferWidth - 24, 18, M11_COLOR_BLACK);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  18, 16, state->title[0] != '\0' ? state->title : "GAME VIEW", &g_text_title);

    m11_draw_map_panel(state, framebuffer, framebufferWidth, framebufferHeight);

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  160, 32, 148, 126, M11_COLOR_BLACK);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  160, 32, 148, 126, M11_COLOR_LIGHT_CYAN);

    snprintf(line, sizeof(line), "%s %s",
             m11_source_name(state->sourceKind),
             state->sourceId[0] != '\0' ? state->sourceId : "launcher");
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 168, 40, line, &g_text_shadow);

    snprintf(line, sizeof(line), "MAP %d/%d  SIZE %dx%d",
             state->world.party.mapIndex + 1,
             (int)state->world.dungeon->header.mapCount,
             (int)map->width,
             (int)map->height);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 168, 54, line, &g_text_shadow);

    snprintf(line, sizeof(line), "POS %d:%d  FACE %s",
             state->world.party.mapX,
             state->world.party.mapY,
             m11_direction_name(state->world.party.direction));
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 168, 68, line, &g_text_shadow);

    snprintf(line, sizeof(line), "TILE %s",
             F0503_DUNGEON_GetElementName_Compat(currentSquare >> 5));
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 168, 82, line, &g_text_shadow);

    snprintf(line, sizeof(line), "THING COUNT %d", squareThingCount);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 168, 96, line, &g_text_shadow);

    if (firstThing != THING_ENDOFLIST && firstThing != THING_NONE) {
        snprintf(line, sizeof(line), "FIRST THING %s",
                 F0505_DUNGEON_GetThingTypeName_Compat(THING_GET_TYPE(firstThing)));
    } else {
        snprintf(line, sizeof(line), "FIRST THING NONE");
    }
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 168, 110, line, &g_text_shadow);

    snprintf(line, sizeof(line), "GROUPS %d  DOORS %d",
             state->world.things ? state->world.things->groupCount : 0,
             state->world.things ? state->world.things->doorCount : 0);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 168, 124, line, &g_text_shadow);

    snprintf(line, sizeof(line), "SENSORS %d HASH %08X",
             state->world.things ? state->world.things->sensorCount : 0,
             (unsigned int)state->lastWorldHash);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 168, 138, line, &g_text_shadow);

    snprintf(line, sizeof(line), "LAST %s", state->lastAction);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 18, 168, line, &g_text_shadow);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 18, 182,
                  state->lastOutcome, &g_text_shadow);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 170, 168,
                  "UP FORWARD  DOWN BACKSTEP", &g_text_small);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 170, 180,
                  "LEFT/RIGHT TURN  ENTER WAIT", &g_text_small);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight, 170, 192,
                  "ESC RETURNS TO LAUNCHER", &g_text_small);
}
