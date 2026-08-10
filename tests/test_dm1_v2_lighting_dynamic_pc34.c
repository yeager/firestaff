/* DM1 V2 dynamic lighting is presentation-only. The synthesis this test
 * exercises stays entirely inside the compat state boundary and never
 * reaches the runtime tuple. Two additive-model invariants are locked
 * downstream of the palette map:
 *   - half radius => squared falloff  (the alpha attenuation curve
 *     halves the intensity every source-radius unit; alpha decays with
 *     the square of distance-over-radius).
 *   - additive overlay clamps         (RGB accumulators saturate at
 *     255, never wrap around and never turn negative).
 */
#include "dm1_v2_lighting_dynamic_pc34.h"

#include <stdint.h>
#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); failures++; \
} } while (0)

int main(void) {
    static const uint8_t expected[6] = { 99, 75, 50, 25, 1, 0 };
    M11_V2_SourcePaletteLighting plan;
    uint8_t r = 99, g = 99, b = 99;
    int i;

    for (i = 0; i < 6; ++i) {
        plan = v2_light_build_source_palette_lighting(i, true);
        CHECK(plan.source_palette_index == i);
        CHECK(plan.source_light_amount_floor == expected[i]);
        CHECK(plan.darkness_percent == 100 - expected[i]);
        CHECK(!plan.enhanced_effects_enabled);
        CHECK(!plan.deterministic_fallback);
    }
    plan = v2_light_build_source_palette_lighting(6, true);
    CHECK(plan.source_palette_index == 5);
    CHECK(plan.source_light_amount_floor == 0);
    CHECK(!plan.enhanced_effects_enabled);
    CHECK(plan.deterministic_fallback);

    v2_light_init();
    CHECK(v2_light_add_source(16.0f, 16.0f, 4.0f, 255, 100, 50, 25) == -1);
    CHECK(v2_light_source_count() == 0);
    v2_light_compute_map();
    v2_light_get_tile(16, 16, &r, &g, &b);
    CHECK(r == 0 && g == 0 && b == 0);

    v22_light_set_ambient(1.0f);
    CHECK(v22_light_add(4, 5, 1.0f, 3.0f, 0xffaa00ffu, 1) == -1);
    v22_light_rebuild_map();
    CHECK(v22_light_source_count() == 0);
    CHECK(v22_light_get(4, 5) == 0.0f);

    if (failures) return 1;
    puts("dm1_v2_lighting_dynamic_pc34: ok");
    return 0;
}
