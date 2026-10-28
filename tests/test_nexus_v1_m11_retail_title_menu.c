#include "m11_game_view.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_title.h"
#include <stdint.h>
#include <stdlib.h>
#include "nexus_v1_retail_title_menu.h"

#include <stdio.h>

int main(void)
{
    M11_GameViewState state;
    int failures = 0;

    M11_GameView_Init(&state);
    state.active = 1;
    state.sourceKind = M11_GAME_SOURCE_NEXUS_DGN;
    state.nexusEngine = (Nexus_V1_Engine *)(uintptr_t)1;
    state.nexusState.title_active = 1;
    state.nexusState.title_frame = 0;
    while (!nexus_title_full_boot_start_ready(state.nexusState.title_frame)) {
        ++state.nexusState.title_frame;
    }
    nexus_v1_retail_title_menu_init(&state.nexusState.retail_title_menu);

    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACTION) !=
            M11_GAME_INPUT_REDRAW ||
        !state.nexusState.retail_title_menu.menu_active ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME) {
        fprintf(stderr, "FAIL: Start opens the retail menu\n");
        ++failures;
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        !state.nexusState.title_active ||
        !state.nexusState.retail_title_menu.menu_active ||
        state.nexusState.champion_select_active) {
        fprintf(stderr,
                "FAIL: retail New Game dispatch stays on the title when no authenticated launcher is initialized\n");
        ++failures;
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE ||
        state.nexusState.startup_save_select_active ||
        state.nexusState.champion_select_active) {
        fprintf(stderr, "FAIL: Down selects retail Continue only\n");
        ++failures;
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME) {
        fprintf(stderr, "FAIL: Up returns Continue to New Game\n");
        ++failures;
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE ||
        M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
        fprintf(stderr, "FAIL: Down advances through Continue to Configuration\n");
        ++failures;
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE) {
        fprintf(stderr, "FAIL: Up returns Configuration to Continue\n");
        ++failures;
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
        fprintf(stderr, "FAIL: Down returns Continue to Configuration\n");
        ++failures;
    }
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
        fprintf(stderr, "FAIL: unsupported lower edge preserves selection\n");
        ++failures;
    }
    state.nexusState.startup_save_slot_mask = 1u;
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.title_active != 1 ||
        state.nexusState.startup_save_select_active ||
        state.nexusState.champion_select_active) {
        fprintf(stderr, "FAIL: Configuration does not enter generic menu routes\n");
        ++failures;
    }

    nexus_v1_retail_title_menu_init(&state.nexusState.retail_title_menu);
    state.nexusState.startup_save_select_active = 0;
    state.nexusState.champion_select_active = 0;
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACTION) !=
            M11_GAME_INPUT_REDRAW ||
        M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN) !=
            M11_GAME_INPUT_REDRAW) {
        fprintf(stderr, "FAIL: retail Continue route can be selected\n");
        ++failures;
    }
    state.nexusState.startup_save_slot_mask = 1u;
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_REDRAW ||
        !state.nexusState.retail_title_menu.menu_active ||
        state.nexusState.startup_save_select_active ||
        state.nexusState.champion_select_active) {
        fprintf(stderr,
                "FAIL: Firestaff save mask does not impersonate Saturn BKR\n");
        ++failures;
    }

    nexus_v1_retail_title_menu_init(&state.nexusState.retail_title_menu);
    state.nexusState.title_active = 1;
    state.nexusState.title_frame = 0;
    while (!nexus_title_full_boot_start_ready(state.nexusState.title_frame)) {
        ++state.nexusState.title_frame;
    }
    if (M11_GameView_HandlePointerButton(
            &state, 10, 10, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        !state.nexusState.retail_title_menu.menu_active) {
        fprintf(stderr, "FAIL: title prompt mouse click opens retail menu\n");
        ++failures;
    }
    if (M11_GameView_HandlePointerButton(
            &state, 150, 156, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        !state.nexusState.title_active ||
        !state.nexusState.retail_title_menu.menu_active ||
        state.nexusState.champion_select_active) {
        fprintf(stderr,
                "FAIL: retail New Game mouse click dispatch stays on title when launcher is unavailable\n");
        ++failures;
    }
    if (M11_GameView_HandlePointerButton(
            &state, 150, 168, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_REDRAW ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE) {
        fprintf(stderr, "FAIL: authentic Continue row responds to mouse\n");
        ++failures;
    }
    {
        Nexus_TitleScreen *title_screen = calloc(1, sizeof(*title_screen));
        if (!title_screen) {
            fprintf(stderr, "FAIL: allocate regional title source receipt\n");
            ++failures;
        } else {
            title_screen->retail_menu_source_region =
                NEXUS_V1_TITLE_MENU_SOURCE_JAPANESE;
            state.nexusTitleScreen = title_screen;
            if (M11_GameView_HandlePointerButton(
                    &state, 150, 181, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.selection !=
                    NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION ||
                !state.nexusState.retail_title_menu.configuration_active) {
                fprintf(stderr,
                        "FAIL: Japanese Configuration row opens from a mouse click\n");
                ++failures;
            }
            state.nexusState.retail_title_menu.music_enabled = 0;
            if (M11_GameView_HandlePointerButton(
                    &state, 185, 83, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.configuration_row !=
                    NEXUS_V1_RETAIL_CONFIGURATION_SOUND ||
                state.nexusState.retail_title_menu.music_enabled != 0 ||
                state.nexusState.retail_title_menu.stereo_enabled != 0) {
                fprintf(stderr,
                        "FAIL: y=83 selects Sound without changing Music\n");
                ++failures;
            }
            if (M11_GameView_HandlePointerButton(
                    &state, 250, 115, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.configuration_row !=
                    NEXUS_V1_RETAIL_CONFIGURATION_LEVEL ||
                !state.nexusState.retail_title_menu.easy_level_enabled ||
                M11_GameView_HandlePointerButton(
                    &state, 40, 185, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.configuration_active) {
                fprintf(stderr,
                        "FAIL: Japanese retail Configuration responds to mouse and Exit\n");
                ++failures;
            }
            state.nexusState.retail_title_menu.music_enabled = 1;
            state.nexusState.retail_title_menu.stereo_enabled = 1;
            state.nexusState.retail_title_menu.selection =
                NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE;
            title_screen->retail_menu_source_region =
                NEXUS_V1_TITLE_MENU_SOURCE_ENGLISH;
            if (M11_GameView_HandlePointerButton(
                    &state, 150, 181, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.selection !=
                    NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION ||
                !state.nexusState.retail_title_menu.configuration_active) {
                fprintf(stderr,
                        "FAIL: English Configuration row opens from a mouse click\n");
                ++failures;
            }
            if (state.nexusState.retail_title_menu.music_enabled != 1) {
                fprintf(stderr, "FAIL: English Configuration opens with Music ON\n");
                ++failures;
            }
            if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_DOWN) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.configuration_row !=
                    NEXUS_V1_RETAIL_CONFIGURATION_SOUND ||
                M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.stereo_enabled) {
                fprintf(stderr,
                        "FAIL: Configuration keyboard navigation changes Stereo to Mono\n");
                ++failures;
            }
            if (M11_GameView_HandlePointerButton(
                    &state, 250, 115, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.configuration_row !=
                    NEXUS_V1_RETAIL_CONFIGURATION_LEVEL ||
                !state.nexusState.retail_title_menu.easy_level_enabled) {
                fprintf(stderr,
                        "FAIL: Configuration mouse selects the authentic Easy option\n");
                ++failures;
            }
            if (M11_GameView_HandlePointerButton(
                    &state, 40, 185, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.configuration_active) {
                fprintf(stderr,
                        "FAIL: Configuration mouse activates the Exit row\n");
                ++failures;
            }
            if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) !=
                    M11_GAME_INPUT_REDRAW ||
                !state.nexusState.retail_title_menu.configuration_active ||
                M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK) !=
                    M11_GAME_INPUT_REDRAW ||
                state.nexusState.retail_title_menu.configuration_active ||
                state.nexusState.retail_title_menu.selection !=
                    NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
                fprintf(stderr,
                        "FAIL: B returns Configuration to the retail title menu\n");
                ++failures;
            }
        }
    }
    if (M11_GameView_HandlePointerButton(
            &state, 150, 195, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
            M11_GAME_INPUT_IGNORED ||
        state.nexusState.retail_title_menu.selection !=
            NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
        fprintf(stderr, "FAIL: clicks outside title rows do not change selection\n");
        ++failures;
    }
    M11_GameView_Shutdown(&state);
    {
        const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
        if (data_dir && data_dir[0] &&
            nexus_v1_launcher_init(data_dir) == 0) {
            int input_kind;
            for (input_kind = 0; input_kind < 2; ++input_kind) {
                M11_GameViewState live_state;
                Nexus_V1_Engine *engine = nexus_v1_launcher_get_engine();
                M11_GameInputResult input_result;
                M11_GameView_Init(&live_state);
                live_state.active = 1;
                live_state.sourceKind = M11_GAME_SOURCE_NEXUS_DGN;
                live_state.nexusEngine = engine;
                live_state.nexusState.title_active = 1;
                live_state.nexusState.title_frame = 0;
                while (!nexus_title_full_boot_start_ready(
                           live_state.nexusState.title_frame)) {
                    ++live_state.nexusState.title_frame;
                }
                nexus_v1_retail_title_menu_init(
                    &live_state.nexusState.retail_title_menu);
                (void)M11_GameView_HandleInput(
                    &live_state, M12_MENU_INPUT_ACTION);
                if (input_kind == 0) {
                    input_result = M11_GameView_HandleInput(
                        &live_state, M12_MENU_INPUT_ACCEPT);
                } else {
                    input_result = M11_GameView_HandlePointerButton(
                        &live_state, 150, 156,
                        DM1_V1_MOUSE_MASK_LEFT_PC34);
                }
                if (!engine || input_result != M11_GAME_INPUT_REDRAW ||
                    !engine->level_loaded || !engine->game.game_started ||
                    engine->game.current_level !=
                        NEXUS_V1_FIRST_PLAYABLE_LEVEL ||
                    engine->game.dungeon_start.status !=
                        NEXUS_V1_DUNGEON_START_READY ||
                    engine->game.party_x != 29 ||
                    engine->game.party_y != 47 ||
                    engine->game.party_dir != NEXUS_DIR_NORTH ||
                    live_state.nexusState.champion_select_active) {
                    fprintf(stderr,
                            "FAIL: authentic retail New Game %s handoff\n",
                            input_kind == 0 ? "keyboard" : "mouse");
                    ++failures;
                } else {
                    printf("PASS: authentic retail New Game %s handoff (level %d, %d,%d,N)\n",
                           input_kind == 0 ? "keyboard" : "mouse",
                           engine->game.current_level,
                           engine->game.party_x,
                           engine->game.party_y);
                }
                M11_GameView_Shutdown(&live_state);
                nexus_v1_launcher_shutdown();
                if (input_kind == 0 &&
                    nexus_v1_launcher_init(data_dir) != 0) {
                    fprintf(stderr,
                            "FAIL: reinitialize authentic Nexus launcher for mouse route\n");
                    ++failures;
                    break;
                }
            }
        } else {
            puts("SKIP: authentic Nexus media unavailable for M11 New Game handoff");
        }
    }
    if (failures == 0) {
        puts("PASS: Nexus retail title input stays within its evidence-bounded state");
    }
    return failures ? 1 : 0;
}
