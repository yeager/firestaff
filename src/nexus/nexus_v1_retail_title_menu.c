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
