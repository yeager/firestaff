#include "redmcsb_f0714_get_read_first_sector_result_pc34_compat.h"

#include <assert.h>
#include <string.h>

typedef struct {
    int16_t result;
    int16_t seen_drive;
    unsigned int calls;
} redmcsb_f0714_capture_pc34_compat;

static int16_t read_first_sector(void *context, int16_t drive_ordinal)
{
    redmcsb_f0714_capture_pc34_compat *capture = context;
    capture->calls++;
    capture->seen_drive = drive_ordinal;
    return capture->result;
}

int main(void)
{
    redmcsb_f0714_capture_pc34_compat capture = { -17, 0, 0 };
    redmcsb_f0714_io_driver_pc34_compat driver = { read_first_sector, &capture };
    (void)driver;
    int16_t result = 0;
    (void)result;

    assert(redmcsb_f0714_get_read_first_sector_result_pc34_compat(
        &driver, 2, &result));
    assert(capture.calls == 1U);
    assert(capture.seen_drive == 2);
    assert(result == -17);
    assert(!redmcsb_f0714_get_read_first_sector_result_pc34_compat(
        &driver, 2, NULL));
    assert(!redmcsb_f0714_get_read_first_sector_result_pc34_compat(
        NULL, 2, &result));
    assert(strstr(redmcsb_f0714_get_read_first_sector_result_source_evidence_pc34(),
                  "IO.C:3907-3920") != NULL);
    return 0;
}
