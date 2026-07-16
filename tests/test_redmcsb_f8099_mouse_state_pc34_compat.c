#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f8099_mouse_state_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct redmcsb_f8099_capture {
    unsigned int cursor_calls;
    unsigned int erase_calls;
    unsigned int draw_calls;
    int16_t horizontal[2];
    int16_t vertical[2];
    char events[5];
    unsigned int event_count;
} redmcsb_f8099_capture;

static void capture_cursor_position(void *context,
                                    int16_t horizontal,
                                    int16_t vertical)
{
    redmcsb_f8099_capture *capture = context;

    capture->horizontal[capture->cursor_calls] = horizontal;
    capture->vertical[capture->cursor_calls] = vertical;
    capture->cursor_calls++;
    capture->events[capture->event_count++] = 'C';
}

static void capture_erase_pointer(void *context)
{
    redmcsb_f8099_capture *capture = context;

    capture->erase_calls++;
    capture->events[capture->event_count++] = 'E';
}

static void capture_draw_pointer(void *context)
{
    redmcsb_f8099_capture *capture = context;

    capture->draw_calls++;
    capture->events[capture->event_count++] = 'D';
}

int main(void)
{
    int16_t x;
    int16_t y;
    int16_t buttons;
    const char *evidence;
    redmcsb_f8099_capture capture = { 0 };
    redmcsb_f8099_mouse_state_pc34_compat state = {
        .formatted_button_status = 3,
        .lock_count = 7,
        .mouse_installed = 1,
        .mouse_pointer_drawn = 1
    };

    redmcsb_f8099_lock_mouse_pc34_compat(&state);
    assert(state.lock_count == 8);
    redmcsb_f8100_unlock_mouse_pc34_compat(&state, capture_cursor_position,
                                            &capture);
    assert(state.lock_count == 7);
    assert(capture.cursor_calls == 1U);
    assert(capture.horizontal[0] == 0);
    assert(capture.vertical[0] == 0);

    memset(&capture, 0, sizeof(capture));
    redmcsb_f8111_set_mouse_pointer_coordinates_pc34_compat(
        &state, 123, 45, capture_cursor_position, capture_erase_pointer,
        capture_draw_pointer, &capture);
    assert(state.lock_count == 7);
    assert(state.current_x == 123);
    assert(state.current_y == 45);
    assert(state.assigned_x == 123);
    assert(state.assigned_y == 45);
    assert(capture.cursor_calls == 2U);
    assert(capture.horizontal[0] == 246);
    assert(capture.vertical[0] == 45);
    assert(capture.horizontal[1] == 246);
    assert(capture.vertical[1] == 45);
    assert(capture.erase_calls == 1U);
    assert(capture.draw_calls == 1U);
    assert(memcmp(capture.events, "CEDC", 4) == 0);

    redmcsb_f8112_get_mouse_state_pc34_compat(&state, &x, &y, &buttons);
    assert(x == 123);
    assert(y == 45);
    assert(buttons == 3);

    evidence = redmcsb_f8099_mouse_state_source_evidence_pc34();
    assert(strstr(evidence, "MEDIA701_I34E") != NULL);
    assert(strstr(evidence, "IBMIO.C:851-859") != NULL);
    assert(strstr(evidence, "IBMIO.C:862-878") != NULL);
    assert(strstr(evidence, "IBMIO.C:1190-1213") != NULL);
    assert(strstr(evidence, "IBMIO.C:1215-1225") != NULL);
    puts("ok: ReDMCSB F8099/F8100/F8111/F8112 mouse state");
    return 0;
}
