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
} SeekCapture;

static int32_t capture_seek(void *context, int32_t handle, int32_t offset,
                            int origin)
{
    SeekCapture *capture = context;
    capture->calls++;
    capture->handle = handle;
    capture->offset = offset;
    capture->origin = origin;
    return capture->result;
}

int main(void)
{
    SeekCapture capture = { 0, 0, 0, -1, 1234 };

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

    capture.result = -1;
    capture.result = INT32_MIN;
    assert(redmcsb_f0919_prim_file_seek_pc34_compat(
               INT32_MIN, INT32_MIN, capture_seek, &capture) == 0);
    assert(capture.calls == 3);
    assert(capture.offset == INT32_MIN);
    assert(strstr(redmcsb_f0919_prim_file_seek_source_evidence_pc34(),
                  "PRIM2C.C:106-123") != NULL);
    return 0;
}
