#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "redmcsb_f0746_is_ems_present_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    int open_call_count;
    int device_info_call_count;
    int output_status_call_count;
    int close_call_count;
    const char *seen_filename;
    int16_t seen_handle;
    bool open_succeeds;
    bool device_info_succeeds;
    bool output_status_succeeds;
    bool close_succeeds;
    int16_t opened_handle;
    uint16_t device_information;
    uint8_t output_status;
} redmcsb_f0746_capture_pc34_compat;

static bool capture_open_read_only(
    void *context,
    const char *filename,
    int16_t *file_handle)
{
    redmcsb_f0746_capture_pc34_compat *capture = context;

    capture->open_call_count++;
    capture->seen_filename = filename;
    *file_handle = capture->opened_handle;
    return capture->open_succeeds;
}

static bool capture_get_device_info(
    void *context,
    int16_t file_handle,
    uint16_t *device_information)
{
    redmcsb_f0746_capture_pc34_compat *capture = context;

    capture->device_info_call_count++;
    capture->seen_handle = file_handle;
    *device_information = capture->device_information;
    return capture->device_info_succeeds;
}

static bool capture_get_output_status(
    void *context,
    int16_t file_handle,
    uint8_t *output_status)
{
    redmcsb_f0746_capture_pc34_compat *capture = context;

    capture->output_status_call_count++;
    capture->seen_handle = file_handle;
    *output_status = capture->output_status;
    return capture->output_status_succeeds;
}

static bool capture_close(void *context, int16_t file_handle)
{
    redmcsb_f0746_capture_pc34_compat *capture = context;

    capture->close_call_count++;
    capture->seen_handle = file_handle;
    return capture->close_succeeds;
}

int main(void)
{
    redmcsb_f0746_capture_pc34_compat capture = {
        0, 0, 0, 0, NULL, 0,
        true, true, true, true,
        INT16_C(-19), UINT16_C(0x0080), UINT8_C(0xff)
    };
    redmcsb_f0746_dos_pc34_compat dos = {
        capture_open_read_only,
        capture_get_device_info,
        capture_get_output_status,
        capture_close,
        &capture
    };
    (void)dos;

    assert(redmcsb_f0746_is_ems_present_pc34_compat(
               &dos, INT16_C(1), INT16_C(-123)) == 0);
    assert(capture.open_call_count == 0);

    capture.open_succeeds = false;
    assert(redmcsb_f0746_is_ems_present_pc34_compat(&dos, 0, 44) == 0);
    assert(capture.open_call_count == 1);
    assert(capture.close_call_count == 0);
    assert(strcmp(capture.seen_filename, "EMMXXXX0") == 0);

    capture.open_succeeds = true;
    capture.device_info_succeeds = false;
    assert(redmcsb_f0746_is_ems_present_pc34_compat(&dos, 0, -77) == -77);
    assert(capture.device_info_call_count == 1);
    assert(capture.output_status_call_count == 0);
    assert(capture.close_call_count == 1);
    assert(capture.seen_handle == INT16_C(-19));

    capture.device_info_succeeds = true;
    capture.device_information = 0;
    assert(redmcsb_f0746_is_ems_present_pc34_compat(&dos, 0, 91) == 91);
    assert(capture.device_info_call_count == 2);
    assert(capture.output_status_call_count == 0);
    assert(capture.close_call_count == 2);

    capture.device_information = UINT16_C(0x0080);
    capture.output_status_succeeds = true;
    capture.output_status = 0;
    assert(redmcsb_f0746_is_ems_present_pc34_compat(&dos, 0, -42) == -42);
    assert(capture.output_status_call_count == 1);
    assert(capture.close_call_count == 3);

    capture.output_status_succeeds = false;
    capture.output_status = UINT8_C(1);
    assert(redmcsb_f0746_is_ems_present_pc34_compat(&dos, 0, 66) == 66);
    assert(capture.output_status_call_count == 2);
    assert(capture.close_call_count == 4);

    capture.output_status_succeeds = true;
    capture.output_status = UINT8_C(1);
    assert(redmcsb_f0746_is_ems_present_pc34_compat(&dos, 0, -42) == 1);
    assert(capture.output_status_call_count == 3);
    assert(capture.close_call_count == 5);

    capture.close_succeeds = false;
    assert(redmcsb_f0746_is_ems_present_pc34_compat(&dos, 0, -42) == 0);
    assert(capture.close_call_count == 6);
    assert(strstr(redmcsb_f0746_is_ems_present_source_evidence_pc34(),
                  "STARTUP2.C:87-145") != NULL);

    puts("ok: ReDMCSB F0746 PC 3.4 EMS presence detection");
    return 0;
}
