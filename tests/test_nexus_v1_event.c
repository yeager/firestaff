#include "nexus_v1_game.h"
#include "nexus_v1_movement.h"
#include <stdio.h>
#include <string.h>

static int g_failures;

static void expect(int cond, const char *msg) {
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); ++g_failures; }
}

int main(void) {
    Nexus_V1_GameState state;
    Nexus_Event ev;

    nexus_v1_game_init(&state, "/tmp/test");

    /* Event names match DM.BIN string table */
    expect(strcmp(nexus_v1_event_name(NEXUS_EV_NO_EVENT), "EV_NO_EVENT") == 0,
           "EV_NO_EVENT name");
    expect(strcmp(nexus_v1_event_name(NEXUS_EV_GO_FORWARD), "EV_GO_FORWARD") == 0,
           "EV_GO_FORWARD name");
    expect(strcmp(nexus_v1_event_name(NEXUS_EV_CONFIG), "EV_CONFIG") == 0,
           "EV_CONFIG name (last event)");
    expect(strcmp(nexus_v1_event_name(NEXUS_EV_COUNT), "EV_UNKNOWN") == 0,
           "out-of-range returns EV_UNKNOWN");

    /* Event count matches DM.BIN (61 events) */
    expect(NEXUS_EV_COUNT == 61, "61 event types from DM.BIN");

    /* Movement events advance tick */
    memset(&ev, 0, sizeof(ev));
    ev.type = NEXUS_EV_GO_FORWARD;
    state.tick_count = 0;
    expect(nexus_v1_event_dispatch(&state, &ev) == 1, "forward dispatches");
    expect(state.tick_count == 1, "forward advances tick");

    ev.type = NEXUS_EV_TURN_LEFT;
    expect(nexus_v1_event_dispatch(&state, &ev) == 1, "turn dispatches");
    expect(state.tick_count == 2, "turn advances tick");

    /* No-event returns 0 */
    ev.type = NEXUS_EV_NO_EVENT;
    expect(nexus_v1_event_dispatch(&state, &ev) == 0, "no-event returns 0");

    /* Null safety */
    expect(nexus_v1_event_dispatch(NULL, &ev) == -1, "null state returns -1");
    expect(nexus_v1_event_dispatch(&state, NULL) == -1, "null event returns -1");

    /* Inventory/spell events accepted */
    ev.type = NEXUS_EV_INVENTORY;
    expect(nexus_v1_event_dispatch(&state, &ev) == 1, "inventory accepted");
    ev.type = NEXUS_EV_DO_SPELL;
    expect(nexus_v1_event_dispatch(&state, &ev) == 1, "spell accepted");

    /* Event-to-command mapping (yam\event.c → command queue) */
    expect(nexus_v1_event_to_command(NEXUS_EV_GO_FORWARD) == NEXUS_CMD_FORWARD,
           "forward → CMD_FORWARD");
    expect(nexus_v1_event_to_command(NEXUS_EV_GO_BACKWARD) == NEXUS_CMD_BACKWARD,
           "backward → CMD_BACKWARD");
    expect(nexus_v1_event_to_command(NEXUS_EV_GO_LEFT) == NEXUS_CMD_STRAFE_LEFT,
           "left → CMD_STRAFE_LEFT");
    expect(nexus_v1_event_to_command(NEXUS_EV_GO_RIGHT) == NEXUS_CMD_STRAFE_RIGHT,
           "right → CMD_STRAFE_RIGHT");
    expect(nexus_v1_event_to_command(NEXUS_EV_TURN_LEFT) == NEXUS_CMD_TURN_LEFT,
           "turn_left → CMD_TURN_LEFT");
    expect(nexus_v1_event_to_command(NEXUS_EV_TURN_RIGHT) == NEXUS_CMD_TURN_RIGHT,
           "turn_right → CMD_TURN_RIGHT");
    expect(nexus_v1_event_to_command(NEXUS_EV_GET_ITEM) == NEXUS_CMD_INTERACT,
           "get_item → CMD_INTERACT");
    expect(nexus_v1_event_to_command(NEXUS_EV_GETITEM) == NEXUS_CMD_INTERACT,
           "getitem → CMD_INTERACT");
    expect(nexus_v1_event_to_command(NEXUS_EV_DO_SPELL) == NEXUS_CMD_CAST_SPELL,
           "do_spell → CMD_CAST_SPELL");
    expect(nexus_v1_event_to_command(NEXUS_EV_GO_MAP) == NEXUS_CMD_TOGGLE_MAP,
           "go_map → CMD_TOGGLE_MAP");
    expect(nexus_v1_event_to_command(NEXUS_EV_NO_EVENT) == NEXUS_CMD_NONE,
           "no_event → CMD_NONE");
    expect(nexus_v1_event_to_command(NEXUS_EV_PAUSE_GAME) == NEXUS_CMD_NONE,
           "pause → CMD_NONE (no direct command)");

    /* Additional dispatched events */
    ev.type = NEXUS_EV_GO_MAP;
    expect(nexus_v1_event_dispatch(&state, &ev) == 1, "map event accepted");
    ev.type = NEXUS_EV_CANCEL;
    expect(nexus_v1_event_dispatch(&state, &ev) == 1, "cancel event accepted");
    ev.type = NEXUS_EV_SET_LEAD;
    expect(nexus_v1_event_dispatch(&state, &ev) == 1, "set_lead event accepted");

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_event: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: Nexus event system with 61 types + command mapping verified");
    return 0;
}
