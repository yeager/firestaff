#include "redmcsb_f0665_highlight_box_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct capture {
    int zone_found;
    int zone_reads;
    int enable_calls;
    int invert_calls;
    int disable_calls;
    int vblank_calls;
    int16_t requested_zone;
    int16_t inverted_zone[4];
} capture;

static int get_zone(void *context, int16_t zone_index, int16_t xyz[4])
{
    capture *state = (capture *)context;

    state->zone_reads += 1;
    state->requested_zone = zone_index;
    if (!state->zone_found) return 0;
    xyz[0] = 12;
    xyz[1] = 13;
    xyz[2] = 40;
    xyz[3] = 50;
    return 1;
}

static void enable_update(void *context)
{
    ((capture *)context)->enable_calls += 1;
}

static void invert_box(void *context, const int16_t xyz[4])
{
    capture *state = (capture *)context;

    state->invert_calls += 1;
    memcpy(state->inverted_zone, xyz, sizeof(state->inverted_zone));
}

static void disable_update(void *context)
{
    ((capture *)context)->disable_calls += 1;
}

static void wait_vblank(void *context)
{
    ((capture *)context)->vblank_calls += 1;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    capture capture_state;
    redmcsb_f0665_highlight_state_pc34_compat state;
    redmcsb_f0665_highlight_runtime_pc34_compat runtime;
    int ok = 1;

    memset(&capture_state, 0, sizeof(capture_state));
    memset(&state, 0, sizeof(state));
    runtime.get_zone = get_zone;
    runtime.enable_screen_update = enable_update;
    runtime.invert_box = invert_box;
    runtime.disable_screen_update = disable_update;
    runtime.wait_vertical_blank = wait_vblank;
    runtime.context = &capture_state;

    ok &= expect(redmcsb_f0665_highlight_box_enable_for_zone_pc34_compat(
                     &state, &runtime, 68) == 0,
                 "F0362 does not call F0665 for unresolved real zone");
    ok &= expect(capture_state.zone_reads == 1 && capture_state.enable_calls == 0 &&
                     capture_state.invert_calls == 0 && !state.highlight_box_enabled,
                 "unresolved zone leaves video and state untouched");

    capture_state.zone_found = 1;
    ok &= expect(redmcsb_f0665_highlight_box_enable_for_zone_pc34_compat(
                     &state, &runtime, 68) == 1,
                 "F0665 dispatches source highlight route");
    ok &= expect(capture_state.requested_zone == 68 && capture_state.enable_calls == 1 &&
                     capture_state.invert_calls == 1 && capture_state.disable_calls == 1 &&
                     capture_state.vblank_calls == 1 && state.highlight_box_enabled,
                 "F0665 enables, inverts, disables, and waits exactly once");
    ok &= expect(state.highlighted_zone[0] == 12 && state.highlighted_zone[3] == 50 &&
                     capture_state.inverted_zone[1] == 13 &&
                     capture_state.inverted_zone[2] == 40,
                 "F0665 copies and inverts the F0638 zone unchanged");
    ok &= expect(strstr(redmcsb_f0665_highlight_box_source_evidence_pc34(),
                        "CLIKMENU.C") != NULL,
                 "source evidence is available");
    return ok ? 0 : 1;
}
