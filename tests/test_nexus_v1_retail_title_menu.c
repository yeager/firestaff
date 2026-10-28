#include "nexus_v1_retail_title_menu.h"

#include <stdio.h>

static int failures;

static void check(int condition, const char *description)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", description);
        ++failures;
    }
}

int main(void)
{
    Nexus_V1_RetailTitleMenuState state;

    nexus_v1_retail_title_menu_init(&state);
    check(state.selection == NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME,
          "retail title menu initializes with New Game selected");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_OPEN,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_MENU_OPENED &&
              state.menu_active,
          "Start opens the retail menu with New Game selected");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_START_NEW_GAME,
          "accepting New Game selects the observed new-game route");

    nexus_v1_retail_title_menu_init(&state);
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_OPEN, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_MENU_OPENED,
          "Start opens menu before navigation");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE,
          "one Down selects Continue");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UP,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME,
          "Up from Continue returns to New Game");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE,
          "Down from New Game returns to Continue");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_NO_CONTINUE_SAVE &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE,
          "Continue without a real save leaves the selection unchanged");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT,
              1) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_LOAD_PANEL,
          "Continue with a caller-confirmed save opens the load panel");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION,
          "a second Down selects Configuration");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UP,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE,
          "Up from Configuration returns to Continue");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION,
          "Down from Continue returns to Configuration");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT,
              0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_CONFIGURATION,
          "accepting Configuration opens the observed configuration page");
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UNSUPPORTED,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION,
          "unverified input does not invent a transition");

    nexus_v1_retail_title_menu_init(&state);
    check(nexus_v1_retail_title_menu_select_pointer(&state, 150, 153, 1) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT &&
              !state.menu_active,
          "pointer cannot select before the retail menu opens");
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_OPEN, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_MENU_OPENED,
          "open retail menu before pointer selection");
    check(nexus_v1_retail_title_menu_select_pointer(&state, 150, 153, 1) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_NEW_GAME,
          "authentic New Game text rectangle selects the first row");
    check(nexus_v1_retail_title_menu_select_pointer(&state, 150, 165, 1) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE,
          "authentic Continue text rectangle selects the second row");
    check(nexus_v1_retail_title_menu_select_pointer(&state, 150, 168, 1) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNCHANGED,
          "pointer includes the Continue row's last pixel");
    check(nexus_v1_retail_title_menu_select_pointer(&state, 150, 177, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE,
          "Configuration pointer stays gated without a regional selected renderer");
    check(nexus_v1_retail_title_menu_select_pointer(&state, 150, 177, 1) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION,
          "source-bound Configuration row accepts pointer selection");
    check(nexus_v1_retail_title_menu_enter_configuration(&state) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_CONFIGURATION &&
              state.configuration_active &&
              state.configuration_row == NEXUS_V1_RETAIL_CONFIGURATION_MUSIC &&
              state.music_enabled && state.stereo_enabled &&
              !state.easy_level_enabled && state.monster_gauge_enabled,
          "English Configuration page opens with captured retail defaults");
    check(nexus_v1_retail_title_menu_select_configuration_pointer(
              &state, 100, 83) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.configuration_row == NEXUS_V1_RETAIL_CONFIGURATION_SOUND &&
              state.music_enabled && state.stereo_enabled,
          "Sound row owns its first pointer scanline without changing Music");
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_UP, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.configuration_row == NEXUS_V1_RETAIL_CONFIGURATION_MUSIC,
          "Up from the Sound pointer boundary returns to Music");
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_RIGHT, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CHANGED &&
              !state.music_enabled,
          "Right changes Music from the captured ON setting");
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.configuration_row == NEXUS_V1_RETAIL_CONFIGURATION_SOUND,
          "Down moves Configuration focus from Music to Sound");
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CHANGED &&
              !state.stereo_enabled,
          "Accept changes Sound from captured Stereo to Mono");
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              nexus_v1_retail_title_menu_reduce(
                  &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN, 0) ==
                  NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              nexus_v1_retail_title_menu_reduce(
                  &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN, 0) ==
                  NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.configuration_row == NEXUS_V1_RETAIL_CONFIGURATION_EXIT,
          "Down reaches the authentic Exit row after the four settings");
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CLOSED &&
              !state.configuration_active && state.menu_active &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONFIGURATION,
          "Accept on Exit returns to the title Configuration row");
    check(nexus_v1_retail_title_menu_enter_configuration(&state) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_CONFIGURATION &&
              !state.music_enabled && !state.stereo_enabled,
          "reopening Configuration preserves changed settings in runtime state");
    check(nexus_v1_retail_title_menu_select_configuration_pointer(
              &state, 260, 115) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CHANGED &&
              state.configuration_row == NEXUS_V1_RETAIL_CONFIGURATION_LEVEL &&
              state.easy_level_enabled,
          "mouse selects the authentic Easy option by its retail sprite bounds");
    check(nexus_v1_retail_title_menu_select_configuration_pointer(
              &state, 40, 185) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CLOSED &&
              !state.configuration_active,
          "mouse activates the source-bound Exit row");
    check(nexus_v1_retail_title_menu_enter_configuration(&state) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SHOW_CONFIGURATION &&
              nexus_v1_retail_title_menu_reduce(
                  &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_LEFT, 0) ==
                  NEXUS_V1_RETAIL_TITLE_MENU_RESULT_UNVERIFIED_INPUT &&
              !state.music_enabled,
          "unverified Left input does not alter the captured setting");
    check(nexus_v1_retail_title_menu_reduce(
              &state, NEXUS_V1_RETAIL_TITLE_MENU_INPUT_BACK, 0) ==
              NEXUS_V1_RETAIL_TITLE_MENU_RESULT_CONFIGURATION_CLOSED &&
              !state.configuration_active && state.menu_active,
          "B returns from Configuration to the retail title menu");

    if (failures != 0) {
        fprintf(stderr, "test_nexus_v1_retail_title_menu: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("ok: evidence-bounded Nexus retail title-menu reducer");
    return 0;
}
