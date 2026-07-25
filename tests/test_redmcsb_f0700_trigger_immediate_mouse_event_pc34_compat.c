#include "redmcsb_f0700_trigger_immediate_mouse_event_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    int call_count;
    int16_t box[4];
    int16_t event;
} redmcsb_f0700_capture_pc34_compat;

static void capture_set_mouse_bound_event(void *context, const int16_t box[4],
                                          int16_t mouse_event)
{
    redmcsb_f0700_capture_pc34_compat *capture = context;

    capture->call_count++;
    memcpy(capture->box, box, sizeof(capture->box));
    capture->event = mouse_event;
}

int main(void)
{
    const int16_t expected_box[4] = { 1, 0, 1, 0 };
    (void)expected_box;
    redmcsb_f0700_capture_pc34_compat capture = { 0 };
    redmcsb_f0700_io_driver_pc34_compat driver = {
        capture_set_mouse_bound_event,
        &capture
    };

    assert(redmcsb_f0700_trigger_immediate_mouse_event_pc34_compat(&driver));
    assert(capture.call_count == 1);
    assert(memcmp(capture.box, expected_box, sizeof(expected_box)) == 0);
    assert(capture.event ==
           REDMCSB_F0700_MOUSE_EVENT_CHANGE_SCREEN_REGION_PC34_COMPAT);

    driver.set_mouse_bound_event = NULL;
    assert(!redmcsb_f0700_trigger_immediate_mouse_event_pc34_compat(&driver));
    assert(!redmcsb_f0700_trigger_immediate_mouse_event_pc34_compat(NULL));
    return 0;
}
