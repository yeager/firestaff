#include "redmcsb_f0774_file_seek_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned int call_count;
    int16_t file_handle;
    int32_t offset;
    bool succeeds;
} test_dos_seek;

static bool capture_seek_from_beginning(void *context,
                                        int16_t file_handle,
                                        int32_t offset)
{
    test_dos_seek *state = context;

    state->call_count++;
    state->file_handle = file_handle;
    state->offset = offset;
    return state->succeeds;
}

static int require(bool condition, const char *message)
{
    if (condition) {
        return 0;
    }
    fprintf(stderr, "requirement failed: %s\n", message);
    return 1;
}

int main(void)
{
    test_dos_seek state = {0U, 0, 0, true};

    if (require(redmcsb_f0774_file_seek_pc34_compat(
                    capture_seek_from_beginning, &state, INT16_C(-23),
                    INT32_C(0x12345678)),
                "carry-clear seek succeeds") ||
        require(state.call_count == 1U, "issues one DOS seek") ||
        require(state.file_handle == INT16_C(-23), "preserves signed handle") ||
        require(state.offset == INT32_C(0x12345678),
                "preserves 32-bit offset")) {
        return 1;
    }

    state.call_count = 0U;
    state.succeeds = false;
    if (require(!redmcsb_f0774_file_seek_pc34_compat(
                     capture_seek_from_beginning, &state, INT16_C(7),
                     INT32_C(-1)),
                "carry-set seek fails") ||
        require(state.call_count == 1U, "failed seek still dispatches once") ||
        require(state.offset == INT32_C(-1), "negative offset is passed") ||
        require(strstr(redmcsb_f0774_file_seek_source_evidence_pc34(),
                       "FILE.C:606-621") != NULL,
                "records source evidence")) {
        return 1;
    }

    puts("ok: ReDMCSB F0774 PC 3.4 file seek");
    return 0;
}
