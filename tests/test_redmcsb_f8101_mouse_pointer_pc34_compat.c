#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f8101_mouse_pointer_pc34_compat.h"

typedef struct test_trace {
    char events[8];
    unsigned int count;
} test_trace;

static int16_t mouse_handler(int16_t x, int16_t y, int16_t mouse_event)
{
    return (int16_t)(x + y + mouse_event);
}

static void record_suspend(void *context)
{
    test_trace *trace = (test_trace *)context;

    trace->events[trace->count++] = 'S';
}

static void record_erase(void *context)
{
    test_trace *trace = (test_trace *)context;

    trace->events[trace->count++] = 'E';
}

static void record_movement(void *context)
{
    test_trace *trace = (test_trace *)context;

    trace->events[trace->count++] = 'M';
}

static void record_history(void *context)
{
    test_trace *trace = (test_trace *)context;

    trace->events[trace->count++] = 'H';
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
    redmcsb_f8101_mouse_pointer_state_pc34_compat mouse_state = {0};
    uint8_t bitmap[REDMCSB_F8101_MOUSE_POINTER_BITMAP_BYTES_PC34];
    test_trace trace = {{0}, 0U};
    unsigned int index;
    int ok = 1;

    for (index = 0U; index < sizeof(bitmap); ++index) {
        bitmap[index] = (uint8_t)(index + 1U);
    }

    redmcsb_f8101_set_mouse_handler_pc34_compat(&mouse_state, mouse_handler);
    ok &= check_int("handler configured", mouse_state.mouse_handler_configured,
                    1);
    ok &= check_int("handler retained", mouse_state.mouse_handler(3, 4, 5),
                    12);

    redmcsb_f8108_register_mouse_pointer_bitmap_pc34_compat(
        &mouse_state, 2, bitmap, -3, 7, record_suspend, record_history,
        &trace);
    trace.events[trace.count] = '\0';
    ok &= check_int("bitmap event sequence", strcmp(trace.events, "SH"), 0);
    ok &= check_int("bitmap first byte", mouse_state.bitmaps[2][0], 1);
    ok &= check_int("bitmap last byte", mouse_state.bitmaps[2][71], 72);
    ok &= check_int("hotspot x", mouse_state.hotspots[2][0], -3);
    ok &= check_int("hotspot y", mouse_state.hotspots[2][1], 7);

    trace.count = 0U;
    mouse_state.bitmaps[1][0] = 99U;
    redmcsb_f8108_register_mouse_pointer_bitmap_pc34_compat(
        &mouse_state, 4, bitmap, 1, 2, record_suspend, record_history,
        &trace);
    ok &= check_int("invalid bitmap no events", (int)trace.count, 0);
    ok &= check_int("invalid bitmap no write", mouse_state.bitmaps[1][0], 99);

    trace.count = 0U;
    mouse_state.mouse_pointer_drawn = 1;
    redmcsb_f8109_set_mouse_pointer_pc34_compat(
        &mouse_state, 3, record_suspend, record_erase, record_movement,
        record_history, &trace);
    trace.events[trace.count] = '\0';
    ok &= check_int("drawn pointer event sequence", strcmp(trace.events,
                    "SEMH"), 0);
    ok &= check_int("drawn pointer type", mouse_state.mouse_pointer_type, 3);

    trace.count = 0U;
    mouse_state.mouse_pointer_drawn = 0;
    redmcsb_f8109_set_mouse_pointer_pc34_compat(
        &mouse_state, -1, record_suspend, record_erase, record_movement,
        record_history, &trace);
    trace.events[trace.count] = '\0';
    ok &= check_int("hidden pointer event sequence", strcmp(trace.events,
                    "SH"), 0);
    ok &= check_int("hidden pointer type", mouse_state.mouse_pointer_type,
                    -1);
    ok &= check_int("source anchors",
                    strstr(redmcsb_f8101_mouse_pointer_source_evidence_pc34(),
                           "IBMIO.C:881-890,1131-1168") != 0, 1);

    return ok ? 0 : 1;
}
