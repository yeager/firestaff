#include "dm1_v2_anim_timing.h"
#include "dm1_v2_hud_overlay_pc34.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

/* DM1 V2 HUD overlay completion note:
 * ReDMCSB keeps champion status-box refresh in TIMELINE.C:F0260 and
 * portrait/status drawing in PANEL.C:F0354.  This V2 overlay is deliberately
 * presentation-only: it draws optional compass/depth/stats and bounded
 * champion/action/rune presentation state into the supplied framebuffer and
 * does not mutate dungeon, champion, or command runtime state.  This is not a
 * finished V2 art, original screenshot parity, real-asset parity, or complete
 * V2 UI parity claim.
 */

static M11_V2_HudOverlay g_v2_hud_state;

static void v2_hud_plot_pixel(uint8_t* fb, int w, int h, int x, int y, uint8_t val) {
    if (x >= 0 && x < w && y >= 0 && y < h) {
        fb[y * w + x] = val;
    }
}

static void v2_hud_draw_rect(uint8_t* fb, int w, int h, int x, int y, int rw, int rh, uint8_t val) {
    for (int dy = 0; dy < rh; dy++) {
        for (int dx = 0; dx < rw; dx++) {
            v2_hud_plot_pixel(fb, w, h, x + dx, y + dy, val);
        }
    }
}

/* Standard 5×5 pixel font — 0..9, dash, period, space
 * Source: DM1 PC 3.4 ROM font glyphs 0..9 drawn from GRAPHICS.DAT M653;
 * dash/period/space are 5×5 single-pixel-column stubs.
 * ReDMCSB: DEFS.H M653_GRAPHIC_FONT=557, PIXEL ATLAS in GRAPHICS.DAT. */
static const uint8_t g_v2_digits[12][5] = {
    /* 0 */ {0x7E, 0x41, 0x41, 0x41, 0x7E},
    /* 1 */ {0x00, 0x08, 0x08, 0x08, 0x00},
    /* 2 */ {0x7E, 0x01, 0x7E, 0x40, 0x7E},
    /* 3 */ {0x7E, 0x01, 0x3E, 0x01, 0x7E},
    /* 4 */ {0x41, 0x41, 0x7F, 0x01, 0x01},
    /* 5 */ {0x7E, 0x40, 0x7E, 0x01, 0x7E},
    /* 6 */ {0x7E, 0x40, 0x7E, 0x41, 0x7E},
    /* 7 */ {0x7E, 0x01, 0x02, 0x04, 0x08},
    /* 8 */ {0x7E, 0x41, 0x7E, 0x41, 0x7E},
    /* 9 */ {0x7E, 0x41, 0x7E, 0x01, 0x7E},
    /* 10=dash  */ {0x00, 0x00, 0x1F, 0x00, 0x00},
    /* 11=dot   */ {0x00, 0x00, 0x00, 0x00, 0x08},
};

static void v2_hud_draw_digit(uint8_t* fb, int w, int h, int x, int y, int digit, uint8_t val) {
    digit = digit % 10;
    for (int row = 0; row < 5; row++) {
        uint8_t bits = g_v2_digits[digit][row];
        for (int col = 0; col < 5; col++) {
            if (bits & (1 << col)) {
                v2_hud_plot_pixel(fb, w, h, x + col, y + row, val);
            }
        }
    }
}

static void v2_hud_draw_text(uint8_t* fb, int w, int h, int x, int y, const char* str, uint8_t val) {
    while (*str) {
        if (*str >= '0' && *str <= '9') {
            v2_hud_draw_digit(fb, w, h, x, y, *str - '0', val);
            x += 6;
        } else if (*str == '-') {
            /* Draw a 5-pixel horizontal dash (center row, 5 cols) */
            for (int dc = 0; dc < 5; dc++) {
                v2_hud_plot_pixel(fb, w, h, x + dc, y + 2, val);
            }
            x += 6;
        } else if (*str == '.') {
            v2_hud_plot_pixel(fb, w, h, x + 2, y + 4, val);
            x += 6;
        } else {
            /* Fallback: small centered dot for unhandleable characters */
            v2_hud_plot_pixel(fb, w, h, x + 2, y + 2, val);
            x += 6;
        }
        str++;
    }
}

static int v2_hud_clamp_pct(int pct) {
    if (pct < 0) return 0;
    if (pct > 100) return 100;
    return pct;
}

static int v2_hud_clamp_index_or_none(int idx, int count) {
    if (idx < 0 || idx >= count) return -1;
    return idx;
}

static void v2_hud_draw_meter(uint8_t* fb,
                              int w,
                              int h,
                              int x,
                              int y,
                              int width,
                              int height,
                              int pct,
                              uint8_t low_val,
                              uint8_t high_val) {
    if (width <= 0 || height <= 0) return;
    pct = v2_hud_clamp_pct(pct);
    int fill = (width * pct + 99) / 100;
    v2_hud_draw_rect(fb, w, h, x, y, width, height, low_val);
    v2_hud_draw_rect(fb, w, h, x, y, fill, height, high_val);
}

static void v2_hud_render_presentation_state(uint8_t* fb, int w, int h,
                                             uint8_t base_val,
                                             uint8_t high_val) {
    /* ReDMCSB: PANEL.C:F0354 and CHAMDRAW.C champion status boxes live in
     * the top 0..28 pixel strip.  This V2 state is presentation-only and
     * mirrors the already source-locked COMMAND.C:375-395 champion route
     * state without calling any V1 transaction owner. */
    for (int i = 0; i < M11_V2_HUD_CHAMPION_COUNT_PC34; ++i) {
        const M11_V2_HudChampionOverlayPc34* c = &g_v2_hud_state.champions[i];
        if (!c->visible) continue;
        int sx = 2 + (i * 69);
        int sy = 1;
        uint8_t leader_val = c->active_leader ? 255u : high_val;
        v2_hud_draw_rect(fb, w, h, sx, sy, 66, 1, leader_val);
        v2_hud_draw_rect(fb, w, h, sx, sy + 27, 66, 1, leader_val);
        v2_hud_draw_rect(fb, w, h, sx, sy, 1, 28, leader_val);
        v2_hud_draw_rect(fb, w, h, sx + 65, sy, 1, 28, leader_val);
        v2_hud_draw_meter(fb, w, h, sx + 5, sy + 5, 42, 3,
                          c->hp_pct, base_val, high_val);
        v2_hud_draw_meter(fb, w, h, sx + 5, sy + 10, 42, 3,
                          c->stamina_pct, base_val, (uint8_t)(high_val * 3u / 4u));
        v2_hud_draw_meter(fb, w, h, sx + 5, sy + 15, 42, 3,
                          c->mana_pct, base_val, (uint8_t)(high_val * 2u / 3u));
        if (c->spell_ready) {
            v2_hud_draw_rect(fb, w, h, sx + 53, sy + 6, 8, 8, 255u);
            v2_hud_plot_pixel(fb, w, h, sx + 56, sy + 9, base_val);
            v2_hud_plot_pixel(fb, w, h, sx + 57, sy + 9, base_val);
        }
    }

    /* ReDMCSB: COMMAND.C:461-482 owns action names/icons and spell/rune
     * subroutes (C101..C109).  The overlay draws machine-checkable cues only;
     * cast/recant/caster state remains data-free presentation state. */
    if (g_v2_hud_state.runes.visible) {
        const M11_V2_HudRuneOverlayPc34* r = &g_v2_hud_state.runes;
        v2_hud_draw_rect(fb, w, h, 233, 42, 87, 31, base_val);
        if (r->caster_ready) {
            v2_hud_draw_rect(fb, w, h, 234, 43, 10, 7, 255u);
        }
        for (int i = 0; i < M11_V2_HUD_RUNE_COUNT_PC34; ++i) {
            int rx = 236 + (i * 12);
            int ry = 52;
            uint8_t rune_val = (r->selected_rune_mask & (uint8_t)(1u << i)) ? high_val : base_val;
            if (i == r->active_rune) rune_val = 255u;
            v2_hud_draw_rect(fb, w, h, rx, ry, 10, 8, rune_val);
            v2_hud_plot_pixel(fb, w, h, rx + 4, ry + 3, base_val);
        }
        v2_hud_draw_rect(fb, w, h, 236, 64, 50, 8, r->cast_enabled ? 255u : base_val);
        v2_hud_draw_rect(fb, w, h, 300, 64, 18, 8, r->recant_enabled ? high_val : base_val);
    }

    if (g_v2_hud_state.action.visible) {
        const M11_V2_HudActionOverlayPc34* a = &g_v2_hud_state.action;
        v2_hud_draw_rect(fb, w, h, 233, 77, 87, 45, base_val);
        for (int i = 0; i < M11_V2_HUD_ACTION_ICON_COUNT_PC34; ++i) {
            int ax = 236 + (i * 20);
            int ay = 86;
            uint8_t icon_val = (i == a->highlighted_icon) ? high_val : (uint8_t)(base_val / 2u);
            if (i == a->active_champion) icon_val = 255u;
            v2_hud_draw_rect(fb, w, h, ax, ay, 16, 12, icon_val);
        }
        if (a->flash_ticks > 0) {
            v2_hud_draw_rect(fb, w, h, 252, 78, 36, 4, 255u);
        }
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
    v2_hud_clear_presentation_state();
    /* v2_hud_init() resets HUD state; g_health_pulse is a separate static
     * that starts at zero (active=0).  Callers explicitly invoke
     * v22_hud_start_health_pulse() before the first v22_hud_health_pulse_alpha(). */
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
    v2_hud_draw_rect(fb, w, h, cx, cy, 16, 16, base_val);
    v2_hud_plot_pixel(fb, w, h, cx + 7, cy + 7, high_val);
    v2_hud_plot_pixel(fb, w, h, cx + 8, cy + 7, high_val);
    v2_hud_plot_pixel(fb, w, h, cx + 7, cy + 8, high_val);
    v2_hud_plot_pixel(fb, w, h, cx + 8, cy + 8, high_val);

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
    v2_hud_plot_pixel(fb, w, h, nx, ny, 255);

    char depth_buf[32];
    snprintf(depth_buf, sizeof(depth_buf), "%d/%d", g_v2_hud_state.depth.current_level, g_v2_hud_state.depth.max_level);
    v2_hud_draw_text(fb, w, h, w - 60, 8, depth_buf, high_val);

    if (g_v2_hud_state.stats_bar_visible) {
        int bar_x = 8;
        int bar_y = h - 16;
        int bar_w = w - 16;
        int bar_h = 8;
        v2_hud_draw_rect(fb, w, h, bar_x, bar_y, bar_w, bar_h, base_val);
        int fill = (int)((float)bar_w * 0.75f);
        v2_hud_draw_rect(fb, w, h, bar_x, bar_y, fill, bar_h, high_val);
    }

    v2_hud_render_presentation_state(fb, w, h, base_val, high_val);
}

void v2_hud_toggle(void) {
    g_v2_hud_state.visible = !g_v2_hud_state.visible;
}

void v2_hud_set_opacity(uint8_t val) {
    g_v2_hud_state.opacity = val;
}

void v2_hud_clear_presentation_state(void) {
    memset(g_v2_hud_state.champions, 0, sizeof(g_v2_hud_state.champions));
    memset(&g_v2_hud_state.action, 0, sizeof(g_v2_hud_state.action));
    memset(&g_v2_hud_state.runes, 0, sizeof(g_v2_hud_state.runes));
    g_v2_hud_state.action.active_champion = -1;
    g_v2_hud_state.action.highlighted_icon = -1;
    g_v2_hud_state.runes.active_rune = -1;
}

void v2_hud_set_champion_overlay_state(int champion_idx,
                                       int hp_pct,
                                       int stamina_pct,
                                       int mana_pct,
                                       bool active_leader,
                                       bool spell_ready) {
    if (champion_idx < 0 || champion_idx >= M11_V2_HUD_CHAMPION_COUNT_PC34) return;
    M11_V2_HudChampionOverlayPc34* c = &g_v2_hud_state.champions[champion_idx];
    c->hp_pct = v2_hud_clamp_pct(hp_pct);
    c->stamina_pct = v2_hud_clamp_pct(stamina_pct);
    c->mana_pct = v2_hud_clamp_pct(mana_pct);
    c->active_leader = active_leader;
    c->spell_ready = spell_ready;
    c->visible = true;
}

void v2_hud_set_action_overlay_state(int active_champion,
                                     int highlighted_icon,
                                     uint8_t flash_ticks) {
    g_v2_hud_state.action.visible = true;
    g_v2_hud_state.action.active_champion =
        v2_hud_clamp_index_or_none(active_champion, M11_V2_HUD_ACTION_ICON_COUNT_PC34);
    g_v2_hud_state.action.highlighted_icon =
        v2_hud_clamp_index_or_none(highlighted_icon, M11_V2_HUD_ACTION_ICON_COUNT_PC34);
    g_v2_hud_state.action.flash_ticks = flash_ticks;
}

void v2_hud_set_rune_overlay_state(uint8_t selected_rune_mask,
                                   int active_rune,
                                   bool cast_enabled,
                                   bool recant_enabled,
                                   bool caster_ready) {
    g_v2_hud_state.runes.visible = true;
    g_v2_hud_state.runes.selected_rune_mask =
        (uint8_t)(selected_rune_mask & ((1u << M11_V2_HUD_RUNE_COUNT_PC34) - 1u));
    g_v2_hud_state.runes.active_rune =
        v2_hud_clamp_index_or_none(active_rune, M11_V2_HUD_RUNE_COUNT_PC34);
    g_v2_hud_state.runes.cast_enabled = cast_enabled;
    g_v2_hud_state.runes.recant_enabled = recant_enabled;
    g_v2_hud_state.runes.caster_ready = caster_ready;
}

void v2_hud_tick_presentation_state(void) {
    if (g_v2_hud_state.action.flash_ticks > 0u) {
        g_v2_hud_state.action.flash_ticks--;
    }
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

/* v22_hud_pulse_v1_tick — advance animation by one V1 tick (55 ms).
 * Call from the per-tick gate; advances g_health_pulse.elapsed_ms and
 * recalculates current so callers can read v22_hud_health_pulse_alpha().
 * Does NOT call v2_anim_update twice — v2_anim_update already advanced
 * elapsed_ms inside v22_hud_pulse_v1_tick's stack copy; only re-read the
 * result after the update to keep the static state consistent.
 * ReDMCSB: TIMELINE.C F0260 champion status-box refresh cadence. */
void v22_hud_pulse_v1_tick(void) {
    /* Copy state to stack so v2_anim_update writes to local a, not g_health_pulse */
    V2_Anim a = g_health_pulse;
    v2_anim_update(&a, (float)V1_TICK_MS);
    /* Write the updated state back to the static */
    g_health_pulse = a;
    /* Manual loop reset for ping-pong: when v2_anim_update detects the
     * duration is exceeded and loops != 0, it swaps from/to but the static's
     * elapsed already has dt_ms added (inside v2_anim_update).  For ping-pong
     * the loop restart is handled entirely inside v2_anim_update; the static
     * is already in the correct swapped-state after the write-back above. */
}

/* pass601a: movement complete signal.
 * Raised by the V2 camera bridge when a move interpolation ends;
 * consumers (e.g. inscription renderer, minimap update) can use this to
 * sync to the discrete V1 game state without polling camera->active.
 * Source-lock: ReDMCSB GAMELOOP.C:90 viewport redraw cadence. */
void v22_hud_notify_move_complete(void) {
    g_v2_hud_state.moveCompletePending = 1;
}

void v22_hud_clear_move_complete(void) {
    g_v2_hud_state.moveCompletePending = 0;
}

int v22_hud_is_move_complete_pending(void) {
    return g_v2_hud_state.moveCompletePending;
}

/* pass601a: turn complete signal — mirrors moveCompletePending for turns.
 * Source-lock: ReDMCSB COMMAND.C:2150-2152 F0365 turn dispatch and
 * GAMELOOP.C:90 viewport redraw cadence. */
void v22_hud_notify_turn_complete(void) {
    g_v2_hud_state.turnCompletePending = 1;
}

void v22_hud_clear_turn_complete(void) {
    g_v2_hud_state.turnCompletePending = 0;
}

int v22_hud_is_turn_complete_pending(void) {
    return g_v2_hud_state.turnCompletePending;
}

void v22_hud_start_health_pulse(void) {
    v2_anim_start(&g_health_pulse, 0.6f, 1.0f,
        V22_HUD_PULSE_TICKS * V1_TICK_MS, V2_EASE_IN_OUT_QUAD);
    g_health_pulse.loops = -1; /* infinite ping-pong */
}

float v22_hud_health_pulse_alpha(void) {
    return v2_anim_value(&g_health_pulse);
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.1 Champion Panel Renderer — pure presentation overlay
 *
 * Draws 4 champion status slots into the bottom ~54px of the framebuffer.
 * Each slot: portrait box (left), name/stats (center), HP/Stamina/Mana bars,
 * hand icon (right).  Layout matches V1 PANEL.C F0395-F0404.
 *
 * V2.1: uses 5×5 pixel font and per-bar color tinting instead of V1 palette.
 * Does NOT mutate game state or command queues.
 * Source: ReDMCSB PANEL.C F0395-F0404; CHAMDRAW.C ~180; STATS.C F0090-F0092.
 * ══════════════════════════════════════════════════════════════════════ */

#define V22_PANEL_SLOT_W   80
#define V22_PANEL_SLOT_H   54
#define V22_PANEL_Y        138  /* top of champion panel in V1 320×200 space */

/* ReDMCSB M653 pixel font — 5×5 glyphs for champion name/stats text. */
static const uint8_t k_v22_font[64][5] = {
    {0x7E,0x41,0x41,0x41,0x7E},  /* 0 */
    {0x00,0x08,0x08,0x08,0x00},  /* 1 */
    {0x7E,0x01,0x7E,0x40,0x7E},  /* 2 */
    {0x7E,0x01,0x3E,0x01,0x7E},  /* 3 */
    {0x41,0x41,0x7F,0x01,0x01},  /* 4 */
    {0x7E,0x40,0x7E,0x01,0x7E},  /* 5 */
    {0x7E,0x40,0x7E,0x41,0x7E},  /* 6 */
    {0x7E,0x01,0x02,0x04,0x08},  /* 7 */
    {0x7E,0x41,0x7E,0x41,0x7E},  /* 8 */
    {0x7E,0x41,0x7E,0x01,0x7E},  /* 9 */
    {0x00,0x00,0x1F,0x00,0x00},  /* 10 dash */
    {0x00,0x00,0x00,0x00,0x08},  /* 11 period */
    {0x00,0x00,0x00,0x00,0x00},  /* 12 space */
    {0x00,0x02,0x00,0x02,0x00},  /* 13 comma */
    {0x00,0x14,0x00,0x00,0x00},  /* 14 colon */
    {0x3E,0x41,0x41,0x41,0x3E},  /* 15 A */
    {0x7F,0x49,0x49,0x49,0x36},  /* 16 B */
    {0x3E,0x41,0x41,0x41,0x22},  /* 17 C */
    {0x7F,0x41,0x41,0x41,0x3E},  /* 18 D */
    {0x7F,0x49,0x49,0x49,0x41},  /* 19 E */
    {0x7F,0x09,0x09,0x09,0x01},  /* 20 F */
    {0x3E,0x41,0x49,0x49,0x7A},  /* 21 G */
    {0x7F,0x08,0x08,0x08,0x7F},  /* 22 H */
    {0x00,0x41,0x7F,0x41,0x00},  /* 23 I */
    {0x20,0x40,0x41,0x3F,0x01},  /* 24 J */
    {0x7F,0x08,0x14,0x22,0x41},  /* 25 K */
    {0x7F,0x40,0x40,0x40,0x40},  /* 26 L */
    {0x7F,0x02,0x0C,0x02,0x7F},  /* 27 M */
    {0x7F,0x04,0x08,0x10,0x7F},  /* 28 N */
    {0x3E,0x41,0x41,0x41,0x3E},  /* 29 O */
    {0x7F,0x09,0x09,0x09,0x06},  /* 30 P */
    {0x3E,0x41,0x51,0x21,0x5E},  /* 31 Q */
    {0x7F,0x09,0x19,0x29,0x46},  /* 32 R */
    {0x46,0x49,0x49,0x49,0x31},  /* 33 S */
    {0x01,0x01,0x7F,0x01,0x01},  /* 34 T */
    {0x3F,0x40,0x40,0x40,0x3F},  /* 35 U */
    {0x1F,0x20,0x40,0x20,0x1F},  /* 36 V */
    {0x3F,0x40,0x38,0x40,0x3F},  /* 37 W */
    {0x63,0x14,0x08,0x14,0x63},  /* 38 X */
    {0x07,0x08,0x70,0x08,0x07},  /* 39 Y */
    {0x61,0x51,0x49,0x45,0x43},  /* 40 Z */
    {0x00,0x00,0x7F,0x41,0x00},  /* 41 [ */
    {0x02,0x04,0x08,0x10,0x20},  /* 42 backslash */
    {0x00,0x41,0x7F,0x00,0x00},  /* 43 ] */
    {0x06,0x08,0x08,0x08,0x06},  /* 44 ^ */
    {0x00,0x00,0x00,0x00,0x3F},  /* 45 underscore */
    {0x08,0x08,0x08,0x08,0x08},  /* 46 backtick */
    {0x3E,0x41,0x41,0x41,0x3E},  /* 47 a */
    {0x7F,0x49,0x49,0x49,0x36},  /* 48 b */
    {0x3E,0x41,0x41,0x41,0x22},  /* 49 c */
    {0x7F,0x41,0x41,0x41,0x3E},  /* 50 d */
    {0x7F,0x49,0x49,0x49,0x41},  /* 51 e */
    {0x7F,0x09,0x09,0x09,0x01},  /* 52 f */
    {0x3E,0x41,0x49,0x49,0x7A},  /* 53 g */
    {0x7F,0x08,0x08,0x08,0x7F},  /* 54 h */
    {0x00,0x41,0x7F,0x41,0x00},  /* 55 i */
    {0x20,0x40,0x41,0x3F,0x01},  /* 56 j */
    {0x7F,0x08,0x14,0x22,0x41},  /* 57 k */
    {0x7F,0x40,0x40,0x40,0x40},  /* 58 l */
    {0x7F,0x02,0x0C,0x02,0x7F},  /* 59 m */
    {0x7F,0x04,0x08,0x10,0x7F},  /* 60 n */
    {0x3E,0x41,0x41,0x41,0x3E},  /* 61 o */
    {0x7F,0x09,0x09,0x09,0x06},  /* 62 p */
    {0x3E,0x41,0x51,0x21,0x5E},  /* 63 q */
};

static int v22_font_glyph(int c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 15 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 47 + (c - 'a');
    if (c == '-') return 10;
    if (c == '.') return 11;
    if (c == ' ') return 12;
    if (c == ',') return 13;
    if (c == ':') return 14;
    return -1;
}

static void v22_draw_glyph(uint8_t* fb, int w, int h,
                            int px, int py, int glyph, uint8_t val) {
    if (glyph < 0 || glyph >= 64) return;
    for (int row = 0; row < 5; row++) {
        uint8_t bits = k_v22_font[glyph][row];
        for (int col = 0; col < 5; col++) {
            if (!(bits & (0x10 >> col))) continue;
            int x = px + col;
            int y = py + row;
            if (x >= 0 && x < w && y >= 0 && y < h) {
                fb[y * w + x] = val;
            }
        }
    }
}

static void v22_draw_text(uint8_t* fb, int w, int h,
                           int px, int py, const char* str, uint8_t val) {
    int x = px;
    while (*str) {
        int g = v22_font_glyph((unsigned char)*str);
        if (g >= 0) v22_draw_glyph(fb, w, h, x, py, g, val);
        x += 6;
        str++;
    }
}

static void v22_draw_bar(uint8_t* fb, int w, int h,
                          int bx, int by, int bw, int bh,
                          float fill_ratio, uint8_t low_val, uint8_t high_val) {
    if (bw <= 0 || bh <= 0) return;
    int fill = (int)((float)bw * fill_ratio);
    if (fill < 0) fill = 0;
    if (fill > bw) fill = bw;
    for (int row = 0; row < bh; row++) {
        for (int col = 0; col < bw; col++) {
            int x = bx + col;
            int y = by + row;
            if (x >= 0 && x < w && y >= 0 && y < h) {
                fb[y * w + x] = (col < fill) ? high_val : low_val;
            }
        }
    }
}

/* Draw one champion slot at panel-relative offset (sx, sy). */
static void v22_render_slot(uint8_t* fb, int w, int h,
                             int sx, int sy,
                             const char* name,
                             int hp_pct, int stam_pct, int mana_pct,
                             uint8_t base_val, uint8_t high_val) {
    /* Slot background */
    for (int dy = 0; dy < V22_PANEL_SLOT_H; dy++) {
        for (int dx = 0; dx < V22_PANEL_SLOT_W; dx++) {
            int px = sx + dx;
            int py = sy + dy;
            if (px >= 0 && px < w && py >= 0 && py < h) {
                fb[py * w + px] = base_val;
            }
        }
    }

    /* Portrait box — 28×46 px, left side of slot.
     * ReDMCSB: CHAMDRAW.C ~180 portrait dimensions; PANEL.C F0395 slot layout. */
    int port_x = sx + 2;
    int port_y = sy + 4;
    int port_w = 28;
    int port_h = 46;
    for (int dy = 0; dy < port_h; dy++) {
        for (int dx = 0; dx < port_w; dx++) {
            int px = port_x + dx;
            int py = port_y + dy;
            if (px >= 0 && px < w && py >= 0 && py < h) {
                int is_border = (dx == 0 || dx == port_w-1 || dy == 0 || dy == port_h-1);
                fb[py * w + px] = is_border ? high_val : (uint8_t)(base_val + 20);
            }
        }
    }

    /* Champion name at x+34, y+4 */
    int name_x = sx + 34;
    int name_y = sy + 4;
    v22_draw_text(fb, w, h, name_x, name_y, name, high_val);

    /* HP bar — ReDMCSB: STATS.C F0090-F0092; PANEL.C F0395 bar positions */
    int bar_x = name_x;
    int bar_y = sy + 20;
    v22_draw_bar(fb, w, h, bar_x, bar_y, 36, 4,
                  hp_pct / 100.0f, base_val, high_val);

    /* Stamina bar — 8px below HP */
    v22_draw_bar(fb, w, h, bar_x, bar_y + 8, 36, 4,
                  stam_pct / 100.0f, base_val, (uint8_t)(high_val * 3 / 4));

    /* Mana bar — 8px below stamina */
    v22_draw_bar(fb, w, h, bar_x, bar_y + 16, 36, 4,
                  mana_pct / 100.0f, base_val, (uint8_t)(high_val * 2 / 3));

    /* Hand icon box — rightmost 14px of slot.
     * ReDMCSB: COMMAND.C:484-497 action hand subroutes. */
    int hand_x = sx + V22_PANEL_SLOT_W - 14;
    int hand_y = sy + 4;
    for (int dy = 0; dy < 14; dy++) {
        for (int dx = 0; dx < 14; dx++) {
            int px = hand_x + dx;
            int py = hand_y + dy;
            if (px >= 0 && px < w && py >= 0 && py < h) {
                int is_border = (dx == 0 || dx == 13 || dy == 0 || dy == 13);
                fb[py * w + px] = is_border ? high_val : base_val;
            }
        }
    }
}

void v22_hud_render_champion_panel(uint8_t* fb, int w, int h,
                                    int champion_hp_pct[4],
                                    int champion_stam_pct[4],
                                    int champion_mana_pct[4]) {
    if (!fb || w <= 0 || h <= 0) return;
    if (!g_v2_hud_state.visible) return;

    uint8_t alpha = g_v2_hud_state.opacity;
    uint8_t base_val = (alpha > 0) ? (alpha / 2) : 0;
    uint8_t high_val = (alpha > 0) ? alpha : 0;

    const char* names[4] = {"Warrior", "Mage", "Merlin", "Ranger"};

    for (int i = 0; i < 4; i++) {
        int sx = i * V22_PANEL_SLOT_W;
        int sy = V22_PANEL_Y;
        int hp = champion_hp_pct ? champion_hp_pct[i] : 75;
        int stam = champion_stam_pct ? champion_stam_pct[i] : 80;
        int mana = champion_mana_pct ? champion_mana_pct[i] : 60;
        v22_render_slot(fb, w, h, sx, sy, names[i], hp, stam, mana, base_val, high_val);
    }
}
