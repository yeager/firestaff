/*
 * theron_v2_hud_overlay_pc34.c — Theron V2 Phase 3 HUD Overlay
 *
 * Phase 3 (initial seed): presentation-only enhanced UI chrome for
 * Theron's Quest V2 (V2.0 / V2.1 / V2.2).  Mirrors
 *   csb_v2_hud_overlay_pc34.c
 *   dm2_v2_hud_overlay.c
 * with Theron-specific surfaces:
 *
 *   V2.0/V2.1 (data-free):
 *     - Compass rose (top-bar)
 *     - Quest item counter (top-bar)
 *     - Dungeon progression "N/7" (top-bar)
 *     - Relic counter "R/7" (top-bar)
 *     - Spell-rune ready indicator (top-bar)
 *     - 4 champion mini-bars (bottom panel)
 *     - Action strip icons (bottom strip)
 *
 *   V2.2 interaction feedback (data-free):
 *     - Hit flash (action icon pulse for one frame)
 *     - Low-health pulse (champion HP bar ~2 Hz)
 *     - Smooth compass needle interpolation
 *
 * This module is deliberately presentation-only: it draws optional
 * overlay elements into the supplied 256x224 indexed framebuffer and
 * does NOT mutate dungeon, champion, companion, world, command, save,
 * or Track 02 bank state.
 *
 * Source-lock anchors:
 *   THQUEST.ASM T520  party placement / start position
 *   THQUEST.ASM T560  dungeon loading
 *   THQUEST.ASM T600  UI overlay zones (top-bar / right / bottom)
 *   THQUEST.ASM T700  timers / world tick
 *   THQUEST.ASM T800  champion persistence + inventory reset
 *   THQUEST.ASM T900  object database / rune magic
 *   HuC6260 / HuC6270 datasheet (PC Engine VDC + VCE)
 *   ReDMCSB PANEL.C F0354 champion status box (sibling pattern)
 *   ReDMCSB DUNGEON.C F0260 stat-bar refresh (sibling pattern)
 *   dmweb Theron overview (7 dungeons + 7 relic goals + rune magic)
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 */

#include "theron_v2_hud_overlay_pc34.h"
#include "theron_v1_viewport.h"     /* TQR_FB_W, TQR_FB_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ══════════════════════════════════════════════════════════════════════
 * Pixel helpers
 *
 * PC Engine native framebuffer is 256x224 indexed (one byte per pixel,
 * palette index 0..255).  These helpers bounds-check against the
 * framebuffer dimensions passed by the caller (so the module works
 * with both 256x224 and the M11 320x200 letterbox).
 * ══════════════════════════════════════════════════════════════════════ */

static void hud_plot(uint8_t *fb, int stride, int w, int h_res, int x, int y, uint8_t val)
{
    if (x >= 0 && x < w && y >= 0 && y < h_res && stride > 0) {
        fb[y * (size_t)stride + x] = val;
    }
}

static void hud_rect(uint8_t *fb, int stride, int w, int h_res,
                     int x, int y, int rw, int rh, uint8_t val)
{
    for (int dy = 0; dy < rh; dy++) {
        for (int dx = 0; dx < rw; dx++) {
            hud_plot(fb, stride, w, h_res, x + dx, y + dy, val);
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * 5x8 bitmap digit font (slightly taller than DM2's 5x5 so digits
 * are readable at the PC Engine's 256x224 native resolution, where
 * the top-bar zone is only 24px tall).
 * ══════════════════════════════════════════════════════════════════════ */

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

static void hud_draw_digit(uint8_t *fb, int stride, int w, int h_res,
                           int x, int y, int digit, uint8_t val)
{
    digit = (digit < 0) ? 0 : (digit > 9 ? 9 : digit);
    for (int row = 0; row < 5; row++) {
        uint8_t bits = g_digit_bits[(unsigned)digit][(unsigned)row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80u >> col)) {
                hud_plot(fb, stride, w, h_res, x + col, y + row, val);
            }
        }
    }
}

static void hud_draw_number(uint8_t *fb, int stride, int w, int h_res,
                            int x, int y, int value, uint8_t val)
{
    if (value < 0) value = 0;
    if (value > 9999) value = 9999;
    if (value < 10) {
        hud_draw_digit(fb, stride, w, h_res, x, y, value, val);
    } else if (value < 100) {
        hud_draw_digit(fb, stride, w, h_res, x, y, value / 10, val);
        hud_draw_digit(fb, stride, w, h_res, x + 8, y, value % 10, val);
    } else {
        hud_draw_digit(fb, stride, w, h_res, x, y, value / 100, val);
        hud_draw_digit(fb, stride, w, h_res, x + 8, y, (value / 10) % 10, val);
        hud_draw_digit(fb, stride, w, h_res, x + 16, y, value % 10, val);
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * 3x5 bitmap uppercase letter font (used for compass labels + action
 * strip icon labels + top-bar zone labels)
 * ══════════════════════════════════════════════════════════════════════ */

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

static void hud_draw_letter(uint8_t *fb, int stride, int w, int h_res,
                            int x, int y, char ch, uint8_t val)
{
    int letter = -1;
    if (ch >= 'A' && ch <= 'Z') letter = ch - 'A';
    else if (ch >= 'a' && ch <= 'z') letter = ch - 'a';
    if (letter < 0 || letter >= 26) return;
    for (int row = 0; row < 5; row++) {
        uint8_t bits = g_letter_bits[(unsigned)letter][(unsigned)row];
        for (int col = 0; col < 3; col++) {
            if (bits & (0x80u >> col)) {
                hud_plot(fb, stride, w, h_res, x + col, y + row, val);
            }
        }
    }
}

static void hud_draw_string(uint8_t *fb, int stride, int w, int h_res,
                            int x, int y, const char *s, uint8_t val)
{
    int lx = x;
    while (*s) {
        if (*s == ' ') {
            lx += 4;
        } else if ((*s >= 'A' && *s <= 'Z') || (*s >= 'a' && *s <= 'z')) {
            hud_draw_letter(fb, stride, w, h_res, lx, y, *s, val);
            lx += 4;
        } else if (*s >= '0' && *s <= '9') {
            hud_draw_digit(fb, stride, w, h_res, lx, y, (*s - '0'), val);
            lx += 8;
        } else if (*s == '/') {
            /* Slash: two pixels diagonal at this small scale. */
            hud_plot(fb, stride, w, h_res, lx + 2, y + 0, val);
            hud_plot(fb, stride, w, h_res, lx + 1, y + 1, val);
            hud_plot(fb, stride, w, h_res, lx + 0, y + 2, val);
            lx += 4;
        } else if (*s == ':') {
            hud_plot(fb, stride, w, h_res, lx, y + 1, val);
            hud_plot(fb, stride, w, h_res, lx, y + 3, val);
            lx += 2;
        }
        s++;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Compass rose (top-bar, top-left)
 * Source: THQUEST.ASM T600 (UI overlay zones)
 *         HuC6260 VDC layout (256x224 indexed framebuffer)
 * ══════════════════════════════════════════════════════════════════════ */

static void hud_compass_draw(uint8_t *fb, int stride, int w, int h_res,
                             int cx, int cy, int direction,
                             uint8_t base, uint8_t high)
{
    /* Base ring (radius ~7, drawn as filled disc) */
    for (int dy = -7; dy <= 7; dy++) {
        for (int dx = -7; dx <= 7; dx++) {
            if (dx * dx + dy * dy <= 49) {
                hud_plot(fb, stride, w, h_res, cx + dx, cy + dy, base);
            }
        }
    }

    /* Needle: line from center to one of the 4 cardinal points. */
    int nx = cx, ny = cy;
    switch (direction & 3) {
    case 0: ny -= 6; break;  /* N — needle up */
    case 1: nx += 6; break;  /* E — needle right */
    case 2: ny += 6; break;  /* S — needle down */
    case 3: nx -= 6; break;  /* W — needle left */
    default: break;
    }
    if (nx == cx) {
        int step = (ny > cy) ? 1 : -1;
        for (int yy = cy; yy != ny + step; yy += step) {
            hud_plot(fb, stride, w, h_res, cx, yy, high);
        }
    } else {
        int step = (nx > cx) ? 1 : -1;
        for (int xx = cx; xx != nx + step; xx += step) {
            hud_plot(fb, stride, w, h_res, xx, cy, high);
        }
    }

    /* N/E/S/W label above the ring */
    static const char s_labels[4] = {'N', 'E', 'S', 'W'};
    hud_draw_letter(fb, stride, w, h_res, cx - 2, cy - 9, s_labels[direction & 3], high);
}

/* ══════════════════════════════════════════════════════════════════════
 * Spell-rune ready indicator
 * Source: THQUEST.ASM T900 (object database / rune magic)
 *
 * Visual:
 *   - Rune ready  -> 4 small filled squares in a row, the rune_index
 *     square highlighted in the high color.
 *   - No rune ready -> empty (no squares drawn).
 * ══════════════════════════════════════════════════════════════════════ */

static void hud_rune_indicator_draw(uint8_t *fb, int stride, int w, int h_res,
                                    int x, int y,
                                    bool rune_ready, bool spell_charging,
                                    int rune_index, uint8_t base, uint8_t high)
{
    if (!rune_ready && !spell_charging) return;
    /* 4 rune slots, each 4x4 px with 2 px gap */
    for (int i = 0; i < 4; i++) {
        int rx = x + i * 6;
        if (i == rune_index && spell_charging) {
            hud_rect(fb, stride, w, h_res, rx, y, 4, 4, high);
        } else {
            hud_rect(fb, stride, w, h_res, rx, y, 4, 4, base);
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Action strip icon
 * Source: THQUEST.ASM T600 (UI overlay zones), dm2_v2_hud_overlay.c
 * (sibling pattern)
 * ══════════════════════════════════════════════════════════════════════ */

static const char *g_action_icon_labels[THERON_V2_ACTION_COUNT] = {
    "ATK", "CST", "USE", "DRP", "MOV"
};

static void hud_draw_action_icon(uint8_t *fb, int stride, int w, int h_res,
                                 int x, int y,
                                 Theron_V2_ActionIcon icon, bool active, bool flash,
                                 uint8_t base, uint8_t high)
{
    (void)active;
    if (flash) {
        hud_rect(fb, stride, w, h_res, x, y, THERON_V2_ACTION_ICON_W, THERON_V2_ACTION_STRIP_H, high);
    } else {
        hud_rect(fb, stride, w, h_res, x, y, THERON_V2_ACTION_ICON_W, THERON_V2_ACTION_STRIP_H, base);
    }
    /* Icon label */
    const char *label = g_action_icon_labels[(unsigned)icon];
    int lx = x + 4;
    int ly = y + 4;
    while (*label) {
        if (*label >= 'A' && *label <= 'Z') {
            hud_draw_letter(fb, stride, w, h_res, lx, ly, *label, high);
            lx += 4;
        }
        label++;
    }
    if (active) {
        /* Active underline strip across the bottom of the icon. */
        for (int i = 0; i < THERON_V2_ACTION_ICON_W; i++) {
            hud_plot(fb, stride, w, h_res,
                     x + i, y + THERON_V2_ACTION_STRIP_H - 1, high);
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Champion mini-bar (HP / Stamina / Mana, three horizontal strips)
 *
 * Color choice (PC Engine 16-color palette, palette indices match the
 * V1 chrome palette constants TR_CHROME_HP=8, TR_CHROME_STAMINA=10,
 * TR_CHROME_MANA=14 from theron_v1_ui_chrome.h):
 *   HP      -> palette index 8  (red)
 *   Stamina -> palette index 10 (tan)
 *   Mana    -> palette index 14 (blue)
 * ══════════════════════════════════════════════════════════════════════ */

#define THERON_V2_CHROME_HP         8
#define THERON_V2_CHROME_STAMINA   10
#define THERON_V2_CHROME_MANA      14
#define THERON_V2_CHROME_BASE       1
#define THERON_V2_CHROME_HIGHLIGHT 15

/* Simple 2 Hz low-HP pulse: each render toggles the pulse phase for
 * champions whose HP < 25%.  We deliberately do not import
 * dm1_v2_anim_timing.h to keep this module self-contained; the
 * sibling CSB / DM2 HUD modules also use a state-local pulse. */
static uint8_t s_low_hp_pulse_phase = 0;

static uint8_t hud_low_hp_pulse_alpha(void)
{
    /* Square wave at ~2 Hz: high on phase 0, low on phase 1. */
    s_low_hp_pulse_phase = (s_low_hp_pulse_phase + 1) & 1;
    return (s_low_hp_pulse_phase == 0) ? 255 : 64;
}

static void hud_draw_champion_bar(uint8_t *fb, int stride, int w, int h_res,
                                  int x, int y,
                                  int champ_idx, int hp_pct, int stamina_pct, int mana_pct,
                                  bool leader, bool spell_ready,
                                  uint8_t high)
{
    (void)champ_idx;
    (void)spell_ready;
    int bar_w = THERON_V2_CHAMP_BAR_W;

    /* HP bar (red) */
    int hp_w = (bar_w * hp_pct) / 100;
    if (hp_pct < 25 && hp_pct > 0) {
        uint8_t pulse = hud_low_hp_pulse_alpha();
        hud_rect(fb, stride, w, h_res, x, y,
                 hp_w, THERON_V2_CHAMP_BAR_H / 3,
                 (pulse == 255) ? THERON_V2_CHROME_HP : high);
    } else {
        hud_rect(fb, stride, w, h_res, x, y,
                 hp_w, THERON_V2_CHAMP_BAR_H / 3,
                 THERON_V2_CHROME_HP);
    }

    /* Stamina bar (tan) */
    int st_w = (bar_w * stamina_pct) / 100;
    hud_rect(fb, stride, w, h_res, x, y + THERON_V2_CHAMP_BAR_H / 3,
             st_w, THERON_V2_CHAMP_BAR_H / 3, THERON_V2_CHROME_STAMINA);

    /* Mana bar (blue) */
    int mn_w = (bar_w * mana_pct) / 100;
    hud_rect(fb, stride, w, h_res, x, y + (THERON_V2_CHAMP_BAR_H * 2) / 3,
             mn_w, THERON_V2_CHAMP_BAR_H / 3, THERON_V2_CHROME_MANA);

    /* Leader star (small filled square at top-right). */
    if (leader) {
        hud_rect(fb, stride, w, h_res,
                 x + bar_w - 5, y, 3, 3, high);
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Lifecycle
 * ══════════════════════════════════════════════════════════════════════ */

void theron_v2_hud_init(Theron_V2_HudOverlay *h)
{
    if (!h) return;
    memset(h, 0, sizeof(*h));
    h->compass.direction = 0;
    h->compass.needle_angle = 0.0f;
    h->compass.animated = true;
    h->quest_items.collected = 0;
    h->quest_items.total = 0;
    h->quest_items.visible = true;
    h->dungeon_progress.current_dungeon = 1;
    h->dungeon_progress.total_dungeons = 7;  /* Theron fixed, DMWeb */
    h->relic_counter.relics_found = 0;
    h->relic_counter.relics_required = 7;    /* Theron fixed, DMWeb */
    h->relic_counter.visible = true;
    h->rune_indicator.rune_ready = false;
    h->rune_indicator.spell_charging = false;
    h->rune_indicator.rune_index = -1;
    h->rune_indicator.visible = true;
    h->action_strip.visible = true;
    h->visible = true;
    h->opacity = 255;
    h->stats_bar_visible = true;
    h->top_bar_visible = true;
    h->hit_flash_active = false;
    h->hit_flash_timer = 0;
    /* Per-champion default: dead / empty until setter is called. */
    for (int i = 0; i < 4; i++) {
        h->champion_bars[i].champion_index = i;
        h->champion_bars[i].hp_pct = 0;
        h->champion_bars[i].stamina_pct = 0;
        h->champion_bars[i].mana_pct = 0;
        h->champion_bars[i].leader = (i == 0);  /* slot 0 = Theron / leader */
        h->champion_bars[i].spell_ready = false;
    }
}

void theron_v2_hud_reset(Theron_V2_HudOverlay *h)
{
    theron_v2_hud_init(h);
}

/* ══════════════════════════════════════════════════════════════════════
 * Parameter setters
 * ══════════════════════════════════════════════════════════════════════ */

void theron_v2_hud_set_direction(Theron_V2_HudOverlay *h, int dir)
{
    if (!h) return;
    if (dir < 0) dir = 0;
    if (dir > 3) dir = 3;
    h->compass.direction = dir;
    h->compass.needle_angle = (float)dir * 90.0f;
}

void theron_v2_hud_set_quest_items(Theron_V2_HudOverlay *h, int collected, int total)
{
    if (!h) return;
    if (collected < 0) collected = 0;
    if (total < 0) total = 0;
    if (total > 9999) total = 9999;
    h->quest_items.collected = collected;
    h->quest_items.total = total;
    h->quest_items.visible = true;
}

void theron_v2_hud_set_dungeon_progress(Theron_V2_HudOverlay *h,
                                        int current_dungeon, int total_dungeons)
{
    if (!h) return;
    if (current_dungeon < 0) current_dungeon = 0;
    if (total_dungeons <= 0) total_dungeons = 1;
    h->dungeon_progress.current_dungeon = current_dungeon;
    h->dungeon_progress.total_dungeons = total_dungeons;
}

void theron_v2_hud_set_relics(Theron_V2_HudOverlay *h, int found, int required)
{
    if (!h) return;
    if (found < 0) found = 0;
    if (required < 0) required = 0;
    h->relic_counter.relics_found = found;
    h->relic_counter.relics_required = required;
    h->relic_counter.visible = true;
}

void theron_v2_hud_set_rune_indicator(Theron_V2_HudOverlay *h,
                                       bool rune_ready, bool spell_charging,
                                       int rune_index)
{
    if (!h) return;
    h->rune_indicator.rune_ready = rune_ready;
    h->rune_indicator.spell_charging = spell_charging;
    h->rune_indicator.rune_index = rune_index;
    h->rune_indicator.visible = true;
}

void theron_v2_hud_set_champion_bar(Theron_V2_HudOverlay *h, int champ_idx,
                                    int hp_pct, int stamina_pct, int mana_pct,
                                    bool leader, bool spell_ready)
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

void theron_v2_hud_set_action_active(Theron_V2_HudOverlay *h, Theron_V2_ActionIcon icon)
{
    if (!h) return;
    for (int i = 0; i < THERON_V2_ACTION_COUNT; i++) {
        h->action_strip.icons[i].active = (i == (int)icon);
    }
}

void theron_v2_hud_trigger_hit_flash(Theron_V2_HudOverlay *h)
{
    if (!h) return;
    h->hit_flash_active = true;
    h->hit_flash_timer = 6;
}

void theron_v2_hud_toggle(Theron_V2_HudOverlay *h)
{
    if (!h) return;
    h->visible = !h->visible;
}

void theron_v2_hud_set_opacity(Theron_V2_HudOverlay *h, uint8_t val)
{
    if (!h) return;
    h->opacity = val;
}

/* ══════════════════════════════════════════════════════════════════════
 * Main render entry
 *
 * Source-lock: THQUEST.ASM T600 (UI overlay zones) for the top-bar
 * zone layout; T800 (champion persistence + inventory reset) for the
 * bottom panel layout; T900 (object database / rune magic) for the
 * spell-rune indicator.  dm2_v2_hud_overlay.c is the sibling pattern.
 * ══════════════════════════════════════════════════════════════════════ */

void theron_v2_hud_render(Theron_V2_HudOverlay *h, uint8_t *fb, int w, int h_res)
{
    if (!h || !fb || w <= 0 || h_res <= 0) return;
    if (!h->visible) return;
    if (h->opacity == 0) return;

    uint8_t base = (uint8_t)(h->opacity / 2);
    uint8_t high = h->opacity;

    /* ── Top bar (y=0..23) ───────────────────────────────────────── */
    if (h->top_bar_visible) {
        /* Compass at top-left */
        hud_compass_draw(fb, w, w, h_res,
                         THERON_V2_HUD_COMPASS_CX, THERON_V2_HUD_COMPASS_CY,
                         h->compass.direction, base, high);

        /* Quest items "Qx/Qy" right of the compass */
        if (h->quest_items.visible) {
            hud_draw_string(fb, w, w, h_res,
                            THERON_V2_HUD_QUEST_X, THERON_V2_HUD_QUEST_Y,
                            "Q", high);
            hud_draw_number(fb, w, w, h_res,
                            THERON_V2_HUD_QUEST_X + 6, THERON_V2_HUD_QUEST_Y,
                            h->quest_items.collected, high);
            hud_draw_string(fb, w, w, h_res,
                            THERON_V2_HUD_QUEST_X + 22, THERON_V2_HUD_QUEST_Y,
                            "/", high);
            hud_draw_number(fb, w, w, h_res,
                            THERON_V2_HUD_QUEST_X + 26, THERON_V2_HUD_QUEST_Y,
                            h->quest_items.total, high);
        }

        /* Dungeon progress "D1/7" at center-top */
        hud_draw_string(fb, w, w, h_res,
                        THERON_V2_HUD_DUNGEON_X, THERON_V2_HUD_DUNGEON_Y,
                        "D", high);
        hud_draw_number(fb, w, w, h_res,
                        THERON_V2_HUD_DUNGEON_X + 6, THERON_V2_HUD_DUNGEON_Y,
                        h->dungeon_progress.current_dungeon, high);
        hud_draw_string(fb, w, w, h_res,
                        THERON_V2_HUD_DUNGEON_X + 22, THERON_V2_HUD_DUNGEON_Y,
                        "/", high);
        hud_draw_number(fb, w, w, h_res,
                        THERON_V2_HUD_DUNGEON_X + 26, THERON_V2_HUD_DUNGEON_Y,
                        h->dungeon_progress.total_dungeons, high);

        /* Relic counter "R1/7" at right-top */
        if (h->relic_counter.visible) {
            hud_draw_string(fb, w, w, h_res,
                            THERON_V2_HUD_RELIC_X, THERON_V2_HUD_RELIC_Y,
                            "R", high);
            hud_draw_number(fb, w, w, h_res,
                            THERON_V2_HUD_RELIC_X + 6, THERON_V2_HUD_RELIC_Y,
                            h->relic_counter.relics_found, high);
            hud_draw_string(fb, w, w, h_res,
                            THERON_V2_HUD_RELIC_X + 22, THERON_V2_HUD_RELIC_Y,
                            "/", high);
            hud_draw_number(fb, w, w, h_res,
                            THERON_V2_HUD_RELIC_X + 26, THERON_V2_HUD_RELIC_Y,
                            h->relic_counter.relics_required, high);
        }

        /* Spell-rune indicator (right of compass, in top-bar) */
        if (h->rune_indicator.visible) {
            hud_rune_indicator_draw(fb, w, w, h_res,
                                    36, 8,
                                    h->rune_indicator.rune_ready,
                                    h->rune_indicator.spell_charging,
                                    h->rune_indicator.rune_index,
                                    base, high);
        }
    }

    /* ── Bottom panel: 4 champion mini-bars (y=184..191) ──────────── */
    if (h->stats_bar_visible) {
        for (int i = 0; i < 4; i++) {
            Theron_V2_HudChampionBar *cb = &h->champion_bars[i];
            int bx = THERON_V2_CHAMP_BAR_X_START + i * (THERON_V2_CHAMP_BAR_W + THERON_V2_CHAMP_BAR_SPACING);
            hud_draw_champion_bar(fb, w, w, h_res,
                                  bx, THERON_V2_CHAMP_BAR_Y,
                                  i, cb->hp_pct, cb->stamina_pct, cb->mana_pct,
                                  cb->leader, cb->spell_ready, high);
        }
    }

    /* ── Bottom action strip (y=208..221) ────────────────────────── */
    if (h->action_strip.visible) {
        for (int i = 0; i < THERON_V2_ACTION_COUNT; i++) {
            int ax = THERON_V2_ACTION_ICONS_X_START + i * (THERON_V2_ACTION_ICON_W + 4);
            Theron_V2_ActionIconState *st = &h->action_strip.icons[i];
            bool flash = h->hit_flash_active && st->active;
            hud_draw_action_icon(fb, w, w, h_res, ax, THERON_V2_ACTION_STRIP_Y,
                                 (Theron_V2_ActionIcon)i, st->active, flash, base, high);
        }
        if (h->hit_flash_timer > 0) {
            h->hit_flash_timer--;
            if (h->hit_flash_timer == 0) {
                h->hit_flash_active = false;
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Source evidence citation
 * ══════════════════════════════════════════════════════════════════════ */

const char *theron_v2_hud_source_evidence(void)
{
    return
        "Theron V2.0/V2.1: compass, quest items, dungeon progress, relic counter,\n"
        "  spell-rune ready indicator, champion bars, action strip\n"
        "  Source: THQUEST.ASM T520  (party placement / start position)\n"
        "  Source: THQUEST.ASM T560  (dungeon loading, header parsing)\n"
        "  Source: THQUEST.ASM T600  (UI overlay zones: top-bar / right / bottom)\n"
        "  Source: THQUEST.ASM T700  (timers / world tick)\n"
        "  Source: THQUEST.ASM T800  (champion persistence + inventory reset)\n"
        "  Source: THQUEST.ASM T900  (object database / rune magic)\n"
        "  Source: HuC6260/HuC6270 datasheet (PC Engine VDC + VCE)\n"
        "  Source: ReDMCSB PANEL.C F0354 (champion status-box rendering, sibling)\n"
        "  Source: ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing, sibling)\n"
        "  Source: dmweb Theron overview (7 dungeons + 7 relic goals + rune magic)\n"
        "  Source: docs/source-lock/tqr_v1_phase2_data_formats_H2339.md\n"
        "Theron V2.2: hit flash, low-HP pulse, smooth compass interpolation\n"
        "  Source: ReDMCSB COMMAND.C action feedback gates (sibling pattern)\n"
        "  Source: ReDMCSB DISPLAY.C pulse animation timing (~2 Hz, sibling)\n"
        "  Source: sibling modules csb_v2_hud_overlay_pc34.c, dm2_v2_hud_overlay.c\n";
}
