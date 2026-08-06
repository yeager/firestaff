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

    /* Event names are available, but dispatch is capture-gated. */
    memset(&ev, 0, sizeof(ev));
    ev.type = NEXUS_EV_GO_FORWARD;
    state.tick_count = 0;
    expect(nexus_v1_event_dispatch(&state, &ev) ==
               NEXUS_V1_EVENT_DISPATCH_UNBOUND,
           "forward remains unbound");
    expect(state.tick_count == 0, "unbound forward does not advance tick");

    ev.type = NEXUS_EV_TURN_LEFT;
    expect(nexus_v1_event_dispatch(&state, &ev) ==
               NEXUS_V1_EVENT_DISPATCH_UNBOUND,
           "turn remains unbound");
    expect(state.tick_count == 0, "unbound turn does not advance tick");

    /* No-event returns 0 */
    ev.type = NEXUS_EV_NO_EVENT;
    expect(nexus_v1_event_dispatch(&state, &ev) ==
               NEXUS_V1_EVENT_DISPATCH_UNBOUND,
           "no-event remains unbound");

    /* Null safety */
    expect(nexus_v1_event_dispatch(NULL, &ev) == -1, "null state returns -1");
    expect(nexus_v1_event_dispatch(&state, NULL) == -1, "null event returns -1");

    /* Inventory/spell events remain unbound. */
    ev.type = NEXUS_EV_INVENTORY;
    expect(nexus_v1_event_dispatch(&state, &ev) ==
               NEXUS_V1_EVENT_DISPATCH_UNBOUND,
           "inventory remains unbound");
    ev.type = NEXUS_EV_DO_SPELL;
    expect(nexus_v1_event_dispatch(&state, &ev) ==
               NEXUS_V1_EVENT_DISPATCH_UNBOUND,
           "spell remains unbound");

    /* No event→command mapping is admitted without Saturn capture. */
    expect(nexus_v1_event_to_command(NEXUS_EV_GO_FORWARD) == NEXUS_CMD_NONE,
           "forward remains unmapped");
    expect(nexus_v1_event_to_command(NEXUS_EV_DO_SPELL) == NEXUS_CMD_NONE,
           "spell remains unmapped");
    expect(nexus_v1_event_to_command(NEXUS_EV_GO_MAP) == NEXUS_CMD_NONE,
           "map remains unmapped");

    /* Additional dispatched events */
    ev.type = NEXUS_EV_GO_MAP;
    expect(nexus_v1_event_dispatch(&state, &ev) ==
               NEXUS_V1_EVENT_DISPATCH_UNBOUND,
           "map event remains unbound");
    ev.type = NEXUS_EV_CANCEL;
    expect(nexus_v1_event_dispatch(&state, &ev) ==
               NEXUS_V1_EVENT_DISPATCH_UNBOUND,
           "cancel event remains unbound");
    ev.type = NEXUS_EV_SET_LEAD;
    expect(nexus_v1_event_dispatch(&state, &ev) ==
               NEXUS_V1_EVENT_DISPATCH_UNBOUND,
           "set_lead event remains unbound");

    if (g_failures) {
        fprintf(stderr, "test_nexus_v1_event: %d failure(s)\n", g_failures);
        return 1;
    }
    puts("ok: Nexus 61 event-name receipts remain capture-gated");
    return 0;
}
