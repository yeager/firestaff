#include "redmcsb_f0920_prim_file_read_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct {
    unsigned int call_count;
    void *seen_context;
    int32_t seen_file_handle;
    void *seen_buffer;
    int32_t seen_length;
    int32_t result;
} redmcsb_f0920_capture_pc34_compat;

static __attribute__((unused)) int32_t capture_read(
    void *context,
    int32_t file_handle,
    void *buffer,
    int32_t length)
{
    redmcsb_f0920_capture_pc34_compat *capture = context;

    capture->call_count++;
    capture->seen_context = context;
    capture->seen_file_handle = file_handle;
    capture->seen_buffer = buffer;
    capture->seen_length = length;
    return capture->result;
}

int main(void)
{
    unsigned char buffer[4] = { 0U, 0U, 0U, 0U };
    (void)buffer;
    redmcsb_f0920_capture_pc34_compat capture = {
        0U, NULL, 0, NULL, 0, INT32_C(4)
    };

    assert(redmcsb_f0920_prim_file_read_pc34_compat(
               INT32_C(7), INT32_C(4), buffer, capture_read, &capture) == 0);
    assert(capture.call_count == 1U);
    assert(capture.seen_context == &capture);
    assert(capture.seen_file_handle == INT32_C(7));
    assert(capture.seen_buffer == buffer);
    assert(capture.seen_length == INT32_C(4));

    capture.result = INT32_C(3);
    assert(redmcsb_f0920_prim_file_read_pc34_compat(
               INT32_C(8), INT32_C(4), buffer, capture_read, &capture) == 1);

    capture.result = -INT32_C(1);
    assert(redmcsb_f0920_prim_file_read_pc34_compat(
               INT32_C(9), INT32_C(4), buffer, capture_read, &capture) == 1);

    capture.result = -INT32_C(2);
    assert(redmcsb_f0920_prim_file_read_pc34_compat(
               INT32_MIN, -INT32_C(2), buffer, capture_read, &capture) == 0);
    assert(capture.call_count == 4U);
    assert(capture.seen_file_handle == INT32_MIN);
    assert(capture.seen_length == -INT32_C(2));

    assert(strstr(redmcsb_f0920_prim_file_read_source_evidence_pc34(),
                  "PRIM2C.C:123-142") != NULL);
    return 0;
}
