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
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_ACCEPT,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_START_NEW_GAME,
          "accepting initial New Game selects the observed new-game route");

    nexus_v1_retail_title_menu_init(&state);
    check(nexus_v1_retail_title_menu_reduce(
              &state,
              NEXUS_V1_RETAIL_TITLE_MENU_INPUT_DOWN,
              0) == NEXUS_V1_RETAIL_TITLE_MENU_RESULT_SELECTION_CHANGED &&
              state.selection == NEXUS_V1_RETAIL_TITLE_MENU_CONTINUE,
          "one Down selects Continue");
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

    if (failures != 0) {
        fprintf(stderr, "test_nexus_v1_retail_title_menu: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("ok: evidence-bounded Nexus retail title-menu reducer");
    return 0;
}
