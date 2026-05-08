#include "dm1_v2_lighting_dynamic_pc34.h"
#include <stdio.h>
#include <stdint.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); failures++; } } while (0)

static void get_rgb(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) {
    *r = *g = *b = 99;
    v2_light_get_tile(x, y, r, g, b);
}

int main(void) {
    uint8_t r, g, b;

    v2_light_init();
    v2_light_compute_map();
    get_rgb(16, 16, &r, &g, &b);
    CHECK(r == 0 && g == 0 && b == 0);
    get_rgb(-1, 16, &r, &g, &b);
    CHECK(r == 0 && g == 0 && b == 0);

    CHECK(v2_light_add_source(16.0f, 16.0f, 4.0f, 255, 100, 50, 25) == 0);
    v2_light_compute_map();
    get_rgb(16, 16, &r, &g, &b);
    CHECK(r == 100 && g == 50 && b == 25);
    get_rgb(18, 16, &r, &g, &b);
    CHECK(r == 25 && g == 12 && b == 6); /* half radius => squared falloff */
    get_rgb(20, 16, &r, &g, &b);
    CHECK(r == 0 && g == 0 && b == 0);   /* radius boundary is exclusive */

    CHECK(v2_light_add_source(16.0f, 16.0f, 2.0f, 255, 240, 240, 240) == 1);
    v2_light_compute_map();
    get_rgb(16, 16, &r, &g, &b);
    CHECK(r == 255 && g == 255 && b == 255); /* additive overlay clamps */

    v2_light_remove_source(1);
    v2_light_compute_map();
    get_rgb(16, 16, &r, &g, &b);
    CHECK(r == 100 && g == 50 && b == 25);

    v2_light_init();
    for (int i = 0; i < M11_V2_LIGHT_MAX_SOURCES; ++i) {
        CHECK(v2_light_add_source(0.0f, 0.0f, 1.0f, 1, 1, 1, 1) == i);
    }
    CHECK(v2_light_add_source(0.0f, 0.0f, 1.0f, 1, 1, 1, 1) == -1);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_lighting_dynamic_pc34: ok");
    return 0;
}
