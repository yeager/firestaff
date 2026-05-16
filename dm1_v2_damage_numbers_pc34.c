#include "dm1_v2_damage_numbers_pc34.h"
#include <string.h>
#include <math.h>

static M11_V2_DamageDisplay g_damage_display;

static const uint8_t g_digit_font[10][5] = {
    {0x7, 0x5, 0x5, 0x5, 0x7}, /* 0 */
    {0x6, 0x6, 0x2, 0x2, 0x7}, /* 1 */
    {0x7, 0x1, 0x7, 0x4, 0x7}, /* 2 */
    {0x7, 0x1, 0x7, 0x1, 0x7}, /* 3 */
    {0x5, 0x5, 0x7, 0x1, 0x1}, /* 4 */
    {0x7, 0x4, 0x7, 0x1, 0x7}, /* 5 */
    {0x7, 0x4, 0x7, 0x5, 0x7}, /* 6 */
    {0x7, 0x1, 0x2, 0x4, 0x4}, /* 7 */
    {0x7, 0x5, 0x7, 0x5, 0x7}, /* 8 */
    {0x7, 0x5, 0x7, 0x1, 0x7}  /* 9 */
};

void v2_damage_init(void) {
    memset(&g_damage_display, 0, sizeof(g_damage_display));
}

void v2_damage_spawn(float x, float y, int value, uint8_t color) {
    if (g_damage_display.count >= 32) return;
    M11_V2_DamagePopup* p = &g_damage_display.popups[g_damage_display.count];
    p->x = x;
    p->y = y;
    p->value = value;
    p->color = color;
    p->life = 2.0f;
    p->vy = -1.5f;
    g_damage_display.count++;
}

void v2_damage_update(float dt) {
    int write_idx = 0;
    for (int i = 0; i < g_damage_display.count; i++) {
        M11_V2_DamagePopup* p = &g_damage_display.popups[i];
        p->y += p->vy * dt;
        p->life -= dt;
        if (p->life > 0.0f) {
            if (write_idx != i) {
                g_damage_display.popups[write_idx] = *p;
            }
            write_idx++;
        }
    }
    g_damage_display.count = write_idx;
}

void v2_damage_render(uint8_t* fb, int w, int h) {
    if (!fb || w <= 0 || h <= 0) return;

    for (int i = 0; i < g_damage_display.count; i++) {
        M11_V2_DamagePopup* p = &g_damage_display.popups[i];
        int val = p->value;
        if (val < 0) val = -val;

        int digits[4];
        int d_count = 0;
        if (val == 0) {
            digits[0] = 0;
            d_count = 1;
        } else {
            int tmp = val;
            while (tmp > 0 && d_count < 4) {
                digits[d_count++] = tmp % 10;
                tmp /= 10;
            }
        }

        float draw_x = p->x;
        for (int d = d_count - 1; d >= 0; d--) {
            int digit = digits[d];
            for (int row = 0; row < 5; row++) {
                uint8_t bits = g_digit_font[digit][row];
                for (int col = 0; col < 4; col++) {
                    if (bits & (1 << col)) {
                        int px = (int)draw_x + col;
                        int py = (int)p->y + row;
                        if (px >= 0 && px < w && py >= 0 && py < h) {
                            fb[py * w + px] = p->color;
                        }
                    }
                }
            }
            draw_x += 5.0f;
        }
    }
}

void v2_damage_clear(void) {
    g_damage_display.count = 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.2 Damage Numbers — Floating combat damage display
 *
 * Spawns a floating number when damage is dealt/received.
 * Numbers drift upward and fade out over ~1 second.
 * Color: red for damage taken, yellow for damage dealt, green for heal.
 * ══════════════════════════════════════════════════════════════════════ */

#define V22_MAX_FLOATING_NUMBERS 16

typedef struct {
    int value;
    float x, y;
    float vy; /* upward velocity */
    float alpha; /* 1.0 = full, 0.0 = gone */
    uint32_t color;
    int active;
} V22_FloatingNumber;

static V22_FloatingNumber g_numbers[V22_MAX_FLOATING_NUMBERS];

void v22_damage_spawn(int value, float x, float y, uint32_t color) {
    int i;
    for (i = 0; i < V22_MAX_FLOATING_NUMBERS; i++) {
        if (!g_numbers[i].active) {
            g_numbers[i].value = value;
            g_numbers[i].x = x;
            g_numbers[i].y = y;
            g_numbers[i].vy = -30.0f; /* drift upward */
            g_numbers[i].alpha = 1.0f;
            g_numbers[i].color = color;
            g_numbers[i].active = 1;
            return;
        }
    }
}

void v22_damage_tick(float dt) {
    int i;
    for (i = 0; i < V22_MAX_FLOATING_NUMBERS; i++) {
        if (!g_numbers[i].active) continue;
        g_numbers[i].y += g_numbers[i].vy * dt;
        g_numbers[i].alpha -= dt; /* fade over 1 second */
        if (g_numbers[i].alpha <= 0.0f) {
            g_numbers[i].active = 0;
        }
    }
}

int v22_damage_get_active(const V22_FloatingNumber **out) {
    if (out) *out = g_numbers;
    int count = 0;
    for (int i = 0; i < V22_MAX_FLOATING_NUMBERS; i++) {
        if (g_numbers[i].active) count++;
    }
    return count;
}

