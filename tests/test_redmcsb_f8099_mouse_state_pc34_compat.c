#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f8099_mouse_state_pc34_compat.h"

typedef struct test_trace {
    char events[8];
    unsigned int event_count;
    unsigned int cursor_count;
    int16_t cursor_x[2];
    int16_t cursor_y[2];
} test_trace;

static void set_cursor_position(void *context, int16_t x, int16_t y)
{
    test_trace *trace = (test_trace *)context;

    trace->events[trace->event_count] = 'C';
    trace->cursor_x[trace->cursor_count] = x;
    trace->cursor_y[trace->cursor_count] = y;
    trace->event_count++;
    trace->cursor_count++;
}

static void erase_mouse_pointer(void *context)
{
    test_trace *trace = (test_trace *)context;

    trace->events[trace->event_count++] = 'E';
}

static void process_mouse_movement(void *context)
{
    test_trace *trace = (test_trace *)context;

    trace->events[trace->event_count++] = 'M';
}

static int check_int(const char *label, int actual, int expected)
{
    if (actual == expected) {
        return 1;
    }
    fprintf(stderr, "%s: got %d, expected %d\n", label, actual, expected);
    return 0;
}

int main(void)
{
    redmcsb_f8099_mouse_state_pc34_compat mouse_state = {
        10, 20, 10, 20, (int16_t)0x8003, 0, 1, 1
    };
    test_trace trace = {{0}, 0U, 0U, {0, 0}, {0, 0}};
    int16_t x = 0;
    int16_t y = 0;
    int16_t buttons = 0;
    int ok = 1;

    redmcsb_f8099_lock_mouse_pc34_compat(&mouse_state);
    ok &= check_int("lock increments", mouse_state.lock_count, 1);
    redmcsb_f8100_unlock_mouse_pc34_compat(&mouse_state, set_cursor_position,
                                            &trace);
    ok &= check_int("unlock decrements", mouse_state.lock_count, 0);
    ok &= check_int("unlock cursor count", (int)trace.event_count, 1);
    ok &= check_int("unlock doubles x", trace.cursor_x[0], 20);
    ok &= check_int("unlock preserves y", trace.cursor_y[0], 20);

    trace.event_count = 0U;
    trace.cursor_count = 0U;
    redmcsb_f8111_set_mouse_pointer_coordinates_pc34_compat(
        &mouse_state, 123, 45, set_cursor_position, erase_mouse_pointer,
        process_mouse_movement, &trace);
    trace.events[trace.event_count] = '\0';
    ok &= check_int("coordinates x", mouse_state.current_x, 123);
    ok &= check_int("coordinates y", mouse_state.current_y, 45);
    ok &= check_int("assigned x", mouse_state.assigned_x, 123);
    ok &= check_int("assigned y", mouse_state.assigned_y, 45);
    ok &= check_int("coordinates balance lock", mouse_state.lock_count, 0);
    ok &= check_int("coordinate event count", (int)trace.event_count, 4);
    ok &= check_int("coordinate event sequence", strcmp(trace.events, "CEMC"), 0);
    ok &= check_int("first coordinate x", trace.cursor_x[0], 246);
    ok &= check_int("second coordinate x", trace.cursor_x[1], 246);
    ok &= check_int("first coordinate y", trace.cursor_y[0], 45);
    ok &= check_int("second coordinate y", trace.cursor_y[1], 45);

    redmcsb_f8112_get_mouse_state_pc34_compat(&mouse_state, &x, &y, &buttons);
    ok &= check_int("state x", x, 123);
    ok &= check_int("state y", y, 45);
    ok &= check_int("state buttons", (unsigned short)buttons, 0x8003);
    ok &= check_int("evidence IBMIO anchors",
                    strstr(redmcsb_f8099_mouse_state_source_evidence_pc34(),
                           "IBMIO.C:851-878,1190-1225") != 0, 1);

    return ok ? 0 : 1;
}
