#include "nexus_v1_startup_menu.h"
#include "nexus_v1_title_sequence.h"

#include <stdio.h>
#include <string.h>

static int g_passed;
static int g_failed;

static void check(int condition, const char *message)
{
    if (condition) {
        ++g_passed;
        printf("PASS %s\n", message);
    } else {
        ++g_failed;
        printf("FAIL %s\n", message);
    }
}

int main(void)
{
    Nexus_V1_StartupAction action;
    Nexus_V1_StartupMenu menu;
    Nexus_V1_StartupSaveRenderRow rows[4];
    char phase[64];
    int startup_active;
    int startup_frame;
    int ready_frame = nexus_v1_title_start_ready_frames();
    int count;

    memset(&action, 0, sizeof(action));
    check(nexus_v1_startup_title_handle_hit(0, 0u, &action) &&
              action.kind == NEXUS_V1_STARTUP_ACTION_HOLD_TITLE,
          "title pointer is held before start-ready frame");
    check(nexus_v1_startup_title_handle_input(
              ready_frame, 1u, NEXUS_V1_STARTUP_INPUT_ACCEPT, &action) &&
              action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_SAVE_SELECT,
          "title accept routes to save select when saves exist");
    check(nexus_v1_startup_title_handle_input(
              ready_frame, 0u, NEXUS_V1_STARTUP_INPUT_ACCEPT, &action) &&
              action.kind == NEXUS_V1_STARTUP_ACTION_SHOW_CHAMPION_SELECT,
          "title accept routes to champion select without saves");
    check(nexus_v1_startup_title_handle_input(
              ready_frame, 0u, NEXUS_V1_STARTUP_INPUT_BACK, &action) &&
              action.kind == NEXUS_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER,
          "title back returns to launcher");

    nexus_v1_startup_menu_init(&menu, "/tmp/firestaff-nexus-test");
    check(nexus_v1_startup_menu_refresh(&menu, 1u) &&
              menu.row_count == 2,
          "save menu refresh exposes slot plus new game");
    count = nexus_v1_startup_menu_build_save_render_rows(
        &menu, rows, 4);
    check(count == 2 &&
              rows[0].kind == NEXUS_V1_STARTUP_ROW_SLOT &&
              rows[0].slot == 0 &&
              rows[1].kind == NEXUS_V1_STARTUP_ROW_NEW_GAME,
          "save render rows expose slot and new game");
    check(nexus_v1_startup_menu_handle_input(
              &menu, NEXUS_V1_STARTUP_INPUT_DOWN, &action) &&
              action.kind == NEXUS_V1_STARTUP_ACTION_NONE &&
              menu.selected_row == 1,
          "save menu down moves selection");
    check(nexus_v1_startup_menu_handle_input(
              &menu, NEXUS_V1_STARTUP_INPUT_ACCEPT, &action) &&
              action.kind == NEXUS_V1_STARTUP_ACTION_NEW_GAME,
          "save menu accept activates new game");
    check(nexus_v1_startup_receipt_phase(
              1, 0, 0, 42, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "nexus-title") == 0 &&
              startup_active == 1 &&
              startup_frame == 42,
          "receipt phase reports Nexus title");
    check(nexus_v1_startup_receipt_phase(
              0, 1, 0, 42, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "nexus-save-select") == 0 &&
              startup_active == 1,
          "receipt phase reports Nexus save select");
    check(nexus_v1_startup_receipt_phase(
              0, 0, 1, 42, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "nexus-champion-select") == 0 &&
              startup_active == 1,
          "receipt phase reports Nexus champion select");
    check(nexus_v1_startup_receipt_phase(
              0, 0, 0, 42, phase, sizeof(phase),
              &startup_active, &startup_frame) &&
              strcmp(phase, "nexus-runtime") == 0 &&
              startup_active == 0,
          "receipt phase reports Nexus runtime");

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed ? 1 : 0;
}
