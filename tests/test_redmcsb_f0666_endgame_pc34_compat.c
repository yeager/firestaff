#include "redmcsb_f0666_endgame_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct capture {
    redmcsb_f0666_endgame_state_pc34_compat *state;
    int hide_calls;
    int close_calls;
    int restore_calls;
    int transfer_calls;
    int order[6];
    int order_count;
} capture;

static void hide_pointer(void *context)
{
    capture *state = (capture *)context;

    state->hide_calls += 1;
    state->order[state->order_count++] = 1;
    state->state->hide_mouse_pointer_request_count += 1;
}

static void close_graphics(void *context)
{
    capture *state = (capture *)context;

    state->close_calls += 1;
    state->order[state->order_count++] = 2;
}

static void restore_cpsx(void *context)
{
    capture *state = (capture *)context;

    state->restore_calls += 1;
    state->order[state->order_count++] = 3;
}

static void transfer_boundary(void *context)
{
    capture *state = (capture *)context;

    state->transfer_calls += 1;
    state->order[state->order_count++] = 4;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    redmcsb_f0666_endgame_state_pc34_compat state;
    redmcsb_f0666_endgame_runtime_pc34_compat runtime;
    capture capture_state;
    int ok = 1;

    memset(&state, 0, sizeof(state));
    memset(&runtime, 0, sizeof(runtime));
    memset(&capture_state, 0, sizeof(capture_state));
    capture_state.state = &state;
    runtime.hide_pointer = hide_pointer;
    runtime.close_graphics_dat = close_graphics;
    runtime.restore_cpsx = restore_cpsx;
    runtime.transfer_to_endgame_boundary = transfer_boundary;
    runtime.context = &capture_state;

    state.hide_mouse_pointer_request_count = -1;
    ok &= expect(redmcsb_f0666_endgame_pc34_compat(&state, &runtime) == 1,
                 "F0666 completes caller-owned PC transaction");
    ok &= expect(capture_state.hide_calls == 2 && state.hide_mouse_pointer_request_count == 1,
                 "F0666 hides pointer until source counter is positive");
    ok &= expect(capture_state.close_calls == 2 && capture_state.restore_calls == 1 &&
                     capture_state.transfer_calls == 1,
                 "F0666 closes graphics twice then restores and transfers");
    ok &= expect(capture_state.order[0] == 1 && capture_state.order[1] == 1 &&
                     capture_state.order[2] == 2 && capture_state.order[3] == 2 &&
                     capture_state.order[4] == 3 && capture_state.order[5] == 4,
                 "F0666 preserves source call ordering through transfer");
    ok &= expect(redmcsb_f0666_endgame_pc34_compat(NULL, &runtime) == 0,
                 "F0666 rejects missing caller-owned state");
    ok &= expect(strstr(redmcsb_f0666_endgame_source_evidence_pc34(),
                        "ENDGAME.C") != NULL,
                 "source evidence is available");
    return ok ? 0 : 1;
}
