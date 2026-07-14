#include "redmcsb_f0717_get_volume_name_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    unsigned int calls;
    int16_t seen_drive_ordinal;
    char *seen_volume_name;
} redmcsb_f0717_capture_pc34_compat;

static void capture_get_volume_name(void *context,
                                    int16_t drive_ordinal,
                                    char *volume_name)
{
    redmcsb_f0717_capture_pc34_compat *capture = context;

    capture->calls++;
    capture->seen_drive_ordinal = drive_ordinal;
    capture->seen_volume_name = volume_name;
    memcpy(volume_name, "CSB", sizeof("CSB"));
}

int main(void)
{
    redmcsb_f0717_capture_pc34_compat capture = { 0U, 0, NULL };
    redmcsb_f0717_io_driver_pc34_compat driver = {
        capture_get_volume_name,
        &capture
    };
    char volume_name[8] = { 0 };

    redmcsb_f0717_get_volume_name_pc34_compat(
        &driver, INT16_C(-1), volume_name);
    assert(capture.calls == 1U);
    assert(capture.seen_drive_ordinal == INT16_C(-1));
    assert(capture.seen_volume_name == volume_name);
    assert(strcmp(volume_name, "CSB") == 0);

    driver.get_volume_name = NULL;
    redmcsb_f0717_get_volume_name_pc34_compat(&driver, 2, volume_name);
    assert(capture.calls == 1U);
    redmcsb_f0717_get_volume_name_pc34_compat(NULL, 2, volume_name);
    assert(capture.calls == 1U);
    assert(strstr(redmcsb_f0717_get_volume_name_source_evidence_pc34(),
                  "IODRV_19_GetVolumeName") != NULL);
    return 0;
}
