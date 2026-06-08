#include "dm2_v1_world_state.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int expect_true(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    fprintf(stderr, "PASS: %s\n", message);
    return 1;
}

int main(void)
{
    int pass = 0;
    int total = 0;
    DM2_WorldState state;
    DM2_WorldState *loaded;
    uint8_t *serialized;
    size_t serialized_size = 0;

    memset(&state, 0, sizeof(state));
    state.current_level = 0;

    dm2_v1_world_state_set_explored(&state, 0, 3, 4, 1);
    dm2_v1_world_state_set_explored(&state, 0, 31, 31, 1);
    dm2_v1_world_state_set_explored(&state, 2, 1, 30, 1);
    dm2_v1_world_state_set_explored(&state, 2, 1, 30, 0);
    dm2_v1_world_state_set_explored(&state, 2, 2, 30, 1);

#define RUN(cond, msg) do { total++; pass += expect_true((cond), (msg)); } while (0)
    RUN(dm2_v1_world_state_get_explored(&state, 0, 3, 4) == 1,
        "level 0 cell reveal is readable before save");
    RUN(dm2_v1_world_state_get_explored(&state, 2, 2, 30) == 1,
        "level 2 cell reveal is readable before save");
    RUN(dm2_v1_world_state_get_explored(&state, 2, 1, 30) == 0,
        "clearing a reveal bit is reflected before save");
    RUN(dm2_v1_world_state_get_explored(&state, 30, 0, 0) == 0,
        "out-of-range level reads as unexplored");

    serialized = dm2_v1_world_state_serialize(&state, &serialized_size);
    RUN(serialized != NULL, "world-state serialization returns a buffer");
    RUN(serialized_size > 64, "serialized buffer carries exploration extension");

    loaded = dm2_v1_world_state_load_from_mem(serialized, serialized_size);
    RUN(loaded != NULL, "serialized world-state reloads from memory");
    if (loaded) {
        RUN(dm2_v1_world_state_get_explored(loaded, 0, 3, 4) == 1,
            "level 0 reveal persists across save/load");
        RUN(dm2_v1_world_state_get_explored(loaded, 0, 31, 31) == 1,
            "edge-cell reveal persists across save/load");
        RUN(dm2_v1_world_state_get_explored(loaded, 2, 2, 30) == 1,
            "different-level reveal persists across save/load");
        RUN(dm2_v1_world_state_get_explored(loaded, 2, 1, 30) == 0,
            "cleared reveal remains clear across save/load");
        RUN(dm2_v1_world_state_get_explored(loaded, 1, 3, 4) == 0,
            "same coordinate on another level remains unexplored");
        dm2_v1_world_state_free(loaded);
    }

    free(serialized);
#undef RUN

    fprintf(stderr, "DM2 V1 world-state minimap persistence: %d/%d passed\n", pass, total);
    return pass == total ? 0 : 1;
}
