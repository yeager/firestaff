#include "nexus_v1_game.h"
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

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_event: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: Nexus event system with 61 DM.BIN event types verified");
    return 0;
}
