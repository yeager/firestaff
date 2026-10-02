#include "m11_game_view.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    if (!dungeon || !dungeon->record_graph_complete ||
        dm2_v1_dungeon_get_square_type(dungeon, 10, 3, 0) != 1 ||
        dm2_v1_runtime_get_leader_hand_object() != 0xffffu ||
        dm2_v1_dungeon_get_first_thing(dungeon, 10, 4, 0) != chain[0])
        goto fail;
    for (int i = 0; i < 5; ++i)
        if (dm2_v1_dungeon_get_next_thing(dungeon, (uint16_t)chain[i]) !=
            chain[i + 1]) goto fail;

    dm2_v1_runtime_set_position(10, 3, 0, 1);
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
        dm2_v1_dungeon_get_first_thing(dungeon, 10, 4, 0) != chain[0] ||
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
    printf("Mac retail linked pickup: %04x from map 10 (4,0) at %d,%d\n",
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
