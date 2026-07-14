#include "redmcsb_f0705_invoke_io_driver_04_pc34_compat.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
    int call_count;
    void *seen_context;
} redmcsb_f0705_capture_pc34_compat;

static void capture_io_driver_04(void *context)
{
    redmcsb_f0705_capture_pc34_compat *capture = context;

    capture->call_count++;
    capture->seen_context = context;
}

int main(void)
{
    redmcsb_f0705_capture_pc34_compat capture = { 0 };
    redmcsb_f0705_io_driver_pc34_compat driver = {
        capture_io_driver_04,
        &capture
    };

    assert(redmcsb_f0705_invoke_io_driver_04_pc34_compat(&driver));
    assert(capture.call_count == 1);
    assert(capture.seen_context == &capture);

    driver.io_driver_04 = NULL;
    assert(!redmcsb_f0705_invoke_io_driver_04_pc34_compat(&driver));
    assert(!redmcsb_f0705_invoke_io_driver_04_pc34_compat(NULL));
    return 0;
}
