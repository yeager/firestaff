#include "m11_game_view.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int source_tile_has_item(
    const DM2_V1_DungeonData *dungeon, int map, int x, int y,
    uint16_t object)
{
    uint16_t cursor = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    for (int i = 0; i < 128 && cursor != 0xfffeu; ++i) {
        if (cursor == object) return 1;
        cursor = dm2_v1_dungeon_get_next_thing(dungeon, cursor);
    }
    return 0;
}

static uint16_t source_tile_find_record(
    const DM2_V1_DungeonData *dungeon, int map, int x, int y,
    uint16_t record_index)
{
    uint16_t cursor = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    for (int i = 0; i < 128 && cursor != 0xfffeu; ++i) {
        if ((cursor & 0x3fffu) == record_index) return cursor;
        cursor = dm2_v1_dungeon_get_next_thing(dungeon, cursor);
    }
    return 0xfffeu;
}

static int exercise_mac_retail_source_item(
    M11_GameViewState *state, DM2_V1_BootProfile *profile,
    DM2_V1_DungeonData *dungeon, unsigned char frame[320u * 200u],
    int db, int map, int item_x, int item_y, int pose_x, int pose_y,
    int dir, uint16_t source_object, uint16_t expected_placed)
{
    DM2_V1_RuntimeViewportClickReceipt hit;
    DM2_V1_BootExpandedRectReceipt rect7, zone[4];
    int click_x = -1, click_y = -1, place_x = -1, place_y = -1;
    uint16_t placed;
    int record_type = -1;
    const uint8_t *record = dm2_v1_dungeon_get_thing_record(
        dungeon, source_object, &record_type, NULL, NULL);
    if (!record || record_type != db ||
        !source_tile_has_item(dungeon, map, item_x, item_y, source_object) ||
        dm2_v1_runtime_get_leader_hand_object() != 0xffffu)
        return 0;
    dm2_v1_runtime_set_position(map, pose_x, pose_y, dir);
    memset(frame, 0, 320u * 200u);
    M11_GameView_Draw(state, frame, 320, 200);
    for (int y = 40; y < 176 && click_x < 0; ++y)
        for (int x = 0; x < 224; ++x) {
            memset(&hit, 0, sizeof(hit));
            if (dm2_v1_runtime_route_viewport_click(x, y, &hit) &&
                hit.accepted && hit.target_kind == 1 &&
                (uint16_t)hit.object_id == source_object) {
                click_x = x; click_y = y; break;
            }
        }
    if (click_x < 0 ||
        M11_GameView_HandlePointerButton(
            state, click_x, click_y, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        dm2_v1_runtime_get_leader_hand_object() != source_object ||
        source_tile_has_item(dungeon, map, item_x, item_y, source_object))
        return 0;
    memset(frame, 0, 320u * 200u);
    M11_GameView_Draw(state, frame, 320, 200);
    if (!dm2_v1_boot_query_expanded_rect_receipt(profile, 7u, &rect7) ||
        !rect7.valid)
        return 0;
    for (int cell = 0; cell < 4; ++cell)
        if (!dm2_v1_boot_query_expanded_rect_receipt(
                profile, (uint16_t)(0x2f8u + cell), &zone[cell]) ||
            !zone[cell].valid)
            return 0;
    for (int cell = 2; cell < 4 && place_x < 0; ++cell)
        for (int y = zone[cell].rect.y;
             y < zone[cell].rect.y + zone[cell].rect.h && place_x < 0; ++y)
            for (int x = zone[cell].rect.x;
                 x < zone[cell].rect.x + zone[cell].rect.w; ++x) {
                int sx = x + rect7.rect.x, sy = y + rect7.rect.y;
                int covered = 0;
                if (sx < 0 || sx >= 224 || sy < 40 || sy >= 176) continue;
                for (int prior = 0; prior < cell; ++prior)
                    if (x >= zone[prior].rect.x && y >= zone[prior].rect.y &&
                        x < zone[prior].rect.x + zone[prior].rect.w &&
                        y < zone[prior].rect.y + zone[prior].rect.h)
                        covered = 1;
                memset(&hit, 0, sizeof(hit));
                if (covered || dm2_v1_runtime_route_viewport_click(sx, sy, &hit))
                    continue;
                place_x = sx; place_y = sy; break;
            }
    if (place_x < 0 ||
        M11_GameView_HandlePointerButton(
            state, place_x, place_y, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        dm2_v1_runtime_get_leader_hand_object() != 0xffffu)
        return 0;
    placed = source_tile_find_record(
        dungeon, map, item_x, item_y, (uint16_t)(source_object & 0x3fffu));
    if (placed != expected_placed ||
        !source_tile_has_item(dungeon, map, item_x, item_y, placed) ||
        dm2_v1_dungeon_get_next_thing(dungeon, placed) != 0xfffeu)
        return 0;
    memset(frame, 0, 320u * 200u);
    M11_GameView_Draw(state, frame, 320, 200);
    click_x = -1;
    for (int y = 40; y < 176 && click_x < 0; ++y)
        for (int x = 0; x < 224; ++x) {
            memset(&hit, 0, sizeof(hit));
            if (dm2_v1_runtime_route_viewport_click(x, y, &hit) &&
                hit.accepted && hit.target_kind == 1 &&
                (uint16_t)hit.object_id == placed) {
                click_x = x; click_y = y; break;
            }
        }
    if (click_x < 0 ||
        M11_GameView_HandlePointerButton(
            state, click_x, click_y, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        dm2_v1_runtime_get_leader_hand_object() != placed ||
        source_tile_has_item(dungeon, map, item_x, item_y, placed))
        return 0;
    memset(frame, 0, 320u * 200u);
    M11_GameView_Draw(state, frame, 320, 200);
    if (M11_GameView_HandlePointerButton(
            state, place_x, place_y, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        dm2_v1_runtime_get_leader_hand_object() != 0xffffu ||
        !source_tile_has_item(dungeon, map, item_x, item_y, placed))
        return 0;
    printf("Mac retail DB%d pickup/place/repick: %04x -> %04x\n",
           db, source_object, placed);
    return 1;
}

int main(void)
{
    const char *retail = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    M11_GameViewState state;
    M11_GameLaunchSpec spec;
    DM2_V1_BootProfile *profile;
    DM2_V1_DungeonData *dungeon = NULL;
    unsigned char frame[320u * 200u];
    DM2_V1_RuntimeViewportClickReceipt hit;
    int click_x = -1, click_y = -1, picked = -1, previous = -1, successor = -1;
    const int chain[] = {0xe80f, 0x2810, 0x6811, 0xa812, 0xe813, 0xfffe};

    if (!retail || !*retail) return 77;
    memset(&state, 0, sizeof(state));
    memset(&spec, 0, sizeof(spec));
    spec.title = "Dungeon Master II Macintosh";
    spec.gameId = "dm2";
    spec.dataDir = retail;
    spec.sourceId = "mac-en-retail";
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    spec.launcherOptionsBound = 1;
    M11_GameView_Init(&state);
    if (!M11_GameView_Start(&state, &spec)) goto fail;
    M11_GameView_SetBootProbeMode(&state, 1);
    memset(frame, 0, sizeof(frame));
    while (state.dm2MacMovieActive)
        M11_GameView_Draw(&state, frame, 320, 200);
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        M11_GameView_HandlePointerButton(
            &state, 100, 60, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW || !state.dm2State.level_loaded) goto fail;

    profile = (DM2_V1_BootProfile *)state.dm2BootProfile;
    dungeon = profile ? (DM2_V1_DungeonData *)profile->dungeon_data : NULL;
    {
        DM2_V1_BootRuntimeReceipt pose;
        if (!dungeon || !dungeon->record_graph_complete ||
            !dm2_v1_boot_runtime_capture(profile, &pose) ||
            pose.current_level != 0 || pose.party_x != 1 ||
            pose.party_y != 8 || pose.party_dir != 0)
            goto fail;
        /* The first north move is a real Mac input transaction.  The
         * corrected source map has a wall at (2,7), so the old east route
         * to the former DB6 floor fixture is invalid. */
        if (M11_GameView_HandlePointerButton(
                &state, 274, 140, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                M11_GAME_INPUT_REDRAW)
            goto fail;
        (void)M11_GameView_AdvanceIdleTick(&state);
        if (dm2_v1_runtime_get_party_x() != 1 ||
            dm2_v1_runtime_get_party_y() != 7 ||
            dm2_v1_runtime_get_party_dir() != 0)
            goto fail;
    }
    if (!exercise_mac_retail_source_item(
            &state, profile, dungeon, frame, 5, 11, 10, 3, 11, 3, 3,
            0xd407u, 0x5407u)) goto fail;
    if (!exercise_mac_retail_source_item(
            &state, profile, dungeon, frame, 8, 17, 3, 8, 3, 7, 2,
            0xa037u, 0x2037u)) goto fail;
    if (!exercise_mac_retail_source_item(
            &state, profile, dungeon, frame, 9, 14, 6, 13, 6, 12, 2,
            0x240eu, 0x240eu)) goto fail;
    if (!dungeon || !dungeon->record_graph_complete ||
        dm2_v1_dungeon_get_square_type(dungeon, 10, 4, 8) != 1 ||
        dm2_v1_runtime_get_leader_hand_object() != 0xffffu ||
        dm2_v1_dungeon_get_first_thing(dungeon, 10, 4, 9) != chain[0])
        goto fail;
    for (int i = 0; i < 5; ++i)
        if (dm2_v1_dungeon_get_next_thing(dungeon, (uint16_t)chain[i]) !=
            chain[i + 1]) goto fail;

    dm2_v1_runtime_set_position(10, 4, 8, 2);
    memset(frame, 0, sizeof(frame));
    M11_GameView_Draw(&state, frame, 320, 200);
    /* The linked tail has an exposed opaque pixel after all five draws. */
    for (int y = 40; y < 176 && click_x < 0; ++y) {
        for (int x = 0; x < 224; ++x) {
            memset(&hit, 0, sizeof(hit));
            if (dm2_v1_runtime_route_viewport_click(x, y, &hit) &&
                hit.accepted && hit.target_kind == 1 &&
                (uint16_t)hit.object_id == (uint16_t)chain[4]) {
                click_x = x; click_y = y; picked = chain[4];
                previous = chain[3]; successor = chain[5];
                break;
            }
        }
    }
    if (click_x < 0)
        fprintf(stderr, "linked frame has %d rendered items\n",
                dm2_v1_runtime_last_asset_item_count());
    if (click_x < 0 ||
        M11_GameView_HandlePointerButton(
            &state, click_x, click_y, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        dm2_v1_runtime_get_leader_hand_object() != (uint32_t)picked ||
        dm2_v1_dungeon_get_first_thing(dungeon, 10, 4, 9) != chain[0] ||
        dm2_v1_dungeon_get_next_thing(dungeon, (uint16_t)previous) !=
            successor ||
        dm2_v1_dungeon_get_next_thing(dungeon, (uint16_t)picked) != 0xfffe)
        goto fail;
    memset(frame, 0, sizeof(frame));
    M11_GameView_Draw(&state, frame, 320, 200);
    memset(&hit, 0, sizeof(hit));
    if (dm2_v1_runtime_route_viewport_click(click_x, click_y, &hit) &&
        hit.accepted && (uint16_t)hit.object_id == (uint16_t)picked)
        goto fail;
    printf("Mac retail linked pickup: %04x from map 10 (4,9) at %d,%d\n",
           picked, click_x, click_y);
    M11_GameView_Shutdown(&state);
    return 0;

fail:
    fprintf(stderr, "FAIL: Mac retail linked pickup (point=%d,%d object=%04x hand=%04x)\n",
            click_x, click_y, picked,
            (unsigned)dm2_v1_runtime_get_leader_hand_object());
    M11_GameView_Shutdown(&state);
    return 1;
}
