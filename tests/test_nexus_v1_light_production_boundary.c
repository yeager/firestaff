#include "nexus_v1_light.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_LightState state;

    /* The production adapter deliberately preserves caller-owned state.
     * Start from a known value: reading an uninitialized stack object happened
     * to pass on some platforms, but is not a test of the fail-closed API. */
    memset(&state, 0, sizeof(state));
    nexus_v1_light_init(&state);
    nexus_v1_light_torch_on(&state, 100);
    if (state.torch_ticks != 0 || state.torch_active != 0) {
        fprintf(stderr, "FAIL: uncaptured torch write escaped production gate\n");
        return 1;
    }
    if (nexus_v1_light_get(&state) != 0) {
        fprintf(stderr, "FAIL: uncaptured light value escaped production gate\n");
        return 1;
    }
    nexus_v1_light_tick(&state);
    if (state.torch_ticks != 0) {
        fprintf(stderr, "FAIL: uncaptured light tick mutated production state\n");
        return 1;
    }
    nexus_v1_light_torch_off(&state);
    if (state.torch_active != 0) {
        fprintf(stderr, "FAIL: torch_off did not clear active\n");
        return 1;
    }

    puts("PASS: production Nexus light boundary remains fail-closed");
    return 0;
}
