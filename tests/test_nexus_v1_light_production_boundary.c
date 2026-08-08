#include "nexus_v1_light.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_LightState state;

    nexus_v1_light_init(&state);
    nexus_v1_light_torch_on(&state, 100);
    if (state.torch_ticks != 100) {
        fprintf(stderr, "FAIL: torch_on did not set ticks\n");
        return 1;
    }
    if (nexus_v1_light_get(&state) <= 0) {
        fprintf(stderr, "FAIL: light_get returned 0 after torch_on\n");
        return 1;
    }
    nexus_v1_light_tick(&state);
    if (state.torch_ticks > 100) {
        fprintf(stderr, "FAIL: light_tick increased torch ticks\n");
        return 1;
    }
    nexus_v1_light_torch_off(&state);
    if (state.torch_active != 0) {
        fprintf(stderr, "FAIL: torch_off did not clear active\n");
        return 1;
    }

    puts("PASS: production Nexus light route returns real values");
    return 0;
}
