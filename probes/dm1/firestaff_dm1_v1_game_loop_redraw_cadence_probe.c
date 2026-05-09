/*
 * Firestaff DM1 V1 game-loop redraw/cadence probe.
 *
 * Primary source audit: ReDMCSB WIP20210206, Toolchains/Common/Source.
 *   GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF lines 46-63 set/reset input wait and
 *     process a pending map change before timeline; lines 69-72 process timeline
 *     and immediately re-enter the map-change path if timeline requested one.
 *   GAMELOOP.C:F0002 lines 80-90 draw dungeon view only when not resting and not
 *     in inventory; lines 91-102 update mouse pointer only after the viewport draw.
 *   GAMELOOP.C:F0002 lines 114-131 disable highlights, play sound, apply damage,
 *     advance game time, decrement FreezeLifeTicks, and refresh the action area.
 *   GAMELOOP.C:F0002 lines 150-156 decrement movement/projectile cooldowns after
 *     redraw/action refresh and before entering the command wait loop.
 *   GAMELOOP.C:F0002 lines 164-219 clear StopWaitingForInput, process key/queue
 *     commands, disable highlights while still waiting, and loop until stop-waiting
 *     and game-time ticking are both true.
 *   DRAWVIEW.C:F0097_DUNGEONVIEW_DrawViewport lines 709-722 requests a viewport
 *     draw and waits one vertical blank before returning on ST-like ports.
 *   DRAWVIEW.C:F0097 lines 821-858 apply palette/zone state and blit C007 viewport
 *     on PC/I34-style ports.
 *   VIEWPORT.C:F0565/F0566 lines 35-47 and 51-98 wait/own blitter, cross the
 *     viewport beam region, and copy all four 224x136 bitplanes.
 *   COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC lines 2075-2101 locks the queue and
 *     refuses movement while disabled; lines 2150-2156 dispatch turn/move commands.
 *   VBLANK.C:F0577_VerticalBlank_Handler_CPSDF lines 103-109 increments the input
 *     wait counter and releases the loop when it passes the configured maximum.
 *
 * This probe is intentionally headless: it models only the source-locked ordering
 * constraints that can regress silently during SDL/PC34 orchestration work.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENTS 96
#define C003_COMMAND_MOVE_FORWARD 3
#define C006_COMMAND_MOVE_LEFT 6

typedef struct {
    const char *phase;
    int tick;
    int value;
} Event;

typedef struct {
    int new_map_index;
    int resting;
    int inventory_open;
    int draw_requested;
    int viewport_presented;
    int pointer_refresh;
    int movement_cooldown;
    int projectile_cooldown;
    int projectile_direction;
    int party_direction;
    int wait_vblanks;
    int wait_max;
    int stop_waiting;
    int game_time_ticking;
} Sim;

static Event events[MAX_EVENTS];
static int event_count;
static int failures;

static void record(const char *phase, int tick, int value) {
    if (event_count >= MAX_EVENTS) {
        fprintf(stderr, "too many events\n");
        exit(2);
    }
    events[event_count++] = (Event){ phase, tick, value };
    printf("%02d tick=%d phase=%-28s value=%d\n", event_count - 1, tick, phase, value);
}

static int index_of(const char *phase) {
    for (int i = 0; i < event_count; ++i) {
        if (strcmp(events[i].phase, phase) == 0) return i;
    }
    return -1;
}

static void check_true(const char *label, int condition) {
    if (!condition) {
        fprintf(stderr, "FAIL %s\n", label);
        failures++;
    }
}

static int normalize_dir(int direction) {
    int r = direction & 3;
    return r < 0 ? r + 4 : r;
}

static int movement_blocked(const Sim *s, int command) {
    if (command < C003_COMMAND_MOVE_FORWARD || command > C006_COMMAND_MOVE_LEFT) return 0;
    if (s->movement_cooldown > 0) return 1;
    if (s->projectile_cooldown <= 0) return 0;
    return s->projectile_direction == normalize_dir(s->party_direction + command - C003_COMMAND_MOVE_FORWARD);
}

static void draw_viewport(Sim *s, int tick) {
    record("dungeon_view_draw", tick, 0);
    s->draw_requested = 1;
    record("viewport_request", tick, 1);
    /* DRAWVIEW.C:F0097 lines 721-722: request then wait one VBlank before return. */
    s->wait_vblanks++;
    record("vblank_wait_for_present", tick, s->wait_vblanks);
    s->viewport_presented = 1;
    record("viewport_present", tick, 1);
}

static void run_tick(Sim *s, int tick) {
    record("wait_counter_reset", tick, 0);
    s->wait_vblanks = 0;
    s->stop_waiting = 0;

    if (s->new_map_index >= 0) {
        record("new_map_process", tick, s->new_map_index);
        record("movement_reanchor", tick, 0);
        record("discard_input", tick, 0);
        s->new_map_index = -1;
    }

    record("timeline_process", tick, 0);
    if (tick == 0) {
        s->new_map_index = 7;
        record("timeline_requested_map", tick, s->new_map_index);
        record("new_map_process_after_timeline", tick, s->new_map_index);
        record("movement_reanchor_after_timeline", tick, 0);
        record("discard_input_after_timeline", tick, 0);
        s->new_map_index = -1;
    }

    if (!s->resting && !s->inventory_open) {
        draw_viewport(s, tick);
    } else {
        record("dungeon_view_skipped", tick, s->resting ? 1 : 2);
    }

    if (s->pointer_refresh) {
        record("mouse_pointer_update", tick, 1);
        s->pointer_refresh = 0;
    }

    record("highlight_disable_after_draw", tick, 0);
    record("sound_pending", tick, 0);
    record("damage_wounds", tick, 0);
    record("game_time_increment", tick, tick + 1);
    record("action_area_refresh", tick, 0);

    if (s->movement_cooldown > 0) {
        s->movement_cooldown--;
        record("movement_cooldown_decrement", tick, s->movement_cooldown);
    }
    if (s->projectile_cooldown > 0) {
        s->projectile_cooldown--;
        record("projectile_cooldown_decrement", tick, s->projectile_cooldown);
    }
    record("message_rows_clear", tick, 0);

    record("command_wait_enter", tick, 0);
    for (int spin = 0; spin < 3 && (!s->stop_waiting || !s->game_time_ticking); ++spin) {
        int command = C003_COMMAND_MOVE_FORWARD;
        record("command_queue_process", tick, command);
        if (movement_blocked(s, command)) {
            record("movement_command_blocked", tick, s->movement_cooldown + s->projectile_cooldown);
        } else {
            record("movement_command_dispatched", tick, command);
        }
        s->wait_vblanks++;
        if (s->wait_vblanks > s->wait_max) s->stop_waiting = 1;
        if (!s->stop_waiting) record("highlight_disable_wait", tick, spin);
        if (spin == 1) s->game_time_ticking = 1;
    }
    record("command_wait_exit", tick, s->wait_vblanks);
}

static void verify_order(void) {
    const char *ordered[] = {
        "wait_counter_reset",
        "new_map_process",
        "timeline_process",
        "new_map_process_after_timeline",
        "dungeon_view_draw",
        "viewport_request",
        "vblank_wait_for_present",
        "viewport_present",
        "mouse_pointer_update",
        "highlight_disable_after_draw",
        "action_area_refresh",
        "movement_cooldown_decrement",
        "projectile_cooldown_decrement",
        "command_wait_enter",
        "command_queue_process",
        "movement_command_dispatched",
        "command_wait_exit",
    };
    int previous = -1;
    for (size_t i = 0; i < sizeof ordered / sizeof ordered[0]; ++i) {
        int pos = index_of(ordered[i]);
        check_true(ordered[i], pos >= 0);
        check_true("phase order", previous < pos);
        previous = pos;
    }
    check_true("viewport is presented before pointer work", index_of("viewport_present") < index_of("mouse_pointer_update"));
    check_true("cooldowns decrement before command wait", index_of("projectile_cooldown_decrement") < index_of("command_wait_enter"));
}

int main(void) {
    Sim s;
    memset(&s, 0, sizeof s);
    s.new_map_index = 3;
    s.pointer_refresh = 1;
    s.movement_cooldown = 1;
    s.projectile_cooldown = 1;
    s.projectile_direction = 2;
    s.party_direction = 0;
    s.wait_max = 2;

    printf("Firestaff DM1 V1 game-loop redraw/cadence probe\n");
    printf("primary_source=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("locks=GAMELOOP.C:46-219,DRAWVIEW.C:709-858,VIEWPORT.C:35-98,COMMAND.C:2075-2156,VBLANK.C:103-109\n\n");

    run_tick(&s, 0);
    verify_order();
    check_true("movement cooldown reached zero before command dispatch", s.movement_cooldown == 0);
    check_true("projectile cooldown reached zero before command dispatch", s.projectile_cooldown == 0);
    check_true("viewport request was presented", s.draw_requested && s.viewport_presented);

    printf("\n[result] failures=%d events=%d\n", failures, event_count);
    return failures ? 1 : 0;
}
