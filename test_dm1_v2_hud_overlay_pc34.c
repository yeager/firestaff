#include "dm1_v2_hud_overlay_pc34.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); failures++; } } while (0)

static int count_nonzero(const uint8_t* fb, int count) {
    int n = 0;
    for (int i = 0; i < count; ++i) {
        if (fb[i] != 0) ++n;
    }
    return n;
}

int main(void) {
    uint8_t fb[96 * 64];
    const int w = 96;
    const int h = 64;

    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(count_nonzero(fb, (int)sizeof(fb)) == 0);

    v2_hud_init();
    v2_hud_set_opacity(200);
    v2_hud_set_direction(0);
    v2_hud_set_level(3, 12);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[0 * w + 16] == 255);          /* north compass needle */
    CHECK(fb[8 * w + 8] == 100);           /* compass background */
    CHECK(fb[(h - 16) * w + 8] == 200);    /* stats fill starts */
    CHECK(count_nonzero(fb, (int)sizeof(fb)) > 300);

    v2_hud_set_direction(1);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[16 * w + 0] == 255);          /* west compass needle */

    v2_hud_set_direction(2);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[24 * w + 16] == 255);         /* south compass needle */

    v2_hud_set_direction(99);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[16 * w + 24] == 255);         /* clamp high to east */

    v2_hud_set_direction(-7);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[0 * w + 16] == 255);          /* clamp low to north */

    v2_hud_toggle();
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(count_nonzero(fb, (int)sizeof(fb)) == 0);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_hud_overlay_pc34: ok");
    return 0;
}
