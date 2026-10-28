#include "nexus_v1_retail_title_menu.h"

void nexus_v1_retail_title_menu_init(Nexus_V1_RetailTitleMenuState *state)
{
    if (state) {
        state->menu_active = 0;
        state->selection = NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME;
        state->configuration_active = 0;
        state->configuration_row = NEXUS_V1_RETAIL_CONFIGURATION_MUSIC;
        state->music_enabled = 1;
        state->stereo_enabled = 1;
        state->easy_level_enabled = 0;
        state->monster_gauge_enabled = 1;
    }
}

Nexus_V1_RetailTitleMenuResult
nexus_v1_retail_title_menu_enter_configuration(
    Nexus_V1_RetailTitleMenuState *state)
{
    if (!state || state->menu_active != 1 ||
        state->selection != NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_INVALID;
    }
    state->configuration_active = 1;
    state->configuration_row = NEXUS_V1_RETAIL_CONFIGURATION_MUSIC;
    return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_CONFIGURATION;
}

Nexus_V1_RetailTitleMenuResult nexus_v1_retail_title_menu_reduce(
    Nexus_V1_RetailTitleMenuState *state,
    Nexus_V1_RetailTitleMenuInput input,
    int continue_save_available)
{
    if (!state ||
        state->selection < NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME ||
        state->selection > NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION ||
        (state->menu_active != 0 && state->menu_active != 1) ||
        (state->configuration_active != 0 &&
         state->configuration_active != 1) ||
        state->configuration_row < NEXUS_V1_RETAIL_CONFIGURATION_MUSIC ||
        state->configuration_row > NEXUS_V1_RETAIL_CONFIGURATION_EXIT ||
        (state->music_enabled != 0 && state->music_enabled != 1) ||
        (state->stereo_enabled != 0 && state->stereo_enabled != 1) ||
        (state->easy_level_enabled != 0 && state->easy_level_enabled != 1) ||
        (state->monster_gauge_enabled != 0 &&
         state->monster_gauge_enabled != 1)) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_INVALID;
    }

    if (state->configuration_active) {
        if (input == NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN) {
            if (state->configuration_row < NEXUS_V1_RETAIL_CONFIGURATION_EXIT) {
                state->configuration_row =
                    (Nexus_V1_RetailConfigurationRow)(
                        state->configuration_row + 1);
                return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED;
            }
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED;
        }
        if (input == NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UP) {
            if (state->configuration_row > NEXUS_V1_RETAIL_CONFIGURATION_MUSIC) {
                state->configuration_row =
                    (Nexus_V1_RetailConfigurationRow)(
                        state->configuration_row - 1);
                return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED;
            }
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED;
        }
        if (input == NEXUS_V1_RETAIL_TITLE_MENU_INPUT_BACK) {
            state->configuration_active = 0;
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CLOSED;
        }
        if (input != NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT &&
            input != NEXUS_V1_RETAIL_TITLE_MENU_INPUT_RIGHT) {
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
        }
        if (state->configuration_row == NEXUS_V1_RETAIL_CONFIGURATION_EXIT) {
            if (input == NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT) {
                state->configuration_active = 0;
                return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CLOSED;
            }
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED;
        }
        switch (state->configuration_row) {
        case NEXUS_V1_RETAIL_CONFIGURATION_MUSIC:
            state->music_enabled = !state->music_enabled;
            break;
        case NEXUS_V1_RETAIL_CONFIGURATION_SOUND:
            state->stereo_enabled = !state->stereo_enabled;
            break;
        case NEXUS_V1_RETAIL_CONFIGURATION_LEVEL:
            state->easy_level_enabled = !state->easy_level_enabled;
            break;
        case NEXUS_V1_RETAIL_CONFIGURATION_MONSTER_GAUGE:
            state->monster_gauge_enabled = !state->monster_gauge_enabled;
            break;
        case NEXUS_V1_RETAIL_CONFIGURATION_EXIT:
        default:
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_INVALID;
        }
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CHANGED;
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

Nexus_V1_RetailTitleMenuResult
nexus_v1_retail_title_menu_select_configuration_pointer(
    Nexus_V1_RetailTitleMenuState *state,
    int x,
    int y)
{
    Nexus_V1_RetailConfigurationRow row;
    static const int row_right_edge[] = {258, 319, 295, 257, 92};
    int row_changed;
    if (!state || state->configuration_active != 1 || x < 0 ||
        x >= 320 || y < 0 || y >= 224) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }
    if (y >= 60 && y < 84) {
        row = NEXUS_V1_RETAIL_CONFIGURATION_MUSIC;
    } else if (y >= 83 && y < 106) {
        row = NEXUS_V1_RETAIL_CONFIGURATION_SOUND;
    } else if (y >= 106 && y < 127) {
        row = NEXUS_V1_RETAIL_CONFIGURATION_LEVEL;
    } else if (y >= 127 && y < 152) {
        row = NEXUS_V1_RETAIL_CONFIGURATION_MONSTER_GAUGE;
    } else if (y >= 179 && y < 196) {
        row = NEXUS_V1_RETAIL_CONFIGURATION_EXIT;
    } else {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }
    if (x < 10 || x >= row_right_edge[row]) {
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT;
    }
    if (row == NEXUS_V1_RETAIL_CONFIGURATION_EXIT) {
        state->configuration_row = row;
        state->configuration_active = 0;
        return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CLOSED;
    }
    row_changed = state->configuration_row != row;
    state->configuration_row = row;
#define SELECT_VALUE(field, value) \
    do { \
        if ((field) != (value)) { \
            (field) = (value); \
            return NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CHANGED; \
        } \
    } while (0)
    if (row == NEXUS_V1_RETAIL_CONFIGURATION_MUSIC &&
        x >= 175 && x < 200) {
        SELECT_VALUE(state->music_enabled, 1);
    } else if (row == NEXUS_V1_RETAIL_CONFIGURATION_MUSIC &&
               x >= 205 && x < 237) {
        SELECT_VALUE(state->music_enabled, 0);
    } else if (row == NEXUS_V1_RETAIL_CONFIGURATION_SOUND &&
               x >= 174 && x < 238) {
        SELECT_VALUE(state->stereo_enabled, 0);
    } else if (row == NEXUS_V1_RETAIL_CONFIGURATION_SOUND &&
               x >= 243 && x < 299) {
        SELECT_VALUE(state->stereo_enabled, 1);
    } else if (row == NEXUS_V1_RETAIL_CONFIGURATION_LEVEL &&
               x >= 175 && x < 231) {
        SELECT_VALUE(state->easy_level_enabled, 0);
    } else if (row == NEXUS_V1_RETAIL_CONFIGURATION_LEVEL &&
               x >= 236 && x < 276) {
        SELECT_VALUE(state->easy_level_enabled, 1);
    } else if (row == NEXUS_V1_RETAIL_CONFIGURATION_MONSTER_GAUGE &&
               x >= 175 && x < 199) {
        SELECT_VALUE(state->monster_gauge_enabled, 1);
    } else if (row == NEXUS_V1_RETAIL_CONFIGURATION_MONSTER_GAUGE &&
               x >= 204 && x < 236) {
        SELECT_VALUE(state->monster_gauge_enabled, 0);
    }
#undef SELECT_VALUE
    return row_changed ? NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED
                       : NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED;
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
