#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0715_get_converted_device_type_pc34_compat.h"

typedef struct {
    int call_count;
    void *seen_context;
    int16_t seen_drive_ordinal;
    int16_t result;
} redmcsb_f0715_capture_pc34_compat;

static int16_t capture_get_converted_device_type(
    void *context,
    int16_t drive_ordinal)
{
    redmcsb_f0715_capture_pc34_compat *capture = context;

    capture->call_count++;
    capture->seen_context = context;
    capture->seen_drive_ordinal = drive_ordinal;
    return capture->result;
}

int main(void)
{
    redmcsb_f0715_capture_pc34_compat capture = { 0, NULL, 0, INT16_C(1) };
    redmcsb_f0715_io_driver_pc34_compat driver = {
        capture_get_converted_device_type,
        &capture
    };
    (void)driver;

    assert(redmcsb_f0715_get_converted_device_type_pc34_compat(
               &driver, INT16_C(2)) == INT16_C(1));
    assert(capture.call_count == 1);
    assert(capture.seen_context == &capture);
    assert(capture.seen_drive_ordinal == INT16_C(2));

    capture.result = INT16_MIN;
    assert(redmcsb_f0715_get_converted_device_type_pc34_compat(
               &driver, INT16_MIN) == INT16_MIN);
    assert(capture.call_count == 2);
    assert(capture.seen_drive_ordinal == INT16_MIN);

    assert(strstr(redmcsb_f0715_get_converted_device_type_source_evidence_pc34(),
                  "IO.C:3922-3926") != NULL);

    puts("ok: ReDMCSB F0715 PC 3.4 converted-device-type dispatch");
    return 0;
}
