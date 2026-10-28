#include "nexus_v1_retail_title_menu.h"

void nexus_v1_retail_title_menu_init(Nexus_V1_RetailTitleMenuState *state)
{
    if (state) {
        state->menu_active = 0;
        state->selection = NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME;
    }
}

Nexus_V1_RetailTitleMenuResult nexus_v1_retail_title_menu_reduce(
    Nexus_V1_RetailTitleMenuState *state,
    Nexus_V1_RetailTitleMenuInput input,
    int continue_save_available)
{
    if (!state ||
        state->selection < NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME ||
        state->selection > NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION ||
        (state->menu_active != 0 && state->menu_active != 1)) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_INVALID;
    }

    if (input == NEXUS_V1_RETAIL_TITLE_MENU_INPUT_OPEN) {
        if (!state->menu_active) {
            state->menu_active = 1;
            state->selection = NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME;
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_MENU_OPENED;
        }
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED;
    }
    if (!state->menu_active) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }

    if (input == NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN) {
        if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME) {
            state->selection = NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE;
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED;
        }
        if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE) {
            state->selection = NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION;
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED;
        }
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }
    if (input == NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UP) {
        if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
            state->selection = NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE;
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED;
        }
        if (state->selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE) {
            state->selection = NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME;
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED;
        }
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }

    if (input != NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }

    switch (state->selection) {
    case NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME:
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_START_NEW_GAME;
    case NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE:
        return continue_save_available
                   ? NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_LOAD_PANEL
                   : NEXUS_V1_RETAIL_TITLE_MENU_RESULT_NO_CONTINUE_SAVE;
    case NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION:
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_CONFIGURATION;
    default:
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_INVALID;
    }
}

Nexus_V1_RetailTitleMenuResult nexus_v1_retail_title_menu_select_pointer(
    Nexus_V1_RetailTitleMenuState *state,
    int x,
    int y,
    int configuration_renderable)
{
    Nexus_V1_RetailTitleMenuSelection selection;

    if (!state || state->menu_active != 1 || x < 0 || y < 0 ||
        state->selection < NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME ||
        state->selection > NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }

    /* These row bounds are the VDP1 text command extents retained by
     * verify_nexus_title_sprite_screenshot.py. The Ymir screenshot is 2x the
     * 320x224 native coordinate space. */
    if (y >= 153 && y < 161 && x >= 129 && x < 193) {
        selection = NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME;
    } else if (y >= 165 && y < 173 && x >= 128 && x < 192) {
        selection = NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE;
    } else if (configuration_renderable && y >= 177 && y < 185 &&
               x >= 109 && x < 213) {
        selection = NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION;
    } else {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }

    if (state->selection == selection) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED;
    }
    state->selection = selection;
    return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED;
}
