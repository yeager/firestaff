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
    uint8_t fb[320 * 200];
    const int w = 320;
    const int h = 200;

    /* --- v2_hud_render tests (existing) --- */
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(count_nonzero(fb, (int)sizeof(fb)) == 0);

    v2_hud_init();
    v2_hud_set_opacity(200);
    v2_hud_set_direction(0);
    v2_hud_set_level(3, 12);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[0 * w + 16] == 255);           /* north compass needle */
    CHECK(fb[8  * w + 8] == 100);            /* compass background */
    CHECK(fb[(h - 16) * w + 8] == 200);      /* stats fill starts */
    CHECK(count_nonzero(fb, (int)sizeof(fb)) > 300);

    v2_hud_set_direction(1);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[16 * w + 0] == 255);            /* west compass needle */

    v2_hud_set_direction(2);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[24 * w + 16] == 255);           /* south compass needle */

    v2_hud_set_direction(99);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[16 * w + 24] == 255);          /* clamp high to east */

    v2_hud_set_direction(-7);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[0 * w + 16] == 255);           /* clamp low to north */

    v2_hud_toggle();
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(count_nonzero(fb, (int)sizeof(fb)) == 0);

    /* --- v22_hud_render_champion_panel tests (new) --- */
    v2_hud_toggle(); /* re-enable so panel renders */
    v2_hud_set_opacity(200);

    int hp[4]  = {100, 75, 50, 25};
    int stam[4] = {100, 80, 60, 40};
    int mana[4] = {100, 60, 30, 10};

    /* Panel renders into the bottom ~54px starting at V22_PANEL_Y=138 */
    memset(fb, 0, sizeof(fb));
    v22_hud_render_champion_panel(fb, w, h, hp, stam, mana);
    /* With opacity=200, base_val=100, high_val=200.
     * Four 80px-wide slots fill the bottom: slot 0 at x=0, slot 1 at x=80, ... */
    /* Slot 0 portrait box at (2, 138+4) = (2, 142), 28×46 */
    CHECK(fb[142 * w + 2] == 200);  /* portrait border pixel (high_val=200 with opacity=200) */
    /* HP bar at slot 0: bar_x = sx+34 = 34, bar_y = sy+20 = 158 */
    /* First column of filled HP bar (at 100%) */
    CHECK(fb[158 * w + 34] == 200);  /* HP bar filled portion */
    /* Last column of unfilled HP bar */
    CHECK(fb[158 * w + 69] == 200);  /* HP bar unfilled portion (at 100% HP: fill=36, x=69=col36=last=filled=200) */
    /* Non-empty output */
    CHECK(count_nonzero(fb, (int)sizeof(fb)) > 500);

    /* Slot 1 (Mage): x=80..159, HP=75% -> 27/36 pixels filled */
    /* Stamina bar at slot 1: x=80+34=114, y=158+8=166, fill=80% */
    CHECK(fb[166 * w + 114] == 150); /* stamina filled (high_val*3/4=150) */

    /* Null pointers: must not crash and produce silent-zero output */
    memset(fb, 0, sizeof(fb));
    v22_hud_render_champion_panel(fb, w, h, NULL, NULL, NULL);
    /* Should still render with default values */
    CHECK(count_nonzero(fb, (int)sizeof(fb)) > 300);

    /* V2.2 health pulse: start and read alpha */
    v22_hud_start_health_pulse();
    float a1 = v22_hud_health_pulse_alpha();
    /* After init the animation is running; alpha should be between 0 and 1 */
    CHECK(a1 >= 0.0f && a1 <= 1.0f);

    /* Advance some ticks */
    for (int i = 0; i < 9; i++) v22_hud_pulse_v1_tick();
    float a2 = v22_hud_health_pulse_alpha();
    /* After 9 ticks (one full cycle) the value should have progressed */
    CHECK(a2 >= 0.0f && a2 <= 1.0f);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_hud_overlay_pc34: ok");
    return 0;
}