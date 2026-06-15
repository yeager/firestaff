/* Nexus V2.2 dynamic lighting smoke test.
 *
 * The CMake `nexus_v2_lighting` ctest target previously had an
 * add_test() entry but no executable, so ctest reported it as
 * "Not Run" (could not find executable test_nexus_v2_lighting).
 * This minimal smoke test exercises the public lighting API so the
 * registered test actually builds and runs.
 *
 * Source-lock evidence string: src/nexus/nexus_v2_lighting.c
 *   (ReDMCSB LIGHT.C F0380 light radius/flicker timing,
 *    COMMAND.C F0209 spell-light colour binding).
 */
#include "nexus_v2_lighting.h"

#include <stdio.h>
#include <string.h>

static int g_fail;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        g_fail++; \
    } \
} while (0)

int main(void) {
    Nexus_V2_LightingState ls;
    uint32_t fb[8 * 8];
    int i;
    int idx;
    uint32_t before;
    uint32_t after;

    nexus_v2_lighting_init(&ls);
    CHECK(ls.light_count == 0, "init clears active light count");
    CHECK(ls.ambient_r > 0.0f && ls.ambient_g > 0.0f && ls.ambient_b > 0.0f,
          "init sets a non-zero ambient base light");

    /* Add a flickering torch near the camera. */
    idx = nexus_v2_light_add(&ls, 0.0f, 0.0f, -1.0f,
                             1.0f, 0.6f, 0.2f, 1.0f, 4.0f, 1);
    CHECK(idx == 0, "first added light takes slot 0");
    CHECK(ls.light_count == 1, "light_count tracks the added light");
    CHECK(ls.lights[0].active == 1, "added light is active");

    /* Fill the remaining slots, then verify overflow is rejected. */
    for (i = 1; i < NEXUS_MAX_LIGHTS; ++i) {
        int slot = nexus_v2_light_add(&ls, (float)i, 0.0f, -1.0f,
                                      1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 0);
        CHECK(slot == i, "subsequent lights fill the next free slot");
    }
    CHECK(nexus_v2_light_add(&ls, 0.0f, 0.0f, 0.0f,
                             1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0) == -1,
          "light pool overflow is gracefully rejected");

    /* Removing a slot frees it for reuse. */
    nexus_v2_light_remove(&ls, 5);
    CHECK(ls.lights[5].active == 0, "removed light is marked inactive");
    CHECK(nexus_v2_light_add(&ls, 0.0f, 0.0f, 0.0f,
                             1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0) == 5,
          "freed slot is reused by the next add");

    /* Tick should advance the flicker phase without crashing. */
    nexus_v2_lighting_tick(&ls, 0.1f);
    CHECK(ls.torch_flicker_phase > 0.0f, "tick advances torch flicker phase");

    /* Apply lighting to a small framebuffer: a fully-lit white pixel
     * near a bright light should change (it starts at full white and
     * gets modulated by accumulated light/ambient). */
    for (i = 0; i < 8 * 8; ++i) {
        fb[i] = 0xFFFFFFFFu; /* opaque white */
    }
    before = fb[0];
    nexus_v2_apply_lighting(fb, 8, 8, &ls, 0.0f, 0.0f, 0.0f);
    after = fb[0];
    CHECK((after & 0xFF000000u) == 0xFF000000u,
          "apply_lighting keeps alpha opaque");
    CHECK(after != before || ls.light_count == 0,
          "apply_lighting modulates pixels when lights are present");

    /* NULL-safety: must not crash. */
    nexus_v2_lighting_init(NULL);
    nexus_v2_lighting_tick(NULL, 0.1f);
    nexus_v2_light_remove(NULL, 0);
    CHECK(nexus_v2_light_add(NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0) == -1,
          "light_add on NULL state is rejected");
    nexus_v2_apply_lighting(NULL, 8, 8, &ls, 0, 0, 0);

    CHECK(nexus_v2_lighting_source_evidence() != NULL &&
          strstr(nexus_v2_lighting_source_evidence(), "LIGHT.C") != NULL,
          "source evidence cites ReDMCSB LIGHT.C");

    if (g_fail != 0) {
        printf("Nexus V2 lighting smoke: %d failure(s)\n", g_fail);
        return 1;
    }
    printf("Nexus V2 lighting smoke passed\n");
    return 0;
}
