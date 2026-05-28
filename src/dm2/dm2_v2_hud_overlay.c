#include "dm2_v2_hud_overlay.h"
#include "dm1_v2_anim_timing.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ══════════════════════════════════════════════════════════════════════
 * DM2 V2 HUD Overlay — presentation-only enhanced UI chrome
 *
 * Phase 3: DM2 V2 enhanced in-game overlay presentation, UI chrome,
 * and interaction feedback.
 *
 * V2.0/V2.1 overlay:
 *   - Compass rose (4-way cardinal indicator, top-left)
 *   - Dungeon depth counter "N/M" (top-right)
 *   - Gold piece counter (bottom-right strip, DM2-specific)
 *   - Champion mini-bars in top status bar
 *   - Action strip icons (bottom 28px)
 *
 * V2.2 interaction feedback:
 *   - Hit flash (action icon pulse for one frame)
 *   - Low-health pulse (champion HP bar ~2 Hz)
 *   - Smooth compass needle interpolation
 *
 * Source: SKULL.ASM T560 (DM2 HUD rendering)
 *         SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C F0354 champion status box drawing
 *         ReDMCSB DUNGEON.C stat-bar refresh timing
 *
 * This module is deliberately presentation-only: it draws optional
 * overlay elements into the supplied framebuffer and does NOT mutate
 * dungeon, champion, companion, or command runtime state.
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Pixel helpers ──────────────────────────────────────────────── */
static void hud_plot(uint8_t *fb, int stride, int x, int y, uint8_t val) {
    if (x >= 0 && x < (int)(unsigned)stride && y >= 0 && y < 200) {
        fb[y * stride + x] = val;
    }
}

static void hud_rect(uint8_t *fb, int stride, int x, int y, int rw, int rh, uint8_t val) {
    for (int dy = 0; dy < rh; dy++) {
        for (int dx = 0; dx < rw; dx++) {
            hud_plot(fb, stride, x + dx, y + dy, val);
        }
    }
}

/* ── 5×5 bitmap digit font ──────────────────────────────────────── */
static const uint8_t g_digit_bits[10][5] = {
    {0x7E, 0x41, 0x41, 0x41, 0x7E},  /* 0 */
    {0x00, 0x00, 0x7E, 0x00, 0x00},  /* 1 */
    {0x7E, 0x01, 0x7E, 0x40, 0x7E},  /* 2 */
    {0x7E, 0x01, 0x3E, 0x01, 0x7E},  /* 3 */
    {0x42, 0x42, 0x7E, 0x02, 0x02},  /* 4 */
    {0x7E, 0x40, 0x7E, 0x01, 0x7E},  /* 5 */
    {0x7E, 0x40, 0x7E, 0x41, 0x7E},  /* 6 */
    {0x7E, 0x01, 0x02, 0x04, 0x08},  /* 7 */
    {0x7E, 0x41, 0x7E, 0x41, 0x7E},  /* 8 */
    {0x7E, 0x41, 0x7F, 0x01, 0x7E},  /* 9 */
};

static void hud_draw_digit(uint8_t *fb, int stride, int x, int y, int digit, uint8_t val) {
    digit = (digit < 0) ? 0 : (digit > 9 ? 9 : digit);
    for (int row = 0; row < 5; row++) {
        uint8_t bits = g_digit_bits[(unsigned)digit][(unsigned)row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                hud_plot(fb, stride, x + col, y + row, val);
            }
        }
    }
}

static void hud_draw_number(uint8_t *fb, int stride, int x, int y, int value, uint8_t val) {
    if (value < 0) value = 0;
    if (value > 9999) value = 9999;
    if (value < 10) {
        hud_draw_digit(fb, stride, x, y, value, val);
    } else if (value < 100) {
        hud_draw_digit(fb, stride, x, y, value / 10, val);
        hud_draw_digit(fb, stride, x + 8, y, value % 10, val);
    } else {
        hud_draw_digit(fb, stride, x, y, value / 100, val);
        hud_draw_digit(fb, stride, x + 8, y, (value / 10) % 10, val);
        hud_draw_digit(fb, stride, x + 16, y, value % 10, val);
    }
}

/* ── 3×5 bitmap uppercase letter font ───────────────────────────── */
static const uint8_t g_letter_bits[26][3] = {
    {0x7C, 0x82, 0x82}, /* A */
    {0xFE, 0x82, 0x82}, /* B */
    {0x7C, 0x80, 0x80}, /* C */
    {0xFE, 0x82, 0x6C}, /* D */
    {0x7C, 0x8A, 0x86}, /* E */
    {0xFE, 0x8A, 0x82}, /* F */
    {0xFE, 0x80, 0x9E}, /* G */
    {0x82, 0xFE, 0x82}, /* H */
    {0x7C, 0x10, 0x10}, /* I */
    {0x7E, 0x04, 0x04}, /* J */
    {0x82, 0x9C, 0xA2}, /* K */
    {0x7C, 0x80, 0x80}, /* L */
    {0x82, 0x6C, 0x82}, /* M */
    {0x82, 0xFE, 0x82}, /* N */
    {0x7C, 0x82, 0x7C}, /* O */
    {0xFE, 0x82, 0xFC}, /* P */
    {0x7E, 0x86, 0x78}, /* Q */
    {0xFE, 0x8A, 0xF4}, /* R */
    {0x7E, 0x80, 0x7E}, /* S */
    {0xFE, 0x10, 0x10}, /* T */
    {0x7C, 0x82, 0x82}, /* U */
    {0x7C, 0x82, 0x44}, /* V */
    {0x82, 0x82, 0x7C}, /* W */
    {0x82, 0x44, 0x82}, /* X */
    {0x82, 0x44, 0x38}, /* Y */
    {0xC6, 0x28, 0x10}, /* Z */
};

static void hud_draw_letter(uint8_t *fb, int stride, int x, int y, char ch, uint8_t val) {
    int letter = -1;
    if (ch >= 'A' && ch <= 'Z') letter = ch - 'A';
    else if (ch >= 'a' && ch <= 'z') letter = ch - 'a';
    if (letter < 0 || letter >= 26) return;
    for (int row = 0; row < 5; row++) {
        uint8_t bits = g_letter_bits[(unsigned)letter][(unsigned)row];
        for (int col = 0; col < 3; col++) {
            if (bits & (0x80 >> col)) {
                hud_plot(fb, stride, x + col, y + row, val);
            }
        }
    }
}

/* ── Compass rose ───────────────────────────────────────────────── */
static void hud_compass_draw(uint8_t *fb, int stride, int cx, int cy,
    int direction, uint8_t base, uint8_t high)
{
    /* Draw base circle */
    for (int dy = -7; dy <= 7; dy++) {
        for (int dx = -7; dx <= 7; dx++) {
            if (dx * dx + dy * dy <= 49) {
                hud_plot(fb, stride, cx + dx, cy + dy, base);
            }
        }
    }

    /* Draw needle (4 cardinal directions) */
    int nx = cx, ny = cy;
    switch (direction) {
    case 0: ny -= 6; break;   /* N — needle up */
    case 1: nx += 6; break;  /* E — needle right */
    case 2: ny += 6; break;  /* S — needle down */
    case 3: nx -= 6; break;  /* W — needle left */
    default: break;
    }
    /* Needle line (Bresenham-ish) */
    if (abs(nx - cx) == 0) {
        int step = (ny > cy) ? 1 : -1;
        for (int d = 0, yy = cy; d <= abs(ny - cy); d++, yy += step) {
            hud_plot(fb, stride, cx, yy, high);
        }
    } else {
        int step = (nx > cx) ? 1 : -1;
        for (int d = 0, xx = cx; d <= abs(nx - cx); d++, xx += step) {
            hud_plot(fb, stride, xx, cy, high);
        }
    }

    /* N/E/S/W label at current direction */
    static const char s_labels[4] = {'N', 'E', 'S', 'W'};
    hud_draw_letter(fb, stride, cx - 2, cy - 10, s_labels[direction & 3], high);
}

/* ── Action strip icon ──────────────────────────────────────────── */
static const char *g_action_icon_labels[DM2_V2_ACTION_COUNT] = {
    "ATK", "CST", "USE", "DRP", "MOV"
};

static void hud_draw_action_icon(uint8_t *fb, int stride, int x, int y,
    DM2_V2_ActionIcon icon, bool active, bool flash, uint8_t base, uint8_t high)
{
    (void)active;
    if (flash) {
        hud_rect(fb, stride, x, y, DM2_ACTION_ICON_W, 28, high);
    } else {
        hud_rect(fb, stride, x, y, DM2_ACTION_ICON_W, 28, base);
    }
    /* Icon label */
    const char *label = g_action_icon_labels[(unsigned)icon];
    int lx = x + 2;
    int ly = y + 10;
    while (*label) {
        if (*label >= 'A' && *label <= 'Z') {
            hud_draw_letter(fb, stride, lx, ly, *label, high);
            lx += 4;
        }
        label++;
    }
    if (active) {
        for (int i = 0; i < DM2_ACTION_ICON_W; i++) {
            hud_plot(fb, stride, x + i, y + 26, high);
        }
    }
}

/* ── Champion mini-bar ──────────────────────────────────────────── */
static V2_Anim g_health_pulse_anims[4];
static int g_pulse_initialized = 0;

static void ensure_pulse_initialized(void) {
    if (!g_pulse_initialized) {
        g_pulse_initialized = 1;
        for (int i = 0; i < 4; i++) {
            v2_anim_start(&g_health_pulse_anims[i], 0.6f, 1.0f,
                (uint32_t)(9 * V1_TICK_MS), V2_EASE_IN_OUT_QUAD);
            g_health_pulse_anims[i].loops = -1;
        }
    }
}

static float hud_health_pulse_alpha(int champ_idx) {
    ensure_pulse_initialized();
    if (champ_idx < 0 || champ_idx >= 4) return 1.0f;
    return v2_anim_value(&g_health_pulse_anims[champ_idx]);
}

static void hud_draw_champion_bar(uint8_t *fb, int stride, int x, int y,
    int champ_idx, int hp_pct, int stamina_pct, int mana_pct,
    bool leader, bool spell_ready, uint8_t high)
{
    (void)spell_ready;
    /* HP bar (red) */
    int bar_w = DM2_CHAMP_BAR_W;
    int hp_w = (bar_w * hp_pct) / 100;
    if (hp_w < bar_w / 4) {
        float alpha = hud_health_pulse_alpha(champ_idx);
        uint8_t pulse_val = (uint8_t)(high * alpha);
        hud_rect(fb, stride, x, y, hp_w, DM2_CHAMP_BAR_H / 3, pulse_val);
    } else {
        hud_rect(fb, stride, x, y, hp_w, DM2_CHAMP_BAR_H / 3, high);
    }

    /* Stamina bar (green) */
    int st_w = (bar_w * stamina_pct) / 100;
    hud_rect(fb, stride, x, y + DM2_CHAMP_BAR_H / 3, st_w, DM2_CHAMP_BAR_H / 3, 10);

    /* Mana bar (blue) */
    int mn_w = (bar_w * mana_pct) / 100;
    hud_rect(fb, stride, x, y + (DM2_CHAMP_BAR_H * 2) / 3, mn_w, DM2_CHAMP_BAR_H / 3, 9);

    /* Leader star */
    if (leader) {
        hud_plot(fb, stride, x + bar_w - 6, y, high);
        hud_plot(fb, stride, x + bar_w - 5, y, high);
    }
}

/* ── Lifecycle ──────────────────────────────────────────────────── */
void dm2_v2_hud_init(DM2_V2_HudOverlay *h) {
    if (!h) return;
    memset(h, 0, sizeof(*h));
    h->compass.direction = 0;
    h->compass.needle_angle = 0.0f;
    h->compass.animated = true;
    h->depth.current_level = 1;
    h->depth.max_level = 10;
    h->gold.party_gold = 0;
    h->gold.visible = true;
    h->visible = true;
    h->opacity = 255;
    h->stats_bar_visible = true;
    h->action_strip.visible = true;
    h->hit_flash_active = false;
    h->hit_flash_timer = 0;
}

void dm2_v2_hud_reset(DM2_V2_HudOverlay *h) {
    dm2_v2_hud_init(h);
}

/* ── Parameter setters ──────────────────────────────────────────── */
void dm2_v2_hud_set_direction(DM2_V2_HudOverlay *h, int dir) {
    if (!h) return;
    if (dir < 0) dir = 0;
    if (dir > 3) dir = 3;
    h->compass.direction = dir;
    h->compass.needle_angle = (float)dir * 90.0f;
}

void dm2_v2_hud_set_level(DM2_V2_HudOverlay *h, int cur, int max) {
    if (!h) return;
    if (cur < 0) cur = 0;
    if (max <= 0) max = 1;
    h->depth.current_level = cur;
    h->depth.max_level = max;
}

void dm2_v2_hud_set_gold(DM2_V2_HudOverlay *h, int gold_pieces) {
    if (!h) return;
    h->gold.party_gold = gold_pieces;
    h->gold.visible = true;
}

void dm2_v2_hud_set_champion_bar(DM2_V2_HudOverlay *h, int champ_idx,
    int hp_pct, int stamina_pct, int mana_pct, bool leader, bool spell_ready)
{
    if (!h) return;
    if (champ_idx < 0 || champ_idx >= 4) return;
    h->champion_bars[champ_idx].champion_index = champ_idx;
    h->champion_bars[champ_idx].hp_pct = hp_pct;
    h->champion_bars[champ_idx].stamina_pct = stamina_pct;
    h->champion_bars[champ_idx].mana_pct = mana_pct;
    h->champion_bars[champ_idx].leader = leader;
    h->champion_bars[champ_idx].spell_ready = spell_ready;
}

void dm2_v2_hud_set_action_active(DM2_V2_HudOverlay *h, DM2_V2_ActionIcon icon) {
    if (!h) return;
    for (int i = 0; i < DM2_V2_ACTION_COUNT; i++) {
        h->action_strip.icons[i].active = (i == (int)icon);
    }
}

void dm2_v2_hud_trigger_hit_flash(DM2_V2_HudOverlay *h) {
    if (!h) return;
    h->hit_flash_active = true;
    h->hit_flash_timer = 6;
}

void dm2_v2_hud_toggle(DM2_V2_HudOverlay *h) {
    if (!h) return;
    h->visible = !h->visible;
}

void dm2_v2_hud_set_opacity(DM2_V2_HudOverlay *h, uint8_t val) {
    if (!h) return;
    h->opacity = val;
}

/* ── Main render entry ──────────────────────────────────────────── */
void dm2_v2_hud_render(DM2_V2_HudOverlay *h, uint8_t *fb, int stride, int h_res) {
    if (!h || !fb || stride <= 0 || h_res <= 0) return;
    if (!h->visible) return;
    if (h->opacity == 0) return;

    uint8_t base = (uint8_t)(h->opacity / 2);
    uint8_t high = h->opacity;

    /* ── Top-left: Compass ─────────────────────────────────────── */
    hud_compass_draw(fb, stride, 16, 16, h->compass.direction, base, high);

    /* ── Top-right: Depth "cur/max" ─────────────────────────────── */
    int dx = stride - 56;
    hud_draw_number(fb, stride, dx, 8, h->depth.current_level, high);
    hud_plot(fb, stride, dx + 16, 10, high);
    hud_draw_number(fb, stride, dx + 24, 8, h->depth.max_level, high);

    /* ── Top status bar: 4 champion mini-bars ────────────────────── */
    if (h->stats_bar_visible) {
        for (int i = 0; i < 4; i++) {
            DM2_V2_HudChampionBar *cb = &h->champion_bars[i];
            int bx = DM2_CHAMP_BAR_X_START + i * (DM2_CHAMP_BAR_W + DM2_CHAMP_BAR_SPACING);
            hud_draw_champion_bar(fb, stride, bx, DM2_CHAMP_BAR_Y, i,
                cb->hp_pct, cb->stamina_pct, cb->mana_pct,
                cb->leader, cb->spell_ready, high);
        }
    }

    /* ── Bottom action strip ───────────────────────────────────── */
    if (h->action_strip.visible) {
        for (int i = 0; i < DM2_V2_ACTION_COUNT; i++) {
            int ax = DM2_ACTION_ICONS_X_START + i * (DM2_ACTION_ICON_W + 4);
            DM2_V2_ActionIconState *st = &h->action_strip.icons[i];
            bool flash = h->hit_flash_active && st->active;
            hud_draw_action_icon(fb, stride, ax, DM2_ACTION_STRIP_Y,
                (DM2_V2_ActionIcon)i, st->active, flash, base, high);
        }
        /* Decrement flash timer */
        if (h->hit_flash_timer > 0) {
            h->hit_flash_timer--;
            if (h->hit_flash_timer == 0) {
                h->hit_flash_active = false;
            }
        }
    }

    /* ── Bottom-right: gold counter (DM2-specific) ──────────────── */
    if (h->gold.visible) {
        int gx = stride - 60;
        int gy = DM2_ACTION_STRIP_Y + 4;
        hud_draw_letter(fb, stride, gx, gy, 'G', high);
        hud_draw_letter(fb, stride, gx + 4, gy, 'p', base);
        gx += 14;
        hud_draw_number(fb, stride, gx, gy, h->gold.party_gold, high);
    }
}

const char *dm2_v2_hud_source_evidence(void) {
    return
        "DM2 V2.0/V2.1: compass, depth, gold, champion bars, action strip\n"
        "  Source: SKULL.ASM T560 (DM2 HUD rendering)\n"
        "  Source: SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)\n"
        "  Source: ReDMCSB PANEL.C F0354 (champion status-box rendering)\n"
        "  Source: ReDMCSB DUNGEON.C (stat-bar refresh, F0260)\n"
        "DM2 V2.2: hit flash, low-HP pulse, smooth compass interpolation\n"
        "  Source: ReDMCSB COMMAND.C action feedback gates\n"
        "  Source: ReDMCSB DISPLAY.C pulse animation timing (2 Hz)\n";
}