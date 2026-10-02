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
    DM2_V1_BootExpandedRectReceipt rect7, zone[4];
    int click_x = -1, click_y = -1;
    int place_x = -1, place_y = -1, place_cell = -1;
    int transparent_x = -1, transparent_y = -1;

    if (!retail || !*retail) {
        puts("SKIP: authentic DM2 Mac ZIP environment is not set");
        return 77;
    }
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
            M11_GAME_INPUT_REDRAW ||
        !state.dm2State.level_loaded) goto fail;

    profile = (DM2_V1_BootProfile *)state.dm2BootProfile;
    dungeon = profile ? (DM2_V1_DungeonData *)profile->dungeon_data : NULL;
    /* Retail Mac DB10 0x2831 is the first record on map 9 (1,6).  The
     * authenticated floor square (1,7) faces it north; no fixture item is
     * added to either record graph. */
    if (!dungeon || !dungeon->record_graph_complete ||
        dm2_v1_dungeon_get_square_type(dungeon, 9, 1, 7) != 1 ||
        dm2_v1_dungeon_get_first_thing(dungeon, 9, 1, 6) != 0x2831 ||
        dm2_v1_runtime_get_leader_hand_object() != 0xffffu) goto fail;
    dm2_v1_runtime_set_position(9, 1, 7, 0);
    memset(frame, 0, sizeof(frame));
    M11_GameView_Draw(&state, frame, 320, 200);
    memset(&hit, 0, sizeof(hit));
    for (int y = 40; y < 176 && click_x < 0; ++y) {
        for (int x = 0; x < 224; ++x) {
            if (dm2_v1_runtime_route_viewport_click(x, y, &hit) &&
                hit.accepted && hit.target_kind == 1 &&
                hit.object_id == 0x2831) {
                click_x = x;
                click_y = y;
                break;
            }
        }
    }
    if (click_x < 0) goto fail;
    for (int y = hit.rect.y; y < hit.rect.y + hit.rect.h &&
                            transparent_x < 0; ++y) {
        for (int x = hit.rect.x; x < hit.rect.x + hit.rect.w; ++x) {
            DM2_V1_RuntimeViewportClickReceipt miss;
            if (x < 0 || x >= 224 || y < 40 || y >= 176) continue;
            memset(&miss, 0, sizeof(miss));
            if (!dm2_v1_runtime_route_viewport_click(x, y, &miss)) {
                transparent_x = x;
                transparent_y = y;
                break;
            }
        }
    }
    if (transparent_x < 0 ||
        M11_GameView_HandlePointerButton(
            &state, transparent_x, transparent_y,
            DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_IGNORED ||
        dm2_v1_runtime_get_leader_hand_object() != 0xffffu ||
        dm2_v1_dungeon_get_first_thing(dungeon, 9, 1, 6) != 0x2831)
        goto fail;
    {
        M11_GameInputResult click_result = M11_GameView_HandlePointerButton(
            &state, click_x, click_y, DM1_V1_MOUSE_MASK_LEFT_PC34);
        if (click_result != M11_GAME_INPUT_REDRAW ||
        dm2_v1_runtime_get_leader_hand_object() != 0x2831u ||
        dm2_v1_dungeon_get_first_thing(dungeon, 9, 1, 6) == 0x2831) {
            goto fail;
        }
    }
    memset(frame, 0, sizeof(frame));
    M11_GameView_Draw(&state, frame, 320, 200);
    if (dm2_v1_runtime_route_viewport_click(click_x, click_y, &hit) &&
        hit.accepted && hit.object_id == 0x2831) goto fail;
    if (!dm2_v1_boot_query_expanded_rect_receipt(profile, 7u, &rect7) ||
        !rect7.valid || rect7.rect.x != 0 || rect7.rect.y != 40)
        goto fail;
    for (int i = 0; i < 4; ++i)
        if (!dm2_v1_boot_query_expanded_rect_receipt(
                profile, (uint16_t)(0x2f8u + i), &zone[i]) || !zone[i].valid)
            goto fail;
    for (int cell = 2; cell < 4 && place_x < 0; ++cell) {
        for (int y = zone[cell].rect.y; y < zone[cell].rect.y +
                 zone[cell].rect.h && place_x < 0; ++y) {
            for (int x = zone[cell].rect.x; x < zone[cell].rect.x +
                     zone[cell].rect.w; ++x) {
                int covered_earlier = 0;
                DM2_V1_RuntimeViewportClickReceipt other;
                int sx = x + rect7.rect.x, sy = y + rect7.rect.y;
                if (sx < rect7.rect.x || sy < rect7.rect.y ||
                    sx >= rect7.rect.x + rect7.rect.w ||
                    sy >= rect7.rect.y + rect7.rect.h) continue;
                for (int prior = 0; prior < cell; ++prior)
                    if (x >= zone[prior].rect.x && y >= zone[prior].rect.y &&
                        x < zone[prior].rect.x + zone[prior].rect.w &&
                        y < zone[prior].rect.y + zone[prior].rect.h)
                        covered_earlier = 1;
                memset(&other, 0, sizeof(other));
                if (covered_earlier ||
                    dm2_v1_runtime_route_viewport_click(sx, sy, &other))
                    continue;
                place_x = sx; place_y = sy; place_cell = cell;
                break;
            }
        }
    }
    if (place_x < 0 ||
        M11_GameView_HandlePointerButton(
            &state, place_x, place_y, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        dm2_v1_runtime_get_leader_hand_object() != 0xffffu ||
        dm2_v1_dungeon_get_first_thing(dungeon, 9, 1, 6) !=
            (0x2831 | (place_cell << 14)) ||
        dm2_v1_dungeon_get_next_thing(
            dungeon, (uint16_t)(0x2831 | (place_cell << 14))) != 0xfffe)
        goto fail;
    memset(frame, 0, sizeof(frame));
    M11_GameView_Draw(&state, frame, 320, 200);
    click_x = -1;
    for (int y = 40; y < 176 && click_x < 0; ++y)
        for (int x = 0; x < 224; ++x) {
            memset(&hit, 0, sizeof(hit));
            if (dm2_v1_runtime_route_viewport_click(x, y, &hit) &&
                hit.accepted && hit.target_kind == 1 &&
                (uint16_t)hit.object_id ==
                    (uint16_t)(0x2831 | (place_cell << 14))) {
                click_x = x; click_y = y; break;
            }
        }
    if (click_x < 0 ||
        M11_GameView_HandlePointerButton(
            &state, click_x, click_y, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        dm2_v1_runtime_get_leader_hand_object() !=
            (uint32_t)(0x2831 | (place_cell << 14)) ||
        dm2_v1_dungeon_get_first_thing(dungeon, 9, 1, 6) != 0xfffe)
        goto fail;
    printf("Mac retail DB10 pickup/place/pickup at %d,%d, cell %d\n",
           place_x, place_y, place_cell);
    M11_GameView_Shutdown(&state);
    return 0;

fail:
    fprintf(stderr, "FAIL: Mac retail DB10 floor round trip (point=%d,%d transparent=%d,%d placement=%d,%d cell=%d hand=%04x tile=%04x type=%d zones=%d,%d,%d,%d)\n",
            click_x, click_y, transparent_x, transparent_y,
            place_x, place_y, place_cell,
            (unsigned)dm2_v1_runtime_get_leader_hand_object(),
            dungeon ? (unsigned)dm2_v1_dungeon_get_first_thing(dungeon, 9, 1, 6) : 0u,
            dungeon ? dm2_v1_dungeon_get_square_type(dungeon, 9, 1, 6) : -1,
            zone[2].rect.x, zone[2].rect.y, zone[2].rect.w, zone[2].rect.h);
    M11_GameView_Shutdown(&state);
    return 1;
}
