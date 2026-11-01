#include "redmcsb_f0919_prim_file_seek_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    int calls;
    int32_t handle;
    int32_t offset;
    int origin;
    int32_t result;
    int error_calls;
    int error_code;
    int events[16];
    int event_count;
} SeekCapture;

static int32_t capture_seek(void *context, int32_t handle, int32_t offset,
                            int origin)
{
    SeekCapture *capture = context;
    capture->calls++;
    capture->handle = handle;
    capture->offset = offset;
    capture->origin = origin;
    capture->events[capture->event_count++] = 2;
    return capture->result;
}

static void capture_error(void *context, int error_code)
{
    SeekCapture *capture = context;
    capture->error_calls++;
    capture->error_code = error_code;
}

static void begin_library(void *context)
{
    SeekCapture *capture = context;
    capture->events[capture->event_count++] = 1;
}

static int32_t capture_amiga_seek(void *context, int32_t handle,
                                  int32_t offset, int origin)
{
    return capture_seek(context, handle, offset, origin);
}

static void end_library(void *context)
{
    SeekCapture *capture = context;
    capture->events[capture->event_count++] = 3;
}

int main(void)
{
    SeekCapture capture = { 0 };
    capture.result = 1234;

    assert(redmcsb_f0919_prim_file_seek_pc34_compat(
               7, 1234, capture_seek, &capture) == 0);
    assert(capture.calls == 1);
    assert(capture.handle == 7);
    assert(capture.offset == 1234);
    assert(capture.origin == 0);

    capture.result = 1233;
    assert(redmcsb_f0919_prim_file_seek_pc34_compat(
               -1, 1234, capture_seek, &capture) == 1);
    assert(capture.calls == 2);
    assert(capture.handle == -1);

    capture.result = INT32_MIN;
    assert(redmcsb_f0919_prim_file_seek_pc34_compat(
               INT32_MIN, INT32_MIN, capture_seek, &capture) == 0);
    assert(capture.calls == 3);
    assert(capture.offset == INT32_MIN);
    capture.result = 500;
    redmcsb_f0919_prim_file_seek_fmtowns_pc34_compat(
        11, 600, capture_seek, capture_error, &capture);
    assert(capture.calls == 4);
    assert(capture.handle == 11);
    assert(capture.origin == 0);
    assert(capture.error_calls == 1);
    assert(capture.error_code == 3);

    capture.result = 600;
    redmcsb_f0919_prim_file_seek_fmtowns_pc34_compat(
        11, 600, capture_seek, capture_error, &capture);
    assert(capture.error_calls == 1);

    capture.event_count = 0;
    capture.result = -1;
    redmcsb_f0919_prim_file_seek_amiga_pc34_compat(
        12, 700, begin_library, capture_amiga_seek, end_library, &capture);
    assert(capture.event_count == 3);
    assert(capture.events[0] == 1);
    assert(capture.events[1] == 2);
    assert(capture.events[2] == 3);
    assert(capture.handle == 12);
    assert(capture.offset == 700);
    assert(capture.origin == 0);
    assert(strstr(redmcsb_f0919_prim_file_seek_source_evidence_pc34(),
                  "PRIM1.C:40-53") != NULL);
    return 0;
}
