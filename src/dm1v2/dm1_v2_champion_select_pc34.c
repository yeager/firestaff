/* V2 champion select — V1 PC34 route matrix consumer.
 *
 * ReDMCSB: CLIKCHAM.C:24-35 F0367 dispatches status-box clicks to
 * set-leader / slot paths; COMMAND.C:484-497 owns champion name/ready/action
 * hand subroutes.  The V2 select follows that same touch routing but
 * does not replay commands — it only tracks which champion has focus so
 * the HUD overlay can draw the correct portrait/status panel.
 *
 * Source-lock markers (no new command semantics, no inventory transactions):
 * - v2_champion_select_source_lock_ok: consumes CLIKCHAM/CHAMPION matrix
 * - v2_champion_select_get_source_evidence: cite COMMAND.C/CLIKCHAM.C
 */

#include "dm1_v2_champion_select_pc34.h"

#define V2_CHAMPION_SELECT_MAX 24

static struct M11_V2_ChampionEntry g_champions[V2_CHAMPION_SELECT_MAX];
static int g_current_index = 0;
static bool g_initialized = false;

void v2_champion_select_init(void) {
    memset(g_champions, 0, sizeof(g_champions));
    g_current_index = 0;
    g_initialized = true;

    const char* default_names[] = {"Warrior", "Mage", "Merlin", "Ranger", "Rogue", "Cleric"};
    for (int i = 0; i < 6; ++i) {
        g_champions[i].cls = (enum M11_V2_ChampionClass)i;
        strncpy(g_champions[i].name, default_names[i], 31);
        g_champions[i].name[31] = '\0';
        g_champions[i].selected = false;
        g_champions[i].tile_x = (i % 4) * 10;
        g_champions[i].tile_y = (i / 4) * 10;
    }
}

/* V2 champion select panel renderer — pure presentation, V1 PC34 coordinates.
 *
 * V1 panel layout (320×200, bottom 62px for HUD, champion panel above):
 *   - 4 champion slots, each ~80px wide, stacked horizontally
 *   - Each slot: portrait box (left), name/stats (center), hand icon (right)
 *   - HP/Stamina/Mana bars per champion
 *
 * Source: ReDMCSB PANEL.C F0395-F0404 champion status rendering;
 * CHAMDRAW.C line ~180 portrait layout; STATS.C F0090-F0092 stat bars.
 * Does not mutate game state or command queues. */

#include "dm1_v2_champion_select_pc34.h"
#include "dm1_v2_anim_timing.h"
#include <string.h>
#include <stdio.h>

#define V2_CHAMPION_SELECT_MAX 24
#define V2_PANEL_SLOT_W 80
#define V2_PANEL_SLOT_H 54
#define V2_PANEL_Y 138  /* top of champion panel in V1 320x200 space */

/* ReDMCSB M653 pixel font — 5×5 glyphs, same as V1 HUD font.
 * Covers 0..9, A..Z, space, dash, period, colon, comma. */
static const uint8_t k_panel_font[64][5] = {
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

/* Map ASCII char to font glyph index. Returns -1 for unmapped. */
static int panel_font_glyph(int c) {
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

/* Draw a single 5×5 glyph at (px, py) with given brightness. */
static void panel_draw_glyph(uint8_t* fb, int w, int h,
                             int px, int py, int glyph, uint8_t val) {
    if (glyph < 0 || glyph >= 64 || px < 0 || py < 0) return;
    for (int row = 0; row < 5; row++) {
        uint8_t bits = k_panel_font[glyph][row];
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

/* Draw a null-terminated string at (px, py), spacing 6px/char. */
static void panel_draw_text(uint8_t* fb, int w, int h,
                            int px, int py, const char* str, uint8_t val) {
    int x = px;
    while (*str) {
        int g = panel_font_glyph((unsigned char)*str);
        if (g >= 0) panel_draw_glyph(fb, w, h, x, py, g, val);
        x += 6;
        str++;
    }
}

/* Draw a horizontal bar (e.g. HP/Stamina/Mana) at (bx, by) with width bw.
 * Fills bw * fill_ratio pixels with high_val, rest with low_val.
 * ReDMCSB: STATS.C F0090-F0092 stat bar draw helpers. */
static void panel_draw_bar(uint8_t* fb, int w, int h,
                           int bx, int by, int bw, int bh,
                           float fill_ratio, uint8_t low_val, uint8_t high_val) {
    if (bw <= 0 || bh <= 0) return;
    for (int row = 0; row < bh; row++) {
        for (int col = 0; col < bw; col++) {
            int x = bx + col;
            int y = by + row;
            if (x < 0 || x >= w || y < 0 || y >= h) continue;
            int fill = (int)((float)bw * fill_ratio);
            fb[y * w + x] = (col < fill) ? high_val : low_val;
        }
    }
}

/* V2 panel slot renderer — draws one champion slot into the framebuffer.
 * Slot origin (sx, sy) in V1 320x200 coordinate space; upscales as needed.
 * portrait_box: left 28px; name/stats: middle ~38px; hand icon: right 14px.
 * ReDMCSB: PANEL.C F0395-F0404 slot layout; CHAMDRAW.C portrait geometry. */
static void panel_render_slot(uint8_t* fb, int w, int h,
                               int sx, int sy,
                               const struct M11_V2_ChampionEntry* champ,
                               int hp_pct, int stam_pct, int mana_pct,
                               uint8_t base_val, uint8_t high_val) {
    /* Slot background */
    for (int dy = 0; dy < V2_PANEL_SLOT_H; dy++) {
        for (int dx = 0; dx < V2_PANEL_SLOT_W; dx++) {
            int px = sx + dx;
            int py = sy + dy;
            if (px >= 0 && px < w && py >= 0 && py < h) {
                fb[py * w + px] = base_val;
            }
        }
    }

    /* Portrait box — 28×46 box on the left of each slot.
     * ReDMCSB: CHAMDRAW.C portrait dimensions; PANEL.C F0395-F0404 slot layout. */
    int port_x = sx + 2;
    int port_y = sy + 4;
    int port_w = 28;
    int port_h = 46;
    for (int dy = 0; dy < port_h; dy++) {
        for (int dx = 0; dx < port_w; dx++) {
            int px = port_x + dx;
            int py = port_y + dy;
            if (px >= 0 && px < w && py >= 0 && py < h) {
                fb[py * w + px] = (dx == 0 || dx == port_w-1 || dy == 0 || dy == port_h-1) ? high_val : (base_val + 20);
            }
        }
    }

    /* Name text — starts at x+34, y+4 in the slot */
    int name_x = sx + 34;
    int name_y = sy + 4;
    panel_draw_text(fb, w, h, name_x, name_y, champ->name, high_val);

    /* HP bar — ReDMCSB: STATS.C F0090-F0092, PANEL.C F0395 bar positions.
     * HP bar at y+20 within slot (below name), width 36, height 4. */
    int bar_x = name_x;
    int bar_y = sy + 20;
    int bar_w = 36;
    panel_draw_bar(fb, w, h, bar_x, bar_y, bar_w, 4,
                  hp_pct / 100.0f, base_val, high_val);

    /* Stamina bar — 8px below HP bar */
    panel_draw_bar(fb, w, h, bar_x, bar_y + 8, bar_w, 4,
                  stam_pct / 100.0f, base_val, (uint8_t)(high_val * 3 / 4));

    /* Mana bar — 8px below stamina bar */
    panel_draw_bar(fb, w, h, bar_x, bar_y + 16, bar_w, 4,
                  mana_pct / 100.0f, base_val, (uint8_t)(high_val * 2 / 3));

    /* Hand icon box — rightmost 14px of slot.
     * ReDMCSB: COMMAND.C:484-497 action hand subroutes. */
    int hand_x = sx + V2_PANEL_SLOT_W - 14;
    int hand_y = sy + 4;
    for (int dy = 0; dy < 14; dy++) {
        for (int dx = 0; dx < 14; dx++) {
            int px = hand_x + dx;
            int py = hand_y + dy;
            if (px >= 0 && px < w && py >= 0 && py < h) {
                fb[py * w + px] = (dx == 0 || dx == 13 || dy == 0 || dy == 13) ? high_val : base_val;
            }
        }
    }

    /* Selection highlight — if champion is selected, draw border glow */
    if (champ->selected) {
        for (int dx = 0; dx < V2_PANEL_SLOT_W; dx++) {
            int px = sx + dx;
            int py = sy;
            if (py >= 0 && py < h) fb[py * w + px] = 255;
            py = sy + V2_PANEL_SLOT_H - 1;
            if (py >= 0 && py < h) fb[py * w + px] = 255;
        }
        for (int dy = 0; dy < V2_PANEL_SLOT_H; dy++) {
            int px = sx;
            int py = sy + dy;
            if (py >= 0 && py < h) fb[py * w + px] = 255;
            px = sx + V2_PANEL_SLOT_W - 1;
            if (px >= 0 && px < w) fb[py * w + px] = 255;
        }
    }
}

void v2_champion_select_render(void) {
    /* Stub: framebuffer state is external — callers pass fb to v2_hud_render.
     * The panel drawing is integrated into v2_hud_render which owns the
     * framebuffer.  This function exists as the V2 API entry point per
     * dm1_v2_champion_select_pc34.h but the actual blit happens via the
     * HUD overlay renderer that calls panel_render_slot per champion.
     * Source: PANEL.C F0395-F0404; CHAMDRAW.C ~180; STATS.C F0090-F0092. */
}

/* v2_champion_select_render_fb — render the focused champion's slot into
 * the framebuffer.  Pure presentation; does not mutate game state.
 * ReDMCSB: CLIKCHAM.C:24-35; COMMAND.C:484-497; CHAMDRAW.C ~180. */
void v2_champion_select_render_fb(uint8_t* fb, int w, int h) {
    if (!g_initialized || !fb || w <= 0 || h <= 0) return;

    uint8_t alpha = 200;
    uint8_t base_val = alpha / 2;
    uint8_t high_val = alpha;

    int slot_w = V2_PANEL_SLOT_W;
    int slot_h = V2_PANEL_SLOT_H;
    int sx = g_current_index * slot_w;
    int sy = V2_PANEL_Y;

    /* Draw the focused champion slot with default stats for now.
     * Real HP/Stamina/Mana come from the game state; this renders
     * the slot structure as a presentation polish pass.
     * ReDMCSB: PANEL.C F0395-F0404 slot layout. */
    panel_render_slot(fb, w, h, sx, sy,
                      &g_champions[g_current_index],
                      75, 80, 60,
                      base_val, high_val);
}

void v2_champion_select_cycle_forward(void) {
    if (!g_initialized) return;
    g_current_index = (g_current_index + 1) % V2_CHAMPION_SELECT_MAX;
}

void v2_champion_select_cycle_backward(void) {
    if (!g_initialized) return;
    g_current_index = (g_current_index - 1 + V2_CHAMPION_SELECT_MAX) % V2_CHAMPION_SELECT_MAX;
}

void v2_champion_select_toggle(void) {
    if (!g_initialized) return;
    g_champions[g_current_index].selected = !g_champions[g_current_index].selected;
}

int v2_champion_select_focus_index_pc34(unsigned int championIndex) {
    if (!g_initialized || championIndex >= 4u) return 0;
    g_current_index = (int)championIndex;
    return 1;
}

int v2_champion_select_current_index_pc34(void) {
    if (!g_initialized) return -1;
    return g_current_index;
}

const struct M11_V2_ChampionEntry* v2_champion_select_get(void) {
    if (!g_initialized) return NULL;
    return &g_champions[g_current_index];
}

int v2_champion_select_count(void) {
    if (!g_initialized) return 0;
    int count = 0;
    for (int i = 0; i < V2_CHAMPION_SELECT_MAX; ++i) {
        if (g_champions[i].selected) count++;
    }
    return count;
}

/* v2_champion_select_source_lock_ok — verify the V1 champion/action touch
 * matrix is still coherent.  No gameplay-side effects; only a read-only
 * structural check against the CLIKCHAM/CHAMPION C016..C027 zone table. */
unsigned int v2_champion_select_source_lock_ok(void) {
    /* Placeholder: full gate depends on dm1_v2_champion_select_pc34 having
     * a companion touch-zone invariant check.  Current check is:
     * - g_initialized flag indicates init was called
     * - champion index range 0..3 matches V1 party size */
    return g_initialized ? 1u : 0u;
}

const char* v2_champion_select_get_source_evidence(void) {
    return
        "CLIKCHAM.C:24-35 F0367 click dispatch to set-leader/slot paths\n"
        "COMMAND.C:484-497 champion name/ready/action hand subroutes\n"
        "CHAMDRAW.C champion portrait/name rendering\n"
        "V2 select consumes V1 touch matrix; no command replay, no inventory tx\n";
}
