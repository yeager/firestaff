/*
 * DM1 V1 Hall of Champions readable inscription runtime probe.
 *
 * The probe finds a real visible TextString in map 0, places the party so
 * that the text-bearing wall is D1C, renders through M11_GameView_Draw, and
 * verifies the readable inscription path restores source wall pixels under
 * the M648 glyphs without Firestaff's old synthetic dark text box.
 *
 * Source-locked to ReDMCSB:
 *   DUNGEON.C:2329 stores 0x80 line separators for inscriptions.
 *   DUNVIEW.C:3679-3682 patches the D1C wall with M712_NEGGRAPHIC_/C735.
 *   DUNVIEW.C:3697-3706 draws M648_GRAPHIC_INSCRIPTION_FONT glyphs.
 */

#include "asset_status_m12.h"
#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_Y = 33,
    HALL_MAP_INDEX = 0,
    COLOR_BLACK = 0,
    COLOR_DARK_GRAY = 12,
    INSCRIPTION_MAX_TEXT = 128
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, fmt, ...) \
    do { \
        if (cond) { \
            printf("PASS " fmt "\n", ##__VA_ARGS__); \
            ++g_pass; \
        } else { \
            printf("FAIL " fmt "\n", ##__VA_ARGS__); \
            ++g_fail; \
        } \
    } while (0)

static int dir_dx(int dir) {
    return (dir & 3) == 1 ? 1 : ((dir & 3) == 3 ? -1 : 0);
}

static int dir_dy(int dir) {
    return (dir & 3) == 2 ? 1 : ((dir & 3) == 0 ? -1 : 0);
}

static int square_index_for(const M11_GameViewState* state, int x, int y) {
    const struct DungeonMapDesc_Compat* map;
    if (!state || !state->world.dungeon ||
        HALL_MAP_INDEX >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[HALL_MAP_INDEX];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return -1;
    }
    return x * (int)map->height + y;
}

static int square_element_for(const M11_GameViewState* state, int x, int y) {
    int idx = square_index_for(state, x, y);
    unsigned char square;
    if (idx < 0 || !state->world.dungeon->tiles ||
        !state->world.dungeon->tiles[HALL_MAP_INDEX].squareData) {
        return -1;
    }
    square = state->world.dungeon->tiles[HALL_MAP_INDEX].squareData[idx];
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int raw_next_thing(const M11_GameViewState* state, unsigned short thing) {
    static const unsigned char kThingDataByteCount[16] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    int type = (int)THING_GET_TYPE(thing);
    int index = (int)THING_GET_INDEX(thing);
    const unsigned char* raw;
    if (!state || !state->world.things || type < 0 || type >= 16 ||
        !state->world.things->rawThingData[type] ||
        index < 0 || index >= state->world.things->thingCounts[type] ||
        kThingDataByteCount[type] == 0) {
        return THING_ENDOFLIST;
    }
    raw = state->world.things->rawThingData[type] +
          index * (int)kThingDataByteCount[type];
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static void normalize_inscription_text(char* text) {
    char* p;
    if (!text) {
        return;
    }
    for (p = text; *p; ++p) {
        if (*p == (char)0x80) {
            *p = '\n';
        }
    }
}

static int find_front_inscription_pose(const M11_GameViewState* state,
                                       int* outPartyX,
                                       int* outPartyY,
                                       int* outDir,
                                       char* outText,
                                       size_t outTextSize) {
    const struct DungeonMapDesc_Compat* map;
    int x;
    int y;
    if (!state || !state->world.dungeon || !state->world.things ||
        !state->world.things->squareFirstThings ||
        !state->world.things->textStrings ||
        HALL_MAP_INDEX >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    map = &state->world.dungeon->maps[HALL_MAP_INDEX];
    for (y = 0; y < (int)map->height; ++y) {
        for (x = 0; x < (int)map->width; ++x) {
            int squareIndex = square_index_for(state, x, y);
            unsigned short thing;
            if (squareIndex < 0) {
                continue;
            }
            if (square_element_for(state, x, y) != DUNGEON_ELEMENT_WALL) {
                continue;
            }
            thing = state->world.things->squareFirstThings[squareIndex];
            while (thing != THING_ENDOFLIST && thing != THING_NONE) {
                int type = (int)THING_GET_TYPE(thing);
                int index = (int)THING_GET_INDEX(thing);
                int cell = (int)THING_GET_CELL(thing);
                if (type == THING_TYPE_TEXTSTRING &&
                    index >= 0 &&
                    index < state->world.things->textStringCount &&
                    state->world.things->textStrings[index].visible) {
                    int dir;
                    for (dir = 0; dir < 4; ++dir) {
                        int partyX;
                        int partyY;
                        if (((dir + 2) & 3) != cell) {
                            continue;
                        }
                        partyX = x - dir_dx(dir);
                        partyY = y - dir_dy(dir);
                        if (square_element_for(state, partyX, partyY) !=
                            DUNGEON_ELEMENT_CORRIDOR) {
                            continue;
                        }
                        if (F0507_DUNGEON_DecodeTextAtOffset_Compat(
                                state->world.things->textData,
                                state->world.things->textDataWordCount,
                                state->world.things->textStrings[index].textDataWordOffset,
                                outText,
                                (int)outTextSize) < 0 ||
                            !outText[0]) {
                            continue;
                        }
                        normalize_inscription_text(outText);
                        *outPartyX = partyX;
                        *outPartyY = partyY;
                        *outDir = dir;
                        return 1;
                    }
                }
                thing = (unsigned short)raw_next_thing(state, thing);
            }
        }
    }
    return 0;
}

static int row_color_count(const unsigned char* fb,
                           int y,
                           int x0,
                           int x1,
                           unsigned char color) {
    int x;
    int count = 0;
    if (!fb || y < 0 || y >= FB_H) {
        return 0;
    }
    if (x0 < 0) x0 = 0;
    if (x1 > FB_W) x1 = FB_W;
    for (x = x0; x < x1; ++x) {
        if ((fb[y * FB_W + x] & 0x0F) == color) {
            ++count;
        }
    }
    return count;
}

static int glyph_pixel_count(const unsigned char* fb,
                             int x,
                             int y,
                             int w,
                             int h) {
    int yy;
    int xx;
    int count = 0;
    for (yy = y; yy < y + h; ++yy) {
        if (yy < 0 || yy >= FB_H) {
            continue;
        }
        for (xx = x; xx < x + w; ++xx) {
            unsigned char idx;
            if (xx < 0 || xx >= FB_W) {
                continue;
            }
            idx = fb[yy * FB_W + xx] & 0x0F;
            if (idx != COLOR_BLACK && idx != COLOR_DARK_GRAY) {
                ++count;
            }
        }
    }
    return count;
}

static int check_rendered_lines(const unsigned char* fb, char* decoded) {
    static const int kLineBottomY[4] = {48, 59, 75, 86};
    char* cursor = decoded;
    int line = 0;
    int ok = 1;
    while (cursor && *cursor && line < 4) {
        char* next = strchr(cursor, '\n');
        char saved = '\0';
        if (next) {
            saved = *next;
            *next = '\0';
        }
        if (*cursor) {
            int charCount = (int)strlen(cursor);
            int textW = DM1_V1_InscriptionTextWidth(charCount);
            int textX = DM1_V1_InscriptionTextX(charCount);
            int textY = VIEWPORT_Y + kLineBottomY[line] - 7;
            int glyphPixels = glyph_pixel_count(fb, textX, textY, textW,
                                                 DM1_V1_INSCRIPTION_GLYPH_HEIGHT);
            int blackRow = row_color_count(fb, textY - 1, textX, textX + textW,
                                           COLOR_BLACK);
            int darkRow = row_color_count(fb, textY - 1, textX, textX + textW,
                                          COLOR_DARK_GRAY);
            CHECK(blackRow < textW - 2,
                  "line %d has no synthetic black patch row count=%d width=%d text=\"%s\"",
                  line, blackRow, textW, cursor);
            CHECK(darkRow < textW - 4,
                  "line %d has no synthetic dark-gray patch row count=%d width=%d",
                  line, darkRow, textW);
            CHECK(glyphPixels >= charCount * 2,
                  "line %d glyph pixels visible count=%d chars=%d",
                  line, glyphPixels, charCount);
            ok = ok && blackRow < textW - 2 &&
                 darkRow < textW - 4 &&
                 glyphPixels >= charCount * 2;
        }
        if (!next) {
            break;
        }
        *next = saved;
        cursor = next + 1;
        ++line;
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir = argc > 1 ? argv[1] : getenv("FIRESTAFF_DATA");
    M12_StartupMenuState menu;
    M11_GameViewState state;
    unsigned char fb[FB_W * FB_H];
    char decoded[INSCRIPTION_MAX_TEXT];
    int partyX = -1;
    int partyY = -1;
    int dir = -1;
    int ok;

    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 HoC inscription readability runtime probe ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.world.party.championCount = 0;

    decoded[0] = '\0';
    if (!find_front_inscription_pose(&state, &partyX, &partyY, &dir,
                                     decoded, sizeof(decoded))) {
        printf("SKIP no front-visible HoC TextString fixture found\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    printf("pose map=0 x=%d y=%d dir=%d text=\"%s\"\n",
           partyX, partyY, dir, decoded);

    state.world.party.mapIndex = HALL_MAP_INDEX;
    state.world.party.mapX = partyX;
    state.world.party.mapY = partyY;
    state.world.party.direction = dir;
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&state, fb, FB_W, FB_H);
    if (state.world.dungeon && HALL_MAP_INDEX < (int)state.world.dungeon->header.mapCount) {
        int wallCount = (int)state.world.dungeon->maps[HALL_MAP_INDEX].wallOrnamentCount;
        int slot;
        printf("wallOrnamentCount=%d ornamentCacheLoaded=%d\n",
               wallCount, state.ornamentCacheLoaded[HALL_MAP_INDEX]);
        for (slot = 0; slot <= wallCount && slot < 16; ++slot) {
            printf("wallOrnamentSlot[%d]=%d\n",
                   slot, state.wallOrnamentIndices[HALL_MAP_INDEX][slot]);
        }
    }

    ok = check_rendered_lines(fb, decoded);
    M11_GameView_Shutdown(&state);

    printf("summary=%d passed %d failed\n", g_pass, g_fail);
    return (ok && g_fail == 0) ? 0 : 1;
}
