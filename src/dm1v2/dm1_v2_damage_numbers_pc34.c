#include "dm1_v2_anim_timing.h"
#include "dm1_v2_damage_numbers_pc34.h"

void v2_damage_init(void) {
    /* ReDMCSB has no floating-damage-number surface or timing route. */
}

void v2_damage_spawn(float x, float y, int value, uint8_t color) {
    (void)x;
    (void)y;
    (void)value;
    (void)color;
}

void v2_damage_update(float dt) {
    (void)dt;
}

void v2_damage_render(uint8_t* fb, int w, int h) {
    (void)fb;
    (void)w;
    (void)h;
}

void v2_damage_clear(void) {
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.2 Damage Numbers — Floating combat damage display
 *
 * Spawns a floating number when damage is dealt/received.
 * Numbers drift upward and fade out over ~1 second.
 * Color: red for damage taken, yellow for damage dealt, green for heal.
*/
