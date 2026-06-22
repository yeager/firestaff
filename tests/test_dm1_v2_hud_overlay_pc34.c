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

static void check_guarded_tiny_render(int w, int h) {
    uint8_t guarded[96];
    int pixels = w * h;

    memset(guarded, 0xA5, sizeof(guarded));
    memset(guarded, 0, (size_t)pixels);
    v2_hud_render(guarded, w, h);
    for (int i = pixels; i < (int)sizeof(guarded); ++i) {
        CHECK(guarded[i] == 0xA5);
    }
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

    /* Presentation-state overlay: champion/action/rune surfaces remain
     * machine-checkable pixels and do not call V1 transaction owners. */
    v2_hud_clear_presentation_state();
    v2_hud_set_opacity(200);
    v2_hud_set_champion_overlay_state(0, 50, 75, 25, true, true);
    v2_hud_set_champion_overlay_state(1, 150, -4, 60, false, false);
    v2_hud_set_champion_overlay_state(-1, 100, 100, 100, true, true);
    v2_hud_set_rune_overlay_state(0x05u, 2, true, true, true);
    v2_hud_set_action_overlay_state(1, 2, 2);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[1 * w + 2] == 255);            /* active leader border */
    CHECK(fb[6 * w + 7] == 200);            /* champion HP fill */
    CHECK(fb[6 * w + 28] == 100);           /* champion HP empty tail */
    CHECK(fb[11 * w + 7] == 150);           /* stamina tint */
    CHECK(fb[16 * w + 7] == 133);           /* mana tint */
    CHECK(fb[7 * w + 55] == 255);           /* spell-ready cue */
    CHECK(fb[6 * w + 76] == 200);           /* clamped high HP fill */
    CHECK(fb[11 * w + 76] == 100);          /* clamped low stamina */
    CHECK(fb[43 * w + 234] == 255);         /* caster-ready strip */
    CHECK(fb[52 * w + 236] == 200);         /* selected rune */
    CHECK(fb[52 * w + 248] == 100);         /* unselected rune */
    CHECK(fb[52 * w + 260] == 255);         /* active rune */
    CHECK(fb[64 * w + 236] == 255);         /* cast rectangle */
    CHECK(fb[64 * w + 300] == 200);         /* recant rectangle */
    CHECK(fb[86 * w + 256] == 255);         /* active action icon */
    CHECK(fb[86 * w + 276] == 200);         /* highlighted action icon */
    CHECK(fb[78 * w + 252] == 255);         /* action flash */
    check_guarded_tiny_render(24, 4);        /* presentation-state clipping */

    v2_hud_tick_presentation_state();
    v2_hud_tick_presentation_state();
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[78 * w + 252] == 100);         /* flash decays to action panel */

    v2_hud_clear_presentation_state();
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[43 * w + 234] == 0);           /* clear hides rune strip */
    CHECK(fb[86 * w + 256] == 0);           /* clear hides action strip */
    CHECK(fb[1 * w + 140] == 0);            /* invalid champion write ignored */

    v2_hud_toggle();
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(count_nonzero(fb, (int)sizeof(fb)) == 0);

    /* Clipped overlay surfaces are presentation-only; tiny 320x200-derived
     * buffers must not write past the caller-owned framebuffer while V1
     * gameplay state remains outside this API. */
    v2_hud_toggle();
    v2_hud_set_opacity(200);
    v2_hud_set_direction(2);
    check_guarded_tiny_render(8, 8);
    check_guarded_tiny_render(16, 4);
    check_guarded_tiny_render(24, 2);

    /* Presentation-state surfaces for the B3 champion/action/rune overlay
     * gap are deterministic pixels only; the setters never call V1 command,
     * inventory, or spell transactions. */
    v2_hud_set_opacity(200);
    v2_hud_set_champion_bar(2, 33, 66, 99, true, true);
    v2_hud_set_champion_bar(-1, 100, 100, 100, true, true);
    v2_hud_set_champion_bar(4, 100, 100, 100, true, true);
    v2_hud_set_action_active(M11_V2_HUD_ACTION_CAST);
    v2_hud_trigger_action_flash();
    v2_hud_set_rune_active(3, true);
    v2_hud_set_rune_active(-1, true);
    v2_hud_set_rune_active(6, true);
    v2_hud_set_spell_controls(true, false);

    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[8 * w + 158] == 200);          /* champion 2 HP fill at 33% */
    CHECK(fb[8 * w + 159] == 100);          /* champion 2 HP clamp boundary */
    CHECK(fb[11 * w + 174] == 150);         /* stamina fill at 66% */
    CHECK(fb[14 * w + 190] == 133);         /* mana fill at 99% */
    CHECK(fb[6 * w + 190] == 200);          /* leader cue */
    CHECK(fb[15 * w + 193] == 200);         /* spell-ready cue */
    CHECK(fb[168 * w + 45] == 255);         /* active action flash border */
    CHECK(fb[169 * w + 46] == 150);         /* active action interior */
    CHECK(fb[157 * w + 196] == 200);        /* active rune 3 */
    CHECK(fb[157 * w + 170] == 100);        /* inactive rune 1 */
    CHECK(fb[156 * w + 238] == 200);        /* cast-ready control */
    CHECK(fb[156 * w + 266] == 100);        /* recant disabled control */

    for (int i = 0; i < 6; ++i) {
        memset(fb, 0, sizeof(fb));
        v2_hud_render(fb, w, h);
    }
    CHECK(fb[168 * w + 45] == 200);         /* flash decays to active border */

    v2_hud_set_action_active((M11_V2_HudActionIcon)99);
    memset(fb, 0, sizeof(fb));
    v2_hud_render(fb, w, h);
    CHECK(fb[168 * w + 45] == 100);         /* invalid action clears highlight */

    /* --- v22_hud_render_champion_panel tests (new) --- */
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
