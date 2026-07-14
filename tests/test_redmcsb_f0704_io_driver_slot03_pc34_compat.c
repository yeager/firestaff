#include "redmcsb_f0704_io_driver_slot03_pc34_compat.h"

#include <assert.h>
#include <stddef.h>

typedef struct {
    int call_count;
} redmcsb_f0704_capture_pc34_compat;

static void capture_slot03(void *context)
{
    redmcsb_f0704_capture_pc34_compat *capture = context;

    capture->call_count++;
}

int main(void)
{
    redmcsb_f0704_capture_pc34_compat capture = { 0 };
    redmcsb_f0704_io_driver_pc34_compat driver = { capture_slot03, &capture };

    assert(redmcsb_f0704_call_io_driver_slot03_pc34_compat(&driver));
    assert(capture.call_count == 1);

    driver.slot03 = NULL;
    assert(!redmcsb_f0704_call_io_driver_slot03_pc34_compat(&driver));
    assert(capture.call_count == 1);
    assert(!redmcsb_f0704_call_io_driver_slot03_pc34_compat(NULL));
    return 0;
}
