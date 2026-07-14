#include "redmcsb_f0775_file_get_size_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned int tell_calls;
    unsigned int end_calls;
    unsigned int restore_calls;
    int16_t handle;
    int32_t mark;
    uint32_t eof;
    int16_t restored_handle;
    int32_t restored_mark;
} file_state;

static int32_t capture_tell(void *context, int16_t file_handle)
{
    file_state *state = context;

    state->tell_calls++;
    state->handle = file_handle;
    return state->mark;
}

static uint32_t capture_seek_to_end(void *context, int16_t file_handle)
{
    file_state *state = context;

    state->end_calls++;
    state->handle = file_handle;
    return state->eof;
}

static void capture_restore(void *context, int16_t file_handle, int32_t offset)
{
    file_state *state = context;

    state->restore_calls++;
    state->restored_handle = file_handle;
    state->restored_mark = offset;
}

static int require(int condition, const char *message)
{
    if (condition) {
        return 0;
    }
    fprintf(stderr, "requirement failed: %s\n", message);
    return 1;
}

int main(void)
{
    file_state state = {0U, 0U, 0U, 0, INT32_C(-17), UINT32_C(0x81234567),
                        0, 0};
    uint32_t result = redmcsb_f0775_file_get_size_pc34_compat(
        capture_tell, capture_seek_to_end, capture_restore, &state,
        INT16_C(-3));

    if (require(result == UINT32_C(0x81234567), "returns raw DX:AX EOF") ||
        require(state.tell_calls == 1U, "takes one original mark") ||
        require(state.end_calls == 1U, "seeks to EOF once") ||
        require(state.restore_calls == 1U, "restores mark once") ||
        require(state.handle == INT16_C(-3), "preserves signed handle") ||
        require(state.restored_handle == INT16_C(-3),
                "restores same signed handle") ||
        require(state.restored_mark == INT32_C(-17),
                "restores exact signed mark") ||
        require(strstr(redmcsb_f0775_file_get_size_source_evidence_pc34(),
                       "FILE.C:647-680") != NULL,
                "records source evidence")) {
        return 1;
    }

    puts("ok: ReDMCSB F0775 PC 3.4 file get size");
    return 0;
}
