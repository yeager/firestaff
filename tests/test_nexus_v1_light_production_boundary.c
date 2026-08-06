#include "nexus_v1_light.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_LightState state;
    Nexus_LightState before;

    memset(&state, 0x4a, sizeof(state));
    before = state;
    nexus_v1_light_init(&state);
    nexus_v1_light_set(&state, 4);
    nexus_v1_light_add(&state, 12);
    nexus_v1_light_torch_on(&state, 100);
    nexus_v1_light_ful_spell(&state, 3, 200);
    nexus_v1_light_tick(&state);
    nexus_v1_light_torch_off(&state);

    if (memcmp(&state, &before, sizeof(state)) != 0 ||
        nexus_v1_light_get(&state) != 0) {
        fprintf(stderr, "FAIL: production Nexus light route mutated or exposed inferred state\n");
        return 1;
    }

    puts("PASS: production Nexus light route is fail-closed");
    return 0;
}
