/*
 * DM1 V1 Hall of Champions all-portrait wall-coordinate gate.
 *
 * This closes the broad "Gando/Halk/Wuuf on wrong walls" class:
 * every real C127 pose in the PC 3.4 Hall must resolve only on its
 * source-visible front wall, and every C026 champion portrait must blit
 * inside the D1C mirror cutout at viewport (96,35,32,29).
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
#include "menu_startup_m12.h"
#include "asset_status_m12.h"
#include "render_sdl_m11.h"

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
} HocExpectedPose;

static const HocExpectedPose kExpectedPoses[] = {
    {2,  1, DIR_EAST,  8, "IAIDO"},
    {2,  1, DIR_SOUTH, 4, "LEIF"},
    {1,  2, DIR_NORTH, 1, "HALK"},
    {1,  3, DIR_EAST, 18, "SONJA"},
    {1,  3, DIR_SOUTH,10, "GANDO"},
    {2,  3, DIR_EAST, 19, "HAWK"},
    {2,  4, DIR_EAST,  6, "SYRA"},
    {2,  4, DIR_SOUTH,15, "MOPHUS"},
    {1,  5, DIR_SOUTH,13, "WUUF"},
    {3,  6, DIR_NORTH,11, "STAMM"},
    {3,  6, DIR_WEST, 22, "GOTHMOG"},
    {2,  7, DIR_SOUTH,16, "CHANI"},
    {3,  7, DIR_SOUTH, 3, "AZIZI"},
    {2,  8, DIR_WEST,  0, "DAROOU"},
    {1, 10, DIR_NORTH, 9, "ZED"},
    {2, 10, DIR_NORTH,12, "LINFLAS"},
    {3, 10, DIR_NORTH,21, "HISSSSA"},
    {3, 11, DIR_SOUTH,20, "ALEX"},
    {2, 13, DIR_NORTH,17, "BORIS"},
    {1, 14, DIR_SOUTH, 2, "WU TSE"},
    {1, 16, DIR_WEST, 23, "NABI"},
    {2, 16, DIR_NORTH, 5, "ELIJA"},
    {2, 17, DIR_SOUTH, 7, "TIGGY"},
    {1, 19, DIR_NORTH,14, "LEYLA"}
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

static int expected_ordinal_for_pose(int x, int y, int dir) {
    size_t i;
    for (i = 0; i < sizeof(kExpectedPoses) / sizeof(kExpectedPoses[0]); ++i) {
        if (kExpectedPoses[i].x == x &&
            kExpectedPoses[i].y == y &&
            kExpectedPoses[i].dir == dir) {
            return kExpectedPoses[i].ordinal;
        }
    }
    return -1;
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

static int find_c127_sensor_on_cell_bit(const M11_GameViewState* state,
                                        int x,
                                        int y,
                                        int cellBit) {
    unsigned short thing = (unsigned short)first_thing_for_square(state, HALL_MAP_INDEX, x, y);
    while (thing != THING_ENDOFLIST && thing != THING_NONE) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_SENSOR &&
            (int)THING_GET_CELL(thing) == cellBit &&
            index >= 0 &&
            index < state->world.things->sensorCount &&
            state->world.things->sensors[index].sensorType == C127_SENSOR_TYPE) {
            return index;
        }
        thing = (unsigned short)raw_next_thing(state, thing);
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
                int expected = expected_ordinal_for_pose(x, y, dir);
                int actual;
                set_pose(state, x, y, dir);
                actual = M11_GameView_GetFrontMirrorOrdinal(state);
                if (expected >= 0) {
                    int pct;
                    char nameBuf[32];
                    nameBuf[0] = '\0';
                    ++found;
                    draw_pose(state, fb, x, y, dir, 1);
                    pct = match_portrait(portraits, fb, expected);
                    (void)M11_GameView_GetMirrorNameByOrdinal(state, actual, nameBuf, (int)sizeof(nameBuf));
                    CHECK(actual == expected,
                          "(%d,%d,%s) reports ordinal %d %s",
                          x, y, dir_name(dir), actual, nameBuf);
                    CHECK(pct >= 90,
                          "(%d,%d,%s) C026 ordinal %d matches D1C cutout at %d%%",
                          x, y, dir_name(dir), expected, pct);
                    ok = ok && actual == expected && pct >= 90;
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
        {1, 2, DIR_WEST,  1, "HALK"},
        {1, 3, DIR_NORTH,10, "GANDO"},
        {1, 5, DIR_NORTH,13, "WUUF"},
        {2, 1, DIR_WEST,  8, "IAIDO"},
        {3, 6, DIR_EAST, 22, "GOTHMOG"}
    };
    size_t i;
    int ok = 1;

    printf("\n[Group B] no stale portrait after turning to wrong walls\n");
    for (i = 0; i < sizeof(kNegativeAfterPositive) / sizeof(kNegativeAfterPositive[0]); ++i) {
        const HocExpectedPose* p = &kNegativeAfterPositive[i];
        int warm;
        int bestOrdinal = -1;
        int bestPct;
        draw_pose(state, fb, 1, 2, DIR_NORTH, 1);
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

static int check_seeded_all_ordinals(M11_GameViewState* state,
                                     const M11_AssetSlot* portraits,
                                     unsigned char* fb) {
    int sensorIndex;
    unsigned short savedData;
    int ordinal;
    int ok = 1;

    printf("\n[Group C] all 24 C026 ordinals share the same D1C wall cutout\n");
    /* The real HALK route: party at (1,2) NORTH sees square (1,1),
     * whose C127 sensor sits on visible cell bit (DIR_NORTH + 2) & 3 = 2.
     * Mutating only sensorData keeps the source wall coordinate constant
     * while sweeping every C026 champion atlas ordinal. */
    sensorIndex = find_c127_sensor_on_cell_bit(state, 1, 1, 2);
    CHECK(sensorIndex >= 0, "found HALK-route C127 sensor index %d", sensorIndex);
    if (sensorIndex < 0) {
        return 0;
    }
    savedData = state->world.things->sensors[sensorIndex].sensorData;
    for (ordinal = 0; ordinal < ALL_PORTRAIT_COUNT; ++ordinal) {
        int actual;
        int pct;
        state->world.things->sensors[sensorIndex].sensorData = (unsigned short)ordinal;
        draw_pose(state, fb, 1, 2, DIR_NORTH, 1);
        actual = M11_GameView_GetFrontMirrorOrdinal(state);
        pct = match_portrait(portraits, fb, ordinal);
        CHECK(actual == ordinal,
              "seeded C127 sensorData=%d resolves front ordinal %d",
              ordinal, actual);
        CHECK(pct >= 90,
              "seeded ordinal %d paints at D1C cutout pct=%d",
              ordinal, pct);
        ok = ok && actual == ordinal && pct >= 90;
    }
    state->world.things->sensors[sensorIndex].sensorData = savedData;
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
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
    ok = check_seeded_all_ordinals(&state, portraits, fb) && ok;

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (ok && g_fail == 0) ? 0 : 1;
}
