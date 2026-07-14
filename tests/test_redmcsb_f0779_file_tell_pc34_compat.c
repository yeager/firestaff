#include "redmcsb_f0779_file_tell_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned int calls;
    int16_t file_handle;
    int32_t mark;
} file_state;

static int32_t capture_tell(void *context, int16_t file_handle)
{
    file_state *state = context;

    state->calls++;
    state->file_handle = file_handle;
    return state->mark;
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
    file_state state = {0U, 0, INT32_C(0x12345678)};
    int32_t result = redmcsb_f0779_file_tell_pc34_compat(
        capture_tell, &state, INT16_C(-7));

    if (require(result == INT32_C(0x12345678),
                "returns the complete current mark") ||
        require(state.calls == 1U, "issues one DOS tell bridge call") ||
        require(state.file_handle == INT16_C(-7),
                "preserves the signed file handle") ||
        require(strstr(redmcsb_f0779_file_tell_source_evidence_pc34(),
                       "FILE.C:624-646") != NULL,
                "records PC 3.4 source evidence")) {
        return 1;
    }

    state.mark = INT32_C(-1);
    result = redmcsb_f0779_file_tell_pc34_compat(capture_tell, &state,
                                                  INT16_C(3));
    if (require(result == INT32_C(-1), "preserves DOS error mark") ||
        require(state.calls == 2U, "does not cache a prior mark") ||
        require(state.file_handle == INT16_C(3),
                "forwards each handle independently")) {
        return 1;
    }

    puts("ok: ReDMCSB F0779 PC 3.4 file tell");
    return 0;
}
