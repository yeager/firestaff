#include "redmcsb_f0710_wait_sound_play_complete_pc34_compat.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
    int call_count;
    void *seen_context;
} redmcsb_f0710_capture_pc34_compat;

static void capture_wait_sound_play_complete(void *context)
{
    redmcsb_f0710_capture_pc34_compat *capture = context;

    capture->call_count++;
    capture->seen_context = context;
}

int main(void)
{
    redmcsb_f0710_capture_pc34_compat capture = { 0 };
    redmcsb_f0710_io_driver_pc34_compat driver = {
        capture_wait_sound_play_complete,
        &capture
    };

    assert(redmcsb_f0710_wait_sound_play_complete_pc34_compat(&driver));
    assert(capture.call_count == 1);
    assert(capture.seen_context == &capture);

    driver.wait_sound_play_complete = NULL;
    assert(!redmcsb_f0710_wait_sound_play_complete_pc34_compat(&driver));
    assert(capture.call_count == 1);
    assert(!redmcsb_f0710_wait_sound_play_complete_pc34_compat(NULL));
    return 0;
}
