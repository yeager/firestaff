/*
 * DM1 V1 Hall of Champions all-portrait wall-coordinate gate.
 *
 * This closes the broad "Gando/Halk/Wuuf on wrong walls" class:
 * every real C127 placement in the PC 3.4 Hall is classified against the
 * source square-aspect route: wall/fake-wall front placements resolve on
 * their source-visible D1C front wall, while open corridor placements must
 * not draw a floating C026 portrait.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against party direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289 only for
 *     M552_FRONT_WALL_ORNAMENT_ORDINAL.
 *   ReDMCSB DUNVIEW.C:3913-3928 draws C026 champion portraits at
 *     G0109 {96,127,35,63}, i.e. 32x29 at viewport-local (96,35).
 *   ReDMCSB DUNVIEW.C:4547-4581 decodes G0289 into the C026 atlas ordinal.
 */

#include "m11_game_view.h"
#include "firestaff_dm1_probe_data_dir.h"
#include "menu_startup_m12.h"
#include "asset_status_m12.h"
#include "render_sdl_m11.h"
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
    HALL_MAP_INDEX = 0,
    PORTRAIT_X = 96,
    PORTRAIT_Y = 33 + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    D1C_FRAME_X = 80,
    D1C_FRAME_Y = 29,
    D1C_FRAME_W = 64,
    D1C_FRAME_H = 43,
    C127_SENSOR_TYPE = 127,
    HOC_SCAN_LIMIT = 32,
    ALL_PORTRAIT_COUNT = 24,
    POSITIVE_WARM_THRESHOLD = 30,
    NEGATIVE_WARM_THRESHOLD = 30
};

typedef struct HocExpectedPose {
    int x;
    int y;
    int dir;
    int ordinal;
    const char* name;
    int sourceDrawsD1C;
} HocExpectedPose;

static const HocExpectedPose kExpectedPoses[] = {
    /* Verified PC34 C127 layout: ordinal=(x,y)face, pose = cell
     * adjacent on the visible side facing the sensor wall. */
    {9,  7, DIR_WEST,  0, "DAROOU",  1}, /* (8,7)E  */
    {7,  9, DIR_NORTH, 1, "HALK",    1}, /* (7,8)S  */
    {9,  9, DIR_SOUTH, 2, "WU TSE",  1}, /* (9,10)N */
    {16, 14, DIR_SOUTH, 3, "AZIZI",  1}, /* (16,15)N */
    {10, 5, DIR_SOUTH, 4, "LEIF",    1}, /* (10,6)N */
    {14, 3, DIR_NORTH, 5, "ELIJA",   1}, /* (14,2)S */
    {15, 4, DIR_EAST,  6, "SYRA",    1}, /* (16,4)W */
    {14, 6, DIR_SOUTH, 7, "TIGGY",   1}, /* (14,7)N */
    {13, 14, DIR_EAST, 8, "IAIDO",   1}, /* (14,14)W */
    {8, 15, DIR_NORTH, 9, "GANDO",   1}, /* (8,14)S */
    {7, 13, DIR_SOUTH, 10, "ZED",    1}, /* (7,14)N */
    {16, 8, DIR_NORTH, 11, "STAMM",  1}, /* (16,7)S */
    {12, 9, DIR_NORTH, 12, "LINFLAS", 1}, /* (12,8)S */
    {7, 16, DIR_SOUTH, 13, "WUUF",   1}, /* (7,17)N */
    {10, 4, DIR_NORTH, 14, "LEYLA",  1}, /* (10,3)S */
    {11, 10, DIR_SOUTH, 15, "MOPHUS", 1}, /* (11,11)N */
    {11, 15, DIR_SOUTH, 16, "CHANI", 1}, /* (11,16)N */
    {13, 12, DIR_NORTH, 17, "BORIS", 1}, /* (13,11)S */
    {9, 13, DIR_EAST,  18, "SONJA",  1}, /* (10,13)W */
    {14, 12, DIR_EAST, 19, "HAWK",   1}, /* (15,12)W */
    {17, 9, DIR_SOUTH, 20, "ALEX",   1}, /* (17,10)N */
    {16, 17, DIR_NORTH, 21, "HISSSSA", 1}, /* (16,16)S */
    {12, 13, DIR_WEST, 22, "GOTHMOG", 1}, /* (11,13)E */
    {6, 13, DIR_WEST,  23, "NABI",   1}  /* (5,13)E */
};

static const HocExpectedPose kPlayerRouteMirrorSweep[] = {
    /* Same verified PC34 C127 layout as kExpectedPoses. */
    {9,  7, DIR_WEST,  0, "DAROOU",  1},
    {7,  9, DIR_NORTH, 1, "HALK",    1},
    {9,  9, DIR_SOUTH, 2, "WU TSE",  1},
    {16, 14, DIR_SOUTH, 3, "AZIZI",  1},
    {10, 5, DIR_SOUTH, 4, "LEIF",    1},
    {14, 3, DIR_NORTH, 5, "ELIJA",   1},
    {15, 4, DIR_EAST,  6, "SYRA",    1},
    {14, 6, DIR_SOUTH, 7, "TIGGY",   1},
    {13, 14, DIR_EAST, 8, "IAIDO",   1},
    {8, 15, DIR_NORTH, 9, "GANDO",   1},
    {7, 13, DIR_SOUTH, 10, "ZED",    1},
    {16, 8, DIR_NORTH, 11, "STAMM",  1},
    {12, 9, DIR_NORTH, 12, "LINFLAS", 1},
    {7, 16, DIR_SOUTH, 13, "WUUF",   1},
    {10, 4, DIR_NORTH, 14, "LEYLA",  1},
    {11, 10, DIR_SOUTH, 15, "MOPHUS", 1},
    {11, 15, DIR_SOUTH, 16, "CHANI", 1},
    {13, 12, DIR_NORTH, 17, "BORIS", 1},
    {9, 13, DIR_EAST,  18, "SONJA",  1},
    {14, 12, DIR_EAST, 19, "HAWK",   1},
    {17, 9, DIR_SOUTH, 20, "ALEX",   1},
    {16, 17, DIR_NORTH, 21, "HISSSSA", 1},
    {12, 13, DIR_WEST, 22, "GOTHMOG", 1},
    {6, 13, DIR_WEST,  23, "NABI",   1}
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, fmt, ...) do { \
    if (cond) { \
        ++g_pass; \
        printf("  PASS: " fmt "\n", __VA_ARGS__); \
    } else { \
        ++g_fail; \
        printf("  FAIL: " fmt "\n", __VA_ARGS__); \
    } \
} while (0)

static const char* dir_name(int dir) {
    switch (dir) {
        case DIR_NORTH: return "N";
        case DIR_EAST: return "E";
        case DIR_SOUTH: return "S";
        case DIR_WEST: return "W";
        default: return "?";
    }
}

static void set_pose(M11_GameViewState* state, int x, int y, int dir) {
    state->world.party.mapIndex = HALL_MAP_INDEX;
    state->world.party.mapX = (int16_t)x;
    state->world.party.mapY = (int16_t)y;
    state->world.party.direction = (uint8_t)dir;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
}

static void clear_candidate_and_party(M11_GameViewState* state) {
    if (!state) {
        return;
    }
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->candidateMirrorRenameActive = 0;
    memset(&state->candidateMirrorRename, 0,
           sizeof(state->candidateMirrorRename));
    memset(state->world.party.champions, 0,
           sizeof(state->world.party.champions));
    state->world.party.championCount = 0;
    state->world.party.activeChampionIndex = -1;
    state->inventoryPanelActive = 0;
}

static const HocExpectedPose* expected_pose_for(int x, int y, int dir) {
    size_t i;
    for (i = 0; i < sizeof(kExpectedPoses) / sizeof(kExpectedPoses[0]); ++i) {
        if (kExpectedPoses[i].x == x &&
            kExpectedPoses[i].y == y &&
            kExpectedPoses[i].dir == dir) {
            return &kExpectedPoses[i];
        }
    }
    return NULL;
}

static int rect_warm_count(const unsigned char* fb, int x, int y, int w, int h) {
    int count = 0;
    int xx;
    int yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09: case 0x0A:
                case 0x0B: case 0x0E:
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

static int match_portrait(const M11_AssetSlot* portraits,
                          const unsigned char* fb,
                          int ordinal) {
    int x;
    int y;
    int compared = 0;
    int matched = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= ALL_PORTRAIT_COUNT) {
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            unsigned char src;
            unsigned char dst;
            if (srcX >= (int)portraits->width || srcY >= (int)portraits->height) {
                continue;
            }
            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) {
                ++matched;
            }
        }
    }
    return compared > 0 ? (matched * 100 / compared) : 0;
}

static int strongest_portrait_match(const M11_AssetSlot* portraits,
                                    const unsigned char* fb,
                                    int* outOrdinal) {
    int bestOrdinal = -1;
    int bestPct = 0;
    int ordinal;
    for (ordinal = 0; ordinal < ALL_PORTRAIT_COUNT; ++ordinal) {
        int pct = match_portrait(portraits, fb, ordinal);
        if (pct > bestPct) {
            bestPct = pct;
            bestOrdinal = ordinal;
        }
    }
    if (outOrdinal) {
        *outOrdinal = bestOrdinal;
    }
    return bestPct;
}

static void draw_pose(M11_GameViewState* state,
                      unsigned char* fb,
                      int x,
                      int y,
                      int dir,
                      int clearFirst) {
    set_pose(state, x, y, dir);
    if (clearFirst) {
        memset(fb, 0, FB_W * FB_H);
    }
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

static int raw_next_thing(const M11_GameViewState* state, unsigned short thing) {
    static const unsigned char s_thingDataByteCount[16] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    int type = THING_GET_TYPE(thing);
    int index = THING_GET_INDEX(thing);
    int byteCount;
    const unsigned char* raw;
    if (!state || !state->world.things || type < 0 || type >= 16 ||
        !state->world.things->rawThingData[type] ||
        index < 0 || index >= state->world.things->thingCounts[type]) {
        return THING_ENDOFLIST;
    }
    byteCount = (int)s_thingDataByteCount[type];
    if (byteCount <= 0) {
        return THING_ENDOFLIST;
    }
    raw = state->world.things->rawThingData[type] + (index * byteCount);
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static int first_thing_for_square(const M11_GameViewState* state,
                                  int mapIndex,
                                  int x,
                                  int y) {
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    if (!state || !state->world.dungeon || !state->world.things ||
        mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return THING_ENDOFLIST;
    }
    map = &state->world.dungeon->maps[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return THING_ENDOFLIST;
    }
    squareIndex = x * (int)map->height + y;
    return state->world.things->squareFirstThings[squareIndex];
}

static int square_index_for(const M11_GameViewState* state,
                            int mapIndex,
                            int x,
                            int y) {
    const struct DungeonMapDesc_Compat* map;
    if (!state || !state->world.dungeon ||
        mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return -1;
    }
    return x * (int)map->height + y;
}

static int square_element_for(const M11_GameViewState* state,
                              int mapIndex,
                              int x,
                              int y) {
    int squareIndex = square_index_for(state, mapIndex, x, y);
    unsigned char square;
    if (squareIndex < 0 || !state->world.dungeon->tiles ||
        !state->world.dungeon->tiles[mapIndex].squareData) {
        return -1;
    }
    square = state->world.dungeon->tiles[mapIndex].squareData[squareIndex];
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static unsigned short make_thing_ref(int type, int index, int cell) {
    return (unsigned short)(((cell & 3) << 14) |
                            ((type & 15) << 10) |
                            (index & 0x03FF));
}

static void dir_vector(int dir, int* outDx, int* outDy) {
    int dx = 0;
    int dy = 0;
    switch (dir & 3) {
        case DIR_NORTH: dy = -1; break;
        case DIR_EAST:  dx =  1; break;
        case DIR_SOUTH: dy =  1; break;
        case DIR_WEST:  dx = -1; break;
        default: break;
    }
    if (outDx) *outDx = dx;
    if (outDy) *outDy = dy;
}

static int find_open_front_corridor_pose(const M11_GameViewState* state,
                                         int* outPartyX,
                                         int* outPartyY,
                                         int* outDir,
                                         int* outFrontX,
                                         int* outFrontY) {
    const struct DungeonMapDesc_Compat* map;
    int x;
    int y;
    int dir;
    if (!state || !state->world.dungeon ||
        HALL_MAP_INDEX >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    map = &state->world.dungeon->maps[HALL_MAP_INDEX];
    for (y = 0; y < (int)map->height; ++y) {
        for (x = 0; x < (int)map->width; ++x) {
            if (square_element_for(state, HALL_MAP_INDEX, x, y) != DUNGEON_ELEMENT_CORRIDOR) {
                continue;
            }
            for (dir = 0; dir < 4; ++dir) {
                int dx;
                int dy;
                int frontX;
                int frontY;
                dir_vector(dir, &dx, &dy);
                frontX = x + dx;
                frontY = y + dy;
                if (square_element_for(state, HALL_MAP_INDEX, frontX, frontY) ==
                    DUNGEON_ELEMENT_CORRIDOR) {
                    if (outPartyX) *outPartyX = x;
                    if (outPartyY) *outPartyY = y;
                    if (outDir) *outDir = dir;
                    if (outFrontX) *outFrontX = frontX;
                    if (outFrontY) *outFrontY = frontY;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Find the index of the C127 sensor that currently carries the
 * given sensorData ordinal (e.g. 1 = HALK on the verified PC34
 * layout).  Layout-agnostic: scans the sensor array directly. */
static int find_c127_sensor_by_data(const M11_GameViewState* state,
                                    int ordinal) {
    int i;
    if (!state || !state->world.things || !state->world.things->sensors) {
        return -1;
    }
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == C127_SENSOR_TYPE &&
            (int)state->world.things->sensors[i].sensorData == ordinal) {
            return i;
        }
    }
    return -1;
}

static int check_actual_hall_poses(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits,
                                   unsigned char* fb) {
    int found = 0;
    int x;
    int y;
    int dir;
    int ok = 1;

    printf("\n[Group A] source-visible C127 poses in map 0\n");
    for (y = 0; y < HOC_SCAN_LIMIT; ++y) {
        for (x = 0; x < HOC_SCAN_LIMIT; ++x) {
            for (dir = 0; dir < 4; ++dir) {
                const HocExpectedPose* expected = expected_pose_for(x, y, dir);
                int actual;
                set_pose(state, x, y, dir);
                actual = M11_GameView_GetFrontMirrorOrdinal(state);
                if (expected) {
                    int pct;
                    char nameBuf[32];
                    nameBuf[0] = '\0';
                    ++found;
                    draw_pose(state, fb, x, y, dir, 1);
                    pct = match_portrait(portraits, fb, expected->ordinal);
                    (void)M11_GameView_GetMirrorNameByOrdinal(state, actual, nameBuf, (int)sizeof(nameBuf));
                    if (expected->sourceDrawsD1C) {
                        CHECK(actual == expected->ordinal,
                              "(%d,%d,%s) %s reports ordinal %d %s",
                              x, y, dir_name(dir), expected->name, actual, nameBuf);
                        CHECK(pct >= 90,
                              "(%d,%d,%s) %s C026 ordinal %d matches D1C cutout at %d%%",
                              x, y, dir_name(dir), expected->name, expected->ordinal, pct);
                        ok = ok && actual == expected->ordinal && pct >= 90;
                    } else {
                        CHECK(actual == -1,
                              "(%d,%d,%s) %s C127 is non-wall and reports no mirror ordinal: %d",
                              x, y, dir_name(dir), expected->name, actual);
                        CHECK(pct < 90,
                              "(%d,%d,%s) %s non-wall C127 does not paint ordinal %d at D1C pct=%d",
                              x, y, dir_name(dir), expected->name, expected->ordinal, pct);
                        ok = ok && actual == -1 && pct < 90;
                    }
                } else if (actual >= 0) {
                    char nameBuf[32];
                    nameBuf[0] = '\0';
                    (void)M11_GameView_GetMirrorNameByOrdinal(state, actual, nameBuf, (int)sizeof(nameBuf));
                    CHECK(0,
                          "unexpected mirror ordinal %d %s at (%d,%d,%s)",
                          actual, nameBuf, x, y, dir_name(dir));
                    ok = 0;
                }
            }
        }
    }
    CHECK(found == (int)(sizeof(kExpectedPoses) / sizeof(kExpectedPoses[0])),
          "found all expected source-visible poses: %d",
          found);
    return ok && found == (int)(sizeof(kExpectedPoses) / sizeof(kExpectedPoses[0]));
}

static int check_negative_redraws(M11_GameViewState* state,
                                  const M11_AssetSlot* portraits,
                                  unsigned char* fb) {
    static const HocExpectedPose kNegativeAfterPositive[] = {
        /* Wrong-wall variants of verified PC34 C127 poses: the front
         * cells (6,9), (7,12), (7,15), (12,14), (13,13) carry no
         * C127 sensor, so each must report -1 with no stale D1C
         * portrait. */
        {7, 9, DIR_WEST,  1, "HALK",    0},
        {7, 13, DIR_NORTH, 10, "ZED",   0},
        {7, 16, DIR_NORTH, 13, "WUUF",  0},
        {13, 14, DIR_WEST, 8, "IAIDO",  0},
        {12, 13, DIR_EAST, 22, "GOTHMOG", 0}
    };
    size_t i;
    int ok = 1;

    printf("\n[Group B] no stale portrait after turning to wrong walls\n");
    for (i = 0; i < sizeof(kNegativeAfterPositive) / sizeof(kNegativeAfterPositive[0]); ++i) {
        const HocExpectedPose* p = &kNegativeAfterPositive[i];
        int warm;
        int bestOrdinal = -1;
        int bestPct;
        draw_pose(state, fb, 7, 9, DIR_NORTH, 1);
        draw_pose(state, fb, p->x, p->y, p->dir, 0);
        warm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H);
        bestPct = strongest_portrait_match(portraits, fb, &bestOrdinal);
        CHECK(M11_GameView_GetFrontMirrorOrdinal(state) == -1,
              "(%d,%d,%s) wrong-wall route reports no front mirror",
              p->x, p->y, dir_name(p->dir));
        CHECK(warm < NEGATIVE_WARM_THRESHOLD,
              "(%d,%d,%s) wrong-wall D1C cutout has no warm portrait pixels: %d",
              p->x, p->y, dir_name(p->dir), warm);
        CHECK(bestPct < 90,
              "(%d,%d,%s) wrong-wall D1C cutout best portrait match ordinal=%d pct=%d",
              p->x, p->y, dir_name(p->dir), bestOrdinal, bestPct);
        ok = ok &&
            M11_GameView_GetFrontMirrorOrdinal(state) == -1 &&
            warm < NEGATIVE_WARM_THRESHOLD &&
            bestPct < 90;
    }
    return ok;
}

static int check_player_route_mirror_sweep(M11_GameViewState* state,
                                           const M11_AssetSlot* portraits,
                                           unsigned char* fb) {
    size_t i;
    int visibleCount = 0;
    int clickableCount = 0;
    int rejectedOpenCount = 0;
    int ok = 1;

    printf("\n[Group E] player-route HoC mirror sweep has no Sonja-only rendering/click drift\n");
    memset(fb, 0, FB_W * FB_H);
    clear_candidate_and_party(state);
    for (i = 0; i < sizeof(kPlayerRouteMirrorSweep) / sizeof(kPlayerRouteMirrorSweep[0]); ++i) {
        const HocExpectedPose* p = &kPlayerRouteMirrorSweep[i];
        int actual;
        int pct;
        int bestOrdinal = -1;
        int bestPct;
        int warm;

        /* Do not clear the framebuffer here.  This catches the packaged-app
         * symptom class where a previous D1C portrait remains visible or a
         * later mirror fails to repaint while walking through HoC. */
        draw_pose(state, fb, p->x, p->y, p->dir, 0);
        actual = M11_GameView_GetFrontMirrorOrdinal(state);
        pct = match_portrait(portraits, fb, p->ordinal);
        bestPct = strongest_portrait_match(portraits, fb, &bestOrdinal);
        warm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H);

        if (p->sourceDrawsD1C) {
            M11_GameInputResult clickRc;
            CHECK(actual == p->ordinal,
                  "route step %zu (%d,%d,%s) %s resolves ordinal %d",
                  i, p->x, p->y, dir_name(p->dir), p->name, actual);
            CHECK(pct >= 90 && bestOrdinal == p->ordinal,
                  "route step %zu %s paints expected portrait pct=%d best=%d/%d",
                  i, p->name, pct, bestOrdinal, bestPct);
            clickRc = M11_GameView_HandlePointerButton(state,
                                                       PORTRAIT_X + PORTRAIT_W / 2,
                                                       PORTRAIT_Y + PORTRAIT_H / 2,
                                                       M11_DM1_MOUSE_MASK_LEFT);
            CHECK(clickRc == M11_GAME_INPUT_REDRAW &&
                  state->candidateMirrorPanelActive == 1 &&
                  state->candidateMirrorOrdinal == p->ordinal,
                  "route step %zu %s click opens candidate ordinal=%d rc=%d",
                  i, p->name, state->candidateMirrorOrdinal, (int)clickRc);
            ok = ok && actual == p->ordinal && pct >= 90 &&
                 bestOrdinal == p->ordinal &&
                 clickRc == M11_GAME_INPUT_REDRAW &&
                 state->candidateMirrorPanelActive == 1 &&
                 state->candidateMirrorOrdinal == p->ordinal;
            if (actual == p->ordinal && pct >= 90 && bestOrdinal == p->ordinal) {
                ++visibleCount;
            }
            if (clickRc == M11_GAME_INPUT_REDRAW &&
                state->candidateMirrorPanelActive == 1 &&
                state->candidateMirrorOrdinal == p->ordinal) {
                ++clickableCount;
            }
            clear_candidate_and_party(state);
        } else {
            CHECK(actual == -1,
                  "route step %zu (%d,%d,%s) %s open/non-wall C127 remains non-front ordinal=%d",
                  i, p->x, p->y, dir_name(p->dir), p->name, actual);
            CHECK(warm < NEGATIVE_WARM_THRESHOLD && bestPct < 90,
                  "route step %zu %s clears D1C portrait warm=%d best=%d/%d",
                  i, p->name, warm, bestOrdinal, bestPct);
            ok = ok && actual == -1 &&
                 warm < NEGATIVE_WARM_THRESHOLD &&
                 bestPct < 90;
            if (actual == -1 && warm < NEGATIVE_WARM_THRESHOLD &&
                bestPct < 90) {
                ++rejectedOpenCount;
            }
        }
    }

    CHECK(visibleCount == 24,
          "player-route visible wall mirrors count=%d",
          visibleCount);
    CHECK(clickableCount == 24,
          "player-route clickable wall mirrors count=%d",
          clickableCount);
    CHECK(rejectedOpenCount == 0,
          "player-route open/non-wall C127 rejection count=%d",
          rejectedOpenCount);
    return ok && visibleCount == 24 && clickableCount == 24 &&
           rejectedOpenCount == 0;
}

static int check_open_front_c127_rejected(M11_GameViewState* state,
                                          const M11_AssetSlot* portraits,
                                          unsigned char* fb) {
    int sensorIndex;
    unsigned short savedData;
    unsigned short savedFirstThing;
    int partyX = -1;
    int partyY = -1;
    int dir = -1;
    int frontX = -1;
    int frontY = -1;
    int frontSquareIndex;
    int visibleWallCell;
    int actual;
    int bestOrdinal = -1;
    int bestPct;
    int ok = 1;

    printf("\n[Group D] open corridor C127 cannot draw a floating portrait\n");
    sensorIndex = find_c127_sensor_by_data(state, 1 /* HALK */);
    CHECK(sensorIndex >= 0, "found reusable HALK-route C127 sensor index %d", sensorIndex);
    CHECK(find_open_front_corridor_pose(state, &partyX, &partyY, &dir, &frontX, &frontY) == 1,
          "found open corridor front pose party=(%d,%d,%s) front=(%d,%d)",
          partyX, partyY, dir_name(dir), frontX, frontY);
    if (sensorIndex < 0 || partyX < 0 || partyY < 0 || dir < 0) {
        return 0;
    }

    frontSquareIndex = square_index_for(state, HALL_MAP_INDEX, frontX, frontY);
    if (frontSquareIndex < 0 || !state->world.things ||
        !state->world.things->squareFirstThings) {
        CHECK(0, "front square index is valid: %d", frontSquareIndex);
        return 0;
    }

    savedData = state->world.things->sensors[sensorIndex].sensorData;
    savedFirstThing = state->world.things->squareFirstThings[frontSquareIndex];
    visibleWallCell = (dir + 2) & 3;

    state->world.things->sensors[sensorIndex].sensorData = 10; /* ZED */
    state->world.things->squareFirstThings[frontSquareIndex] =
        make_thing_ref(THING_TYPE_SENSOR, sensorIndex, visibleWallCell);

    draw_pose(state, fb, partyX, partyY, dir, 1);
    actual = M11_GameView_GetFrontMirrorOrdinal(state);
    bestPct = strongest_portrait_match(portraits, fb, &bestOrdinal);

    CHECK(actual == -1,
          "open front corridor with C127 reports no mirror ordinal: %d",
          actual);
    CHECK(bestPct < 90,
          "open front corridor D1C cutout does not contain GANDO/C026 portrait: ordinal=%d pct=%d",
          bestOrdinal, bestPct);

    ok = ok && actual == -1 && bestPct < 90;

    state->world.things->squareFirstThings[frontSquareIndex] = savedFirstThing;
    state->world.things->sensors[sensorIndex].sensorData = savedData;
    return ok;
}

static int check_seeded_all_ordinals(M11_GameViewState* state,
                                     const M11_AssetSlot* portraits,
                                     unsigned char* fb) {
    int sensorIndex;
    unsigned short savedData;
    int ordinal;
    int ok = 1;

    printf("\n[Group C] all 24 C026 ordinals share the same D1C wall cutout and C127 click route\n");
    /* The real HALK route (verified PC34 layout): party at (7,9)
     * NORTH sees square (7,8), whose C127 sensor sits on visible
     * cell bit (DIR_NORTH + 2) & 3 = 2 (south wall of (7,8)).
     * Mutating only sensorData keeps the source wall coordinate constant
     * while sweeping every C026 champion atlas ordinal. */
    sensorIndex = find_c127_sensor_by_data(state, 1 /* HALK */);
    CHECK(sensorIndex >= 0, "found HALK-route C127 sensor index %d", sensorIndex);
    if (sensorIndex < 0) {
        return 0;
    }
    savedData = state->world.things->sensors[sensorIndex].sensorData;
    for (ordinal = 0; ordinal < ALL_PORTRAIT_COUNT; ++ordinal) {
        int actual;
        int pct;
        M11_GameInputResult clickRc;
        state->world.things->sensors[sensorIndex].sensorData = (unsigned short)ordinal;
        draw_pose(state, fb, 7, 9, DIR_NORTH, 1);
        actual = M11_GameView_GetFrontMirrorOrdinal(state);
        pct = match_portrait(portraits, fb, ordinal);
        CHECK(actual == ordinal,
              "seeded C127 sensorData=%d resolves front ordinal %d",
              ordinal, actual);
        CHECK(pct >= 90,
              "seeded ordinal %d paints at D1C cutout pct=%d",
              ordinal, pct);
        clickRc = M11_GameView_HandlePointerButton(state,
                                                   PORTRAIT_X + PORTRAIT_W / 2,
                                                   PORTRAIT_Y + PORTRAIT_H / 2,
                                                   M11_DM1_MOUSE_MASK_LEFT);
        CHECK(clickRc == M11_GAME_INPUT_REDRAW,
              "seeded ordinal %d C080/C127 portrait click requests redraw rc=%d",
              ordinal, (int)clickRc);
        CHECK(state->candidateMirrorPanelActive == 1 &&
              state->candidateMirrorOrdinal == ordinal &&
              state->world.party.championCount == 1,
              "seeded ordinal %d opens candidate panel ordinal=%d championCount=%d",
              ordinal, state->candidateMirrorOrdinal,
              state->world.party.championCount);
        ok = ok && actual == ordinal && pct >= 90 &&
             clickRc == M11_GAME_INPUT_REDRAW &&
             state->candidateMirrorPanelActive == 1 &&
             state->candidateMirrorOrdinal == ordinal &&
             state->world.party.championCount == 1;
        clear_candidate_and_party(state);
    }
    state->world.things->sensors[sensorIndex].sensorData = savedData;
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    char narrowed[1024];
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    int d1cX = -1;
    int d1cY = -1;
    int d1cW = -1;
    int d1cH = -1;
    int assetsAvailable;
    int ok;

    dataDir = argc > 1 ? argv[1] : getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = firestaff_dm1_probe_narrow_data_dir(dataDir, narrowed, sizeof(narrowed));

    printf("=== DM1 V1 Hall of Champions all-portrait wall-coordinate gate ===\n");
    printf("dataDir=%s\n", dataDir);
    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    assetsAvailable = M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1");
    if (!assetsAvailable) {
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
    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("SKIP C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    ok = M11_GameView_GetD1CWallOrnamentZone(&state, &d1cX, &d1cY, &d1cW, &d1cH);
    CHECK(ok == 1 && d1cX == D1C_FRAME_X && d1cY == D1C_FRAME_Y &&
              d1cW == D1C_FRAME_W && d1cH == D1C_FRAME_H,
          "D1C mirror frame is source box (%d,%d,%d,%d)",
          d1cX, d1cY, d1cW, d1cH);

    ok = check_actual_hall_poses(&state, portraits, fb);
    ok = check_negative_redraws(&state, portraits, fb) && ok;
    ok = check_player_route_mirror_sweep(&state, portraits, fb) && ok;
    ok = check_open_front_c127_rejected(&state, portraits, fb) && ok;
    ok = check_seeded_all_ordinals(&state, portraits, fb) && ok;

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (ok && g_fail == 0) ? 0 : 1;
}
