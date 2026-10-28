#include "m11_game_view.h"
#include "nexus_v1_engine.h"
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
                    NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
                fprintf(stderr,
                        "FAIL: source-bound Japanese Configuration row responds to mouse\n");
                ++failures;
            }
            if (M11_GameView_HandlePointerButton(
                    &state, 150, 181, DM1_V1_MOUSE_MASK_LEFT_PC34) !=
                    M11_GAME_INPUT_IGNORED ||
                state.nexusState.retail_title_menu.selection !=
                    NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
                fprintf(stderr,
                        "FAIL: clicking selected Configuration does not redraw\n");
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
    if (failures == 0) {
        puts("PASS: Nexus retail title input stays within its evidence-bounded state");
    }
    return failures ? 1 : 0;
}
