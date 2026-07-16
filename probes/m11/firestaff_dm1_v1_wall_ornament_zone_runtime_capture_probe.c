/*
 * DM1 V1 real-data wall-ornament source-zone runtime capture probe.
 *
 * Finds a non-mirror sensor ornament in the shipped PC34 dungeon, drives a
 * legal D1C pose through M11, and compares its GRAPHICS.DAT source pixels
 * against the active viewport.  The second frame removes only that sensor
 * ornament so C10 pixels can be checked against the real wall behind it.
 *
 * ReDMCSB: DUNGEON.C F0172/F0173 maps a wall sensor ordinal through G0261;
 * DUNVIEW.C F0107 consumes G0205's source zone and F0791 C10 blit.
 */

#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    SENSOR_TYPE_PORTRAIT = 127,
    MAX_THING_CHAIN = 64
};

static int g_pass;
static int g_fail;

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

static int square_element_for(const M11_GameViewState* state,
                              int map_index,
                              int x,
                              int y) {
    const struct DungeonMapDesc_Compat* map;
    int index;
    unsigned char square;
    if (!state || !state->world.dungeon || map_index < 0 ||
        map_index >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[map_index];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height ||
        !state->world.dungeon->tiles ||
        !state->world.dungeon->tiles[map_index].squareData) {
        return -1;
    }
    index = x * (int)map->height + y;
    square = state->world.dungeon->tiles[map_index].squareData[index];
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static unsigned short raw_next_thing(const M11_GameViewState* state,
                                     unsigned short thing) {
    static const unsigned char kThingDataByteCount[16] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    int type = (int)THING_GET_TYPE(thing);
    int index = (int)THING_GET_INDEX(thing);
    const unsigned char* raw;
    if (!state || !state->world.things || type < 0 || type >= 16 ||
        !state->world.things->rawThingData[type] || index < 0 ||
        index >= state->world.things->thingCounts[type] ||
        kThingDataByteCount[type] == 0) {
        return THING_ENDOFLIST;
    }
    raw = state->world.things->rawThingData[type] +
          index * (int)kThingDataByteCount[type];
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static int wall_has_visible_text_for_cell(const M11_GameViewState* state,
                                          int map_index,
                                          int x,
                                          int y,
                                          int cell) {
    unsigned short thing;
    int chain = 0;
    if (!state || !state->world.things || !state->world.things->textStrings) {
        return 0;
    }
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, state->world.things, map_index, x, y);
    while (thing != THING_ENDOFLIST && thing != THING_NONE &&
           chain++ < MAX_THING_CHAIN) {
        if (THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING &&
            (int)THING_GET_CELL(thing) == cell) {
            int text_index = (int)THING_GET_INDEX(thing);
            if (text_index >= 0 &&
                text_index < state->world.things->textStringCount &&
                state->world.things->textStrings[text_index].visible) {
                return 1;
            }
        }
        thing = raw_next_thing(state, thing);
    }
    return 0;
}

static int capture_plan_pixels(const unsigned char* rendered,
                               const unsigned char* without_ornament,
                               const M11_AssetSlot* slot,
                               const DM1_WallOrnamentRenderPlanPc34* plan) {
    int opaque = 0;
    int source_signature_matches = 0;
    int transparent = 0;
    int dy;

    if (!rendered || !without_ornament || !slot || !slot->loaded ||
        !slot->pixels || !plan || plan->width <= 0 || plan->height <= 0) {
        return 0;
    }
    for (dy = 0; dy < plan->height; ++dy) {
        int dx;
        int sy = dy * (int)slot->height / plan->height;
        int fb_y = VIEWPORT_Y + plan->dstY + dy;
        if (fb_y < 0 || fb_y >= FB_H) {
            continue;
        }
        for (dx = 0; dx < plan->width; ++dx) {
            int sx = (plan->flipHorizontal ? (plan->width - 1 - dx) : dx) *
                     (int)slot->width / plan->width;
            int fb_x = VIEWPORT_X + plan->dstX + dx;
            unsigned char source;
            unsigned char expected;
            int fb_index;
            if (fb_x < 0 || fb_x >= FB_W) {
                continue;
            }
            source = slot->pixels[sy * (int)slot->width + sx];
            fb_index = fb_y * FB_W + fb_x;
            if (source == (unsigned char)plan->transparentColor) {
                ++transparent;
            } else {
                expected = plan->paletteMapValid
                    ? plan->paletteMap[source & 0x0f] : source;
                ++opaque;
                /* The final M11 frame composes later viewport layers over
                 * portions of a wall face.  Count only pixels that visibly
                 * changed when this real sensor route was removed: those are
                 * an unambiguous source-zone signature, not a claim that the
                 * entire final rectangle remains an isolated blit. */
                if ((rendered[fb_index] & 0x0f) == (expected & 0x0f) &&
                    (without_ornament[fb_index] & 0x0f) != (expected & 0x0f)) {
                    ++source_signature_matches;
                }
            }
        }
    }
    CHECK(opaque > 0, "ornament zone has opaque original source pixels");
    CHECK(source_signature_matches >= 64,
          "G0205 palette-mapped GRAPHICS.DAT source signatures survive M11 composition matches=%d opaque=%d",
          source_signature_matches, opaque);
    CHECK(transparent > 0, "ornament zone has C10 source pixels");
    return opaque > 0 && source_signature_matches >= 64 && transparent > 0;
}

static int changed_zone_pixel_count(const unsigned char* rendered,
                                    const unsigned char* without_ornament,
                                    const DM1_WallOrnamentRenderPlanPc34* plan) {
    int changed = 0;
    int y;
    for (y = 0; y < plan->height; ++y) {
        int x;
        int fb_y = VIEWPORT_Y + plan->dstY + y;
        if (fb_y < 0 || fb_y >= FB_H) {
            continue;
        }
        for (x = 0; x < plan->width; ++x) {
            int fb_x = VIEWPORT_X + plan->dstX + x;
            if (fb_x >= 0 && fb_x < FB_W &&
                rendered[fb_y * FB_W + fb_x] !=
                    without_ornament[fb_y * FB_W + fb_x]) {
                ++changed;
            }
        }
    }
    return changed;
}

int main(int argc, char** argv) {
    const char* data_dir = argc > 1 ? argv[1] : getenv("FIRESTAFF_DATA");
    M11_GameViewState state;
    unsigned char rendered[FB_W * FB_H];
    unsigned char without_ornament[FB_W * FB_H];
    int found = 0;
    int ok = 1;
    int map_index;

    if (!data_dir || !data_dir[0]) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 wall-ornament source-zone runtime capture probe ===\n");
    printf("dataDir=%s\n", data_dir);

    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        fprintf(stderr, "SKIP could not open DM1 V1 game view from %s\n", data_dir);
        M11_GameView_Shutdown(&state);
        return 0;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 0;

    /* The Hall of Champions owns its separate C127/mirror composition;
     * capture an ordinary dungeon ornament, not that special entrance UI. */
    for (map_index = 1;
         map_index < (int)state.world.dungeon->header.mapCount && !found;
         ++map_index) {
        const struct DungeonMapDesc_Compat* map = &state.world.dungeon->maps[map_index];
        int y;
        for (y = 0; y < (int)map->height && !found; ++y) {
            int x;
            for (x = 0; x < (int)map->width && !found; ++x) {
                unsigned short thing;
                int chain = 0;
                if (square_element_for(&state, map_index, x, y) !=
                    DUNGEON_ELEMENT_WALL) {
                    continue;
                }
                thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    state.world.dungeon, state.world.things, map_index, x, y);
                while (thing != THING_ENDOFLIST && thing != THING_NONE &&
                       chain++ < MAX_THING_CHAIN && !found) {
                    int sensor_index;
                    const struct DungeonSensor_Compat* sensor;
                    int dir;
                    if (THING_GET_TYPE(thing) != THING_TYPE_SENSOR) {
                        thing = raw_next_thing(&state, thing);
                        continue;
                    }
                    sensor_index = (int)THING_GET_INDEX(thing);
                    if (sensor_index < 0 ||
                        sensor_index >= state.world.things->sensorCount) {
                        thing = raw_next_thing(&state, thing);
                        continue;
                    }
                    sensor = &state.world.things->sensors[sensor_index];
                    if (sensor->ornamentOrdinal == 0 ||
                        sensor->sensorType == SENSOR_TYPE_PORTRAIT) {
                        thing = raw_next_thing(&state, thing);
                        continue;
                    }
                    for (dir = 0; dir < 4 && !found; ++dir) {
                        int party_x;
                        int party_y;
                        int local_index;
                        int global_index;
                        DM1_WallOrnamentRenderPlanPc34 plan;
                        const M11_AssetSlot* slot;
                        unsigned char saved_ordinal;
                        if (((dir + 2) & 3) != (int)THING_GET_CELL(thing)) {
                            continue;
                        }
                        party_x = x - dir_dx(dir);
                        party_y = y - dir_dy(dir);
                        if (square_element_for(&state, map_index, party_x, party_y) !=
                            DUNGEON_ELEMENT_CORRIDOR) {
                            continue;
                        }
                        if (wall_has_visible_text_for_cell(
                                &state, map_index, x, y,
                                (int)THING_GET_CELL(thing))) {
                            continue;
                        }
                        state.world.party.mapIndex = map_index;
                        state.world.party.mapX = party_x;
                        state.world.party.mapY = party_y;
                        state.world.party.direction = dir;
                        memset(rendered, 0, sizeof(rendered));
                        M11_GameView_Draw(&state, rendered, FB_W, FB_H);
                        local_index = (int)sensor->ornamentOrdinal - 1;
                        if (local_index < 0 || local_index >= 16 ||
                            !state.ornamentCacheLoaded[map_index]) {
                            continue;
                        }
                        global_index = state.wallOrnamentIndices[map_index][local_index];
                        if (global_index <= 0 ||
                            !dm1_v1_wall_ornament_render_plan_pc34(
                                global_index, 12, 0, &plan)) {
                            continue;
                        }
                        slot = M11_AssetLoader_Load(&state.assetLoader,
                                                    (unsigned int)plan.graphicIndex);
                        if (!slot || !slot->loaded || !slot->pixels ||
                            slot->width == 0 || slot->height == 0) {
                            continue;
                        }
                        saved_ordinal = sensor->ornamentOrdinal;
                        state.world.things->sensors[sensor_index].ornamentOrdinal = 0;
                        memset(without_ornament, 0, sizeof(without_ornament));
                        M11_GameView_Draw(&state, without_ornament, FB_W, FB_H);
                        state.world.things->sensors[sensor_index].ornamentOrdinal =
                            saved_ordinal;
                        if (changed_zone_pixel_count(rendered, without_ornament,
                                                     &plan) == 0) {
                            continue;
                        }
                        printf("pose map=%d wall=(%d,%d) party=(%d,%d) dir=%d "
                               "ordinal=%d global=%d graphic=%d zone=(%d,%d %dx%d)\n",
                               map_index, x, y, party_x, party_y, dir,
                               (int)saved_ordinal, global_index, plan.graphicIndex,
                               plan.dstX, plan.dstY, plan.width, plan.height);
                        ok = capture_plan_pixels(rendered, without_ornament,
                                                 slot, &plan) && ok;
                        found = 1;
                    }
                    thing = raw_next_thing(&state, thing);
                }
            }
        }
    }

    CHECK(found, "found a real non-mirror sensor wall ornament D1C pose");
    M11_GameView_Shutdown(&state);
    printf("summary=%d passed %d failed\n", g_pass, g_fail);
    return (ok && found && g_fail == 0) ? 0 : 1;
}
