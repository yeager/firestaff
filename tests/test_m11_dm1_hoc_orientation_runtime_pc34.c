/* Real PC34 Hall-of-Champions orientation regression.
 *
 * The initial Hall pose may face each cardinal direction before a champion
 * has been selected. ReDMCSB DUNVIEW.C F0128 must keep the viewport visible
 * for every pose; a turn must not clear it merely because a far view square
 * lies outside the 5x5 Hall map.
 */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum {
    FRAMEBUFFER_WIDTH = 320,
    FRAMEBUFFER_HEIGHT = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_WIDTH = 224,
    VIEWPORT_HEIGHT = 136
};

static int viewport_nonblack_pixel_count(const unsigned char *framebuffer)
{
    int count = 0;
    int y;
    for (y = VIEWPORT_Y; y < VIEWPORT_Y + VIEWPORT_HEIGHT; ++y) {
        int x;
        for (x = 0; x < VIEWPORT_WIDTH; ++x) {
            if (framebuffer[y * FRAMEBUFFER_WIDTH + x] != 0) {
                ++count;
            }
        }
    }
    return count;
}

static int step_x(int direction)
{
    return (direction & 3) == DIR_EAST ? 1 :
           (direction & 3) == DIR_WEST ? -1 : 0;
}

static int step_y(int direction)
{
    return (direction & 3) == DIR_SOUTH ? 1 :
           (direction & 3) == DIR_NORTH ? -1 : 0;
}

static int select_first_real_hoc_mirror(M11_GameViewState *state,
                                        unsigned char *framebuffer)
{
    const struct DungeonMapDesc_Compat *map;
    int y;

    if (!state || !state->world.dungeon || !state->world.things ||
        !state->world.things->sensors) {
        return 0;
    }
    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, x, y);
            int safety = 0;
            while (thing != THING_NONE && thing != THING_ENDOFLIST &&
                   safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    const int sensor_index = (int)THING_GET_INDEX(thing);
                    const int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                    const int party_x = x - step_x(direction);
                    const int party_y = y - step_y(direction);
                    if (sensor_index >= 0 &&
                        sensor_index < state->world.things->sensorCount &&
                        state->world.things->sensors[sensor_index].sensorType == 127 &&
                        party_x >= 0 && party_y >= 0 &&
                        party_x < (int)map->width && party_y < (int)map->height) {
                        state->world.party.mapIndex = 0;
                        state->world.party.mapX = party_x;
                        state->world.party.mapY = party_y;
                        state->world.party.direction = direction;
                        memset(framebuffer, 0,
                               FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                        M11_GameView_Draw(state, framebuffer,
                                          FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
                        if (M11_GameView_GetFrontMirrorOrdinal(state) ==
                            (int)state->world.things->sensors[sensor_index].sensorData) {
                            printf("real_hoc_mirror: wall=%d,%d party=%d,%d dir=%d ordinal=%u\n",
                                   x, y, party_x, party_y, direction,
                                   (unsigned int)state->world.things->sensors[sensor_index].sensorData);
                            return 1;
                        }
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(
                    state->world.things, thing);
            }
        }
    }
    return 0;
}

static int count_real_hoc_f0115_items(const M11_GameViewState *state)
{
    const struct DungeonMapDesc_Compat *map;
    int item_count = 0;
    int y;

    if (!state || !state->world.dungeon || !state->world.things) {
        return 0;
    }
    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            DM1_F0115WorldCandidatesPc34 candidates;
            if (dm1_v1_f0115_world_candidates_pc34(
                    &state->world, 0, x, y, NULL, NULL, &candidates) &&
                candidates.valid) {
                item_count += candidates.itemCount;
            }
        }
    }
    return item_count;
}

/* Find a real door in the mounted PC 3.4 dungeon, place the presentation
 * tuple one square in front of it, and prove that F0128's authenticated
 * DOORPASS1 step reached the scheduler callback.  No dungeon byte is
 * changed: the temporary party tuple is only a viewport probe over the
 * original DUNGEON.DAT. */
static int exercise_real_f0128_door_pass1(
    M11_GameViewState *state, unsigned char *framebuffer)
{
    int mapIndex;
    if (!state || !state->world.dungeon) return 0;
    for (mapIndex = 0;
         mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map =
            &state->world.dungeon->maps[mapIndex];
        const unsigned char *squares =
            state->world.dungeon->tiles[mapIndex].squareData;
        int x;
        if (!squares) continue;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                int direction;
                unsigned char square = squares[x * (int)map->height + y];
                if (((square & DUNGEON_SQUARE_MASK_TYPE) >> 5) !=
                    DUNGEON_ELEMENT_DOOR) continue;
                for (direction = 0; direction < 4; ++direction) {
                    M11_Dm1F0128PerSquareSchedulerReceipt receipt;
                    int partyX = x - step_x(direction);
                    int partyY = y - step_y(direction);
                    if (partyX < 0 || partyY < 0 ||
                        partyX >= (int)map->width ||
                        partyY >= (int)map->height) continue;
                    state->world.party.mapIndex = mapIndex;
                    state->world.party.mapX = partyX;
                    state->world.party.mapY = partyY;
                    state->world.party.direction = direction;
                    memset(framebuffer, 0,
                           FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                    M11_GameView_Draw(state, framebuffer,
                                      FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
                    memset(&receipt, 0, sizeof(receipt));
                    M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(
                        &receipt);
                    if (receipt.sourcePlanDispatched &&
                        receipt.preDoorFloorOrnamentCallbackStepCount > 0 &&
                        receipt.doorPass1CallbackStepCount > 0 &&
                        receipt.doorFrameCallbackStepCount > 0 &&
                        receipt.doorButtonCallbackStepCount > 0 &&
                        receipt.doorMaterialCallbackStepCount > 0) {
                        printf("real_f0128_door: map=%d door=%d,%d party=%d,%d dir=%d f0108_steps=%d pass1_steps=%d frame_steps=%d f0110_steps=%d f0111_steps=%d\n",
                               mapIndex, x, y, partyX, partyY, direction,
                               receipt.preDoorFloorOrnamentCallbackStepCount,
                               receipt.doorPass1CallbackStepCount,
                               receipt.doorFrameCallbackStepCount,
                               receipt.doorButtonCallbackStepCount,
                               receipt.doorMaterialCallbackStepCount);
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* Verify the actual F0110 button pixels in the final PC34 page. Callback
 * receipts alone do not detect a bad scale, transparency key, palette pass,
 * or a later F0111 overwrite. Compare a button-enabled render with the same
 * original door temporarily lacking its button, then validate every visible
 * source pixel against authentic GRAPHICS.DAT graphic 315. */
static int exercise_real_f0110_button_pixels(
    M11_GameViewState *state, unsigned char *withButton,
    unsigned char *withoutButton)
{
    int mapIndex;
    const M11_AssetSlot *button;
    const DM1_WallFrame *frame = dm1_v1_viewport_get_door_button_frame_pc34(
        1, DM1_VIEW_DOOR_BUTTON_D1C);

    if (!state || !state->world.dungeon || !state->world.things || !frame) {
        return 0;
    }
    button = M11_AssetLoader_Load(&state->assetLoader, 315u);
    if (!button || !button->loaded || !button->pixels ||
        button->width != 16u || button->height != 19u) {
        fprintf(stderr,
                "real PC34 F0110 button graphic 315 unavailable: slot=%p loaded=%d pixels=%p size=%ux%u\n",
                (const void *)button, button ? button->loaded : 0,
                button ? (const void *)button->pixels : NULL,
                button ? button->width : 0u, button ? button->height : 0u);
        return 0;
    }
    for (mapIndex = 0;
         mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map =
            &state->world.dungeon->maps[mapIndex];
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    state->world.dungeon, state->world.things,
                    mapIndex, x, y);
                int safety = 0;
                while (thing != THING_NONE && thing != THING_ENDOFLIST &&
                       safety++ < 64) {
                    if (THING_GET_TYPE(thing) == THING_TYPE_DOOR) {
                        const int doorIndex = (int)THING_GET_INDEX(thing);
                        if (doorIndex >= 0 &&
                            doorIndex < state->world.things->doorCount &&
                            state->world.things->doors[doorIndex].button) {
                            int direction;
                            for (direction = 0; direction < 4; ++direction) {
                                const int partyX = x - step_x(direction);
                                const int partyY = y - step_y(direction);
                                M11_Dm1F0128PerSquareSchedulerReceipt receipt;
                                int transparentPixels = 0;
                                int visiblePixels = 0;
                                int changedPixels = 0;
                                int dy;
                                if (partyX < 0 || partyY < 0 ||
                                    partyX >= (int)map->width ||
                                    partyY >= (int)map->height) {
                                    continue;
                                }
                                state->world.party.mapIndex = mapIndex;
                                state->world.party.mapX = partyX;
                                state->world.party.mapY = partyY;
                                state->world.party.direction = direction;
                                memset(withButton, 0,
                                       FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                                M11_GameView_Draw(state, withButton,
                                                  FRAMEBUFFER_WIDTH,
                                                  FRAMEBUFFER_HEIGHT);
                                memset(&receipt, 0, sizeof(receipt));
                                M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(
                                    &receipt);
                                if (!receipt.sourcePlanDispatched ||
                                    receipt.doorButtonCallbackStepCount <= 0) {
                                    continue;
                                }
                                state->world.things->doors[doorIndex].button = 0;
                                memset(withoutButton, 0,
                                       FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                                M11_GameView_Draw(state, withoutButton,
                                                  FRAMEBUFFER_WIDTH,
                                                  FRAMEBUFFER_HEIGHT);
                                state->world.things->doors[doorIndex].button = 1;
                                for (dy = 0; dy < 9; ++dy) {
                                    int dx;
                                    for (dx = 0; dx < 16; ++dx) {
                                        /* F0110 D1C consumes the native
                                         * 16x19 graphic through a 16x9
                                         * F0791 zone: it clips rows 9..18,
                                         * it does not shrink the bitmap. */
                                        const unsigned char source =
                                            button->pixels[dy * (int)button->width + dx];
                                        const int px = frame->left_x + dx;
                                        const int py = VIEWPORT_Y + frame->top_y + dy;
                                        const int offset = py * FRAMEBUFFER_WIDTH + px;
                                        if (source == 10u) {
                                            ++transparentPixels;
                                            if (withButton[offset] != withoutButton[offset]) {
                                                fprintf(stderr,
                                                        "F0110 C10 transparency mismatch map=%d door=%d,%d pixel=%d,%d\n",
                                                        mapIndex, x, y, dx, dy);
                                                return 0;
                                            }
                                        } else {
                                            const unsigned char expected =
                                                (unsigned char)(0x10u | (source & 0x0fu));
                                            ++visiblePixels;
                                            if (withButton[offset] != withoutButton[offset]) {
                                                ++changedPixels;
                                                if (withButton[offset] != expected) {
                                                    fprintf(stderr,
                                                            "F0110 source pixel mismatch map=%d door=%d,%d button=%d,%d expected=%u actual=%u\n",
                                                            mapIndex, x, y, dx, dy,
                                                            expected, withButton[offset]);
                                                    return 0;
                                                }
                                            }
                                        }
                                    }
                                }
                                if (changedPixels > 0 && transparentPixels > 0) {
                                    printf("real PC34 F0110 button pixels: map=%d door=(%d,%d) party=(%d,%d,d%d) visible=%d changed=%d transparent=%d\n",
                                           mapIndex, x, y, partyX, partyY,
                                           direction, visiblePixels, changedPixels,
                                           transparentPixels);
                                    return 1;
                                }
                            }
                        }
                    }
                    thing = F0512_DUNGEON_GetThingNext_Compat(
                        state->world.things, thing);
                }
            }
        }
    }
    fputs("no real PC34 door-button frame exposed visible F0110 pixels\n", stderr);
    return 0;
}

/* Compare the distant D2C/D3C button in the final framebuffer with the
 * F0129 Q16 sampling sequence in ReDMCSB BLTSHRNK.C:202-390. */
static int exercise_real_f0129_button_pixels(
    M11_GameViewState *state,
    unsigned char *withButton,
    unsigned char *withoutButton,
    DM1_ViewDoorButtonIndex viewIndex,
    int forwardDistance)
{
    const struct DungeonMapDesc_Compat *map;
    const M11_AssetSlot *button;
    const DM1_WallFrame *frame;
    const unsigned char *paletteMap;
    int doorX;

    if (!state || !state->world.dungeon || !state->world.things ||
        state->world.dungeon->header.mapCount <= 0) return 0;
    map = &state->world.dungeon->maps[0];
    button = M11_AssetLoader_Load(&state->assetLoader, 315u);
    frame = dm1_v1_viewport_get_door_button_frame_pc34(1, viewIndex);
    paletteMap = dm1_v1_viewport_get_door_button_palette_remap_pc34(viewIndex);
    if (!button || !button->loaded || !button->pixels ||
        button->width != 16u || button->height != 19u || !frame ||
        !paletteMap || forwardDistance < 1 || forwardDistance > 2) return 0;

    for (doorX = 0; doorX < (int)map->width; ++doorX) {
        int doorY;
        for (doorY = 0; doorY < (int)map->height; ++doorY) {
            unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, doorX, doorY);
            int safety = 0;
            while (thing != THING_NONE && thing != THING_ENDOFLIST &&
                   safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_DOOR) {
                    const int doorIndex = (int)THING_GET_INDEX(thing);
                    if (doorIndex >= 0 &&
                        doorIndex < state->world.things->doorCount &&
                        state->world.things->doors[doorIndex].button) {
                        int direction;
                        for (direction = 0; direction < 4; ++direction) {
                            const int partyX = doorX -
                                (forwardDistance + 1) * step_x(direction);
                            const int partyY = doorY -
                                (forwardDistance + 1) * step_y(direction);
                            M11_Dm1F0128PerSquareSchedulerReceipt receipt;
                            const int dstW = (int)frame->right_x -
                                (int)frame->left_x + 1;
                            const int dstH = (int)frame->bottom_y -
                                (int)frame->top_y + 1;
                            const uint32_t xStep = (uint32_t)(
                                ((uint64_t)button->width << 16) / (uint32_t)dstW);
                            const uint32_t yStep = (uint32_t)(
                                ((uint64_t)button->height << 16) / (uint32_t)dstH);
                            int changedPixels = 0;
                            int transparentPixels = 0;
                            uint32_t yPosition = (yStep >> 1) + UINT32_C(0x7fff);
                            int dy;
                            if (partyX < 0 || partyY < 0 ||
                                partyX >= (int)map->width ||
                                partyY >= (int)map->height) continue;
                            state->world.party.mapIndex = 0;
                            state->world.party.mapX = partyX;
                            state->world.party.mapY = partyY;
                            state->world.party.direction = direction;
                            memset(withButton, 0,
                                   FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                            M11_GameView_Draw(state, withButton,
                                              FRAMEBUFFER_WIDTH,
                                              FRAMEBUFFER_HEIGHT);
                            memset(&receipt, 0, sizeof(receipt));
                            M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(
                                &receipt);
                            if (!receipt.sourcePlanDispatched ||
                                receipt.doorButtonCallbackStepCount <= 0) continue;

                            state->world.things->doors[doorIndex].button = 0;
                            memset(withoutButton, 0,
                                   FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                            M11_GameView_Draw(state, withoutButton,
                                              FRAMEBUFFER_WIDTH,
                                              FRAMEBUFFER_HEIGHT);
                            state->world.things->doors[doorIndex].button = 1;

                            for (dy = 0; dy < dstH; ++dy,
                                 yPosition += yStep) {
                                const uint32_t sy = yPosition >> 16;
                                uint32_t xPosition = (xStep >> 1) +
                                    UINT32_C(0x7fff);
                                int dx;
                                for (dx = 0; dx < dstW;
                                     ++dx, xPosition += xStep) {
                                    const uint32_t sx = xPosition >> 16;
                                    const unsigned char source =
                                        button->pixels[(size_t)sy * button->width + sx];
                                    const int px = VIEWPORT_X +
                                        frame->left_x + dx;
                                    const int py = VIEWPORT_Y +
                                        frame->top_y + dy;
                                    const int offset = py * FRAMEBUFFER_WIDTH + px;
                                    if (source == 10u) {
                                        ++transparentPixels;
                                        if (withButton[offset] !=
                                            withoutButton[offset]) return 0;
                                    } else if (withButton[offset] !=
                                               withoutButton[offset]) {
                                        const unsigned char expected =
                                            (unsigned char)(0x10u |
                                                (paletteMap[source & 0x0fu] & 0x0fu));
                                        if (withButton[offset] != expected) {
                                            fprintf(stderr,
                                                "F0129 button mismatch view=%d map=0 door=%d,%d pixel=%d,%d expected=%u actual=%u\n",
                                                (int)viewIndex, doorX, doorY,
                                                dx, dy, expected,
                                                withButton[offset]);
                                            return 0;
                                        }
                                        ++changedPixels;
                                    }
                                }
                            }
                            if (changedPixels > 0 && transparentPixels > 0) {
                                printf("real PC34 F0129 button pixels: view=%d door=(%d,%d) party=(%d,%d,d%d) changed=%d transparent=%d\n",
                                       (int)viewIndex, doorX, doorY,
                                       partyX, partyY, direction,
                                       changedPixels, transparentPixels);
                                return 1;
                            }
                        }
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(
                    state->world.things, thing);
            }
        }
    }
    fprintf(stderr, "no authentic HoC F0129 button pixels for view=%d\n",
            (int)viewIndex);
    return 0;
}

/* Locate a real PC 3.4 square whose F0127 span admits D0C F0115_MAIN.
 * Only the transient party presentation tuple changes; dungeon bytes remain
 * untouched and the callback receipt proves consumption of the mounted map. */
static int exercise_real_f0128_d0c_f0115(
    M11_GameViewState *state, unsigned char *framebuffer)
{
    int mapIndex;
    if (!state || !state->world.dungeon) return 0;
    for (mapIndex = 0;
         mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map =
            &state->world.dungeon->maps[mapIndex];
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                M11_Dm1F0128PerSquareSchedulerReceipt receipt;
                state->world.party.mapIndex = mapIndex;
                state->world.party.mapX = x;
                state->world.party.mapY = y;
                state->world.party.direction = DIR_NORTH;
                memset(framebuffer, 0,
                       FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                M11_GameView_Draw(state, framebuffer,
                                  FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
                memset(&receipt, 0, sizeof(receipt));
                M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
                if (receipt.sourcePlanDispatched &&
                    receipt.d0ThingCallbackStepCount > 0) {
                    printf("real_f0127_d0c_f0115: map=%d x=%d y=%d steps=%d\n",
                           mapIndex, x, y,
                           receipt.d0ThingCallbackStepCount);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int exercise_real_f0128_d0_side_transaction(
    M11_GameViewState *state, unsigned char *framebuffer)
{
    int mapIndex;
    if (!state || !state->world.dungeon) return 0;
    for (mapIndex = 0;
         mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map =
            &state->world.dungeon->maps[mapIndex];
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                M11_Dm1F0128PerSquareSchedulerReceipt receipt;
                state->world.party.mapIndex = mapIndex;
                state->world.party.mapX = x;
                state->world.party.mapY = y;
                state->world.party.direction = DIR_NORTH;
                memset(framebuffer, 0,
                       FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                M11_GameView_Draw(state, framebuffer,
                                  FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
                memset(&receipt, 0, sizeof(receipt));
                M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
                if (receipt.sourcePlanDispatched &&
                    receipt.d0SideThingFieldCallbackStepCount > 0) {
                    printf("real_f0125_f0126_side_transaction: map=%d x=%d y=%d steps=%d\n",
                           mapIndex, x, y,
                           receipt.d0SideThingFieldCallbackStepCount);
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int collect_real_hoc_f0115_graphics(const M11_GameViewState *state,
                                           unsigned char seen[512])
{
    const struct DungeonMapDesc_Compat *map;
    int count = 0;
    int y;

    if (!state || !state->world.dungeon || !seen) {
        return 0;
    }
    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            DM1_F0115WorldCandidatesPc34 candidates;
            int item;
            if (!dm1_v1_f0115_world_candidates_pc34(
                    &state->world, 0, x, y, NULL, NULL, &candidates) ||
                !candidates.valid) {
                continue;
            }
            for (item = 0; item < candidates.itemCount; ++item) {
                unsigned int graphic = dm1_item_sprite_index(
                    candidates.items[item].thingType,
                    candidates.items[item].subtype);
                if (graphic < 512u && !seen[graphic]) {
                    seen[graphic] = 1;
                    ++count;
                }
            }
        }
    }
    return count;
}

static int draw_real_hoc_f0115_items(M11_GameViewState *state,
                                     unsigned char *framebuffer,
                                     const unsigned char expected[512])
{
    const struct DungeonMapDesc_Compat *map;
    unsigned char seen[512];
    int expectedCount = 0;
    int seenCount = 0;
    int y;

    if (!state || !state->world.dungeon || !expected) {
        return 0;
    }
    memset(seen, 0, sizeof(seen));
    for (y = 0; y < 512; ++y) {
        expectedCount += expected[y] != 0;
    }
    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int direction;
            for (direction = 0; direction < 4; ++direction) {
                M11_Dm1FloorItemHostPresentationReceipt floor;
                M11_Dm1FloorItemHostPresentationReceipt alcove;
                state->world.party.mapX = x;
                state->world.party.mapY = y;
                state->world.party.direction = direction;
                memset(framebuffer, 0,
                       FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT);
                M11_GameView_Draw(state, framebuffer,
                                  FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
                memset(&floor, 0, sizeof(floor));
                memset(&alcove, 0, sizeof(alcove));
                M11_GameView_GetDm1FloorItemHostPresentationReceipt(&floor);
                M11_GameView_GetDm1AlcoveItemHostPresentationReceipt(&alcove);
                if (floor.valid && floor.usesF0791Blit &&
                    floor.destinationPixelsChanged &&
                    floor.graphicsId >= 0 && floor.graphicsId < 512 &&
                    expected[floor.graphicsId]) {
                    seen[floor.graphicsId] = 1;
                }
                if (alcove.valid && alcove.usesF0791Blit &&
                    alcove.destinationPixelsChanged &&
                    alcove.graphicsId >= 0 && alcove.graphicsId < 512 &&
                    expected[alcove.graphicsId]) {
                    seen[alcove.graphicsId] = 1;
                }
            }
        }
    }
    for (y = 0; y < 512; ++y) {
        seenCount += seen[y] != 0;
    }
    return seenCount == expectedCount;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    unsigned char framebuffer[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
    unsigned char withoutButton[FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT];
    unsigned char expected_hoc_graphics[512];
    static const M12_PresentationMode kV2Modes[] = {
        M12_PRESENTATION_V20_FILTERED,
        M12_PRESENTATION_V21_UPSCALED,
        M12_PRESENTATION_V22_MODERN
    };
    int direction;
    int hoc_item_count;
    size_t modeIndex;

    if (!data_dir || !data_dir[0]) {
        puts("skip: FIRESTAFF_DM1_DATA_DIR is not set");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, data_dir)) {
        fprintf(stderr, "failed to start DM1 from real PC34 data\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.mapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 3;
    state.world.party.championCount = 0;

    hoc_item_count = count_real_hoc_f0115_items(&state);
    if (hoc_item_count <= 0) {
        fprintf(stderr,
                "real PC34 HoC has no F0115 floor/alcove item candidate\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    memset(expected_hoc_graphics, 0, sizeof(expected_hoc_graphics));
    if (collect_real_hoc_f0115_graphics(&state, expected_hoc_graphics) <= 0 ||
        !draw_real_hoc_f0115_items(&state, framebuffer,
                                    expected_hoc_graphics)) {
        fprintf(stderr,
                "one or more real PC34 HoC F0115 item graphics never reached an F0791 draw\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        M11_Dm1F0128PerSquareSchedulerReceipt receipt;
        memset(&receipt, 0, sizeof(receipt));
        M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
        if (!receipt.sourcePlanDispatched ||
            receipt.d0PrimitiveCallbackStepCount <= 0 ||
            receipt.wallMaterialCallbackStepCount <= 0 ||
            receipt.wallOrnamentCallbackStepCount <= 0 ||
            receipt.frontMirrorCallbackStepCount <= 0 ||
            receipt.thievesEyeRestoreCallbackStepCount <= 0 ||
            receipt.foregroundCallbackStepCount <= 0) {
            fprintf(stderr,
                    "real PC34 F0104/F0107/F0125-F0127/foreground material did not execute through the F0128 callback: dispatched=%d wall_steps=%d ornament_steps=%d mirror_steps=%d eye_restore_steps=%d d0_steps=%d d0_thing_steps=%d foreground_steps=%d\n",
                    receipt.sourcePlanDispatched,
                    receipt.wallMaterialCallbackStepCount,
                    receipt.wallOrnamentCallbackStepCount,
                    receipt.frontMirrorCallbackStepCount,
                    receipt.thievesEyeRestoreCallbackStepCount,
                    receipt.d0PrimitiveCallbackStepCount,
                    receipt.d0ThingCallbackStepCount,
                    receipt.foregroundCallbackStepCount);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        printf("real_f0128_wall_material: wall_steps=%d f0107_steps=%d mirror_steps=%d eye_restore_steps=%d d0_steps=%d d0_thing_steps=%d foreground_steps=%d\n",
               receipt.wallMaterialCallbackStepCount,
               receipt.wallOrnamentCallbackStepCount,
               receipt.frontMirrorCallbackStepCount,
               receipt.thievesEyeRestoreCallbackStepCount,
               receipt.d0PrimitiveCallbackStepCount,
               receipt.d0ThingCallbackStepCount,
               receipt.foregroundCallbackStepCount);
    }
    if (!exercise_real_f0128_d0c_f0115(&state, framebuffer)) {
        fprintf(stderr,
                "real PC34 F0127 D0C F0115 never executed through the F0128 callback\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!exercise_real_f0128_d0_side_transaction(&state, framebuffer)) {
        fprintf(stderr,
                "real PC34 F0125/F0126 F0115/F0113 never executed through the F0128 callback\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!exercise_real_f0128_door_pass1(&state, framebuffer)) {
        fprintf(stderr,
                "real PC34 door pass 1/F0111 material never executed through the F0128 callback\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!exercise_real_f0110_button_pixels(&state, framebuffer,
                                            withoutButton)) {
        M11_GameView_Shutdown(&state);
        return 1;
    }
    if (!exercise_real_f0129_button_pixels(
            &state, framebuffer, withoutButton, DM1_VIEW_DOOR_BUTTON_D2C, 1) ||
        !exercise_real_f0129_button_pixels(
            &state, framebuffer, withoutButton, DM1_VIEW_DOOR_BUTTON_D3C, 2)) {
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* V2 modes replace presentation, never the source-owned F0115 and C127
     * runtime lanes. Exercise every supported V2 mode against the same real
     * HoC objects and mirror data before the V1-only source geometry checks
     * below. A missing or incomplete V2.2 pack must not make live original
     * objects disappear. */
    for (modeIndex = 0; modeIndex < sizeof(kV2Modes) / sizeof(kV2Modes[0]);
         ++modeIndex) {
        state.presentationMode = kV2Modes[modeIndex];
        if (!draw_real_hoc_f0115_items(&state, framebuffer,
                                       expected_hoc_graphics) ||
            !select_first_real_hoc_mirror(&state, framebuffer)) {
            fprintf(stderr,
                    "real PC34 HoC material or C127 mirror failed in V2 mode %d\n",
                    (int)kV2Modes[modeIndex]);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.mapX = 1;
    state.world.party.mapY = 3;

    /* Exercise the production turn input path, not only direct direction
     * assignment. This is the route used by Q/E, Home/End and the on-screen
     * turn arrows. */
    state.world.party.direction = DIR_SOUTH;
    for (direction = 0; direction < 2; ++direction) {
        M11_Dm1F0128PerSquareSchedulerReceipt receipt;
        int nonblack;
        if (M11_GameView_HandleInput(&state,
                                     M12_MENU_INPUT_TURN_RIGHT) ==
            M11_GAME_INPUT_IGNORED) {
            fprintf(stderr, "HoC turn %d was ignored\n", direction);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer,
                          FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        memset(&receipt, 0, sizeof(receipt));
        M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
        nonblack = viewport_nonblack_pixel_count(framebuffer);
        if (!receipt.valid || !receipt.planReady ||
            !receipt.planDrivenContentLoop || nonblack < 256) {
            fprintf(stderr,
                    "HoC production turn %d: valid=%d plan=%d driven=%d nonblack=%d\n",
                    direction, receipt.valid, receipt.planReady,
                    receipt.planDrivenContentLoop, nonblack);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }

    for (direction = 0; direction < 4; ++direction) {
        M11_Dm1F0128PerSquareSchedulerReceipt receipt;
        int nonblack;

        state.world.party.direction = direction;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer,
                          FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
        memset(&receipt, 0, sizeof(receipt));
        M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
        nonblack = viewport_nonblack_pixel_count(framebuffer);
        if (!receipt.valid || !receipt.planReady ||
            !receipt.planDrivenContentLoop || nonblack < 256) {
            fprintf(stderr,
                    "HoC direction %d: valid=%d plan=%d driven=%d nonblack=%d\n",
                    direction, receipt.valid, receipt.planReady,
                    receipt.planDrivenContentLoop, nonblack);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }

    /* Click the real C026 portrait through the exact source-screen route,
     * then use C160 and the newly visible HUD status surface. */
    if (!select_first_real_hoc_mirror(&state, framebuffer) ||
        M11_GameView_HandlePointerButton(&state, 112, 82,
                                         DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        !state.candidateMirrorPanelActive ||
        M11_GameView_HandlePointerButton(&state, 130, 114,
                                         DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        state.world.party.championCount != 1 ||
        state.candidateMirrorPanelActive) {
        fprintf(stderr, "real HoC mirror C026/C040/C160 pointer route failed\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        DM1_V1_ChampionStatusRectPc34 bar;
        if (!dm1_v1_champion_status_bar_rect_pc34(0, 0, &bar) ||
            M11_GameView_HandlePointerButton(&state,
                                             bar.x + bar.w / 2,
                                             bar.y + bar.h / 2,
                                             DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                M11_GAME_INPUT_REDRAW || !state.inventoryPanelActive) {
            /* COMMAND.C G0447 keeps C012 (the status box) and C007 (the
             * bar graph) distinct. Use the source C187 bar rectangle rather
             * than inferring a coordinate from the adjacent C151 status box.
             * A broad status-box hit here would conceal the native C007 path
             * and regress the real HoC route. */
            fprintf(stderr,
                    "live champion HUD C007 inventory route failed\n");
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    M11_GameView_Shutdown(&state);
    printf("ok: real PC34 HoC has %d F0115 items; turns, item material, "
           "mirror selection and champion HUD click work\n", hoc_item_count);
    return 0;
}
