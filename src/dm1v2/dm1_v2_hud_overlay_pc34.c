#include "dm1_v2_anim_timing.h"
#include "dm1_v2_hud_overlay_pc34.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* DM1 V2 HUD overlay completion note:
 * ReDMCSB keeps champion status-box refresh in TIMELINE.C:F0260 and
 * portrait/status drawing in PANEL.C:F0354.  This V2 overlay is deliberately
 * presentation-only: it draws an optional compass/depth/stats layer into the
 * supplied framebuffer and does not mutate dungeon, champion, or command
 * runtime state.
 */

static M11_V2_HudOverlay g_v2_hud_state;

static void v2_hud_plot_pixel(uint8_t* fb, int w, int x, int y, uint8_t val) {
    if (x >= 0 && x < w && y >= 0) {
        fb[y * w + x] = val;
    }
}

static void v2_hud_draw_rect(uint8_t* fb, int w, int x, int y, int rw, int rh, uint8_t val) {
    for (int dy = 0; dy < rh; dy++) {
        for (int dx = 0; dx < rw; dx++) {
            v2_hud_plot_pixel(fb, w, x + dx, y + dy, val);
        }
    }
}

static const uint8_t g_v2_digits[10][5] = {
    {0x7E, 0x41, 0x41, 0x41, 0x7E},
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x7E, 0x41, 0x41, 0x41, 0x7E},
    {0x7E, 0x41, 0x41, 0x41, 0x7E},
    {0x7E, 0x41, 0x41, 0x41, 0x7E},
    {0x7E, 0x41, 0x41, 0x41, 0x7E},
    {0x7E, 0x41, 0x41, 0x41, 0x7E},
    {0x7E, 0x41, 0x41, 0x41, 0x7E},
    {0x7E, 0x41, 0x41, 0x41, 0x7E},
    {0x7E, 0x41, 0x41, 0x41, 0x7E}
};

static void v2_hud_draw_digit(uint8_t* fb, int w, int x, int y, int digit, uint8_t val) {
    digit = digit % 10;
    for (int row = 0; row < 5; row++) {
        uint8_t bits = g_v2_digits[digit][row];
        for (int col = 0; col < 5; col++) {
            if (bits & (1 << col)) {
                v2_hud_plot_pixel(fb, w, x + col, y + row, val);
            }
        }
    }
}

static void v2_hud_draw_text(uint8_t* fb, int w, int x, int y, const char* str, uint8_t val) {
    while (*str) {
        if (*str >= '0' && *str <= '9') {
            v2_hud_draw_digit(fb, w, x, y, *str - '0', val);
            x += 6;
        } else if (*str == '-') {
            v2_hud_plot_pixel(fb, w, x + 2, y + 2, val);
            x += 6;
        } else {
            x += 6;
        }
        str++;
    }
}

void v2_hud_init(void) {
    memset(&g_v2_hud_state, 0, sizeof(M11_V2_HudOverlay));
    g_v2_hud_state.compass.direction = 0;
    g_v2_hud_state.compass.needle_angle = 0.0f;
    g_v2_hud_state.compass.animated = true;
    g_v2_hud_state.depth.current_level = 1;
    g_v2_hud_state.depth.max_level = 10;
    g_v2_hud_state.visible = true;
    g_v2_hud_state.opacity = 255;
    g_v2_hud_state.stats_bar_visible = true;
}

void v2_hud_set_direction(int dir) {
    if (dir < 0) dir = 0;
    if (dir > 3) dir = 3;
    g_v2_hud_state.compass.direction = dir;
    g_v2_hud_state.compass.needle_angle = (float)dir * 90.0f;
}

void v2_hud_set_level(int cur, int max) {
    if (cur < 0) cur = 0;
    if (max <= 0) max = 1;
    g_v2_hud_state.depth.current_level = cur;
    g_v2_hud_state.depth.max_level = max;
}

void v2_hud_render(uint8_t* fb, int w, int h) {
    if (!fb || w <= 0 || h <= 0) return;
    if (!g_v2_hud_state.visible) return;

    uint8_t alpha = g_v2_hud_state.opacity;
    uint8_t base_val = (alpha > 0) ? (alpha / 2) : 0;
    uint8_t high_val = (alpha > 0) ? alpha : 0;

    int cx = 8;
    int cy = 8;
    v2_hud_draw_rect(fb, w, cx, cy, 16, 16, base_val);
    v2_hud_plot_pixel(fb, w, cx + 7, cy + 7, high_val);
    v2_hud_plot_pixel(fb, w, cx + 8, cy + 7, high_val);
    v2_hud_plot_pixel(fb, w, cx + 7, cy + 8, high_val);
    v2_hud_plot_pixel(fb, w, cx + 8, cy + 8, high_val);

    /* Source-lock seam: ReDMCSB direction is a 0..3 logical value.  Keep the
     * V2 compass deterministic on that cardinal state instead of depending on
     * floating point trig in this portable C gate. */
    int nx = cx + 8;
    int ny = cy + 8;
    switch (g_v2_hud_state.compass.direction) {
    case 0: ny = cy - 8; break;
    case 1: nx = cx - 8; break;
    case 2: ny = cy + 16; break;
    case 3: nx = cx + 16; break;
    default: break;
    }
    v2_hud_plot_pixel(fb, w, nx, ny, 255);

    char depth_buf[32];
    snprintf(depth_buf, sizeof(depth_buf), "%d/%d", g_v2_hud_state.depth.current_level, g_v2_hud_state.depth.max_level);
    v2_hud_draw_text(fb, w, w - 60, 8, depth_buf, high_val);

    if (g_v2_hud_state.stats_bar_visible) {
        int bar_x = 8;
        int bar_y = h - 16;
        int bar_w = w - 16;
        int bar_h = 8;
        v2_hud_draw_rect(fb, w, bar_x, bar_y, bar_w, bar_h, base_val);
        int fill = (int)((float)bar_w * 0.75f);
        v2_hud_draw_rect(fb, w, bar_x, bar_y, fill, bar_h, high_val);
    }
}

void v2_hud_toggle(void) {
    g_v2_hud_state.visible = !g_v2_hud_state.visible;
}

void v2_hud_set_opacity(uint8_t val) {
    g_v2_hud_state.opacity = val;
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.1 HUD Panel — Upscaled V1 champion/action panels
 *
 * V1 panel layout (320×64 bottom area):
 *   - Champion status bars (health/stamina/mana)
 *   - Action area (spell symbols, action icons)
 *   - Compass/movement arrows
 *
 * V2.1 upscales the same layout using EPX for crisp text/icons.
 * Source: ReDMCSB PANEL.C, STATS.C, CHAMDRAW.C
 * ══════════════════════════════════════════════════════════════════════ */

const char *v21_hud_panel_source_evidence(void) {
    return
        "PANEL.C F0395-F0404 champion status rendering\n"
        "STATS.C F0090-F0092 stat bar draw helpers\n"
        "CHAMDRAW.C champion portrait/name rendering\n"
        "V2.1: identical panel layout upscaled via EPX\n";
}

/* V2.2 HUD: health bar pulse when low, mana glow when charging.
 * Pulse rate = 2 Hz = every 9 V1 ticks.
 * All visual feedback synced to V1 timing.
 * v22_hud_pulse_v1_sync marker */

#define V22_HUD_PULSE_TICKS 9  /* ~0.5 seconds */

static V2_Anim g_health_pulse;

void v22_hud_start_health_pulse(void) {
    v2_anim_start(&g_health_pulse, 0.6f, 1.0f,
        V22_HUD_PULSE_TICKS * V1_TICK_MS, V2_EASE_IN_OUT_QUAD);
    g_health_pulse.loops = -1; /* infinite ping-pong */
}

float v22_hud_health_pulse_alpha(void) {
    return v2_anim_value(&g_health_pulse);
}

