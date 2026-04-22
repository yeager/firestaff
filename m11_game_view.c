#include "m11_game_view.h"

#include "asset_status_m12.h"
#include "fs_portable_compat.h"
#include "render_sdl_m11.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward declaration: set by M11_GameView_Draw to give nested draw
 * helpers access to the current game state for asset-backed rendering. */
static const M11_GameViewState* g_drawState = NULL;

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

/* Thread-local-ish pointer to the original DM1 font for the current
 * render pass.  Set at the top of M11_GameView_Render and cleared
 * after.  Allows m11_draw_text to automatically use the original
 * font when assets are available without changing every call site. */
static const M11_FontState* g_activeOriginalFont = NULL;

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

/* Forward declaration for original-font text renderer */
static void m11_draw_text_original(
    const M11_FontState* font,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int x,
    int y,
    const char* text,
    const M11_TextStyle* style);

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
    /* Use original DM1 font when available */
    if (g_activeOriginalFont && M11_Font_IsLoaded(g_activeOriginalFont)) {
        m11_draw_text_original(g_activeOriginalFont, framebuffer,
            framebufferWidth, framebufferHeight, x, y, text, style);
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

/* Draw text using the original DM1 font when available, with shadow.
 * Falls back to the builtin hardcoded font otherwise. */
static void m11_draw_text_original(
    const M11_FontState* font,
    unsigned char* framebuffer,
    int framebufferWidth,
    int framebufferHeight,
    int x,
    int y,
    const char* text,
    const M11_TextStyle* style)
{
    const M11_TextStyle* s = style ? style : &g_text_small;
    if (!text) return;
    if (font && M11_Font_IsLoaded(font)) {
        /* Shadow pass */
        if (s->shadowDx != 0 || s->shadowDy != 0) {
            M11_Font_DrawString(font, framebuffer, framebufferWidth,
                framebufferHeight, x + s->shadowDx, y + s->shadowDy,
                text, s->shadowColor, -1, s->scale);
        }
        /* Foreground pass */
        M11_Font_DrawString(font, framebuffer, framebufferWidth,
            framebufferHeight, x, y, text, s->color, -1, s->scale);
    } else {
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      x, y, text, style);
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

static M11_AudioMarker m11_audio_marker_from_emission(const struct TickEmission_Compat* emission) {
    if (!emission) {
        return M11_AUDIO_MARKER_NONE;
    }
    switch (emission->kind) {
        case EMIT_PARTY_MOVED:
            return M11_AUDIO_MARKER_FOOTSTEP;
        case EMIT_DOOR_STATE:
            return M11_AUDIO_MARKER_DOOR;
        case EMIT_DAMAGE_DEALT:
        case EMIT_KILL_NOTIFY:
        case EMIT_CHAMPION_DOWN:
            return M11_AUDIO_MARKER_COMBAT;
        case EMIT_SPELL_EFFECT:
            return M11_AUDIO_MARKER_SPELL;
        case EMIT_SOUND_REQUEST:
            switch (emission->payload[0]) {
                case 1:
                case 2:
                    return M11_AUDIO_MARKER_DOOR;
                case 3:
                    return M11_AUDIO_MARKER_FOOTSTEP;
                case 4:
                    return M11_AUDIO_MARKER_COMBAT;
                case 6:
                    return M11_AUDIO_MARKER_SPELL;
                default:
                    return M11_AUDIO_MARKER_CREATURE;
            }
        default:
            break;
    }
    return M11_AUDIO_MARKER_NONE;
}

static void m11_audio_emit_for_emission(M11_GameViewState* state,
                                        const struct TickEmission_Compat* emission) {
    M11_AudioMarker marker;
    if (!state || !emission) {
        return;
    }
    marker = m11_audio_marker_from_emission(emission);
    if (marker == M11_AUDIO_MARKER_NONE) {
        return;
    }
    (void)M11_Audio_EmitMarker(&state->audioState, marker);
    state->audioEventCount += 1;
}

/* m11_join_path replaced by FSP_JoinPath from fs_portable_compat. */

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
        return FSP_JoinPath(out, outSize, dataDir, "DUNGEON.DAT");
    }
    if (strcmp(gameId, "csb") == 0) {
        return FSP_JoinPath(out, outSize, dataDir, "CSB.DAT");
    }
    if (strcmp(gameId, "dm2") == 0) {
        return FSP_JoinPath(out, outSize, dataDir, "DM2DUNGEON.DAT");
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

/* ── DM1 random ornament computation (F0169/F0170 from ReDMCSB) ──
 * Floor ornaments in DM1 are assigned per-square using a deterministic
 * pseudorandom algorithm seeded by the dungeon header's OrnamentRandomSeed.
 * The algorithm hashes map coordinates + map index/dimensions to produce
 * a per-square ornament index.  If the square allows random ornaments
 * (bit 3 of the corridor square byte) and the map has floor ornaments,
 * the square gets a floor ornament ordinal (1-based, 0 = none).
 * Ref: ReDMCSB DUNGEON.C F0169_DUNGEON_GetRandomOrnamentIndex,
 *      F0170_DUNGEON_GetRandomOrnamentOrdinal,
 *      F0172_DUNGEON_SetSquareAspect (corridor case). */

/* Compute the floor ornament ordinal for a corridor/pit/stairs/teleporter
 * square.  Returns 1-based ordinal, or 0 if none.
 * This mirrors F0172_DUNGEON_SetSquareAspect's floor ornament path.
 * Two sources of floor ornaments:
 *  1. Random floor ornaments based on MASK0x0008 bit in the square byte
 *  2. Sensor things with ornamentOrdinal > 0 (explicit placement)
 * Ref: ReDMCSB DUNGEON.C F0172 lines ~2189-2196. */
static int m11_compute_floor_ornament_ordinal(
    const M11_GameViewState* state,
    int mapIndex,
    int mapX,
    int mapY,
    unsigned char square) {
    int elementType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    const struct DungeonMapDesc_Compat* map;
    int randomAllowed;
    int randomFloorOrnCount;
    unsigned short ornSeed;
    unsigned int value2;
    int idx;
    int ordinal = 0;

    /* Floor ornaments only appear on corridor, pit, stairs, teleporter */
    if (elementType != DUNGEON_ELEMENT_CORRIDOR &&
        elementType != DUNGEON_ELEMENT_PIT &&
        elementType != DUNGEON_ELEMENT_STAIRS &&
        elementType != DUNGEON_ELEMENT_TELEPORTER) {
        return 0;
    }
    if (!state || !state->world.dungeon ||
        mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }

    map = &state->world.dungeon->maps[mapIndex];
    ornSeed = state->world.dungeon->header.ornamentRandomSeed;
    randomFloorOrnCount = (int)map->randomFloorOrnamentCount;

    /* Random floor ornament from square byte bit 3 (MASK0x0008) */
    randomAllowed = (square & 0x08) != 0;
    if (randomAllowed && randomFloorOrnCount > 0) {
        /* value2 = 3000 + (mapIndex << 6) + mapWidth + mapHeight
         * Ref: ReDMCSB F0170 call from F0172 */
        value2 = (unsigned int)(3000 + (mapIndex << 6) +
                                (int)map->width + (int)map->height);
        idx = (int)((((((unsigned int)(2000 + (mapX << 5) + mapY)) * 31417u) >> 1) +
                     (value2 * 11u) + ornSeed) >> 2) % 30;
        if (idx < randomFloorOrnCount) {
            ordinal = idx + 1; /* 1-based */
        }
    }

    /* Sensor-placed floor ornaments override random ones.
     * Scan for sensor things with ornamentOrdinal on this square.
     * Ref: ReDMCSB F0172 — C0_SENSOR_FLOOR_ORNAMENT_ORDINAL path. */
    if (state->world.things && state->world.things->sensors) {
        unsigned short scanThing = m11_get_first_square_thing(
            &state->world, mapIndex, mapX, mapY);
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_SENSOR) {
                int sIdx = THING_GET_INDEX(scanThing);
                if (sIdx >= 0 && sIdx < state->world.things->sensorCount) {
                    /* Floor ornament sensor: ornamentOrdinal > 0 on
                     * non-wall squares indicates a floor ornament.
                     * Use the same field we use for wall ornaments. */
                    int sOrd = (int)state->world.things->sensors[sIdx].ornamentOrdinal;
                    if (sOrd > 0) {
                        ordinal = sOrd;
                        break;
                    }
                }
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    return ordinal;
}

/* ── DM1 Creature Aspect Data (G0219_as_Graphic558_CreatureAspects) ──
 * Each creature type has an aspect descriptor that controls:
 *  - FirstNativeBitmapRelativeIndex: offset from the creature graphic set base
 *  - FirstDerivedBitmapIndex: index into derived bitmap cache
 *  - CoordinateSet (upper 4 bits of CoordinateSet_TransparentColor)
 *  - TransparentColor (lower 4 bits)
 *  - ReplacementColorSetIndices (color 9 in low nibble, color 10 in high)
 *
 * The coordinate set selects which of 11 pre-defined viewport position
 * tables to use for placing the creature at each depth/sub-cell.
 *
 * Values extracted from ReDMCSB DEFS.H / disassembly of DM1 PC v3.4.
 * The front/side/attack width+height fields exist only in later versions
 * (S10+) and are not needed for our rendering — we use GRAPHICS.DAT
 * bitmap dimensions directly.
 *
 * Format: { FirstNativeBitmapRelativeIndex, FirstDerivedBitmapIndex,
 *           CoordinateSet_TransparentColor, ReplacementColorSetIndices }
 */
typedef struct {
    unsigned short firstNativeBitmapRelativeIndex;
    unsigned short firstDerivedBitmapIndex;
    unsigned char  coordinateSet_transparentColor;
    unsigned char  replacementColorSetIndices;
} M11_CreatureAspect;

#define M11_CREATURE_COORD_SET(a) (((a)->coordinateSet_transparentColor >> 4) & 0x0F)
#define M11_CREATURE_TRANSPARENT_COLOR(a) ((a)->coordinateSet_transparentColor & 0x0F)
#define M11_CREATURE_REPL_COLOR9(a) ((a)->replacementColorSetIndices & 0x0F)
#define M11_CREATURE_REPL_COLOR10(a) (((a)->replacementColorSetIndices >> 4) & 0x0F)

static const M11_CreatureAspect s_creatureAspects[27] = {
    /* Type  0: GiantScorpion   — coordSet 1, transparent 10, repl 0x11 */
    { 0,  495, 0x1A, 0x11 },
    /* Type  1: SwampSlime       — coordSet 0, transparent  2, repl 0x00 */
    { 6,  507, 0x02, 0x00 },
    /* Type  2: Giggler          — coordSet 1, transparent 10, repl 0x22 */
    { 12, 519, 0x1A, 0x22 },
    /* Type  3: PainRat          — coordSet 0, transparent 10, repl 0x33 */
    { 18, 531, 0x0A, 0x33 },
    /* Type  4: Ruster           — coordSet 1, transparent 10, repl 0x44 */
    { 24, 543, 0x1A, 0x44 },
    /* Type  5: Screamer         — coordSet 0, transparent 10, repl 0x00 */
    { 30, 555, 0x0A, 0x00 },
    /* Type  6: Rockpile         — coordSet 2, transparent  0, repl 0x00 */
    { 36, 567, 0x20, 0x00 },
    /* Type  7: GhostRive        — coordSet 1, transparent 10, repl 0x55 */
    { 42, 579, 0x1A, 0x55 },
    /* Type  8: WaterElemental   — coordSet 0, transparent 10, repl 0x00 */
    { 48, 591, 0x0A, 0x00 },
    /* Type  9: Couatl           — coordSet 1, transparent 10, repl 0x66 */
    { 54, 603, 0x1A, 0x66 },
    /* Type 10: StoneGolem       — coordSet 2, transparent 10, repl 0x00 */
    { 60, 615, 0x2A, 0x00 },
    /* Type 11: Mummy            — coordSet 0, transparent 10, repl 0x77 */
    { 66, 627, 0x0A, 0x77 },
    /* Type 12: Skeleton         — coordSet 1, transparent 10, repl 0x00 */
    { 72, 639, 0x1A, 0x00 },
    /* Type 13: MagentaWorm      — coordSet 0, transparent 10, repl 0x88 */
    { 78, 651, 0x0A, 0x88 },
    /* Type 14: Trolin           — coordSet 1, transparent 10, repl 0x99 */
    { 84, 663, 0x1A, 0x99 },
    /* Type 15: GiantWasp        — coordSet 1, transparent 10, repl 0x00 */
    { 90, 675, 0x1A, 0x00 },
    /* Type 16: Antman           — coordSet 1, transparent 10, repl 0xAA */
    { 54, 687, 0x1A, 0xAA },
    /* Type 17: Vexirk           — coordSet 0, transparent 10, repl 0xBB */
    { 96, 699, 0x0A, 0xBB },
    /* Type 18: AnimatedArmour   — coordSet 2, transparent 10, repl 0x00 */
    { 102, 711, 0x2A, 0x00 },
    /* Type 19: Materializer     — coordSet 0, transparent 10, repl 0x00 */
    { 108, 723, 0x0A, 0x00 },
    /* Type 20: RedDragon        — coordSet 2, transparent 10, repl 0x00 */
    { 114, 735, 0x2A, 0x00 },
    /* Type 21: Oitu             — coordSet 2, transparent  0, repl 0x00 */
    { 120, 747, 0x20, 0x00 },
    /* Type 22: Demon            — coordSet 1, transparent 10, repl 0xCC */
    { 126, 759, 0x1A, 0xCC },
    /* Type 23: LordChaos        — coordSet 2, transparent 10, repl 0x00 */
    { 132, 771, 0x2A, 0x00 },
    /* Type 24: LordOrder        — coordSet 2, transparent 10, repl 0x00 */
    { 138, 783, 0x2A, 0x00 },
    /* Type 25: GreyLord         — coordSet 2, transparent 10, repl 0x00 */
    { 144, 795, 0x2A, 0x00 },
    /* Type 26: LordChaosRedDragon — coordSet 2, transparent 10, repl 0x00 */
    { 114, 807, 0x2A, 0x00 }
};

/* ── DM1 Creature Viewport Coordinate Sets (G0224) ──
 * Exact front-cell viewport coordinates extracted from original item 558
 * documentation (Graphic558 / G0224). Each entry is {centerX, bottomY}
 * in the original 224×136 viewport.
 *
 * We only encode the 3 coordinate sets actually referenced by the DM1
 * creature aspect table in this renderer (sets 0, 1 and 2).
 *
 * For set 0 / set 2 the five slots map to c1..c5.
 * For set 1 the five slots map to c6..c10.
 * Depth index mapping here is 0=D1, 1=D2, 2=D3.
 */
static const unsigned char s_creatureFrontCoordSets[3][3][5][2] = {
    /* Depth 0 (D1) */
    {
        /* Set 0: c1..c5 */  {{ 83,103 }, {141,103 }, {148,119 }, { 76,119 }, {109,111 }},
        /* Set 1: c6..c10 */ {{ 81,119 }, {142,119 }, {111,105 }, {111,111 }, {111,119 }},
        /* Set 2: c1..c5 */  {{ 83, 94 }, {141, 94 }, {148, 98 }, { 76, 98 }, {109, 96 }}
    },
    /* Depth 1 (D2) */
    {
        /* Set 0: c1..c5 */  {{ 92, 81 }, {131, 81 }, {132, 90 }, { 91, 90 }, {111, 85 }},
        /* Set 1: c6..c10 */ {{ 91, 90 }, {132, 90 }, {111, 83 }, {111, 85 }, {111, 90 }},
        /* Set 2: c1..c5 */  {{ 92, 73 }, {131, 73 }, {132, 78 }, { 91, 78 }, {111, 75 }}
    },
    /* Depth 2 (D3) */
    {
        /* Set 0: c1..c5 */  {{ 95, 70 }, {127, 70 }, {129, 75 }, { 93, 75 }, {111, 72 }},
        /* Set 1: c6..c10 */ {{ 94, 75 }, {128, 75 }, {111, 70 }, {111, 72 }, {111, 75 }},
        /* Set 2: c1..c5 */  {{ 95, 59 }, {127, 59 }, {129, 61 }, { 93, 61 }, {111, 60 }}
    }
};

void M11_GameView_GetCreatureFrontSlotPoint(int coordSet,
                                            int depthIndex,
                                            int visibleCount,
                                            int slotIndex,
                                            int* outCenterX,
                                            int* outBottomY) {
    int pointIndex = 4;
    if (outCenterX) *outCenterX = 111;
    if (outBottomY) *outBottomY = (depthIndex <= 0) ? 111 : (depthIndex == 1 ? 85 : 72);
    if (coordSet < 0 || coordSet > 2 || depthIndex < 0 || depthIndex > 2 ||
        slotIndex < 0 || slotIndex > 3) {
        return;
    }

    if (coordSet == 1) {
        if (visibleCount <= 1) {
            pointIndex = 4; /* c10 */
        } else {
            pointIndex = slotIndex < 2 ? slotIndex : 4; /* c6/c7, clamp extras */
        }
    } else {
        if (visibleCount <= 1) {
            pointIndex = 4; /* c5 */
        } else {
            pointIndex = slotIndex;
            if (pointIndex > 3) pointIndex = 3;
        }
    }

    if (outCenterX) *outCenterX = (int)s_creatureFrontCoordSets[depthIndex][coordSet][pointIndex][0];
    if (outBottomY) *outBottomY = (int)s_creatureFrontCoordSets[depthIndex][coordSet][pointIndex][1];
}

/* Query the creature aspect's coordinate set index (0-10) for a type. */
static int m11_creature_coordinate_set(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return M11_CREATURE_COORD_SET(&s_creatureAspects[creatureType]);
}

/* Query the creature's transparent color index from aspect data. */
static int m11_creature_transparent_color(int creatureType) {
    if (creatureType < 0 || creatureType >= 27) return 0;
    return M11_CREATURE_TRANSPARENT_COLOR(&s_creatureAspects[creatureType]);
}

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
static int m11_viewport_cell_is_wall_free(int elementType);
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

    /* Trigger visual feedback for the attack */
    M11_GameView_NotifyDamageFlash(state, group->creatureType);
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
        m11_audio_emit_for_emission(state, e);
        switch (e->kind) {
            case EMIT_DAMAGE_DEALT: {
                int champIdx = state->world.party.activeChampionIndex;
                int dmgDealt = (int)e->payload[2];
                m11_log_event(state, M11_COLOR_LIGHT_RED,
                              "T%u: DAMAGE %d DEALT",
                              (unsigned int)state->world.gameTick,
                              dmgDealt);
                /* Award combat XP to the active champion */
                m11_award_combat_xp(state, champIdx, dmgDealt);
                /* Trigger GRAPHICS.DAT graphic 14 viewport overlay */
                M11_GameView_NotifyCreatureHit(state, dmgDealt);
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
            case EMIT_GAME_WON:
                if (!state->gameWon) {
                    state->gameWon = 1;
                    state->gameWonTick = state->world.gameTick;
                    m11_log_event(state, M11_COLOR_LIGHT_GREEN,
                                  "T%u: THE QUEST IS COMPLETE!",
                                  (unsigned int)state->world.gameTick);
                    m11_set_status(state, "ENDGAME", "VICTORY!");
                    snprintf(state->inspectTitle,
                             sizeof(state->inspectTitle), "VICTORY");
                    snprintf(state->inspectDetail,
                             sizeof(state->inspectDetail),
                             "LORD CHAOS IS DEFEATED. THE FIRESTAFF IS RESTORED.");
                }
                break;
            case EMIT_PARTY_DEAD:
                if (!state->partyDead) {
                    state->partyDead = 1;
                    m11_log_event(state, M11_COLOR_LIGHT_RED,
                                  "T%u: ALL CHAMPIONS HAVE FALLEN",
                                  (unsigned int)state->world.gameTick);
                    m11_set_status(state, "DEATH", "PARTY WIPED");
                    snprintf(state->inspectTitle,
                             sizeof(state->inspectTitle), "GAME OVER");
                    snprintf(state->inspectDetail,
                             sizeof(state->inspectDetail),
                             "THE DUNGEON CLAIMS YOUR SOULS. LOAD A SAVE OR RETURN TO MENU.");
                }
                break;
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
    (void)M11_Audio_Init(&state->audioState);
    /* V1 presentation: debug HUD off by default, opt-in via env */
    {
        const char* dbg = getenv("FIRESTAFF_DEBUG_HUD");
        state->showDebugHUD = (dbg && dbg[0] == '1') ? 1 : 0;
    }
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
    M11_Audio_Shutdown(&state->audioState);
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
    {
        int savedDebugHUD = state->showDebugHUD;
        M11_GameView_Shutdown(state);
        M11_GameView_Init(state);
        state->showDebugHUD = savedDebugHUD;
    }
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
                /* Try to load the original DM1 font from GRAPHICS.DAT */
                M11_Font_Init(&state->originalFont);
                if (M11_Font_LoadFromGraphicsDat(
                        &state->originalFont,
                        state->assetLoader.fileState,
                        state->assetLoader.runtimeState)) {
                    state->originalFontAvailable = 1;
                }
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
    /* No idle ticks during overlays, endgame, or dialog. */
    if (state->gameWon || state->dialogOverlayActive ||
        state->mapOverlayActive || state->inventoryPanelActive) {
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
    {
        int orchResult = F0884_ORCH_AdvanceOneTick_Compat(
            &state->world, &input, &state->lastTickResult);
        if (orchResult == ORCH_FAIL) {
            m11_set_status(state, actionLabel, "TICK REJECTED");
            return 0;
        }
        /* ORCH_PARTY_DEAD and ORCH_GAME_WON are handled via emissions
         * below — the tick was still successfully processed. */
    }
    state->lastWorldHash = state->lastTickResult.worldHashPost;

    /* Process emissions into the message log (also sets gameWon/partyDead
     * via EMIT_GAME_WON / EMIT_PARTY_DEAD handling). */
    M11_GameView_ProcessTickEmissions(state);

    /* Apply survival mechanics */
    m11_apply_survival_drain(state);
    m11_apply_rest_recovery(state);

    /* Torch fuel burn-down */
    M11_GameView_UpdateTorchFuel(state);

    /* Creature AI: movement and autonomous damage */
    m11_process_creature_ticks(state);

    /* Advance animation frame counters and decay timers */
    M11_GameView_TickAnimation(state);

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

    /* Dialog overlay: any input dismisses it, then return to gameplay. */
    if (state->dialogOverlayActive) {
        if (input == M12_MENU_INPUT_BACK) {
            M11_GameView_DismissDialogOverlay(state);
            m11_set_status(state, "RETURN", "BACK TO LAUNCHER");
            return M11_GAME_INPUT_RETURN_TO_MENU;
        }
        if (input != M12_MENU_INPUT_NONE) {
            M11_GameView_DismissDialogOverlay(state);
            return M11_GAME_INPUT_REDRAW;
        }
        return M11_GAME_INPUT_IGNORED;
    }

    /* Endgame: only ESC (return to menu) and quickload accepted. */
    if (state->gameWon) {
        if (input == M12_MENU_INPUT_BACK) {
            m11_set_status(state, "RETURN", "BACK TO LAUNCHER");
            return M11_GAME_INPUT_RETURN_TO_MENU;
        }
        return M11_GAME_INPUT_IGNORED;
    }

    /* Map/inventory toggle — always available (closes overlay if other open). */
    if (input == M12_MENU_INPUT_MAP_TOGGLE) {
        state->inventoryPanelActive = 0;
        M11_GameView_ToggleMapOverlay(state);
        return M11_GAME_INPUT_REDRAW;
    }
    if (input == M12_MENU_INPUT_INVENTORY_TOGGLE) {
        state->mapOverlayActive = 0;
        M11_GameView_ToggleInventoryPanel(state);
        return M11_GAME_INPUT_REDRAW;
    }

    /* While an overlay is open, block gameplay input; ESC/BACK closes it. */
    if (state->mapOverlayActive || state->inventoryPanelActive) {
        if (input == M12_MENU_INPUT_BACK) {
            state->mapOverlayActive = 0;
            state->inventoryPanelActive = 0;
            return M11_GAME_INPUT_REDRAW;
        }
        /* In inventory panel, accept up/down/left/right to navigate slots */
        if (state->inventoryPanelActive) {
            if (input == M12_MENU_INPUT_UP && state->inventorySelectedSlot > 0) {
                state->inventorySelectedSlot--;
                return M11_GAME_INPUT_REDRAW;
            }
            if (input == M12_MENU_INPUT_DOWN && state->inventorySelectedSlot < 20) {
                state->inventorySelectedSlot++;
                return M11_GAME_INPUT_REDRAW;
            }
        }
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

    /* Click dismisses dialog overlay. */
    if (state->dialogOverlayActive) {
        M11_GameView_DismissDialogOverlay(state);
        return M11_GAME_INPUT_REDRAW;
    }

    /* In endgame, clicks are ignored (use ESC). */
    if (state->gameWon) {
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

/* Forward declaration for floor ornament rendering (defined after door ornaments). */
static int m11_draw_floor_ornament(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int fbW, int fbH,
                                   const M11_ViewRect* rect,
                                   int ornamentOrdinal,
                                   int depthIndex,
                                   int sideHint);

enum {
    M11_MAX_CELL_CREATURES = 4, /* DM1 supports up to 4 creature groups per square */
    M11_MAX_CELL_ITEMS    = 4  /* DM1 scatters up to 4 floor items visibly */
};

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
    /* All creature group types on this square (up to 4, -1 terminated) */
    int creatureTypes[M11_MAX_CELL_CREATURES];
    int creatureCountsPerGroup[M11_MAX_CELL_CREATURES]; /* creature count per group (count+1) */
    int creatureGroupCount; /* number of valid entries in creatureTypes[] */
    /* First floor item info for sprite rendering (legacy single-item) */
    int firstItemThingType;    /* THING_TYPE_WEAPON..JUNK, or -1 if no item */
    int firstItemSubtype;      /* weapon/armour/potion/junk subtype, or -1 */
    /* All visible floor items (up to 4) for multi-item scatter */
    int floorItemTypes[M11_MAX_CELL_ITEMS];    /* THING_TYPE_*, -1 sentinel */
    int floorItemSubtypes[M11_MAX_CELL_ITEMS]; /* subtype per item, -1 sentinel */
    int floorItemCount; /* number of valid entries in floorItem arrays */
    /* Wall/door ornament ordinal from thing data (0-15, -1 if none) */
    int wallOrnamentOrdinal;
    int doorOrnamentOrdinal;
    /* First projectile graphic index (416-438) from GRAPHICS.DAT, or -1 */
    int firstProjectileGfxIndex;
    /* First projectile direction relative to party facing.
     * 0 = moving away from party, 1 = moving right, 2 = toward party,
     * 3 = moving left.  -1 if no projectile or direction unknown.
     * Used for DM1-faithful projectile sprite mirroring. */
    int firstProjectileRelDir;
    /* First projectile sub-cell position (0-3, or -1 if none).
     * In DM1, projectiles occupy a specific sub-cell (quadrant) within
     * the tile.  0=NW, 1=NE, 2=SW, 3=SE (absolute).  The relative
     * sub-cell position (relative to party facing) determines the
     * projectile's viewport offset within the cell face area.
     * Ref: ReDMCSB DUNVIEW.C F115 — M11_CELL(P141_T_Thing) positioning. */
    int firstProjectileCell;
    /* Floor ornament ordinal for this square (1-based ordinal, 0 = none).
     * In DM1, corridor/pit/stair/teleporter squares may have floor
     * ornaments assigned via random generation or sensor things.
     * Ref: ReDMCSB DUNGEON.C F169 — SquareAspect[C4_FLOOR_ORNAMENT_ORDINAL]. */
    int floorOrnamentOrdinal;
    /* First explosion type (0-50), or -1 */
    int firstExplosionType;
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

/* Draw a GRAPHICS.DAT-backed projectile sprite in the given area.
 * relativeDir: projectile direction relative to party facing (0-3), or -1.
 *   0 = flying away, 1 = flying right, 2 = flying toward, 3 = flying left.
 * relativeCell: projectile sub-cell relative to party facing (0-3), or -1.
 *   0 = back-left, 1 = back-right, 2 = front-left, 3 = front-right.
 *   When >= 0, the sprite is offset within the face area to match the
 *   DM1 quadrant-based projectile positioning.  When -1, centered.
 * In DM1, projectile sprites are mirrored horizontally when the relative
 * direction is 1 (right-bound); left-bound (3) and forward/backward (0,2)
 * use the normal sprite orientation.
 * Ref: ReDMCSB DUNVIEW.C F115 — projectile coordinate lookup uses
 * G218_aaaauc_Graphic558_ObjectCoordinateSets to position projectiles
 * within the viewport cell based on their sub-cell.
 * Returns 1 if a real sprite was drawn, 0 for fallback. */
static int m11_draw_projectile_sprite(const M11_GameViewState* state,
                                      unsigned char* framebuffer,
                                      int framebufferWidth,
                                      int framebufferHeight,
                                      int x, int y, int w, int h,
                                      int gfxIndex, int depthIndex,
                                      int relativeDir,
                                      int relativeCell) {
    const M11_AssetSlot* slot;
    int drawW, drawH, drawX, drawY;
    int scale;
    int useMirror;
    if (!state || !state->assetsAvailable || gfxIndex < 416 ||
        gfxIndex >= 439) return 0;
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, (unsigned int)gfxIndex);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;
    /* Scale projectile sprite by depth: 100% at depth 0, 66% at 1, 40% at 2 */
    scale = depthIndex == 0 ? 100 : (depthIndex == 1 ? 66 : 40);
    drawW = (int)slot->width * scale / 100;
    drawH = (int)slot->height * scale / 100;
    if (drawW < 3) drawW = 3;
    if (drawH < 3) drawH = 3;
    if (drawW > w) drawW = w;
    if (drawH > h) drawH = h;
    /* DM1 sub-cell projectile positioning.
     * In the original, each projectile is drawn at a specific quadrant
     * of the viewport cell face.  The relative cell (0-3) maps to:
     *   0 = back-left:   upper-left quadrant
     *   1 = back-right:  upper-right quadrant
     *   2 = front-left:  lower-left quadrant
     *   3 = front-right: lower-right quadrant
     * We offset the sprite center toward the appropriate quadrant.
     * At greater depth the offset is reduced proportionally. */
    if (relativeCell >= 0 && relativeCell <= 3) {
        int qx = w / 4;  /* quarter-width offset */
        int qy = h / 4;  /* quarter-height offset */
        /* Reduce offset at depth to match perspective convergence */
        if (depthIndex >= 1) { qx = qx * 2 / 3; qy = qy * 2 / 3; }
        if (depthIndex >= 2) { qx = qx / 2;     qy = qy / 2; }
        switch (relativeCell) {
            case 0: /* back-left: upper-left */
                drawX = x + (w / 2 - qx) - drawW / 2;
                drawY = y + (h / 2 - qy) - drawH / 2;
                break;
            case 1: /* back-right: upper-right */
                drawX = x + (w / 2 + qx) - drawW / 2;
                drawY = y + (h / 2 - qy) - drawH / 2;
                break;
            case 2: /* front-left: lower-left */
                drawX = x + (w / 2 - qx) - drawW / 2;
                drawY = y + (h / 2 + qy) - drawH / 2;
                break;
            default: /* front-right: lower-right */
                drawX = x + (w / 2 + qx) - drawW / 2;
                drawY = y + (h / 2 + qy) - drawH / 2;
                break;
        }
        /* Clamp to face bounds */
        if (drawX < x) drawX = x;
        if (drawY < y) drawY = y;
        if (drawX + drawW > x + w) drawX = x + w - drawW;
        if (drawY + drawH > y + h) drawY = y + h - drawH;
    } else {
        drawX = x + (w - drawW) / 2;
        drawY = y + (h - drawH) / 2;
    }
    /* DM1 projectile facing: mirror horizontally for right-bound (relDir 1).
     * Ref: ReDMCSB VIEWPORT.C — projectile sprites are stored facing left
     * and flipped for right-ward travel relative to party facing. */
    useMirror = (relativeDir == 1) ? 1 : 0;
    /* Use transparentColor=0 (palette index 0 = black) for projectile
     * compositing, matching DM1 behaviour.  Previous code used -1
     * (no transparency), which caused black-bordered projectile
     * rectangles over corridor backgrounds. */
    if (useMirror) {
        M11_AssetLoader_BlitScaledMirror(slot, framebuffer, framebufferWidth,
                                         framebufferHeight,
                                         drawX, drawY, drawW, drawH, 0);
    } else {
        M11_AssetLoader_BlitScaled(slot, framebuffer, framebufferWidth,
                                   framebufferHeight,
                                   drawX, drawY, drawW, drawH, 0);
    }
    return 1;
}

/* Draw a pit visual effect: darkened floor area with depth gradient.
 * In DM1, pits appear as dark openings in the floor.  We darken the
 * lower portion of the cell face to simulate the pit opening. */
static void m11_draw_pit_effect(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                int x, int y, int w, int h,
                                int depthIndex) {
    int pitY = y + h / 3;
    int pitH = h - h / 3;
    (void)depthIndex;
    /* Draw dark pit opening on the floor area */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x + 2, pitY, w - 4, pitH - 2, M11_COLOR_BLACK);
    /* Outline for definition — dark gray border around the pit */
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  x + 1, pitY - 1, w - 2, pitH, M11_COLOR_DARK_GRAY);
    /* Inner shadow lines for depth perception */
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   x + 3, x + w - 4, pitY + 2, M11_COLOR_NAVY);
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   x + 4, x + w - 5, pitY + pitH - 4, M11_COLOR_NAVY);
}

/* Draw a teleporter visual effect: colored shimmer/rift.
 * In DM1, teleporters have a distinctive visual presence in the
 * viewport — a shimmering effect.  We render a colored frame with
 * an inner glow pattern. */
static void m11_draw_teleporter_effect(unsigned char* framebuffer,
                                       int framebufferWidth,
                                       int framebufferHeight,
                                       int x, int y, int w, int h,
                                       int depthIndex) {
    int cx = x + w / 2;
    int cy = y + h / 2;
    int rw = w * 2 / 3;
    int rh = h * 2 / 3;
    unsigned char color = depthIndex == 0 ? M11_COLOR_LIGHT_CYAN :
                          (depthIndex == 1 ? M11_COLOR_CYAN : M11_COLOR_NAVY);
    /* Outer shimmer frame */
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  cx - rw / 2, cy - rh / 2, rw, rh, color);
    /* Inner cross pattern */
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   cx - rw / 4, cx + rw / 4, cy, color);
    m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                   cx, cy - rh / 4, cy + rh / 4, color);
    /* Corner dots for shimmer */
    if (rw > 8 && rh > 8) {
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                      cx - rw / 3, cy - rh / 3, M11_COLOR_WHITE);
        m11_put_pixel(framebuffer, framebufferWidth, framebufferHeight,
                      cx + rw / 3, cy + rh / 3, M11_COLOR_WHITE);
    }
}

static void m11_draw_effect_cue(unsigned char* framebuffer,
                                int framebufferWidth,
                                int framebufferHeight,
                                int x,
                                int y,
                                int w,
                                int h,
                                const M11_ViewportCell* cell,
                                int depthIndex) {
    int cx = x + w / 2;
    int cy = y + h / 2;
    if (!cell) {
        return;
    }
    /* Projectile sprite from GRAPHICS.DAT, or fallback crosshair */
    if (cell->summary.projectiles > 0) {
        if (!g_drawState ||
            !m11_draw_projectile_sprite(g_drawState, framebuffer,
                                        framebufferWidth, framebufferHeight,
                                        x, y, w, h,
                                        cell->firstProjectileGfxIndex,
                                        depthIndex,
                                        cell->firstProjectileRelDir,
                                        cell->firstProjectileCell)) {
            /* Fallback: cyan crosshair */
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           cx - 3, cx + 3, cy, M11_COLOR_LIGHT_CYAN);
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                           cx, cy - 3, cy + 3, M11_COLOR_LIGHT_CYAN);
        }
    }
    /* Teleporter shimmer effect */
    if (cell->summary.teleporters > 0) {
        m11_draw_teleporter_effect(framebuffer, framebufferWidth, framebufferHeight,
                                   x, y, w, h, depthIndex);
    }
    /* Pit darkness effect */
    if (cell->elementType == DUNGEON_ELEMENT_PIT) {
        m11_draw_pit_effect(framebuffer, framebufferWidth, framebufferHeight,
                            x, y, w, h, depthIndex);
    }
    /* DM1 explosion-type-specific viewport visual effects.
     * In the original, different explosion types produce distinct visual
     * feedback: fire explosions flash orange/red, lightning is cyan/white,
     * poison is green, and "open door" / "spell" types use unique colors.
     * Ref: ReDMCSB TIMELINE.C explosion type categories:
     *   Types 0-7: fire/fireballs (orange/red)
     *   Types 8-11: poison cloud (green)
     *   Types 12-18: lightning/energy (cyan/white)
     *   Types 40-50: special effects (magenta) */
    if (cell->summary.explosions > 0) {
        unsigned char expColor;
        int expType = cell->firstExplosionType;
        int expR = 5 + (depthIndex == 0 ? 2 : 0);
        if (expType >= 0 && expType <= 7) {
            /* Fire/fireball explosions: orange-red burst */
            expColor = M11_COLOR_LIGHT_RED;
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          cx - expR, cy - expR, expR * 2 + 1, expR * 2 + 1, expColor);
            /* Inner bright core */
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          cx - expR / 2, cy - expR / 2,
                          expR + 1, expR + 1, M11_COLOR_YELLOW);
        } else if (expType >= 8 && expType <= 11) {
            /* Poison cloud: green haze */
            expColor = M11_COLOR_GREEN;
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          cx - expR, cy - expR, expR * 2 + 1, expR * 2 + 1, expColor);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          cx - expR / 2, cy - expR / 2,
                          expR + 1, expR + 1, M11_COLOR_LIGHT_GREEN);
        } else if (expType >= 12 && expType <= 18) {
            /* Lightning/energy: cyan-white flash */
            expColor = M11_COLOR_LIGHT_CYAN;
            m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                           cx - expR, cx + expR, cy, M11_COLOR_WHITE);
            m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                           cx, cy - expR, cy + expR, M11_COLOR_WHITE);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          cx - expR, cy - expR, expR * 2 + 1, expR * 2 + 1, expColor);
        } else {
            /* All other explosion types: generic magenta burst */
            expColor = M11_COLOR_MAGENTA;
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          cx - expR / 2, cy - expR / 2,
                          expR + 1, expR + 1, expColor);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          cx - expR, cy - expR, expR * 2 + 1, expR * 2 + 1, expColor);
        }
    }
    if (cell->summary.sensors > 0 || cell->summary.textStrings > 0) {
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

/* ── Per-map ornament index cache ──
 * Load the wall and door ornament index tables for a given map from
 * DUNGEON.DAT metadata.  In the original, the metadata follows the
 * square tile data: creatureTypeCount bytes of creature type indices,
 * then wallOrnamentCount bytes of wall ornament graphic indices,
 * then floorOrnamentCount bytes of floor ornament graphic indices,
 * then doorOrnamentCount bytes of door ornament graphic indices.
 * Ref: ReDMCSB DUNGEON.C F0173_DUNGEON_ReadMapData_CPSE.
 * We read these lazily and cache in the game view state. */
static void m11_ensure_ornament_cache(M11_GameViewState* state, int mapIndex) {
    const struct DungeonDatState_Compat* dun;
    const struct DungeonMapDesc_Compat* m;
    long rawDataFileOffset;
    long mapFileOffset;
    int squareCount;
    unsigned char metaBuf[128]; /* enough for all metadata bytes */
    int metaSize;
    int offset;
    FILE* fp;
    int i;
    int totalColumns;
    long thingDataTotalBytes;

    if (!state || !state->world.dungeon || !state->world.dungeon->loaded) return;
    dun = state->world.dungeon;
    if (mapIndex < 0 || mapIndex >= (int)dun->header.mapCount) return;
    if (mapIndex >= (int)32) return;
    if (state->ornamentCacheLoaded[mapIndex]) return;

    m = &dun->maps[mapIndex];
    squareCount = (int)m->width * (int)m->height;

    /* Compute metadata size: creature types + wall ornaments (+ inscription slot) +
     * floor ornaments + door ornaments. */
    metaSize = (int)m->creatureTypeCount + (int)m->wallOrnamentCount + 1 +
               (int)m->floorOrnamentCount + (int)m->doorOrnamentCount;
    if (metaSize <= 0 || metaSize > (int)sizeof(metaBuf)) {
        state->ornamentCacheLoaded[mapIndex] = 1;
        return;
    }

    /* Reconstruct the DUNGEON.DAT path from the GRAPHICS.DAT path
     * stored in the asset loader (same directory). */
    {
        char datPath[512];
        const char* gfxPath = state->assetLoader.graphicsDatPath;
        const char* lastSlash;
        int dirLen;
        if (!state->assetsAvailable || gfxPath[0] == '\0') {
            state->ornamentCacheLoaded[mapIndex] = 1;
            return;
        }
        /* Find directory portion of GRAPHICS.DAT path */
        lastSlash = strrchr(gfxPath, '/');
        if (!lastSlash) lastSlash = strrchr(gfxPath, '\\');
        if (lastSlash) {
            dirLen = (int)(lastSlash - gfxPath);
        } else {
            dirLen = 0;
        }
        if (dirLen > 0) {
            snprintf(datPath, sizeof(datPath), "%.*s/DUNGEON.DAT", dirLen, gfxPath);
        } else {
            snprintf(datPath, sizeof(datPath), "DUNGEON.DAT");
        }
        fp = fopen(datPath, "rb");
        if (!fp && dirLen > 0) {
            snprintf(datPath, sizeof(datPath), "%.*s/dungeon.dat", dirLen, gfxPath);
            fp = fopen(datPath, "rb");
        }
        if (!fp) {
            state->ornamentCacheLoaded[mapIndex] = 1;
            return;
        }
    }

    /* Compute raw data section file offset (same formula as
     * F0501_DUNGEON_LoadTileData_Compat). */
    totalColumns = 0;
    for (i = 0; i < (int)dun->header.mapCount; ++i) {
        totalColumns += dun->maps[i].width;
    }
    thingDataTotalBytes = 0;
    for (i = 0; i < 16; ++i) {
        thingDataTotalBytes += (long)dun->header.thingCounts[i] *
                               (long)s_thingDataByteCount[i];
    }
    rawDataFileOffset = DUNGEON_HEADER_SIZE +
                        (long)dun->header.mapCount * DUNGEON_MAP_DESC_SIZE +
                        (long)totalColumns * 2 +
                        (long)dun->header.squareFirstThingCount * 2 +
                        thingDataTotalBytes;
    mapFileOffset = rawDataFileOffset + (long)m->rawMapDataByteOffset;

    /* Seek past square data to the metadata area */
    if (fseek(fp, mapFileOffset + (long)squareCount, SEEK_SET) != 0) {
        fclose(fp);
        state->ornamentCacheLoaded[mapIndex] = 1;
        return;
    }

    if ((int)fread(metaBuf, 1, (size_t)metaSize, fp) != metaSize) {
        fclose(fp);
        state->ornamentCacheLoaded[mapIndex] = 1;
        return;
    }
    fclose(fp);

    /* Parse: skip creature type bytes, then read wall ornament indices.
     * Each byte is a global ornament index used by the rendering engine.
     * Ref: ReDMCSB DUNGEON.C F0173_DUNGEON_ReadMapData_CPSE —
     *   CopyBytes(MapMetaData + CreatureTypeCount, G0261, WallOrnamentCount)
     *   G0261[InscriptionWallOrnamentIndex] = C0_WALL_ORNAMENT_INSCRIPTION */
    offset = (int)m->creatureTypeCount;
    for (i = 0; i < 16; ++i) {
        if (i < (int)m->wallOrnamentCount + 1 && (offset + i) < metaSize) {
            state->wallOrnamentIndices[mapIndex][i] = (int)metaBuf[offset + i];
        } else {
            state->wallOrnamentIndices[mapIndex][i] = i; /* identity fallback */
        }
    }
    offset += (int)m->wallOrnamentCount + 1; /* +1 for inscription ornament */

    /* Read floor ornament indices.
     * Ref: ReDMCSB DUNGEON.C F173 — G262_auc_CurrentMapFloorOrnamentIndices.
     * Each byte is a global floor ornament graphic set index.  In the
     * original, floor ornaments start at graphic index 247 and each
     * ornament has 6 perspective variants (D3L..D1R).
     * Floor ornament graphic = 247 + globalIndex * 6 + viewOffset. */
    for (i = 0; i < 16; ++i) {
        if (i < (int)m->floorOrnamentCount && (offset + i) < metaSize) {
            state->floorOrnamentIndices[mapIndex][i] = (int)metaBuf[offset + i];
        } else {
            state->floorOrnamentIndices[mapIndex][i] = -1; /* no ornament */
        }
    }
    offset += (int)m->floorOrnamentCount;

    /* Read door ornament indices */
    for (i = 0; i < 16; ++i) {
        if (i < (int)m->doorOrnamentCount && (offset + i) < metaSize) {
            state->doorOrnamentIndices[mapIndex][i] = (int)metaBuf[offset + i];
        } else {
            state->doorOrnamentIndices[mapIndex][i] = i; /* identity fallback */
        }
    }

    state->ornamentCacheLoaded[mapIndex] = 1;
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
    { int ci; for (ci = 0; ci < M11_MAX_CELL_CREATURES; ++ci) { cell.creatureTypes[ci] = -1; cell.creatureCountsPerGroup[ci] = 0; } }
    cell.creatureGroupCount = 0;
    cell.firstItemThingType = -1;
    cell.firstItemSubtype = -1;
    { int fi; for (fi = 0; fi < M11_MAX_CELL_ITEMS; ++fi) { cell.floorItemTypes[fi] = -1; cell.floorItemSubtypes[fi] = -1; } }
    cell.floorItemCount = 0;
    cell.wallOrnamentOrdinal = -1;
    cell.doorOrnamentOrdinal = -1;
    cell.firstProjectileGfxIndex = -1;
    cell.firstProjectileRelDir = -1;
    cell.firstProjectileCell = -1;
    cell.floorOrnamentOrdinal = 0;
    cell.firstExplosionType = -1;

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

    /* Extract all creature group types on this square (up to 4).
     * DM1 can have multiple creature groups on a single square; the
     * viewport draws them stacked/offset.  We also keep the legacy
     * single creatureType field pointing at the first group. */
    if (cell.summary.groups > 0 && state->world.things && state->world.things->groups) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_GROUP) {
                int gIdx = THING_GET_INDEX(scanThing);
                if (gIdx >= 0 && gIdx < state->world.things->groupCount &&
                    cell.creatureGroupCount < M11_MAX_CELL_CREATURES) {
                    int ct = (int)state->world.things->groups[gIdx].creatureType;
                    if (cell.creatureGroupCount == 0) {
                        cell.creatureType = ct; /* legacy compat */
                    }
                    cell.creatureTypes[cell.creatureGroupCount] = ct;
                    cell.creatureCountsPerGroup[cell.creatureGroupCount] =
                        (int)state->world.things->groups[gIdx].count + 1;
                    cell.creatureGroupCount++;
                }
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Extract floor item types and subtypes for sprite rendering.
     * Collect up to M11_MAX_CELL_ITEMS items for multi-item scatter.
     * The first one also populates the legacy single-item fields. */
    if (cell.summary.items > 0 && state->world.things) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE &&
               scanSafety < 64 && cell.floorItemCount < M11_MAX_CELL_ITEMS) {
            int tType = THING_GET_TYPE(scanThing);
            int tIdx = THING_GET_INDEX(scanThing);
            if (m11_thing_is_item(tType)) {
                int itemSubtype = -1;
                switch (tType) {
                    case THING_TYPE_WEAPON:
                        if (state->world.things->weapons && tIdx >= 0 && tIdx < state->world.things->weaponCount)
                            itemSubtype = (int)state->world.things->weapons[tIdx].type;
                        break;
                    case THING_TYPE_ARMOUR:
                        if (state->world.things->armours && tIdx >= 0 && tIdx < state->world.things->armourCount)
                            itemSubtype = (int)state->world.things->armours[tIdx].type;
                        break;
                    case THING_TYPE_POTION:
                        if (state->world.things->potions && tIdx >= 0 && tIdx < state->world.things->potionCount)
                            itemSubtype = (int)state->world.things->potions[tIdx].type;
                        break;
                    case THING_TYPE_JUNK:
                        if (state->world.things->junks && tIdx >= 0 && tIdx < state->world.things->junkCount)
                            itemSubtype = (int)state->world.things->junks[tIdx].type;
                        break;
                    case THING_TYPE_SCROLL:
                        itemSubtype = 0;
                        break;
                    case THING_TYPE_CONTAINER:
                        if (state->world.things->containers && tIdx >= 0 && tIdx < state->world.things->containerCount)
                            itemSubtype = (int)state->world.things->containers[tIdx].type;
                        break;
                    default:
                        itemSubtype = 0;
                        break;
                }
                if (cell.floorItemCount == 0) {
                    cell.firstItemThingType = tType;
                    cell.firstItemSubtype = itemSubtype;
                }
                cell.floorItemTypes[cell.floorItemCount] = tType;
                cell.floorItemSubtypes[cell.floorItemCount] = itemSubtype;
                cell.floorItemCount++;
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

    /* Extract first projectile graphic index from GRAPHICS.DAT.
     * In DM1, projectile graphics 416-438 map to the projectile's
     * associated object type.  The projectile thing's slot field
     * references the thrown object.  We use a simplified mapping:
     * projectile slot & 0x1F gives a subtype index into the 23
     * projectile graphic range.  Ref: ReDMCSB DUNGEON.C projectile
     * rendering, G0249_aui_PROJECTILE_GRAPHIC_INDEX lookup. */
    if (cell.summary.projectiles > 0 && state->world.things &&
        state->world.things->projectiles) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_PROJECTILE) {
                int pIdx = THING_GET_INDEX(scanThing);
                if (pIdx >= 0 && pIdx < state->world.things->projectileCount) {
                    /* Map projectile slot to graphic index 416..438 */
                    int slot = (int)state->world.things->projectiles[pIdx].slot;
                    int gfxOff = slot & 0x1F;
                    if (gfxOff > 22) gfxOff = 0; /* clamp to 23 entries */
                    cell.firstProjectileGfxIndex = 416 + gfxOff;
                }
                break;
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Extract first projectile direction from the runtime projectile list.
     * Match a runtime ProjectileInstance_Compat to this cell's map position
     * and compute the direction relative to party facing.
     * In DM1, projectile sprites are horizontally mirrored when the
     * missile's relative direction is 1 (right) vs 3 (left).  Missiles
     * heading toward (2) or away (0) from the party use the normal sprite.
     * Ref: ReDMCSB VIEWPORT.C projectile rendering direction logic. */
    if (cell.firstProjectileGfxIndex >= 0) {
        int pi;
        int partyMap = state->world.party.mapIndex;
        int partyDir = state->world.party.direction;
        for (pi = 0; pi < state->world.projectiles.count; ++pi) {
            const struct ProjectileInstance_Compat* rp = &state->world.projectiles.entries[pi];
            if (rp->slotIndex < 0) continue;
            if (rp->mapIndex == partyMap && rp->mapX == mapX && rp->mapY == mapY) {
                cell.firstProjectileRelDir = (rp->direction - partyDir) & 3;
                /* Extract sub-cell position (0-3) for DM1-faithful
                 * projectile viewport offset.  The cell field in the
                 * runtime data is absolute (0=NW,1=NE,2=SW,3=SE).
                 * We store the relative sub-cell: rotate by party
                 * facing so 0 = back-left, 1 = back-right,
                 * 2 = front-left, 3 = front-right from the party's
                 * perspective.  This matches DM1's view-cell mapping.
                 * Ref: ReDMCSB DUNVIEW.C L0139_i_Cell = M21_NORMALIZE(
                 *   A0126_i_ViewCell + P142_i_Direction). */
                cell.firstProjectileCell = (rp->cell - partyDir) & 3;
                break;
            }
        }
    }

    /* Extract first explosion type */
    if (cell.summary.explosions > 0 && state->world.things &&
        state->world.things->explosions) {
        unsigned short scanThing = firstThing;
        int scanSafety = 0;
        while (scanThing != THING_ENDOFLIST && scanThing != THING_NONE && scanSafety < 64) {
            if (THING_GET_TYPE(scanThing) == THING_TYPE_EXPLOSION) {
                int eIdx = THING_GET_INDEX(scanThing);
                if (eIdx >= 0 && eIdx < state->world.things->explosionCount) {
                    cell.firstExplosionType = (int)state->world.things->explosions[eIdx].type;
                }
                break;
            }
            scanThing = m11_raw_next_thing(state->world.things, scanThing);
            ++scanSafety;
        }
    }

    /* Extract floor ornament ordinal from random generation or sensor things.
     * In DM1, corridor/pit/stairs/teleporter squares can have floor ornaments
     * assigned via the deterministic random algorithm or sensor-placed overrides.
     * Ref: ReDMCSB DUNGEON.C F0172_DUNGEON_SetSquareAspect. */
    if (cell.valid && m11_viewport_cell_is_wall_free(cell.elementType)) {
        cell.floorOrnamentOrdinal = m11_compute_floor_ornament_ordinal(
            state, state->world.party.mapIndex, mapX, mapY, square);
    }

    if (outCell) {
        *outCell = cell;
    }
    return 1;
}

/* Helper: return 1 for element types that can have floor ornaments. */
static int m11_viewport_cell_is_wall_free(int elementType) {
    return elementType == DUNGEON_ELEMENT_CORRIDOR ||
           elementType == DUNGEON_ELEMENT_PIT ||
           elementType == DUNGEON_ELEMENT_STAIRS ||
           elementType == DUNGEON_ELEMENT_TELEPORTER;
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
        /* Show a dialog overlay so the text plaque is prominently
         * displayed; the player dismisses it with any key. */
        M11_GameView_ShowDialogOverlay(state, detail);
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
    M11_GameView_ProcessTickEmissions(state);
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

/* NOTE: g_drawState forward-declared near file top. */

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

    /* ── DM1-faithful Z-order: floor ornaments → floor items → creatures → projectiles ──
     * In the original, floor ornaments are drawn first (painted on the
     * floor texture), then floor items (lying on the ground), then
     * creatures (standing above the items), then projectiles and
     * explosions (in flight, topmost layer).
     * Ref: ReDMCSB DUNVIEW.C F115 — per-cell draw loop. */

    /* Layer 0: Floor ornaments (painted on the floor, below everything) */
    if (cell->floorOrnamentOrdinal > 0 && g_drawState) {
        M11_ViewRect floorRect;
        floorRect.x = faceX;
        floorRect.y = faceY;
        floorRect.w = faceW;
        floorRect.h = faceH;
        m11_draw_floor_ornament(g_drawState, framebuffer,
                                framebufferWidth, framebufferHeight,
                                &floorRect, cell->floorOrnamentOrdinal,
                                depthIndex, 0 /* center cell */);
    }

    /* Layer 1: Floor items (lowest physical objects — on the ground) */
    if (cell->floorItemCount > 0) {
        int ii;
        int itemsToShow = cell->floorItemCount;
        for (ii = 0; ii < itemsToShow; ++ii) {
            if (cell->floorItemTypes[ii] < 0) continue;
            if (!g_drawState ||
                !m11_draw_item_sprite(g_drawState, framebuffer,
                                      framebufferWidth, framebufferHeight,
                                      faceX + 2, faceY + 2, faceW - 4, faceH - 4,
                                      cell->floorItemTypes[ii],
                                      cell->floorItemSubtypes[ii],
                                      depthIndex)) {
                /* Fallback cue only for the first item to avoid clutter */
                if (ii == 0) {
                    m11_draw_item_cue(framebuffer, framebufferWidth, framebufferHeight,
                                      faceX + 2, faceY + 2, faceW - 4, faceH - 4,
                                      cell->summary.items);
                }
            }
        }
    }

    /* Layer 2: Creatures (standing on the floor, above items) */
    if (cell->creatureGroupCount > 0) {
        int gi;
        int groupCount = cell->creatureGroupCount;
        int slotW = (groupCount > 1) ? (faceW - 8) * 2 / (groupCount + 1) : faceW - 8;
        int slotH = faceH - 10;
        int stepX = (groupCount > 1) ? ((faceW - 8) - slotW) / (groupCount - 1) : 0;
        for (gi = 0; gi < groupCount; ++gi) {
            int cx = faceX + 4 + gi * stepX;
            int cy = faceY + 5;
            int countInGroup = cell->creatureCountsPerGroup[gi];
            int visibleDups, di;
            if (cell->creatureTypes[gi] < 0) continue;
            visibleDups = countInGroup;
            if (visibleDups > 4) visibleDups = 4;
            if (visibleDups < 1) visibleDups = 1;
            if (visibleDups == 1) {
                if (!g_drawState ||
                    !m11_draw_creature_sprite(g_drawState, framebuffer,
                                              framebufferWidth, framebufferHeight,
                                              cx, cy, slotW, slotH,
                                              cell->creatureTypes[gi], depthIndex)) {
                    m11_draw_creature_cue(framebuffer, framebufferWidth, framebufferHeight,
                                          cx, cy, slotW, slotH, depthIndex);
                }
            } else {
                int dupOffX[4], dupOffY[4];
                int dupW = slotW * 3 / 4;
                int dupH = slotH * 3 / 4;
                int ofsX = (slotW - dupW) / 2;
                int ofsY = (slotH - dupH) / 2;
                if (ofsX < 1) ofsX = 1;
                if (ofsY < 1) ofsY = 1;
                /* DM1 creature front-cell positioning from original
                 * Graphic558 coordinates. The stored points are center X and
                 * bottom Y in the 224x136 viewport, so convert them into the
                 * local face rectangle and then anchor the duplicate sprite
                 * by its bottom center. */
                {
                    int coordSet = m11_creature_coordinate_set(cell->creatureTypes[gi]);
                    int dIdx = depthIndex < 3 ? depthIndex : 2;
                    if (coordSet >= 0 && coordSet <= 2) {
                        for (di = 0; di < visibleDups && di < 4; ++di) {
                            int origCenterX;
                            int origBottomY;
                            int localCenterX;
                            int localBottomY;
                            M11_GameView_GetCreatureFrontSlotPoint(coordSet, dIdx,
                                                                   visibleDups, di,
                                                                   &origCenterX,
                                                                   &origBottomY);
                            localCenterX = (origCenterX * faceW) / 224;
                            localBottomY = (origBottomY * faceH) / 136;
                            dupOffX[di] = localCenterX - dupW / 2;
                            dupOffY[di] = localBottomY - dupH;
                            if (dupOffX[di] < 0) dupOffX[di] = 0;
                            if (dupOffY[di] < 0) dupOffY[di] = 0;
                            if (dupOffX[di] + dupW > slotW + ofsX * 2)
                                dupOffX[di] = slotW + ofsX * 2 - dupW;
                            if (dupOffY[di] + dupH > slotH + ofsY * 2)
                                dupOffY[di] = slotH + ofsY * 2 - dupH;
                        }
                    } else {
                        /* Fallback: grid layout */
                        dupOffX[0] = 0;        dupOffY[0] = 0;
                        dupOffX[1] = ofsX * 2; dupOffY[1] = 0;
                        dupOffX[2] = 0;        dupOffY[2] = ofsY * 2;
                        dupOffX[3] = ofsX * 2; dupOffY[3] = ofsY * 2;
                    }
                }
                for (di = 0; di < visibleDups; ++di) {
                    int dx = cx + dupOffX[di];
                    int dy = cy + dupOffY[di];
                    if (!g_drawState ||
                        !m11_draw_creature_sprite(g_drawState, framebuffer,
                                                  framebufferWidth, framebufferHeight,
                                                  dx, dy, dupW, dupH,
                                                  cell->creatureTypes[gi], depthIndex)) {
                        m11_draw_creature_cue(framebuffer, framebufferWidth, framebufferHeight,
                                              dx, dy, dupW, dupH, depthIndex);
                    }
                }
            }
            if (countInGroup > 1 && slotW >= 12 && slotH >= 12) {
                char countStr[4];
                int badgeX = cx + slotW - 8;
                int badgeY = cy + slotH - 8;
                snprintf(countStr, sizeof(countStr), "%d", countInGroup);
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              badgeX - 1, badgeY - 1, 9, 9, M11_COLOR_BLACK);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              badgeX, badgeY, countStr, &g_text_small);
            }
        }
    }

    /* Layer 3: Projectiles and explosions (in flight, topmost) */
    m11_draw_effect_cue(framebuffer, framebufferWidth, framebufferHeight,
                        faceX + 3, faceY + 3, faceW - 6, faceH - 6, cell,
                        depthIndex);
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

    /* Projectile viewport sprites (416-438, 23 entries).
     * In DM1 these are the small flying-object graphics drawn in the
     * corridor when a missile is in flight.  Sizes range from 9x7 to
     * 60x25.  Ref: ReDMCSB DEFS.H C416..C438. */
    M11_GFX_PROJECTILE_BASE = 416,
    M11_GFX_PROJECTILE_COUNT = 23,
    M11_GFX_PROJECTILE_END  = 439,

    /* Wall ornament graphics (per wall set, 16 ornaments each).
     * In CSB/DM, wall ornaments start at graphic index 321 and are
     * organized as 16 ornaments per wall set.  We map using the map's
     * wallSet + ornament ordinal. */
    M11_GFX_WALL_ORNAMENT_BASE = 101, /* first wall ornament graphic */
    M11_GFX_WALL_ORNAMENTS_PER_SET = 16,
    M11_GFX_DOOR_ORNAMENT_BASE = 165, /* first door ornament graphic */
    M11_GFX_DOOR_ORNAMENTS_PER_SET = 16,

    /* Floor ornament graphics.
     * In DM1, floor ornaments start at graphic index 247 and each
     * ornament has 6 perspective variants for the 6 visible floor
     * positions (D3L, D3C, D3R, D2L, D2C, D2R).
     * Graphic index = 247 + globalOrnamentIndex * 6 + viewOffset.
     * Ref: ReDMCSB DEFS.H C247_GRAPHIC_FIRST_FLOOR_ORNAMENT. */
    M11_GFX_FLOOR_ORNAMENT_BASE = 247,
    M11_GFX_FLOOR_ORNAMENT_VARIANTS = 6,

    /* Stair graphics */
    M11_GFX_STAIRS_DOWN_D2 = 93,  /* 33x136 */
    M11_GFX_STAIRS_DOWN_D1 = 95,  /* 60x111 */
    M11_GFX_STAIRS_UP_D2   = 94,  /* 33x136 */
    M11_GFX_STAIRS_UP_D1   = 96,  /* 60x111 */

    /* Champion icon strip (graphic 28 in original CSB/DM).
     * Horizontal strip of 4×4 = 16 directional icons, each 19×14.
     * Icon index = ((championCell - partyDirection) & 3) * 4 + frame.
     * Used for champion visual in the inventory portrait and HUD. */
    M11_GFX_CHAMPION_ICONS = 28,
    M11_CHAMPION_ICON_W    = 19,
    M11_CHAMPION_ICON_H    = 14,

    /* Spell area background (graphic 9 in original CSB/DM).
     * Drawn as the grid backdrop behind the 6 rune symbol buttons.
     * 87×25 in GRAPHICS.DAT.
     * Ref: DEFS.H line 2216 C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND.
     * Graphic 11 (C011_GRAPHIC_MENU_SPELL_AREA_LINES, 14×39) holds
     * the rune symbol line overlays drawn on top of this background. */
    M11_GFX_SPELL_AREA_BG = 9,
    M11_GFX_SPELL_AREA_LINES = 11,

    /* Action area background (graphic 10 in original CSB/DM).
     * 87×45 in GRAPHICS.DAT. Ref: C010_GRAPHIC_MENU_ACTION_AREA. */
    M11_GFX_ACTION_AREA = 10,

    /* Empty panel background (graphic 20 in original CSB/DM).
     * 144×73 in GRAPHICS.DAT. Ref: C020_GRAPHIC_PANEL_EMPTY. */
    M11_GFX_PANEL_EMPTY = 20,

    /* Slot box graphics (18×18 each in GRAPHICS.DAT).
     * Used by DrawIconInSlotBox (ReDMCSB INVNTORY.C / OBJECT.C).
     * Ref: DEFS.H lines 2193-2196. */
    M11_GFX_SLOT_BOX_NORMAL      = 33, /* C033_GRAPHIC_SLOT_BOX_NORMAL */
    M11_GFX_SLOT_BOX_WOUNDED     = 34, /* C034_GRAPHIC_SLOT_BOX_WOUNDED */
    M11_GFX_SLOT_BOX_ACTING_HAND = 35, /* C035_GRAPHIC_SLOT_BOX_ACTING_HAND */

    /* Champion status box frames (67×29 each in GRAPHICS.DAT).
     * Ref: DEFS.H lines 2171-2172, 2197-2199. */
    M11_GFX_STATUS_BOX           =  7, /* C007_GRAPHIC_STATUS_BOX */
    M11_GFX_STATUS_BOX_DEAD      =  8, /* C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION */

    /* Champion portrait strip (256×87 in GRAPHICS.DAT).
     * 8 columns × 3 rows of 32×29 portraits.
     * Ref: DEFS.H line 2186, M027_PORTRAIT_X, M028_PORTRAIT_Y. */
    M11_GFX_CHAMPION_PORTRAITS   = 26, /* C026_GRAPHIC_CHAMPION_PORTRAITS */
    M11_PORTRAIT_W               = 32,
    M11_PORTRAIT_H               = 29,

    /* Food / water / poisoned labels.
     * Ref: DEFS.H lines 2190-2192. */
    M11_GFX_FOOD_LABEL           = 30, /* C030_GRAPHIC_FOOD_LABEL (34×9) */
    M11_GFX_WATER_LABEL          = 31, /* C031_GRAPHIC_WATER_LABEL (46×9) */
    M11_GFX_POISONED_LABEL       = 32, /* C032_GRAPHIC_POISONED_LABEL (96×15) */

    /* Party shield border overlays (67×29 each in GRAPHICS.DAT).
     * Drawn on top of champion status box when party effect is active.
     * Ref: DEFS.H lines 2197-2199. */
    M11_GFX_BORDER_PARTY_SHIELD      = 37, /* C037 */
    M11_GFX_BORDER_PARTY_FIRESHIELD  = 38, /* C038 */
    M11_GFX_BORDER_PARTY_SPELLSHIELD = 39, /* C039 */

    /* Damage indicator overlays.
     * C014: 88×45, drawn on viewport when creature takes damage.
     * C015: 45×7,  drawn on champion status box for non-inventory champion.
     * C016: 32×29, drawn on inventory view for the active champion.
     * Ref: ReDMCSB CHAMPION.C F0291 / MELEE.C display code. */
    M11_GFX_DAMAGE_TO_CREATURE        = 14, /* C014_GRAPHIC_DAMAGE_TO_CREATURE (88×45) */
    M11_GFX_DAMAGE_TO_CHAMPION_SMALL  = 15, /* C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL (45×7) */
    M11_GFX_DAMAGE_TO_CHAMPION_BIG    = 16  /* C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG (32×29) */
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
 *
 * V1-faithful approach: instead of remapping palette indices through a
 * subjective dim_table, we set the per-pixel VGA palette brightness
 * level in the upper 4 bits of each framebuffer byte.  This matches
 * the original game, which achieves depth darkness by programming
 * different RGB values into the VGA DAC registers for the same colour
 * indices — same index, darker palette.  The RGBA conversion stage
 * in render_sdl_m11 reads the per-pixel level and looks up the
 * correct brightness from G9010_auc_VgaPaletteAll_Compat.
 *
 * Each depth pass adds 1 to the palette level, clamped to
 * M11_PALETTE_LEVELS-1 (the darkest).  Palette level 0 is brightest
 * (normal light), level 5 is the darkest. */
static void m11_apply_depth_dimming(unsigned char* framebuffer,
                                    int fbW,
                                    int fbH,
                                    int rx,
                                    int ry,
                                    int rw,
                                    int rh,
                                    int depthIndex) {
    int yy, xx;
    if (depthIndex <= 0) return;
    for (yy = ry; yy < ry + rh && yy < fbH; ++yy) {
        if (yy < 0) continue;
        for (xx = rx; xx < rx + rw && xx < fbW; ++xx) {
            unsigned char raw;
            int idx;
            int currentLevel;
            int newLevel;
            if (xx < 0) continue;
            raw = framebuffer[yy * fbW + xx];
            idx = M11_FB_DECODE_INDEX(raw);
            currentLevel = M11_FB_DECODE_LEVEL(raw);
            newLevel = currentLevel + depthIndex;
            if (newLevel >= M11_PALETTE_LEVELS) {
                newLevel = M11_PALETTE_LEVELS - 1;
            }
            framebuffer[yy * fbW + xx] = M11_FB_ENCODE(idx, newLevel);
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

/* Resolve an inventory thingId to a GRAPHICS.DAT item sprite index.
 * Returns the graphic index suitable for M11_AssetLoader_Load, or 0
 * if the thing type/subtype cannot be mapped.  This is the inventory
 * counterpart of the viewport floor-item m11_item_sprite_index(). */
static unsigned int m11_inventory_thing_sprite_index(
    const struct DungeonThings_Compat* things,
    unsigned short thingId) {
    int thingType, thingIndex, subtype;
    if (!things || thingId == THING_NONE || thingId == THING_ENDOFLIST) return 0;
    thingType  = THING_GET_TYPE(thingId);
    thingIndex = THING_GET_INDEX(thingId);
    subtype = 0;
    switch (thingType) {
        case THING_TYPE_WEAPON:
            if (things->weapons && thingIndex >= 0 && thingIndex < things->weaponCount)
                subtype = things->weapons[thingIndex].type;
            break;
        case THING_TYPE_ARMOUR:
            if (things->armours && thingIndex >= 0 && thingIndex < things->armourCount)
                subtype = things->armours[thingIndex].type;
            break;
        case THING_TYPE_SCROLL:
            return M11_GFX_ITEM_SCROLL_BASE;
        case THING_TYPE_POTION:
            if (things->potions && thingIndex >= 0 && thingIndex < things->potionCount)
                subtype = things->potions[thingIndex].type;
            break;
        case THING_TYPE_CONTAINER:
            if (things->containers && thingIndex >= 0 && thingIndex < things->containerCount)
                subtype = things->containers[thingIndex].type;
            break;
        case THING_TYPE_JUNK:
            if (things->junks && thingIndex >= 0 && thingIndex < things->junkCount)
                subtype = things->junks[thingIndex].type;
            break;
        default:
            return 0;
    }
    return m11_item_sprite_index(thingType, subtype);
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

    /* DM1-faithful floor item scatter placement.
     * In the original, items on the floor are placed in one of 4 sub-cell
     * positions (NW/NE/SW/SE) based on their thing-list position within
     * the square.  Since we only render the first item, we use the
     * subtype as a scatter seed to pick one of the 4 positions, giving
     * visual variety across different item types instead of always
     * centering them.  This matches the original's spatial distribution
     * where items rarely overlap exactly at the center. */
    {
        int scatter = ((unsigned int)(subtype + thingType)) & 3;
        int halfW = (w - drawW) / 2;
        int qx = halfW / 2;  /* quarter offset for scatter */
        int qy = 2;          /* small vertical scatter */
        switch (scatter) {
            case 0: /* NW quadrant */
                drawX = x + halfW - qx;
                drawY = y + h - drawH - 2 - qy;
                break;
            case 1: /* NE quadrant */
                drawX = x + halfW + qx;
                drawY = y + h - drawH - 2 - qy;
                break;
            case 2: /* SW quadrant */
                drawX = x + halfW - qx;
                drawY = y + h - drawH - 2 + qy;
                break;
            default: /* SE quadrant */
                drawX = x + halfW + qx;
                drawY = y + h - drawH - 2 + qy;
                break;
        }
    }

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
    int mapIdx;
    int ornGlobalIdx;

    if (!state || !state->assetsAvailable || ornamentOrdinal < 0) return 0;
    if (!state->world.dungeon) return 0;

    mapIdx = state->world.party.mapIndex;

    /* Ensure the per-map ornament index cache is loaded.
     * Cast away const: the cache is a lazy-init optimization that
     * doesn't change observable game state. */
    m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);

    /* Resolve the per-map ordinal to a global ornament index via
     * the cached G0261 table.  Fall back to ordinal if the cache
     * couldn't be loaded (identity mapping). */
    if (mapIdx >= 0 && mapIdx < (int)32 &&
        state->ornamentCacheLoaded[mapIdx] &&
        ornamentOrdinal < 16) {
        ornGlobalIdx = state->wallOrnamentIndices[mapIdx][ornamentOrdinal];
    } else {
        /* Fallback: direct mapping (old behaviour) */
        int wallSet = 0;
        if (mapIdx >= 0 && mapIdx < (int)state->world.dungeon->header.mapCount) {
            wallSet = (int)state->world.dungeon->maps[mapIdx].wallSet;
        }
        ornGlobalIdx = wallSet * M11_GFX_WALL_ORNAMENTS_PER_SET + ornamentOrdinal;
    }
    gfxIdx = (unsigned int)(M11_GFX_WALL_ORNAMENT_BASE + ornGlobalIdx);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    /* DM1-faithful wall ornament depth scaling.
     * The original uses specific perspective-scaled ornament sizes at
     * each depth level.  We approximate these as fractions of the wall
     * face rect, derived from ReDMCSB viewport geometry:
     *   depth 0 (nearest): ornament at ~50% of face
     *   depth 1 (mid):     ornament at ~38% of face
     *   depth 2 (far):     ornament at ~28% of face
     *   depth 3 (farthest): ornament at ~20% of face */
    {
        /* Scale numerator/denominator pairs per depth */
        static const int s_ornScaleNum[4] = { 50, 38, 28, 20 };
        int scaleIdx = depthIndex < 4 ? depthIndex : 3;
        ornW = rect->w * s_ornScaleNum[scaleIdx] / 100;
        ornH = (ornW * (int)slot->height) / (int)slot->width;
        if (ornH > rect->h * s_ornScaleNum[scaleIdx] / 100) {
            ornH = rect->h * s_ornScaleNum[scaleIdx] / 100;
            ornW = (ornH * (int)slot->width) / (int)slot->height;
        }
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
    int mapIdx;
    int ornGlobalIdx;

    if (!state || !state->assetsAvailable || ornamentOrdinal < 0) return 0;
    if (!state->world.dungeon) return 0;

    mapIdx = state->world.party.mapIndex;

    /* Ensure ornament cache and resolve via per-map door ornament table */
    m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);

    if (mapIdx >= 0 && mapIdx < (int)32 &&
        state->ornamentCacheLoaded[mapIdx] &&
        ornamentOrdinal < 16) {
        ornGlobalIdx = state->doorOrnamentIndices[mapIdx][ornamentOrdinal];
    } else {
        /* Fallback: direct mapping */
        int doorSet = 0;
        if (mapIdx >= 0 && mapIdx < (int)state->world.dungeon->header.mapCount) {
            doorSet = (int)state->world.dungeon->maps[mapIdx].doorSet0;
        }
        ornGlobalIdx = doorSet * M11_GFX_DOOR_ORNAMENTS_PER_SET + ornamentOrdinal;
    }
    gfxIdx = (unsigned int)(M11_GFX_DOOR_ORNAMENT_BASE + ornGlobalIdx);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    /* DM1-faithful door ornament depth scaling.
     * Door ornaments scale with perspective similarly to wall ornaments
     * but are slightly smaller (centered on the door panel).
     *   depth 0: ~40%, depth 1: ~30%, depth 2: ~22%, depth 3: ~16% */
    {
        static const int s_doorOrnScaleNum[4] = { 40, 30, 22, 16 };
        int scaleIdx = depthIndex < 4 ? depthIndex : 3;
        ornW = rect->w * s_doorOrnScaleNum[scaleIdx] / 100;
        ornH = (ornW * (int)slot->height) / (int)slot->width;
        if (ornH > rect->h * s_doorOrnScaleNum[scaleIdx] / 100) {
            ornH = rect->h * s_doorOrnScaleNum[scaleIdx] / 100;
            ornW = (ornH * (int)slot->width) / (int)slot->height;
        }
    }
    if (ornW < 3 || ornH < 3) return 0;

    drawX = rect->x + (rect->w - ornW) / 2;
    drawY = rect->y + (rect->h - ornH) / 2;

    M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                               drawX, drawY, ornW, ornH, 0);
    return 1;
}

/* Draw a floor ornament from GRAPHICS.DAT at the given viewport position.
 * In DM1, floor ornaments are drawn on the floor area of open cells
 * (corridor, pit, stairs, teleporter).  Each ornament graphic set has
 * 6 perspective variants indexed by the visible floor position:
 *   0=D3L, 1=D3C, 2=D3R, 3=D2L, 4=D2C, 5=D2R
 * For D1 (depth 0) cells, DM1 uses variant 4 (D2C) scaled up.
 * The global ornament index is resolved via the per-map floor ornament
 * index table (G0261 analog for floor ornaments).
 * Returns 1 if a real ornament was drawn.
 * Ref: ReDMCSB DUNVIEW.C F115 floor ornament rendering path. */
static int m11_draw_floor_ornament(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int fbW,
                                   int fbH,
                                   const M11_ViewRect* rect,
                                   int ornamentOrdinal,
                                   int depthIndex,
                                   int sideHint) {
    unsigned int gfxIdx;
    const M11_AssetSlot* slot;
    int ornW, ornH, drawX, drawY;
    int mapIdx;
    int ornGlobalIdx;
    int variant;

    if (!state || !state->assetsAvailable || ornamentOrdinal <= 0) return 0;
    if (!state->world.dungeon) return 0;

    mapIdx = state->world.party.mapIndex;

    /* Ensure the per-map ornament index cache is loaded. */
    m11_ensure_ornament_cache((M11_GameViewState*)state, mapIdx);

    /* Resolve per-map ordinal (1-based) to global ornament index.
     * ornamentOrdinal - 1 gives the 0-based per-map index. */
    {
        int localIdx = ornamentOrdinal - 1;
        if (mapIdx >= 0 && mapIdx < (int)32 &&
            state->ornamentCacheLoaded[mapIdx] &&
            localIdx >= 0 && localIdx < 16) {
            ornGlobalIdx = state->floorOrnamentIndices[mapIdx][localIdx];
        } else {
            ornGlobalIdx = localIdx; /* fallback: identity mapping */
        }
    }
    if (ornGlobalIdx < 0) return 0;

    /* Select perspective variant based on depth and lateral position.
     * DM1 variant mapping:
     *   D3: side<0 → 0(D3L), side==0 → 1(D3C), side>0 → 2(D3R)
     *   D2: side<0 → 3(D2L), side==0 → 4(D2C), side>0 → 5(D2R)
     *   D1: use variant 4 (D2C, scaled larger) */
    if (depthIndex >= 2) {
        variant = (sideHint < 0) ? 0 : (sideHint > 0) ? 2 : 1;
    } else if (depthIndex == 1) {
        variant = (sideHint < 0) ? 3 : (sideHint > 0) ? 5 : 4;
    } else {
        variant = 4; /* D1: reuse D2C variant scaled up */
    }

    gfxIdx = (unsigned int)(M11_GFX_FLOOR_ORNAMENT_BASE +
                            ornGlobalIdx * M11_GFX_FLOOR_ORNAMENT_VARIANTS + variant);
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, gfxIdx);
    if (!slot || slot->width == 0 || slot->height == 0) return 0;

    /* Scale ornament to fit the floor area of the cell rect.
     * Floor ornaments occupy the bottom portion of the cell face.
     *   depth 0: ~60% width, placed in bottom 40% of face
     *   depth 1: ~50% width, placed in bottom 35%
     *   depth 2: ~40% width, placed in bottom 30% */
    {
        static const int s_floorOrnScaleW[3] = { 60, 50, 40 };
        static const int s_floorOrnScaleH[3] = { 40, 35, 30 };
        int scaleIdx = depthIndex < 3 ? depthIndex : 2;
        int maxW = rect->w * s_floorOrnScaleW[scaleIdx] / 100;
        int maxH = rect->h * s_floorOrnScaleH[scaleIdx] / 100;
        ornW = maxW;
        ornH = (ornW * (int)slot->height) / (int)slot->width;
        if (ornH > maxH) {
            ornH = maxH;
            ornW = (ornH * (int)slot->width) / (int)slot->height;
        }
    }
    if (ornW < 3 || ornH < 3) return 0;

    /* Center horizontally in the cell, place at the bottom (floor area) */
    drawX = rect->x + (rect->w - ornW) / 2;
    drawY = rect->y + rect->h - ornH - 2;

    /* Side offset: shift toward the inner corridor edge */
    if (sideHint < 0) drawX += rect->w / 6;
    else if (sideHint > 0) drawX -= rect->w / 6;

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
/* Forward declaration for extended creature sprite draw. */
static int m11_draw_creature_sprite_ex(const M11_GameViewState* state,
                                       unsigned char* framebuffer,
                                       int fbW, int fbH,
                                       int x, int y, int w, int h,
                                       int creatureType,
                                       int depthIndex,
                                       int sideHint);

static unsigned int m11_creature_sprite_base(int creatureType) {
    /* ReDMCSB creature graphic set mapping.
     * Each creature type has a specific graphic set index (column 2 in
     * G0243_as_Graphic559_CreatureInfo).  Each set has 3 sprites:
     * small, medium, large at consecutive GRAPHICS.DAT indices.
     * Sets 0-3: base 246 + set*3 (indices 246-257)
     * Sets 4-18: irregular spacing in the 439+ range.
     * Ref: ReDMCSB DEFS.H, G0243_as_Graphic559_CreatureInfo. */
    static const unsigned int s_set_bases[] = {
        246,  /* set 0 */
        249,  /* set 1 */
        252,  /* set 2 */
        255,  /* set 3 */
        439,  /* set 4 */
        442,  /* set 5 */
        445,  /* set 6 */
        448,  /* set 7 */
        451,  /* set 8 */
        454,  /* set 9 */
        457,  /* set 10 */
        460,  /* set 11 */
        463,  /* set 12 */
        466,  /* set 13 */
        469,  /* set 14 */
        472,  /* set 15 */
        475,  /* set 16 */
        478,  /* set 17 */
        481   /* set 18 */
    };
    /* Creature type -> graphic set index from ReDMCSB
     * G0243_as_Graphic559_CreatureInfo (second column). */
    static const int s_type_to_set[27] = {
         4,  /* 0  GiantScorpionScorpion */
        14,  /* 1  SwampSlimeSlimeDevil */
         6,  /* 2  Giggler */
         0,  /* 3  PainRatHellHound */
        18,  /* 4  Ruster */
        17,  /* 5  Screamer */
         3,  /* 6  Rockpile */
         7,  /* 7  GhostRive */
         2,  /* 8  WaterElemental */
        10,  /* 9  Couatl */
        13,  /* 10 StoneGolem */
         0,  /* 11 Mummy */
        11,  /* 12 Skeleton */
         9,  /* 13 MagentaWormWorm */
        16,  /* 14 Trolin */
         5,  /* 15 GiantWaspMuncher */
        10,  /* 16 Antman */
        15,  /* 17 Vexirk */
        12,  /* 18 AnimatedArmourDethKnight */
         0,  /* 19 MaterializerZytaz */
         8,  /* 20 RedDragon */
         3,  /* 21 Oitu */
        16,  /* 22 Demon */
         0,  /* 23 LordChaos */
         1,  /* 24 LordOrder */
         0,  /* 25 GreyLord */
         0   /* 26 LordChaosRedDragon */
    };
    int setIdx;
    if (creatureType < 0 || creatureType > 26) return 0;
    setIdx = s_type_to_set[creatureType];
    if (setIdx < 0 || setIdx >= (int)(sizeof(s_set_bases)/sizeof(s_set_bases[0]))) return 0;
    return s_set_bases[setIdx];
}

/* Draw a creature sprite from GRAPHICS.DAT at the given viewport position.
 * depthIndex 0 = near (large sprite), 1 = mid, 2 = far (small sprite).
 * sideHint: 0 = center, -1 = left side cell, +1 = right side cell.
 * When sideHint != 0 the sprite is drawn mirrored so creatures
 * appear to face inward toward the corridor center.
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
    return m11_draw_creature_sprite_ex(state, framebuffer, fbW, fbH,
                                       x, y, w, h, creatureType,
                                       depthIndex, 0);
}

static int m11_draw_creature_sprite_ex(const M11_GameViewState* state,
                                       unsigned char* framebuffer,
                                       int fbW,
                                       int fbH,
                                       int x,
                                       int y,
                                       int w,
                                       int h,
                                       int creatureType,
                                       int depthIndex,
                                       int sideHint) {
    unsigned int base;
    unsigned int spriteIdx;
    const M11_AssetSlot* slot;
    int spriteW, spriteH;
    int drawW, drawH, drawX, drawY;
    int useAttackPose = 0;
    int useMirror = 0;

    if (!state->assetsAvailable || creatureType < 0) return 0;
    base = m11_creature_sprite_base(creatureType);
    if (base == 0) return 0;

    /* Check if this creature is currently attacking (attack cue active
     * and matches the creature type at depth 0).
     * In DM1, the attack animation applies to ALL sprites of the
     * attacking creature type in the front cell row (depth 0),
     * including side-pane creatures.  Restricting the attack pose
     * to center-only was a fidelity gap. */
    if (state->attackCueTimer > 0 &&
        state->attackCueCreatureType == creatureType &&
        depthIndex == 0) {
        useAttackPose = 1;
    }

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

    /* Side creatures are mirrored: left side = normal (facing right),
     * right side = mirrored (facing left), so they both face inward. */
    useMirror = (sideHint > 0) ? 1 : 0;

    /* Scale to fit within the face rect while preserving aspect ratio.
     * DM1 perspective fidelity: side-cell creatures are drawn smaller
     * than center-cell creatures at the same depth.  In the original,
     * side panes are narrower (roughly 60-70% of center width) and
     * creatures shrink proportionally.  We apply a 70% scale factor
     * for side cells to match the corridor perspective illusion.
     * At depth 1+ the side panes are even narrower, so we apply an
     * additional 80% reduction per extra depth step for side cells. */
    {
        int maxW = w - 4;
        int maxH = h - 4;
        if (sideHint != 0) {
            /* DM1 side-cell perspective proportion: ~70% of center */
            maxW = maxW * 70 / 100;
            maxH = maxH * 70 / 100;
            /* Further reduce at greater depth for side cells */
            if (depthIndex >= 1) {
                maxW = maxW * 80 / 100;
                maxH = maxH * 80 / 100;
            }
            if (depthIndex >= 2) {
                maxW = maxW * 80 / 100;
                maxH = maxH * 80 / 100;
            }
        }
        drawW = maxW;
        drawH = (drawW * spriteH) / spriteW;
        if (drawH > maxH) {
            drawH = maxH;
            drawW = (drawH * spriteW) / spriteH;
        }
    }
    if (drawW < 4 || drawH < 4) return 0;

    /* DM1 side-cell positioning: offset toward the corridor center.
     * Left-side creatures shift right, right-side shift left, so they
     * appear at the inner edge of the side pane.  Center creatures
     * remain centered. */
    if (sideHint < 0) {
        /* Left side: push toward right (inner) edge */
        drawX = x + w - drawW - 2;
    } else if (sideHint > 0) {
        /* Right side: push toward left (inner) edge */
        drawX = x + 2;
    } else {
        drawX = x + (w - drawW) / 2;
    }
    drawY = y + (h - drawH) / 2;

    /* Attack cue should keep creature identity stable. Use a subtle
     * forward lunge on the same sprite instead of swapping graphic sets. */
    if (useAttackPose) {
        int lungeW = drawW / 8;
        int lungeH = drawH / 10;
        if (lungeW < 1) lungeW = 1;
        if (lungeH < 1) lungeH = 1;
        if (drawW + lungeW <= w) {
            drawX -= lungeW / 2;
            drawW += lungeW;
        }
        if (drawH + lungeH <= h) {
            drawY -= lungeH;
            drawH += lungeH;
        }
    }

    /* Idle animation: on frame 1, bob the creature up by 1-2 pixels
     * to give a breathing / pulsing effect.  Frame 0 is the base pose.
     * Attack pose skips the bob for a snappier look. */
    if (!useAttackPose) {
        int animFrame = M11_GameView_CreatureAnimFrame(state, creatureType);
        if (animFrame == 1) {
            int bob = (depthIndex == 0) ? 2 : 1;
            drawY -= bob;
        }
    }

    /* Use creature aspect transparent color for sprite compositing.
     * In DM1, each creature type specifies its transparent (key) color
     * index via the CoordinateSet_TransparentColor field.  Most creatures
     * use color index 10 (light green / magenta), but some use 0 (black)
     * or other indices.  Ref: ReDMCSB DEFS.H M072_TRANSPARENT_COLOR. */
    {
        int transpColor = m11_creature_transparent_color(creatureType);
        if (useMirror) {
            M11_AssetLoader_BlitScaledMirror(slot, framebuffer, fbW, fbH,
                                             drawX, drawY, drawW, drawH, transpColor);
        } else {
            M11_AssetLoader_BlitScaled(slot, framebuffer, fbW, fbH,
                                       drawX, drawY, drawW, drawH, transpColor);
        }
    }
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

    /* Side-pane wall ornaments: DM1 renders wall ornaments on side walls */
    if (cell->elementType == DUNGEON_ELEMENT_WALL && cell->wallOrnamentOrdinal >= 0 && g_drawState) {
        M11_ViewRect sideWallOrnRect;
        sideWallOrnRect.x = paneX;
        sideWallOrnRect.y = paneY;
        sideWallOrnRect.w = paneW;
        sideWallOrnRect.h = paneH;
        m11_draw_wall_ornament(g_drawState, framebuffer,
                               framebufferWidth, framebufferHeight,
                               &sideWallOrnRect, cell->wallOrnamentOrdinal,
                               depthIndex);
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
        /* Draw door ornament on side-visible doors.
         * DM1 renders door ornaments on the side-view door frame
         * when the door has a decorative element (button, keyhole, etc.). */
        if (cell->doorOrnamentOrdinal >= 0 && g_drawState) {
            M11_ViewRect sideOrnRect;
            sideOrnRect.x = paneX;
            sideOrnRect.y = paneY;
            sideOrnRect.w = paneW;
            sideOrnRect.h = paneH;
            m11_draw_door_ornament(g_drawState, framebuffer,
                                   framebufferWidth, framebufferHeight,
                                   &sideOrnRect, cell->doorOrnamentOrdinal,
                                   depthIndex);
        }
        /* Re-draw the accent border on top so the door element
         * accent colour stays visible in probe invariants. */
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      paneX, paneY, paneW, paneH, accent);
    }

    if (m11_viewport_cell_is_open(cell)) {
        /* Floor ornaments in side cells (below items and creatures) */
        if (cell->floorOrnamentOrdinal > 0 && g_drawState) {
            M11_ViewRect sideFloorRect;
            sideFloorRect.x = paneX;
            sideFloorRect.y = paneY;
            sideFloorRect.w = paneW;
            sideFloorRect.h = paneH;
            m11_draw_floor_ornament(g_drawState, framebuffer,
                                    framebufferWidth, framebufferHeight,
                                    &sideFloorRect, cell->floorOrnamentOrdinal,
                                    depthIndex, side);
        }
        /* Multi-creature stacking with duplication in side cells.
         * Duplicate sprites with vertical offsets in narrow pane. */
        if (cell->creatureGroupCount > 0) {
            int gi;
            int groupCount = cell->creatureGroupCount;
            int slotH = (groupCount > 1)
                ? (paneH - 2) * 2 / (groupCount + 1)
                : paneH - 2;
            int stepY = (groupCount > 1)
                ? ((paneH - 2) - slotH) / (groupCount - 1)
                : 0;
            for (gi = 0; gi < groupCount; ++gi) {
                int cy = paneY + 1 + gi * stepY;
                int countInGroup = cell->creatureCountsPerGroup[gi];
                int visibleDups, di;
                if (cell->creatureTypes[gi] < 0) continue;
                visibleDups = countInGroup;
                if (visibleDups > 3) visibleDups = 3;
                if (visibleDups < 1) visibleDups = 1;
                if (visibleDups == 1) {
                    if (!g_drawState ||
                        !m11_draw_creature_sprite_ex(g_drawState, framebuffer,
                                                     framebufferWidth, framebufferHeight,
                                                     paneX + 1, cy,
                                                     paneW - 2, slotH,
                                                     cell->creatureTypes[gi], depthIndex,
                                                     side)) {
                        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                      paneX + paneW / 2 - 1, cy + slotH / 2 - 2,
                                      3, 5, depthIndex == 0 ? M11_COLOR_LIGHT_GREEN : M11_COLOR_GREEN);
                    }
                } else {
                    int dupH = slotH * 2 / 3;
                    int ofsY = (slotH - dupH) / (visibleDups > 1 ? visibleDups - 1 : 1);
                    if (ofsY < 1) ofsY = 1;
                    for (di = 0; di < visibleDups; ++di) {
                        int dy = cy + di * ofsY;
                        if (!g_drawState ||
                            !m11_draw_creature_sprite_ex(g_drawState, framebuffer,
                                                         framebufferWidth, framebufferHeight,
                                                         paneX + 1, dy,
                                                         paneW - 2, dupH,
                                                         cell->creatureTypes[gi], depthIndex,
                                                         side)) {
                            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                          paneX + paneW / 2 - 1, dy + dupH / 2 - 2,
                                          3, 5, depthIndex == 0 ? M11_COLOR_LIGHT_GREEN : M11_COLOR_GREEN);
                        }
                    }
                }
                if (countInGroup > 1 && paneW >= 10 && slotH >= 10) {
                    char countStr[4];
                    int badgeX = paneX + paneW - 9;
                    int badgeY = cy + slotH - 8;
                    snprintf(countStr, sizeof(countStr), "%d", countInGroup);
                    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                  badgeX - 1, badgeY - 1, 9, 9, M11_COLOR_BLACK);
                    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                  badgeX, badgeY, countStr, &g_text_small);
                }
            }
        }
        /* ── Side-pane item sprites ──
         * DM1 draws item sprites in side cells similarly to front cells,
         * just smaller to fit the narrower pane. */
        if (cell->floorItemCount > 0) {
            int ii;
            int itemArea = paneH / 3;
            int itemBaseY = paneY + paneH - itemArea - 2;
            for (ii = 0; ii < cell->floorItemCount; ++ii) {
                if (cell->floorItemTypes[ii] < 0) continue;
                if (!g_drawState ||
                    !m11_draw_item_sprite(g_drawState, framebuffer,
                                          framebufferWidth, framebufferHeight,
                                          paneX + 1, itemBaseY,
                                          paneW - 2, itemArea,
                                          cell->floorItemTypes[ii],
                                          cell->floorItemSubtypes[ii],
                                          depthIndex + 1)) {
                    if (ii == 0) {
                        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                      paneX + paneW / 2 - 2, paneY + paneH - 4,
                                      5, 2, M11_COLOR_YELLOW);
                    }
                }
            }
        } else if (cell->summary.items > 0) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          paneX + paneW / 2 - 2, paneY + paneH - 4,
                          5, 2, M11_COLOR_YELLOW);
        }
        /* Side-pane projectile sprites: real GRAPHICS.DAT sprites */
        if (cell->summary.projectiles > 0) {
            int projArea = paneH / 3;
            int projY = paneY + (paneH - projArea) / 2;
            if (projArea < 6) projArea = 6;
            if (!g_drawState ||
                !m11_draw_projectile_sprite(g_drawState, framebuffer,
                                            framebufferWidth, framebufferHeight,
                                            paneX + 1, projY,
                                            paneW - 2, projArea,
                                            cell->firstProjectileGfxIndex,
                                            depthIndex + 1,
                                            cell->firstProjectileRelDir,
                                            cell->firstProjectileCell)) {
                int pcx = paneX + paneW / 2;
                int pcy = paneY + paneH / 2;
                m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                               pcx - 2, pcx + 2, pcy, M11_COLOR_LIGHT_CYAN);
                m11_draw_vline(framebuffer, framebufferWidth, framebufferHeight,
                               pcx, pcy - 2, pcy + 2, M11_COLOR_LIGHT_CYAN);
            }
        }
        if (cell->summary.explosions > 0 && cell->summary.projectiles == 0) {
            int ecx = paneX + paneW / 2;
            int ecy = paneY + paneH / 2;
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          ecx - 1, ecy - 1, 3, 3, M11_COLOR_LIGHT_RED);
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
        int totalCreatures = 0;
        int gi;
        for (gi = 0; gi < cell->creatureGroupCount && gi < M11_MAX_CELL_CREATURES; ++gi) {
            totalCreatures += cell->creatureCountsPerGroup[gi];
        }
        if (totalCreatures > 0) {
            snprintf(out, outSize, "%s %dC", prefix ? prefix : "?", totalCreatures);
        } else {
            snprintf(out, outSize, "%s %dG", prefix ? prefix : "?", cell->summary.groups);
        }
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

/* ================================================================
 * Light-level computation
 *
 * Combines magical light (FUL spell, MAGIC_TORCH), lit torches in
 * champion hand slots, and the ILLUMULET junk item to produce a
 * 0..255 light level that drives viewport depth dimming.
 *
 * Light sources (additive, clamped to 255):
 *   - magicalLightAmount from world.magic (FUL/OH spell chain)
 *   - Each lit TORCH weapon in a hand slot:  +80
 *   - ILLUMULET junk in any hand slot:       +50
 *   - FLAMITT weapon (index 3) lit:          +100
 * ================================================================ */

#define M11_LIGHT_TORCH_BONUS       80
#define M11_LIGHT_ILLUMULET_BONUS   50
#define M11_LIGHT_FLAMITT_BONUS     100
#define M11_LIGHT_MAX               255

/* Torch weapon sub-type index in s_weaponTypeNames[] */
#define M11_WEAPON_SUBTYPE_TORCH    2
#define M11_WEAPON_SUBTYPE_FLAMITT  3
/* ILLUMULET junk sub-type index in s_junkTypeNames[] */
#define M11_JUNK_SUBTYPE_ILLUMULET  4

static int m11_compute_light_level(const M11_GameViewState* state) {
    int light;
    int ci;
    if (!state) {
        return 0;
    }

    /* Start with the magical light amount from the spell system. */
    light = state->world.magic.magicalLightAmount;
    if (light < 0) light = 0;

    /* Scan each champion's hand slots for light-emitting items. */
    for (ci = 0; ci < CHAMPION_MAX_PARTY; ++ci) {
        const struct ChampionState_Compat* champ = &state->world.party.champions[ci];
        int slot;
        if (!champ->present) continue;

        for (slot = CHAMPION_SLOT_HAND_LEFT; slot <= CHAMPION_SLOT_HAND_RIGHT; ++slot) {
            unsigned short thing = champ->inventory[slot];
            int thingType, thingIndex;
            if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;

            thingType = THING_GET_TYPE(thing);
            thingIndex = THING_GET_INDEX(thing);

            if (thingType == THING_TYPE_WEAPON &&
                state->world.things &&
                thingIndex >= 0 && thingIndex < state->world.things->weaponCount) {
                const struct DungeonWeapon_Compat* w = &state->world.things->weapons[thingIndex];
                if (w->type == M11_WEAPON_SUBTYPE_TORCH && w->lit) {
                    /* Scale light by remaining fuel fraction */
                    int fuel = (thingIndex >= 0 && thingIndex < M11_TORCH_FUEL_CAPACITY)
                               ? state->torchFuel[thingIndex] : M11_TORCH_INITIAL_FUEL;
                    int bonus = (M11_LIGHT_TORCH_BONUS * fuel) / M11_TORCH_INITIAL_FUEL;
                    if (bonus < 1 && fuel > 0) bonus = 1;
                    light += bonus;
                }
                if (w->type == M11_WEAPON_SUBTYPE_FLAMITT && w->lit) {
                    int fuel = (thingIndex >= 0 && thingIndex < M11_TORCH_FUEL_CAPACITY)
                               ? state->torchFuel[thingIndex] : M11_FLAMITT_INITIAL_FUEL;
                    int bonus = (M11_LIGHT_FLAMITT_BONUS * fuel) / M11_FLAMITT_INITIAL_FUEL;
                    if (bonus < 1 && fuel > 0) bonus = 1;
                    light += bonus;
                }
            }

            if (thingType == THING_TYPE_JUNK &&
                state->world.things &&
                thingIndex >= 0 && thingIndex < state->world.things->junkCount) {
                const struct DungeonJunk_Compat* j = &state->world.things->junks[thingIndex];
                if (j->type == M11_JUNK_SUBTYPE_ILLUMULET) {
                    light += M11_LIGHT_ILLUMULET_BONUS;
                }
            }
        }
    }

    if (light > M11_LIGHT_MAX) light = M11_LIGHT_MAX;
    return light;
}

/* Query the current party light level (0..255). Exposed for probes. */
int M11_GameView_GetLightLevel(const M11_GameViewState* state) {
    return m11_compute_light_level(state);
}

/* ================================================================
 * Torch fuel burn-down
 * ================================================================ */

int M11_GameView_GetTorchFuel(const M11_GameViewState* state, int weaponIndex) {
    if (!state || weaponIndex < 0 || weaponIndex >= M11_TORCH_FUEL_CAPACITY) {
        return 0;
    }
    return state->torchFuel[weaponIndex];
}

void M11_GameView_UpdateTorchFuel(M11_GameViewState* state) {
    int ci;
    if (!state || !state->active || !state->world.things) {
        return;
    }

    /* Scan all champion hand slots for lit torches/flamitts. */
    for (ci = 0; ci < CHAMPION_MAX_PARTY; ++ci) {
        struct ChampionState_Compat* champ = &state->world.party.champions[ci];
        int slot;
        if (!champ->present) continue;

        for (slot = CHAMPION_SLOT_HAND_LEFT; slot <= CHAMPION_SLOT_HAND_RIGHT; ++slot) {
            unsigned short thing = champ->inventory[slot];
            int thingType, thingIndex;
            if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;

            thingType = THING_GET_TYPE(thing);
            thingIndex = THING_GET_INDEX(thing);

            if (thingType != THING_TYPE_WEAPON) continue;
            if (thingIndex < 0 || thingIndex >= state->world.things->weaponCount) continue;
            if (thingIndex >= M11_TORCH_FUEL_CAPACITY) continue;

            {
                struct DungeonWeapon_Compat* w = &state->world.things->weapons[thingIndex];
                int isTorch = (w->type == M11_WEAPON_SUBTYPE_TORCH);
                int isFlamitt = (w->type == M11_WEAPON_SUBTYPE_FLAMITT);

                if (!isTorch && !isFlamitt) continue;
                if (!w->lit) continue;

                /* Initialize fuel on first encounter */
                if (!state->torchFuelInitialized[thingIndex]) {
                    state->torchFuel[thingIndex] = isTorch
                        ? M11_TORCH_INITIAL_FUEL
                        : M11_FLAMITT_INITIAL_FUEL;
                    state->torchFuelInitialized[thingIndex] = 1;
                }

                /* Burn one unit of fuel */
                state->torchFuel[thingIndex]--;

                /* Log when fuel is low */
                if (state->torchFuel[thingIndex] == (M11_TORCH_INITIAL_FUEL / 4) && isTorch) {
                    m11_log_event(state, M11_COLOR_YELLOW,
                                  "T%u: TORCH DIMS",
                                  (unsigned int)state->world.gameTick);
                }
                if (state->torchFuel[thingIndex] == (M11_FLAMITT_INITIAL_FUEL / 4) && isFlamitt) {
                    m11_log_event(state, M11_COLOR_YELLOW,
                                  "T%u: FLAMITT DIMS",
                                  (unsigned int)state->world.gameTick);
                }

                /* Extinguish when fuel is exhausted */
                if (state->torchFuel[thingIndex] <= 0) {
                    state->torchFuel[thingIndex] = 0;
                    state->torchFuelInitialized[thingIndex] = 0;
                    w->lit = 0;
                    m11_log_event(state, M11_COLOR_LIGHT_RED,
                                  "T%u: %s BURNS OUT",
                                  (unsigned int)state->world.gameTick,
                                  isTorch ? "TORCH" : "FLAMITT");
                }
            }
        }
    }
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

    m11_get_active_champion_label(state, champion, sizeof(champion));

    if (state->showDebugHUD) {
        /* Debug mode: show MENU label, source kind, full metadata */
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      222, 34, "MENU", &g_text_small);
        snprintf(line, sizeof(line), "%s %s", m11_source_name(state->sourceKind), champion);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      250, 34, line, &g_text_small);
    } else {
        /* V1 mode: champion name only, centered in panel */
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      222, 34, champion, &g_text_small);
    }

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

    if (state->showDebugHUD) {
        /* Debug mode: show I/S/L button labels */
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

    /* Light level indicator: a small bar and label below the buttons.
     * Bright = YELLOW, normal = BROWN, dim = DARK_GRAY, dark = BLACK. */
    {
        int lightLevel = m11_compute_light_level(state);
        unsigned char lightColor;
        const char* lightLabel;
        int barW;
        if (lightLevel >= 200) {
            lightColor = M11_COLOR_YELLOW;
            lightLabel = "BRIGHT";
        } else if (lightLevel >= 120) {
            lightColor = M11_COLOR_BROWN;
            lightLabel = "LIT";
        } else if (lightLevel >= 50) {
            lightColor = M11_COLOR_DARK_GRAY;
            lightLabel = "DIM";
        } else {
            lightColor = M11_COLOR_BLACK;
            lightLabel = "DARK";
        }
        /* Draw light bar (max 80px wide, scaled to light level) */
        barW = (lightLevel * 80) / M11_LIGHT_MAX;
        if (barW < 1 && lightLevel > 0) barW = 1;
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      222, 68, 80, 3, M11_COLOR_BLACK);
        if (barW > 0) {
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          222, 68, barW, 3, lightColor);
        }
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      222, 73, lightLabel, &g_text_small);
    }
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

    /* Apply dynamic depth dimming based on the party's current light
     * level.  The original DM1 uses torches, FUL spell, ILLUMULET, and
     * FLAMITT to illuminate the dungeon.  We map the computed 0..255
     * light level to dimming passes per depth band:
     *
     *   light >= 200  (bright):  no dimming at any depth
     *   light >= 120  (normal):  depth 2 dimmed once
     *   light >= 50   (dim):     depth 1 once, depth 2 twice
     *   light <  50   (dark):    depth 0 once, depth 1 twice, depth 2 three times
     */
    {
        int lightLevel = m11_compute_light_level(state);
        if (lightLevel < 50) {
            /* Very dark: heavy dimming everywhere */
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[0].x, frames[0].y,
                                    frames[0].w, frames[0].h, 1);
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[1].x, frames[1].y,
                                    frames[1].w, frames[1].h, 2);
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[2].x, frames[2].y,
                                    frames[2].w, frames[2].h, 3);
        } else if (lightLevel < 120) {
            /* Dim: moderate dimming */
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[1].x, frames[1].y,
                                    frames[1].w, frames[1].h, 1);
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[2].x, frames[2].y,
                                    frames[2].w, frames[2].h, 2);
        } else if (lightLevel < 200) {
            /* Normal: only the far band dimmed once */
            m11_apply_depth_dimming(framebuffer, framebufferWidth, framebufferHeight,
                                    frames[2].x, frames[2].y,
                                    frames[2].w, frames[2].h, 1);
        }
        /* lightLevel >= 200: bright, no dimming */
    }
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
        int drewStatusBox = 0;

        if (slot < state->world.party.championCount && state->world.party.champions[slot].present) {
            char name[16];
            const struct ChampionState_Compat* champ = &state->world.party.champions[slot];
            int isDead = (champ->hp.current == 0);
            m11_format_champion_name(champ->name, name, sizeof(name));

            /* GRAPHICS.DAT-backed status box frame (67×29).
             * Use graphic 8 (dead) or graphic 7 (normal) as the
             * status box background.  Falls back to procedural. */
            if (state->assetsAvailable) {
                unsigned int boxGfx = isDead ? M11_GFX_STATUS_BOX_DEAD
                                             : M11_GFX_STATUS_BOX;
                const M11_AssetSlot* boxAsset = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader, boxGfx);
                if (boxAsset && boxAsset->width == 67 && boxAsset->height == 29) {
                    M11_AssetLoader_BlitRegion(boxAsset,
                        0, 0, 67, 29,
                        framebuffer, framebufferWidth, framebufferHeight,
                        x, y, 0);
                    drewStatusBox = 1;
                }
            }
            if (!drewStatusBox) {
                /* Procedural fallback */
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x, y, 71, 28, M11_COLOR_BLACK);
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x, y, 71, 28, M11_COLOR_LIGHT_CYAN);
            }

            /* Active champion: double yellow highlight border
             * (ReDMCSB highlights the active champion status box) */
            if (slot == activeIndex) {
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 1, y + 1, 69, 26, M11_COLOR_YELLOW);
                m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                              x + 2, y + 2, 67, 24, M11_COLOR_YELLOW);
            }

            /* Champion icon from GRAPHICS.DAT graphic 28 (19×14 per
             * champion).  Blit into the left side of the status box,
             * with the name to the right. Falls back to text-only. */
            {
                int drewIcon = 0;
                int nameOffX = 4;
                if (state->assetsAvailable) {
                    const M11_AssetSlot* iconStrip = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader, M11_GFX_CHAMPION_ICONS);
                    if (iconStrip && iconStrip->width > 0 && iconStrip->height > 0) {
                        int iconCol = champ->portraitIndex & 3;
                        int srcX = iconCol * M11_CHAMPION_ICON_W;
                        if (srcX + M11_CHAMPION_ICON_W <= (int)iconStrip->width) {
                            M11_AssetLoader_BlitRegion(iconStrip,
                                srcX, 0, M11_CHAMPION_ICON_W, M11_CHAMPION_ICON_H,
                                framebuffer, framebufferWidth, framebufferHeight,
                                x + 3, y + 3, 0);
                            drewIcon = 1;
                            nameOffX = 3 + M11_CHAMPION_ICON_W + 2;
                        }
                    }
                }
                if (slot == activeIndex) {
                    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                  x + nameOffX, y + 3, name, &g_text_shadow);
                } else {
                    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                                  x + nameOffX, y + 3, name, &g_text_small);
                }
                (void)drewIcon;
            }
            {
            int hpWidth = champ->hp.maximum > 0 ? (int)(champ->hp.current * 59) / (int)champ->hp.maximum : 0;
            int staminaWidth = champ->stamina.maximum > 0 ? (int)(champ->stamina.current * 59) / (int)champ->stamina.maximum : 0;
            int manaWidth = champ->mana.maximum > 0 ? (int)(champ->mana.current * 59) / (int)champ->mana.maximum : 0;
            if (isDead) {
                M11_TextStyle ds = g_text_small;
                ds.color = M11_COLOR_RED;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              x + 4, y + 12, "DEAD", &ds);
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

            /* GRAPHICS.DAT-backed shield border overlays (67×29).
             * Drawn with transparency on top of the status box when the
             * party has an active shield spell.
             * Ref: ReDMCSB INVNTORY.C — G0310_i_ShieldDefenseType selects
             *   C037 (party shield), C038 (fire shield), or C039 (spell shield).
             * Priority: spell > fire > party (highest active wins). */
            if (state->assetsAvailable && !isDead) {
                unsigned int borderGfx = 0;
                if (state->world.magic.spellShieldDefense > 0)
                    borderGfx = M11_GFX_BORDER_PARTY_SPELLSHIELD;
                else if (state->world.magic.fireShieldDefense > 0)
                    borderGfx = M11_GFX_BORDER_PARTY_FIRESHIELD;
                else if (state->world.magic.partyShieldDefense > 0)
                    borderGfx = M11_GFX_BORDER_PARTY_SHIELD;
                if (borderGfx) {
                    const M11_AssetSlot* borderAsset = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader, borderGfx);
                    if (borderAsset && borderAsset->width == 67 &&
                        borderAsset->height == 29) {
                        M11_AssetLoader_BlitRegion(borderAsset,
                            0, 0, 67, 29,
                            framebuffer, framebufferWidth, framebufferHeight,
                            x, y, 0); /* transparentColor=0 (black) */
                    }
                }
            }

            /* GRAPHICS.DAT-backed POISONED label (96×15, graphic 32).
             * Drawn below the status box when champion is poisoned.
             * Ref: ReDMCSB INVNTORY.C — drawn when poisonDose > 0. */
            if (state->assetsAvailable && champ->poisonDose > 0) {
                const M11_AssetSlot* poisonLbl = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader,
                    M11_GFX_POISONED_LABEL);
                if (poisonLbl && poisonLbl->width > 0 &&
                    poisonLbl->height > 0) {
                    /* Center the 96-wide label within the 67-wide slot;
                     * in DM1 this spills across adjacent boxes, which is
                     * the correct original behaviour. */
                    int lblX = x + ((int)67 - (int)poisonLbl->width) / 2;
                    int lblY = y + 29; /* just below the status box */
                    M11_AssetLoader_BlitRegion(poisonLbl,
                        0, 0, (int)poisonLbl->width, (int)poisonLbl->height,
                        framebuffer, framebufferWidth, framebufferHeight,
                        lblX, lblY, 0);
                }
            }

            /* GRAPHICS.DAT-backed damage indicator (45×7, graphic 15).
             * Overlaid on the status box when the champion just took
             * damage (timer > 0).  The damage number is drawn on top.
             * Ref: ReDMCSB CHAMPION.C F0291. */
            if (state->championDamageTimer[slot] > 0) {
                if (state->assetsAvailable) {
                    const M11_AssetSlot* dmgAsset = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader,
                        M11_GFX_DAMAGE_TO_CHAMPION_SMALL);
                    if (dmgAsset && dmgAsset->width == 45 &&
                        dmgAsset->height == 7) {
                        int dmgX = x + (67 - 45) / 2;
                        int dmgY = y + (29 - 7) / 2;
                        M11_AssetLoader_BlitRegion(dmgAsset,
                            0, 0, 45, 7,
                            framebuffer, framebufferWidth, framebufferHeight,
                            dmgX, dmgY, 0);
                    }
                }
                /* Always draw the damage number (even without assets) */
                {
                    char dmgNum[8];
                    M11_TextStyle dmgStyle = g_text_small;
                    dmgStyle.color = M11_COLOR_WHITE;
                    snprintf(dmgNum, sizeof(dmgNum), "%d",
                             state->championDamageAmount[slot]);
                    m11_draw_text(framebuffer, framebufferWidth,
                                  framebufferHeight,
                                  x + 67 / 2 - 4, y + 29 / 2 - 3,
                                  dmgNum, &dmgStyle);
                }
            }
        } else {
            /* Empty slot: procedural placeholder */
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x, y, 71, 28, M11_COLOR_BLACK);
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          x, y, 71, 28, M11_COLOR_DARK_GRAY);
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

/* ── Full-screen map overlay (M key) ──
 *
 * Period-faithful presentation modeled after DM1 / ReDMCSB automap style:
 * - Full-screen black fill to guarantee no HUD bleed-through
 * - Stone-gray single border (not bright yellow/brown double frame)
 * - Compact title without debug dimensions
 * - Muted tile palette: walls dark, corridors mid-gray, specials subdued
 * - Minimal footer — just the dismiss key, not a keybinding reference card
 */

/* Map-specific muted tile colors — period-faithful restraint.
 * Walls blend into the background; corridors are the primary readable
 * feature.  Special tiles (doors, stairs, pits) use subdued accents
 * rather than the saturated utility-palette colors. */
static unsigned char m11_map_tile_color(int elementType) {
    switch (elementType) {
        case DUNGEON_ELEMENT_WALL:       return M11_COLOR_BLACK;      /* walls vanish into bg */
        case DUNGEON_ELEMENT_CORRIDOR:   return M11_COLOR_DARK_GRAY;  /* explored floor */
        case DUNGEON_ELEMENT_PIT:        return M11_COLOR_BROWN;      /* subtle hazard */
        case DUNGEON_ELEMENT_STAIRS:     return M11_COLOR_LIGHT_GRAY; /* notable but muted */
        case DUNGEON_ELEMENT_DOOR:       return M11_COLOR_BROWN;      /* structural feature */
        case DUNGEON_ELEMENT_TELEPORTER: return M11_COLOR_NAVY;       /* arcane, not neon */
        case DUNGEON_ELEMENT_FAKEWALL:   return M11_COLOR_DARK_GRAY;  /* blends with corridor */
        default: return M11_COLOR_BLACK;
    }
}

static void m11_draw_fullscreen_map(const M11_GameViewState* state,
                                   unsigned char* framebuffer,
                                   int framebufferWidth,
                                   int framebufferHeight) {
    int panelX, panelY, panelW, panelH;
    const struct DungeonMapDesc_Compat* mapDesc = NULL;
    int mapW = 0, mapH = 0;
    int cellSize;
    int offsetX, offsetY;
    int gx, gy;

    if (!state || !state->active || !state->world.dungeon) return;
    if (state->world.party.mapIndex < 0 ||
        state->world.party.mapIndex >= (int)state->world.dungeon->header.mapCount) return;
    mapDesc = &state->world.dungeon->maps[state->world.party.mapIndex];
    mapW = (int)mapDesc->width;
    mapH = (int)mapDesc->height;
    if (mapW <= 0 || mapH <= 0) return;

    /* Full-screen black fill — guarantees no HUD content leaks through
     * at any edge, regardless of panel inset.  This is the primary
     * clipping-cleanliness measure (S2.3). */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  0, 0, framebufferWidth, framebufferHeight, M11_COLOR_BLACK);

    /* Panel inset — generous margins for a deliberate, composed feel
     * rather than an edge-to-edge utility window. */
    panelX = 8;
    panelY = 6;
    panelW = framebufferWidth - 16;
    panelH = framebufferHeight - 12;

    /* Single stone-gray border — period-appropriate, not flashy */
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  panelX, panelY, panelW, panelH, M11_COLOR_DARK_GRAY);

    /* Title — compact, no debug dimensions */
    {
        char mapTitle[32];
        M11_TextStyle titleStyle = g_text_shadow;
        titleStyle.color = M11_COLOR_LIGHT_GRAY;
        snprintf(mapTitle, sizeof(mapTitle), "LEVEL %d",
                 state->world.party.mapIndex + 1);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      panelX + 6, panelY + 4, mapTitle, &titleStyle);
    }

    /* Compute cell size to fit map in available area */
    {
        int availW = panelW - 12;
        int availH = panelH - 28; /* title + footer headroom */
        int cw = availW / mapW;
        int ch = availH / mapH;
        cellSize = cw < ch ? cw : ch;
        if (cellSize < 2) cellSize = 2;
        if (cellSize > 12) cellSize = 12;
        offsetX = panelX + 6 + (availW - mapW * cellSize) / 2;
        offsetY = panelY + 18 + (availH - mapH * cellSize) / 2;
    }

    /* Draw each tile */
    for (gy = 0; gy < mapH; ++gy) {
        for (gx = 0; gx < mapW; ++gx) {
            unsigned char square = 0;
            unsigned char fill = M11_COLOR_BLACK;
            int drawX = offsetX + gx * cellSize;
            int drawY = offsetY + gy * cellSize;
            int ok = m11_get_square_byte(&state->world,
                                         state->world.party.mapIndex,
                                         gx, gy, &square);
            if (ok) {
                if (m11_is_explored(state, gx, gy)) {
                    fill = m11_map_tile_color(square >> 5);
                } else {
                    /* Unexplored: pure black — indistinguishable from void */
                    fill = M11_COLOR_BLACK;
                }
            }
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          drawX, drawY, cellSize - 1, cellSize - 1, fill);

            /* Party position marker — white dot, restrained */
            if (gx == state->world.party.mapX && gy == state->world.party.mapY) {
                int cx = drawX + cellSize / 2 - 1;
                int cy = drawY + cellSize / 2 - 1;
                int ms = cellSize > 4 ? cellSize - 3 : 2;
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              cx, cy, ms, ms, M11_COLOR_WHITE);
                if (cellSize >= 6) {
                    m11_draw_party_arrow(framebuffer, framebufferWidth, framebufferHeight,
                                         cx - 1, cy - 1, ms + 2,
                                         state->world.party.direction, M11_COLOR_LIGHT_GRAY);
                }
            }
        }
    }

    /* Footer — minimal dismiss hint, not a keybinding cheat sheet */
    {
        M11_TextStyle footerStyle = g_text_small;
        footerStyle.color = M11_COLOR_DARK_GRAY;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      panelX + 6, panelY + panelH - 12, "PRESS M", &footerStyle);
    }
}

/* ── Full inventory panel (I key) ──
 * ReDMCSB-faithful composition: portrait+name at top, hands row,
 * body column, containers, backpack grid.  All equipment slots are
 * uniform 16×16 icon boxes matching the original SLOT_BOX structure
 * from Fontanel’s INVNTORY.C / OBJECT.C DrawIconInSlotBox.
 *
 * Original DM1 layout reference (viewport-relative):
 *   [Portrait 32×29]  [Name]       Hands: [Ready][Action]
 *   [Stats bars]       Body:  HEAD  NECK  TORSO  LEGS  FEET
 *                      Containers: POUCH×2  QUIVER×4
 *                      Backpack: 4×2 grid
 *                      Panel: Food/Water or object details
 */

/* Helper: draw one equipment slot box using original GRAPHICS.DAT
 * slot box bitmaps (18×18).  Falls back to procedural 16×16 box
 * when the asset is unavailable.
 *
 * Original DM1 uses three slot box variants:
 *   graphic 33 = normal slot (C033_GRAPHIC_SLOT_BOX_NORMAL)
 *   graphic 34 = wounded champion slot (C034_GRAPHIC_SLOT_BOX_WOUNDED)
 *   graphic 35 = acting-hand slot (C035_GRAPHIC_SLOT_BOX_ACTING_HAND)
 *
 * Ref: DEFS.H lines 2193-2195, ReDMCSB INVNTORY.C DrawIconInSlotBox. */
static void m11_draw_inv_slot(const M11_GameViewState* state,
                              const struct ChampionState_Compat* champ,
                              unsigned char* fb, int fbW, int fbH,
                              int sx, int sy,
                              int slotIdx, const char* shortLabel) {
    /* Original slot boxes are 18×18 in GRAPHICS.DAT */
    static const int SZ_ORIG = 18;
    static const int SZ = 16; /* fallback procedural size */
    unsigned short thingId = champ->inventory[slotIdx];
    int selected = (slotIdx == state->inventorySelectedSlot);
    int isDead = (champ->hp.current == 0);
    int isActingHand = (slotIdx == CHAMPION_SLOT_HAND_LEFT ||
                        slotIdx == CHAMPION_SLOT_HAND_RIGHT);
    int drewSlotBox = 0;
    int slotBoxW = SZ, slotBoxH = SZ; /* effective size for item overlay */

    /* Try original GRAPHICS.DAT slot box graphic */
    if (state->assetsAvailable) {
        unsigned int gfxIdx;
        if (isActingHand)
            gfxIdx = M11_GFX_SLOT_BOX_ACTING_HAND;
        else if (isDead)
            gfxIdx = M11_GFX_SLOT_BOX_WOUNDED;
        else
            gfxIdx = M11_GFX_SLOT_BOX_NORMAL;

        const M11_AssetSlot* boxSlot = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader, gfxIdx);
        if (boxSlot && boxSlot->width == SZ_ORIG && boxSlot->height == SZ_ORIG) {
            M11_AssetLoader_Blit(boxSlot, fb, fbW, fbH, sx, sy, 0);
            drewSlotBox = 1;
            slotBoxW = SZ_ORIG;
            slotBoxH = SZ_ORIG;
        }
    }

    if (!drewSlotBox) {
        /* Procedural fallback */
        unsigned char borderColor = selected ? M11_COLOR_LIGHT_GREEN : M11_COLOR_DARK_GRAY;
        m11_fill_rect(fb, fbW, fbH, sx, sy, SZ, SZ, M11_COLOR_BLACK);
        m11_draw_rect(fb, fbW, fbH, sx, sy, SZ, SZ, borderColor);
    }

    /* Selection highlight: bright border around the slot box */
    if (selected) {
        m11_draw_rect(fb, fbW, fbH, sx, sy, slotBoxW, slotBoxH,
                      M11_COLOR_LIGHT_GREEN);
    }

    if (thingId != THING_NONE && thingId != THING_ENDOFLIST) {
        /* Occupied — attempt GRAPHICS.DAT sprite, fall back to type tag */
        int drewSprite = 0;
        if (state->assetsAvailable && state->world.things) {
            unsigned int gfxIdx = m11_inventory_thing_sprite_index(
                state->world.things, thingId);
            if (gfxIdx > 0 && gfxIdx < M11_GFX_ITEM_SPRITE_END) {
                const M11_AssetSlot* slot = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader, gfxIdx);
                if (slot && slot->width > 0 && slot->height > 0) {
                    /* Scale item sprite into the slot box (1px inset) */
                    M11_AssetLoader_BlitScaled(slot, fb, fbW, fbH,
                                              sx + 1, sy + 1,
                                              slotBoxW - 2, slotBoxH - 2, 0);
                    drewSprite = 1;
                }
            }
        }
        if (!drewSprite) {
            /* Fallback: 2-char type tag */
            int thingType = THING_GET_TYPE(thingId);
            const char* tag = "?";
            M11_TextStyle s = g_text_small;
            s.color = M11_COLOR_WHITE;
            switch (thingType) {
                case THING_TYPE_WEAPON:    tag = "Wp"; break;
                case THING_TYPE_ARMOUR:    tag = "Ar"; break;
                case THING_TYPE_SCROLL:    tag = "Sc"; break;
                case THING_TYPE_POTION:    tag = "Po"; break;
                case THING_TYPE_CONTAINER: tag = "Ch"; break;
                case THING_TYPE_JUNK:      tag = "Jk"; break;
                default: break;
            }
            m11_draw_text(fb, fbW, fbH, sx + 3, sy + 4, tag, &s);
        }
    } else if (!drewSlotBox) {
        /* Empty + no original graphic — faint position hint */
        M11_TextStyle dim = g_text_small;
        dim.color = M11_COLOR_DARK_GRAY;
        m11_draw_text(fb, fbW, fbH, sx + 2, sy + 4, shortLabel, &dim);
    }
}

static void m11_draw_inventory_panel(const M11_GameViewState* state,
                                    unsigned char* framebuffer,
                                    int framebufferWidth,
                                    int framebufferHeight) {
    /* Overlay positioned like the DM1 viewport-replacement inventory,
     * centered with proportional margins. */
    int panelX = 8, panelY = 8;
    int panelW = framebufferWidth - 16;
    int panelH = framebufferHeight - 16;
    const struct ChampionState_Compat* champ;
    char champion[32];
    int isDead;
    int bpI;

    /* Portrait proportions from ReDMCSB: 32×29 pixels */
    static const int PORT_W = 32;
    static const int PORT_H = 29;
    /* Slot box size: 18×18 when using original GRAPHICS.DAT assets,
     * 16×16 as procedural fallback. */
    int SZ = 16;
    static const int GAP = 2; /* spacing between adjacent slot boxes */

    /* Layout anchors */
    int portX, portY;   /* portrait position */
    int nameX, nameY;   /* champion name */
    int handY;          /* hands row Y */
    int handLX, handRX; /* hand slot X positions */
    int bodyX, bodyY;   /* body column anchor (Head at top) */
    int contX, contY;   /* container section anchor */
    int bpX, bpY;       /* backpack grid anchor */
    int panelBottom;    /* bottom info panel Y */

    if (!state || !state->active) return;
    if (state->world.party.activeChampionIndex < 0 ||
        state->world.party.activeChampionIndex >= CHAMPION_MAX_PARTY) return;
    champ = &state->world.party.champions[state->world.party.activeChampionIndex];
    isDead = (champ->hp.current == 0);

    /* Use 18×18 slot boxes when GRAPHICS.DAT assets are available */
    if (state->assetsAvailable) {
        const M11_AssetSlot* testBox = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader, M11_GFX_SLOT_BOX_NORMAL);
        if (testBox && testBox->width == 18 && testBox->height == 18)
            SZ = 18;
    }

    /* ── Panel background — original graphic 20 (144×73), or
     * procedural double-border as fallback ── */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  panelX, panelY, panelW, panelH, M11_COLOR_BLACK);
    if (state->assetsAvailable) {
        const M11_AssetSlot* panelBg = M11_AssetLoader_Load(
            (M11_AssetLoader*)&state->assetLoader, M11_GFX_PANEL_EMPTY);
        if (panelBg && panelBg->width > 0 && panelBg->height > 0) {
            /* Blit the original panel background scaled to fit */
            M11_AssetLoader_BlitScaled(panelBg,
                framebuffer, framebufferWidth, framebufferHeight,
                panelX, panelY, panelW, panelH, 0);
        }
    }
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  panelX, panelY, panelW, panelH, M11_COLOR_BROWN);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  panelX + 1, panelY + 1, panelW - 2, panelH - 2, M11_COLOR_DARK_GRAY);

    /* ── Champion identity ── portrait + name + stat bars */
    portX = panelX + 5;
    portY = panelY + 4;
    m11_get_active_champion_label(state, champion, sizeof(champion));

    /* Portrait box — GRAPHICS.DAT champion portrait (graphic 26,
     * C026_GRAPHIC_CHAMPION_PORTRAITS, 256×87 strip of 32×29 portraits).
     * M027_PORTRAIT_X(index) = (index & 7) * 32
     * M028_PORTRAIT_Y(index) = (index >> 3) * 29
     * Falls back to small icon (graphic 28) or silhouette. */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  portX, portY, PORT_W, PORT_H, M11_COLOR_DARK_GRAY);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  portX, portY, PORT_W, PORT_H,
                  isDead ? M11_COLOR_RED : M11_COLOR_LIGHT_CYAN);
    {
        int drewPortrait = 0;
        if (state->assetsAvailable) {
            /* Try full 32×29 portrait from graphic 26 first */
            const M11_AssetSlot* portraits = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader, M11_GFX_CHAMPION_PORTRAITS);
            if (portraits && portraits->width >= 256 && portraits->height >= 29) {
                int pIdx = champ->portraitIndex & 0x1F; /* 0–23 range */
                int srcPX = (pIdx & 7) * M11_PORTRAIT_W;
                int srcPY = (pIdx >> 3) * M11_PORTRAIT_H;
                if (srcPX + M11_PORTRAIT_W <= (int)portraits->width &&
                    srcPY + M11_PORTRAIT_H <= (int)portraits->height) {
                    M11_AssetLoader_BlitRegion(portraits,
                        srcPX, srcPY, M11_PORTRAIT_W, M11_PORTRAIT_H,
                        framebuffer, framebufferWidth, framebufferHeight,
                        portX, portY, 0);
                    drewPortrait = 1;
                }
            }
            /* Fallback: small icon from graphic 28 */
            if (!drewPortrait) {
                const M11_AssetSlot* iconStrip = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader, M11_GFX_CHAMPION_ICONS);
                if (iconStrip && iconStrip->width > 0 && iconStrip->height > 0) {
                    int iconCol = champ->portraitIndex & 3;
                    int srcX = iconCol * M11_CHAMPION_ICON_W;
                    if (srcX + M11_CHAMPION_ICON_W <= (int)iconStrip->width) {
                        M11_AssetLoader_BlitRegion(iconStrip,
                            srcX, 0, M11_CHAMPION_ICON_W, M11_CHAMPION_ICON_H,
                            framebuffer, framebufferWidth, framebufferHeight,
                            portX + 1, portY + 1, 0);
                        drewPortrait = 1;
                    }
                }
            }
        }
        if (!drewPortrait) {
            /* Fallback: minimal face silhouette */
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          portX + 9,  portY + 8, 3, 3, M11_COLOR_WHITE);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          portX + 20, portY + 8, 3, 3, M11_COLOR_WHITE);
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          portX + 10, portY + 19, 12, 2,
                          isDead ? M11_COLOR_RED : M11_COLOR_LIGHT_GRAY);
        }
    }

    /* Champion name — prominent, right of portrait */
    nameX = portX + PORT_W + 6;
    nameY = portY + 2;
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  nameX, nameY, champion, &g_text_title);

    if (isDead) {
        M11_TextStyle deadStyle = g_text_small;
        deadStyle.color = M11_COLOR_RED;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      nameX, nameY + 14, "DEAD", &deadStyle);
    }

    /* Stat bars — compact DM-style HP/STA/MANA right of portrait */
    {
        int barX = nameX;
        int barY = nameY + (isDead ? 22 : 14);
        int barMaxW = 80;
        int barH = 3;
        int hpW = champ->hp.maximum > 0
            ? (int)(champ->hp.current * barMaxW) / (int)champ->hp.maximum : 0;
        int staW = champ->stamina.maximum > 0
            ? (int)(champ->stamina.current * barMaxW) / (int)champ->stamina.maximum : 0;
        int manaW = champ->mana.maximum > 0
            ? (int)(champ->mana.current * barMaxW) / (int)champ->mana.maximum : 0;

        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY, barMaxW, barH, M11_COLOR_DARK_GRAY);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY, hpW, barH,
                      isDead ? M11_COLOR_DARK_GRAY : M11_COLOR_LIGHT_RED);

        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY + barH + 1, barMaxW, barH, M11_COLOR_DARK_GRAY);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY + barH + 1, staW, barH, M11_COLOR_LIGHT_GREEN);

        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY + 2 * (barH + 1), barMaxW, barH, M11_COLOR_DARK_GRAY);
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      barX, barY + 2 * (barH + 1), manaW, barH, M11_COLOR_LIGHT_BLUE);
    }

    /* ── Hands row — ReDMCSB places Ready Hand + Action Hand as the
     * first two slots, prominently above the body equipment column.
     * In the original, these are the primary interaction slots. ── */
    handY = portY + PORT_H + 4;
    handLX = panelX + 5;
    handRX = handLX + SZ + GAP;

    /* Thin section label */
    {
        M11_TextStyle hlbl = g_text_small;
        hlbl.color = M11_COLOR_BROWN;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      handLX, handY - 1, "HANDS", &hlbl);
    }
    handY += 7;
    m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                      handLX, handY, CHAMPION_SLOT_HAND_LEFT, "L");
    m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                      handRX, handY, CHAMPION_SLOT_HAND_RIGHT, "R");

    /* ── Body column — Head → Neck → Torso → Legs → Feet ──
     * Vertical column to the right of hands, matching ReDMCSB’s
     * equipment slot arrangement.  Each slot is a 16×16 icon box. */
    bodyX = handRX + SZ + 10;
    bodyY = handY;
    {
        static const int bodySlots[5] = {
            CHAMPION_SLOT_HEAD, CHAMPION_SLOT_NECK, CHAMPION_SLOT_TORSO,
            CHAMPION_SLOT_LEGS, CHAMPION_SLOT_FEET
        };
        static const char* bodyLabels[5] = { "Hd", "Nk", "To", "Lg", "Ft" };
        int bi;
        M11_TextStyle blbl = g_text_small;
        blbl.color = M11_COLOR_BROWN;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      bodyX, handY - 8, "BODY", &blbl);
        for (bi = 0; bi < 5; ++bi) {
            m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                              bodyX, bodyY + bi * (SZ + GAP),
                              bodySlots[bi], bodyLabels[bi]);
        }
    }

    /* ── Container region: Pouches + Quiver ──
     * Positioned to the right of the body column, matching the
     * original’s right-side container layout. */
    contX = bodyX + SZ + 10;
    contY = handY;
    {
        M11_TextStyle clbl = g_text_small;
        clbl.color = M11_COLOR_BROWN;

        /* Pouches (2 slots side by side) */
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      contX, contY - 1, "POUCH", &clbl);
        contY += 7;
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX, contY, CHAMPION_SLOT_POUCH_1, "1");
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX + SZ + GAP, contY, CHAMPION_SLOT_POUCH_2, "2");

        /* Quiver (4 slots in a row) */
        contY += SZ + GAP + 4;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      contX, contY, "QUIVER", &clbl);
        contY += 8;
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX, contY, CHAMPION_SLOT_QUIVER_1, "1");
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX + (SZ + GAP), contY, CHAMPION_SLOT_QUIVER_2, "2");
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX + 2 * (SZ + GAP), contY, CHAMPION_SLOT_QUIVER_3, "3");
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          contX + 3 * (SZ + GAP), contY, CHAMPION_SLOT_QUIVER_4, "4");
    }

    /* ── Backpack grid — 4×2, the largest container section ──
     * ReDMCSB: BACKPACK slots 13–20, drawn as 4-wide × 2-tall grid
     * at the bottom of the equipment area. */
    bpX = panelX + 5;
    bpY = handY + 5 * (SZ + GAP) + 6;
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   panelX + 3, bpY - 3, panelW - 6, M11_COLOR_DARK_GRAY);
    {
        M11_TextStyle plbl = g_text_small;
        plbl.color = M11_COLOR_BROWN;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      bpX, bpY, "BACKPACK", &plbl);
    }
    bpY += 8;
    for (bpI = 0; bpI < 8; ++bpI) {
        int col = bpI % 4;
        int row = bpI / 4;
        char bpLabel[4];
        snprintf(bpLabel, sizeof(bpLabel), "%d", bpI + 1);
        m11_draw_inv_slot(state, champ, framebuffer, framebufferWidth, framebufferHeight,
                          bpX + col * (SZ + GAP),
                          bpY + row * (SZ + GAP),
                          CHAMPION_SLOT_BACKPACK_1 + bpI, bpLabel);
    }

    /* ── Bottom panel area — food/water status (ReDMCSB: G424_i_PanelContent) ── */
    panelBottom = bpY + 2 * (SZ + GAP) + 4;
    m11_draw_hline(framebuffer, framebufferWidth, framebufferHeight,
                   panelX + 3, panelBottom - 2, panelW - 6, M11_COLOR_DARK_GRAY);
    {
        int avgFood = (int)champ->food;
        int avgWater = (int)champ->water;
        int labelDrawn = 0;
        int fwX = panelX + 5;
        int fwY = panelBottom;

        /* GRAPHICS.DAT-backed food/water labels (graphics 30, 31).
         * Food label: 34×9, Water label: 46×9.
         * Falls back to text rendering when assets unavailable. */
        if (state->assetsAvailable) {
            const M11_AssetSlot* foodLbl = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader, M11_GFX_FOOD_LABEL);
            const M11_AssetSlot* waterLbl = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader, M11_GFX_WATER_LABEL);
            if (foodLbl && foodLbl->width > 0 && foodLbl->height > 0 &&
                waterLbl && waterLbl->width > 0 && waterLbl->height > 0) {
                char numBuf[8];
                M11_TextStyle fwNumStyle = g_text_small;
                fwNumStyle.color = M11_COLOR_LIGHT_GRAY;
                M11_AssetLoader_BlitRegion(foodLbl,
                    0, 0, (int)foodLbl->width, (int)foodLbl->height,
                    framebuffer, framebufferWidth, framebufferHeight,
                    fwX, fwY, 0);
                snprintf(numBuf, sizeof(numBuf), "%d", avgFood > 999 ? 999 : avgFood);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              fwX + (int)foodLbl->width + 2, fwY, numBuf, &fwNumStyle);
                fwX += (int)foodLbl->width + 30;
                M11_AssetLoader_BlitRegion(waterLbl,
                    0, 0, (int)waterLbl->width, (int)waterLbl->height,
                    framebuffer, framebufferWidth, framebufferHeight,
                    fwX, fwY, 0);
                snprintf(numBuf, sizeof(numBuf), "%d", avgWater > 999 ? 999 : avgWater);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              fwX + (int)waterLbl->width + 2, fwY, numBuf, &fwNumStyle);
                labelDrawn = 1;
            }
        }
        if (!labelDrawn) {
            char foodWater[48];
            M11_TextStyle fwStyle = g_text_small;
            fwStyle.color = M11_COLOR_LIGHT_GRAY;
            snprintf(foodWater, sizeof(foodWater), "FOOD %d  WATER %d",
                     avgFood > 999 ? 999 : avgFood,
                     avgWater > 999 ? 999 : avgWater);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          fwX, fwY, foodWater, &fwStyle);
        }
    }

    /* ── Inventory damage overlay: GRAPHICS.DAT graphic 16 (32×29) ── */
    /* In DM1, when the inventory view is open and the active champion
     * takes damage, graphic 16 ("damage to champion big", 32×29) is
     * drawn on the portrait area instead of graphic 15 (small).
     * Ref: ReDMCSB CHAMPION.C F0291 — G0423_i_InventoryChampionOrdinal. */
    {
        int activeSlot = state->world.party.activeChampionIndex;
        if (activeSlot >= 0 && activeSlot < 4 &&
            state->championDamageTimer[activeSlot] > 0) {
            int dmgDrawn = 0;
            if (state->assetsAvailable) {
                const M11_AssetSlot* dmg16 = M11_AssetLoader_Load(
                    (M11_AssetLoader*)&state->assetLoader,
                    M11_GFX_DAMAGE_TO_CHAMPION_BIG);
                if (dmg16 && dmg16->width == 32 && dmg16->height == 29) {
                    M11_AssetLoader_BlitRegion(dmg16,
                        0, 0, 32, 29,
                        framebuffer, framebufferWidth, framebufferHeight,
                        portX, portY, 0);
                    dmgDrawn = 1;
                }
            }
            if (!dmgDrawn) {
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              portX, portY, PORT_W, PORT_H, M11_COLOR_LIGHT_RED);
            }
            /* Damage number centered on portrait */
            {
                char dmgNum[8];
                M11_TextStyle dmgStyle = g_text_small;
                dmgStyle.color = M11_COLOR_WHITE;
                snprintf(dmgNum, sizeof(dmgNum), "%d",
                         state->championDamageAmount[activeSlot]);
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              portX + PORT_W / 2 - 4,
                              portY + PORT_H / 2 - 3,
                              dmgNum, &dmgStyle);
            }
        }
    }

    /* ── Footer — minimal DM-style key hints ── */
    {
        M11_TextStyle footStyle = g_text_small;
        footStyle.color = M11_COLOR_DARK_GRAY;
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      panelX + 5, panelY + panelH - 10,
                      "I CLOSE  TAB CHAMPION", &footStyle);
    }
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
    /* Activate original DM1 font for this render pass */
    g_activeOriginalFont = state->originalFontAvailable
        ? &state->originalFont : NULL;
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  0, 0, framebufferWidth, framebufferHeight, M11_COLOR_NAVY);
    if (!state || !state->active) {
        g_drawState = NULL;
        g_activeOriginalFont = NULL;
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

    /* ── V1 screen layout ──
     * Three major zones (DM-like composition):
     *   1. Top bar:     thin title strip (dungeon name only)
     *   2. Middle:      viewport (left) + right status column
     *   3. Bottom:      party panel + single message line
     *
     * Debug/helper elements are only drawn when showDebugHUD is set. */

    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  8, 8, framebufferWidth - 16, framebufferHeight - 16, M11_COLOR_DARK_GRAY);
    m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                  8, 8, framebufferWidth - 16, framebufferHeight - 16, M11_COLOR_YELLOW);

    /* Top title bar — dungeon title only (no keybinding helpers) */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  12, 12, framebufferWidth - 24, 12, M11_COLOR_BLACK);
    m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                  18, 13, state->title[0] != '\0' ? state->title : "DUNGEON MASTER", &g_text_shadow);

    if (state->showDebugHUD) {
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
    }

    /* Viewport zone — large, left side */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  12, 24, 196, 120, M11_COLOR_BLACK);
    m11_draw_viewport(state, framebuffer, framebufferWidth, framebufferHeight);

    /* Overlay UI frame border elements from GRAPHICS.DAT */
    m11_draw_ui_frame_assets(state, framebuffer, framebufferWidth, framebufferHeight);

    /* Right status column — simplified: direction, champion, light */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  214, 24, 94, 120, M11_COLOR_BLACK);
    m11_draw_utility_panel(state, framebuffer, framebufferWidth, framebufferHeight, mapDesc);

    if (state->showDebugHUD) {
        /* Map mini-panel, focus card, message log — debug only */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      218, 74, 86, 68, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      218, 74, 86, 68, M11_COLOR_LIGHT_BLUE);
        m11_draw_map_panel(state, framebuffer, framebufferWidth, framebufferHeight);
        m11_draw_focus_card(framebuffer, framebufferWidth, framebufferHeight, state, &aheadCell);
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
    }

    /* Bottom zone — party panel is the dominant element */
    m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                  12, 146, 296, 46, M11_COLOR_BLACK);

    if (state->showDebugHUD) {
        /* Diagnostic square summary lines — debug only */
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

        /* Prompt strip and focus helpers — debug only */
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

        m11_draw_feedback_strip(framebuffer, framebufferWidth, framebufferHeight,
                                state, &aheadCell);
    } else {
        /* V1 mode: single contextual message line in the bottom area,
         * placed where DM1 shows brief status text. */
        {
            const M11_LogEntry* lastMsg = (state->messageLog.count > 0)
                ? m11_log_entry_at(&state->messageLog, 0) : NULL;
            if (lastMsg && lastMsg->text[0] != '\0') {
                M11_TextStyle msgStyle = g_text_small;
                msgStyle.color = lastMsg->color;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              16, 149, lastMsg->text, &msgStyle);
            }
        }
    }

    m11_draw_control_strip(framebuffer, framebufferWidth, framebufferHeight, &aheadCell);
    m11_draw_party_panel(state, framebuffer, framebufferWidth, framebufferHeight);

    /* Spell panel overlay */
    if (state->spellPanelOpen) {
        /* ── P4+P6 V1 Presentation: DM1-style rune-dominant spell panel
         * with GRAPHICS.DAT-backed spell area grid ── */
        int spI;
        int pnlX = 24, pnlY = 36, pnlW = 180, pnlH = 110;
        int row = state->spellRuneRow < 4 ? state->spellRuneRow : 3;
        const char* rowNames[4] = { "POWER", "ELEMENT", "FORM", "CLASS" };

        /* Panel background — DM1-style double-bordered rectangle.
         * The original spell panel is drawn on top of the action area, not
         * using graphic 20 (C020_GRAPHIC_PANEL_EMPTY) which is the inventory
         * panel background and has a different layout.  The spell casting UI
         * in the original uses the game's default panel region with the
         * spell area background (graphic 9) as a sub-region for the rune
         * buttons.  We preserve the procedural panel frame here. */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      pnlX, pnlY, pnlW, pnlH, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      pnlX, pnlY, pnlW, pnlH, M11_COLOR_BROWN);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      pnlX + 2, pnlY + 2, pnlW - 4, pnlH - 4, M11_COLOR_BROWN);

        /* ── Selected rune sequence (prominent, top of panel) ── */
        {
            char buf[64];
            int bI;
            int seqY = pnlY + 6;
            buf[0] = '\0';
            for (bI = 0; bI < state->spellBuffer.runeCount; ++bI) {
                int rv = state->spellBuffer.runes[bI];
                int rr = (rv - 0x60) / 6;
                int rc = (rv - 0x60) % 6;
                if (rr >= 0 && rr < 4 && rc >= 0 && rc < 6) {
                    if (bI > 0) strncat(buf, "  ", sizeof(buf) - strlen(buf) - 1);
                    strncat(buf, g_rune_names[rr][rc], sizeof(buf) - strlen(buf) - 1);
                }
            }
            if (buf[0] != '\0') {
                M11_TextStyle seqStyle = g_text_title;
                seqStyle.color = M11_COLOR_LIGHT_GREEN;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              pnlX + 8, seqY, buf, &seqStyle);
            } else {
                /* Empty sequence placeholder */
                M11_TextStyle dimStyle = g_text_small;
                dimStyle.color = M11_COLOR_DARK_GRAY;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              pnlX + 8, seqY + 2, "- - - -", &dimStyle);
            }
        }

        /* ── Separator line below rune sequence ── */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      pnlX + 6, pnlY + 22, pnlW - 12, 1, M11_COLOR_BROWN);

        /* ── Active rune row: category label + GRAPHICS.DAT-backed button grid ── */
        if (state->spellBuffer.runeCount < 4) {
            int runeW = 26, runeH = 20;
            int rowStartX = pnlX + 6;
            int runeY = pnlY + 38;
            int gridW = 6 * (runeW + 2) - 2; /* total rune button grid width */
            M11_TextStyle catStyle = g_text_small;
            catStyle.color = M11_COLOR_YELLOW;
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          pnlX + 8, pnlY + 26, rowNames[row], &catStyle);

            /* Blit the spell area background (graphic 9) as the rune button
             * grid backdrop.  DEFS.H:2216 C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND.
             * The original DM1 draws this 87×25 bitmap as the base, then overlays
             * graphic 11 (C011_GRAPHIC_MENU_SPELL_AREA_LINES) for the symbol rows.
             * We blit graphic 9 scaled to the button area.  Graphic 11 overlay is
             * deferred: its 14×39 layout requires per-row extraction matching the
             * original F0397_MENUS_DrawAvailableSymbols scanline logic. */
            {
                int drewGrid = 0;
                if (state->assetsAvailable) {
                    const M11_AssetSlot* spellBg = M11_AssetLoader_Load(
                        (M11_AssetLoader*)&state->assetLoader, M11_GFX_SPELL_AREA_BG);
                    if (spellBg && spellBg->width > 0 && spellBg->height > 0) {
                        M11_AssetLoader_BlitScaled(spellBg,
                            framebuffer, framebufferWidth, framebufferHeight,
                            rowStartX, runeY, gridW, runeH, -1);
                        drewGrid = 1;
                    }
                }
                if (!drewGrid) {
                    /* Fallback: individual coloured rune button rects */
                    for (spI = 0; spI < 6; ++spI) {
                        int bx = rowStartX + spI * (runeW + 2);
                        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                                      bx, runeY, runeW, runeH, M11_COLOR_NAVY);
                        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                                      bx, runeY, runeW, runeH, M11_COLOR_LIGHT_BLUE);
                    }
                }
            }
            /* Overlay rune names on top of the grid (text on graphic,
             * matching original DM1 F0397_MENUS_DrawAvailableSymbols) */
            for (spI = 0; spI < 6; ++spI) {
                int bx = rowStartX + spI * (runeW + 2);
                M11_TextStyle runeStyle = g_text_shadow;
                runeStyle.color = M11_COLOR_WHITE;
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              bx + 3, runeY + 6, g_rune_names[row][spI],
                              &runeStyle);
            }
        } else {
            /* All 4 runes selected — show cast-ready state */
            M11_TextStyle readyStyle = g_text_title;
            readyStyle.color = M11_COLOR_LIGHT_GREEN;
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          pnlX + 40, pnlY + 42, "CAST", &readyStyle);
        }

        /* ── Mana indicator (compact bar + value) ── */
        if (state->world.party.activeChampionIndex >= 0 &&
            state->world.party.activeChampionIndex < CHAMPION_MAX_PARTY) {
            const struct ChampionState_Compat* sc =
                &state->world.party.champions[state->world.party.activeChampionIndex];
            int barX = pnlX + 8, barY = pnlY + 64;
            int barW = pnlW - 16, barH = 6;
            int fillW;
            char manaStr[24];
            M11_TextStyle manaStyle = g_text_small;
            manaStyle.color = M11_COLOR_LIGHT_CYAN;
            /* Mana bar outline */
            m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                          barX, barY, barW, barH, M11_COLOR_DARK_GRAY);
            /* Mana bar fill */
            fillW = (sc->mana.maximum > 0)
                ? (int)((long)sc->mana.current * (barW - 2) / sc->mana.maximum)
                : 0;
            if (fillW > barW - 2) fillW = barW - 2;
            if (fillW > 0) {
                m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                              barX + 1, barY + 1, fillW, barH - 2,
                              M11_COLOR_LIGHT_BLUE);
            }
            snprintf(manaStr, sizeof(manaStr), "%d/%d",
                     (int)sc->mana.current, (int)sc->mana.maximum);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          barX, barY + barH + 2, manaStr, &manaStyle);
        }

        /* ── Remaining rune rows shown dimmed below active row ── */
        if (state->spellBuffer.runeCount < 4) {
            int futRow;
            int dimY = pnlY + 76;
            M11_TextStyle dimStyle = g_text_small;
            dimStyle.color = M11_COLOR_DARK_GRAY;
            for (futRow = row + 1; futRow < 4; ++futRow) {
                m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                              pnlX + 8, dimY, rowNames[futRow], &dimStyle);
                dimY += 10;
            }
        }
    }

    /* Endgame victory overlay */
    if (state->gameWon) {
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      40, 40, 240, 120, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      40, 40, 240, 120, M11_COLOR_LIGHT_GREEN);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      42, 42, 236, 116, M11_COLOR_LIGHT_GREEN);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      90, 52, "QUEST COMPLETE", &g_text_title);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      56, 80, "LORD CHAOS IS DEFEATED.", &g_text_shadow);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      52, 92, "THE FIRESTAFF IS RESTORED.", &g_text_shadow);
        {
            char wonLine[48];
            snprintf(wonLine, sizeof(wonLine), "VICTORY AT TICK %u",
                     (unsigned int)state->gameWonTick);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          80, 112, wonLine, &g_text_small);
        }
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      80, 132, "ESC TO RETURN TO MENU", &g_text_small);
    }

    /* Dialog box overlay for text plaque inspection */
    if (state->dialogOverlayActive && state->dialogOverlayText[0] != '\0') {
        int dlgX = 30, dlgY = 50, dlgW = 260, dlgH = 80;
        int textY;
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      dlgX, dlgY, dlgW, dlgH, M11_COLOR_BLACK);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      dlgX, dlgY, dlgW, dlgH, M11_COLOR_YELLOW);
        m11_draw_rect(framebuffer, framebufferWidth, framebufferHeight,
                      dlgX + 2, dlgY + 2, dlgW - 4, dlgH - 4, M11_COLOR_BROWN);
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      dlgX + 8, dlgY + 8, "TEXT PLAQUE", &g_text_title);
        /* Word-wrap the dialog text into the box (simple two-line split) */
        textY = dlgY + 28;
        if (strlen(state->dialogOverlayText) <= 40) {
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          dlgX + 12, textY, state->dialogOverlayText,
                          &g_text_shadow);
        } else {
            /* Split at nearest space before character 40 */
            char line1[48], line2[80];
            int splitPos = 40;
            while (splitPos > 0 && state->dialogOverlayText[splitPos] != ' ') {
                --splitPos;
            }
            if (splitPos == 0) splitPos = 40;
            memcpy(line1, state->dialogOverlayText, (size_t)splitPos);
            line1[splitPos] = '\0';
            snprintf(line2, sizeof(line2), "%s",
                     state->dialogOverlayText + splitPos + (state->dialogOverlayText[splitPos] == ' ' ? 1 : 0));
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          dlgX + 12, textY, line1, &g_text_shadow);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          dlgX + 12, textY + 14, line2, &g_text_shadow);
        }
        m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                      dlgX + 50, dlgY + dlgH - 16,
                      "PRESS ANY KEY TO DISMISS", &g_text_small);
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

    /* ── Damage flash: red border overlay when party takes melee hit ── */
    if (state->damageFlashTimer > 0) {
        int vx = 12, vy = 24, vw = 196, vh = 118;
        int thickness = 2;
        /* Top border */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      vx, vy, vw, thickness, M11_COLOR_LIGHT_RED);
        /* Bottom border */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      vx, vy + vh - thickness, vw, thickness, M11_COLOR_LIGHT_RED);
        /* Left border */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      vx, vy, thickness, vh, M11_COLOR_LIGHT_RED);
        /* Right border */
        m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                      vx + vw - thickness, vy, thickness, vh, M11_COLOR_LIGHT_RED);
    }

    /* ── Attack cue: diagonal slash marks when creature attacks ── */
    if (state->attackCueTimer > 0) {
        int cx = 110, cy = 78; /* centre of viewport face */
        int len = 14;
        int i;
        /* Draw two crossing diagonal lines (X slash) */
        for (i = 0; i < len; ++i) {
            int px1 = cx - len / 2 + i;
            int py1 = cy - len / 2 + i;
            int px2 = cx + len / 2 - i;
            int py2 = cy - len / 2 + i;
            if (px1 >= 0 && px1 < framebufferWidth &&
                py1 >= 0 && py1 < framebufferHeight)
                framebuffer[py1 * framebufferWidth + px1] = M11_COLOR_YELLOW;
            if (px2 >= 0 && px2 < framebufferWidth &&
                py2 >= 0 && py2 < framebufferHeight)
                framebuffer[py2 * framebufferWidth + px2] = M11_COLOR_YELLOW;
        }
    }

    /* ── Creature-hit overlay: GRAPHICS.DAT graphic 14 on viewport ── */
    /* In the original DM1, when the party deals melee damage to a
     * creature, the "damage to creature" graphic (C014, 88×45) is
     * drawn centered in the viewport's action-result zone with the
     * damage number overlaid.  Size scales with damage magnitude:
     * >40 = full (88×45), 15-40 = medium (64×37), <15 = small (42×37).
     * Ref: ReDMCSB MELEE.C, G2093-G2096. */
    if (state->creatureHitOverlayTimer > 0) {
        int vCX = 110, vCY = 78; /* viewport center */
        int drawn = 0;
        if (state->assetsAvailable) {
            const M11_AssetSlot* dmg14 = M11_AssetLoader_Load(
                (M11_AssetLoader*)&state->assetLoader,
                M11_GFX_DAMAGE_TO_CREATURE);
            if (dmg14 && dmg14->width > 0 && dmg14->height > 0) {
                int blitW, blitH;
                if (state->creatureHitDamageAmount > 40) {
                    blitW = 88; blitH = 45;
                } else if (state->creatureHitDamageAmount > 15) {
                    blitW = 64; blitH = 37;
                } else {
                    blitW = 42; blitH = 37;
                }
                M11_AssetLoader_BlitScaled(dmg14, framebuffer,
                    framebufferWidth, framebufferHeight,
                    vCX - blitW / 2, vCY - blitH / 2,
                    blitW, blitH, 0);
                drawn = 1;
            }
        }
        if (!drawn) {
            /* Fallback: tinted rectangle */
            m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,
                          vCX - 32, vCY - 16, 64, 32, M11_COLOR_LIGHT_RED);
        }
        /* Damage number centered on overlay */
        {
            char hitNum[8];
            M11_TextStyle hitStyle = g_text_small;
            hitStyle.color = M11_COLOR_WHITE;
            snprintf(hitNum, sizeof(hitNum), "%d",
                     state->creatureHitDamageAmount);
            m11_draw_text(framebuffer, framebufferWidth, framebufferHeight,
                          vCX - 6, vCY - 3, hitNum, &hitStyle);
        }
    }

    /* ── Full-screen overlay panels (drawn last, on top of everything) ── */
    if (state->mapOverlayActive) {
        m11_draw_fullscreen_map(state, framebuffer, framebufferWidth, framebufferHeight);
    }
    if (state->inventoryPanelActive) {
        m11_draw_inventory_panel(state, framebuffer, framebufferWidth, framebufferHeight);
    }

    g_drawState = NULL;
    g_activeOriginalFont = NULL;
}

/* ── Creature animation implementation ── */

void M11_GameView_TickAnimation(M11_GameViewState* state) {
    if (!state) return;
    state->animTick++;
    if (state->damageFlashTimer > 0) state->damageFlashTimer--;
    if (state->attackCueTimer > 0)   state->attackCueTimer--;
    if (state->creatureHitOverlayTimer > 0) state->creatureHitOverlayTimer--;
    { int ci; for (ci = 0; ci < 4; ++ci) {
        if (state->championDamageTimer[ci] > 0) state->championDamageTimer[ci]--;
    }}
}

void M11_GameView_NotifyDamageFlash(M11_GameViewState* state,
                                    int creatureType) {
    if (!state) return;
    state->damageFlashTimer = M11_DAMAGE_FLASH_DURATION;
    state->attackCueTimer   = M11_ATTACK_CUE_DURATION;
    state->attackCueCreatureType = creatureType;
}

int M11_GameView_GetDamageFlashTimer(const M11_GameViewState* state) {
    return state ? state->damageFlashTimer : 0;
}

int M11_GameView_GetAttackCueTimer(const M11_GameViewState* state) {
    return state ? state->attackCueTimer : 0;
}

void M11_GameView_NotifyChampionDamage(M11_GameViewState* state,
                                       int championSlot,
                                       int damageAmount) {
    if (!state || championSlot < 0 || championSlot >= 4) return;
    state->championDamageTimer[championSlot] = M11_DAMAGE_FLASH_DURATION;
    state->championDamageAmount[championSlot] = damageAmount;
}

uint32_t M11_GameView_GetAnimTick(const M11_GameViewState* state) {
    return state ? state->animTick : 0;
}

void M11_GameView_NotifyCreatureHit(M11_GameViewState* state,
                                    int damageAmount) {
    if (!state) return;
    state->creatureHitOverlayTimer = M11_CREATURE_HIT_OVERLAY_DURATION;
    state->creatureHitDamageAmount = damageAmount;
}

int M11_GameView_GetCreatureHitOverlayTimer(const M11_GameViewState* state) {
    return state ? state->creatureHitOverlayTimer : 0;
}

int M11_GameView_CreatureAnimFrame(const M11_GameViewState* state,
                                   int creatureType) {
    /* Stagger animation per creature type so groups don't all
     * move in lockstep.  Returns 0 or 1. */
    uint32_t phase;
    if (!state) return 0;
    phase = state->animTick + (uint32_t)creatureType * 3;
    return (int)((phase / M11_CREATURE_ANIM_PERIOD) & 1);
}

int M11_GameView_GetAttackCueCreatureType(const M11_GameViewState* state) {
    return state ? state->attackCueCreatureType : -1;
}

/* ── Full-screen map overlay API ── */

int M11_GameView_ToggleMapOverlay(M11_GameViewState* state) {
    if (!state) return 0;
    state->mapOverlayActive = !state->mapOverlayActive;
    return state->mapOverlayActive;
}

int M11_GameView_IsMapOverlayActive(const M11_GameViewState* state) {
    return state ? state->mapOverlayActive : 0;
}

/* ── Full inventory panel API ── */

int M11_GameView_ToggleInventoryPanel(M11_GameViewState* state) {
    if (!state) return 0;
    state->inventoryPanelActive = !state->inventoryPanelActive;
    if (state->inventoryPanelActive) {
        state->inventorySelectedSlot = 0;
    }
    return state->inventoryPanelActive;
}

int M11_GameView_IsInventoryPanelActive(const M11_GameViewState* state) {
    return state ? state->inventoryPanelActive : 0;
}

int M11_GameView_GetInventorySelectedSlot(const M11_GameViewState* state) {
    return state ? state->inventorySelectedSlot : -1;
}

const char* M11_GameView_SlotName(int slotIndex) {
    switch (slotIndex) {
        case CHAMPION_SLOT_HEAD:       return "HEAD";
        case CHAMPION_SLOT_NECK:       return "NECK";
        case CHAMPION_SLOT_TORSO:      return "TORSO";
        case CHAMPION_SLOT_LEGS:       return "LEGS";
        case CHAMPION_SLOT_FEET:       return "FEET";
        case CHAMPION_SLOT_POUCH_1:    return "POUCH 1";
        case CHAMPION_SLOT_POUCH_2:    return "POUCH 2";
        case CHAMPION_SLOT_QUIVER_1:   return "QUIVER 1";
        case CHAMPION_SLOT_QUIVER_2:   return "QUIVER 2";
        case CHAMPION_SLOT_QUIVER_3:   return "QUIVER 3";
        case CHAMPION_SLOT_QUIVER_4:   return "QUIVER 4";
        case CHAMPION_SLOT_BACKPACK_1: return "BACK 1";
        case CHAMPION_SLOT_BACKPACK_2: return "BACK 2";
        case CHAMPION_SLOT_BACKPACK_3: return "BACK 3";
        case CHAMPION_SLOT_BACKPACK_4: return "BACK 4";
        case CHAMPION_SLOT_BACKPACK_5: return "BACK 5";
        case CHAMPION_SLOT_BACKPACK_6: return "BACK 6";
        case CHAMPION_SLOT_BACKPACK_7: return "BACK 7";
        case CHAMPION_SLOT_BACKPACK_8: return "BACK 8";
        case CHAMPION_SLOT_HAND_LEFT:  return "L HAND";
        case CHAMPION_SLOT_HAND_RIGHT: return "R HAND";
        default: return "SLOT";
    }
}

/* ── Dialog / endgame query API ── */

int M11_GameView_IsGameWon(const M11_GameViewState* state) {
    return state ? state->gameWon : 0;
}

uint32_t M11_GameView_GetGameWonTick(const M11_GameViewState* state) {
    return state ? state->gameWonTick : 0;
}

int M11_GameView_IsDialogOverlayActive(const M11_GameViewState* state) {
    return state ? state->dialogOverlayActive : 0;
}

int M11_GameView_DismissDialogOverlay(M11_GameViewState* state) {
    if (!state || !state->dialogOverlayActive) return 0;
    state->dialogOverlayActive = 0;
    state->dialogOverlayText[0] = '\0';
    return 1;
}

int M11_GameView_ShowDialogOverlay(M11_GameViewState* state,
                                   const char* text) {
    if (!state || !text) return 0;
    state->dialogOverlayActive = 1;
    snprintf(state->dialogOverlayText, sizeof(state->dialogOverlayText),
             "%s", text);
    return 1;
}

/* ── Creature aspect query API ── */

int M11_GameView_GetCreatureCoordinateSet(int creatureType) {
    return m11_creature_coordinate_set(creatureType);
}

int M11_GameView_GetCreatureTransparentColor(int creatureType) {
    return m11_creature_transparent_color(creatureType);
}

int M11_GameView_GetFloorOrnamentOrdinal(const M11_GameViewState* state,
                                         int relForward, int relSide) {
    M11_ViewportCell cell;
    if (!state || !state->active) return 0;
    if (!m11_sample_viewport_cell(state, relForward, relSide, &cell)) return 0;
    return cell.floorOrnamentOrdinal;
}
