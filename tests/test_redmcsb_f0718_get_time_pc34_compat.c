#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0718_get_time_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    int call_count;
    void *seen_context;
    int16_t result;
} redmcsb_f0718_capture_pc34_compat;

static int16_t capture_get_time(void *context)
{
    redmcsb_f0718_capture_pc34_compat *capture = context;

    capture->call_count++;
    capture->seen_context = context;
    return capture->result;
}

int main(void)
{
    redmcsb_f0718_capture_pc34_compat capture = { 0, NULL, INT16_MAX };
    redmcsb_f0718_io_driver_pc34_compat driver = {
        capture_get_time,
        &capture
    };
    (void)driver;

    assert(redmcsb_f0718_get_time_pc34_compat(&driver) == INT16_MAX);
    assert(capture.call_count == 1);
    assert(capture.seen_context == &capture);

    capture.result = INT16_MIN;
    assert(redmcsb_f0718_get_time_pc34_compat(&driver) == INT16_MIN);
    assert(capture.call_count == 2);

    assert(strstr(redmcsb_f0718_get_time_source_evidence_pc34(),
                  "IO.C:3941-3945") != NULL);

    puts("ok: ReDMCSB F0718 PC 3.4 get-time dispatch");
    return 0;
}
