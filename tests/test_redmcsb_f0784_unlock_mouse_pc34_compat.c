#include "redmcsb_f0784_unlock_mouse_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct {
    unsigned int calls;
    void *received_context;
} TestState;

static void unlock_mouse(void *context)
{
    TestState *state = (TestState *)context;

    state->calls++;
    state->received_context = context;
}

int main(void)
{
    TestState state = {0U, NULL};
    redmcsb_f0784_io_driver_pc34_compat io_driver = {
        unlock_mouse,
        &state
    };

    redmcsb_f0784_unlock_mouse_pc34_compat(&io_driver);

    if (state.calls != 1U || state.received_context != &state) {
        return 1;
    }
    if (strstr(redmcsb_f0784_unlock_mouse_source_evidence_pc34(),
               "IODRV_06_UnlockMouse") == NULL) {
        return 1;
    }

    puts("redmcsb_f0784_unlock_mouse_pc34_compat: PASS");
    return 0;
}
