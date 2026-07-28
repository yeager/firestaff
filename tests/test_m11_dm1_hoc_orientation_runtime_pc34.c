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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    FRAMEBUFFER_WIDTH = 320,
    FRAMEBUFFER_HEIGHT = 200,
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
                    floor.graphicsId >= 0 && floor.graphicsId < 512 &&
                    expected[floor.graphicsId]) {
                    seen[floor.graphicsId] = 1;
                }
                if (alcove.valid && alcove.usesF0791Blit &&
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
    unsigned char expected_hoc_graphics[512];
    int direction;
    int hoc_item_count;

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
        DM1_V1_ChampionStatusRectPc34 rect;
        if (!dm1_v1_champion_status_box_rect_pc34(0, &rect) ||
            M11_GameView_HandlePointerButton(&state,
                                             rect.x + rect.w / 2,
                                             rect.y + rect.h / 2,
                                             DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                M11_GAME_INPUT_REDRAW || !state.inventoryPanelActive) {
            fprintf(stderr, "live champion HUD click did not open inventory\n");
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    M11_GameView_Shutdown(&state);
    printf("ok: real PC34 HoC has %d F0115 items; turns, item material, "
           "mirror selection and champion HUD click work\n", hoc_item_count);
    return 0;
}
