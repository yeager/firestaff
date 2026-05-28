#ifndef FIRESTAFF_NEXUS_V2_HUD_OVERLAY_H
#define FIRESTAFF_NEXUS_V2_HUD_OVERLAY_H
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════════
 * Nexus V2 HUD Overlay — presentation-only enhanced UI chrome
 *
 * Phase 3: Nexus V2 enhanced in-game overlay presentation, UI chrome,
 * and interaction feedback.
 *
 * Architecture:
 *   This module is deliberately presentation-only: it draws optional
 *   overlay elements into the supplied framebuffer and does NOT mutate
 *   dungeon, champion, or command runtime state.
 *
 * V2.0/V2.1 overlay elements:
 *   - Compass rose (4-way directional indicator)
 *   - Dungeon depth counter  (e.g. "4/10")
 *   - Gold piece counter     (Nexus party gold)
 *   - Health/stamina/mana mini-bars for 4 party champions
 *   - Action strip icons    (Attack/Cast/Use/Drop/Move)
 *   - Saturn panel indicators (menu, map, party icons)
 *
 * V2.2 interaction feedback:
 *   - Hit flash on action icons
 *   - Low-health pulse on champion bars
 *   - Compass smooth rotation between cardinal directions
 *
 * Source: Saturn NEXUS.BIN HUD surface data
 *         DMDF parser documentation (DMDF/DGN level format)
 *         Saturn SDK VDP1 bitmap surfaces, VDP2 background layers
 *         ReDMCSB PANEL.C / DUNGEON.C (champion status refresh)
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Compass ─────────────────────────────────────────────────────── */
typedef struct {
    int direction;         /* 0=N, 1=E, 2=S, 3=W (V1 cardinal) */
    float needle_angle;    /* smooth animated angle (degrees) */
    bool animated;
} Nexus_V2_HudCompass;

/* ── Depth indicator ───────────────────────────────────────────── */
typedef struct {
    int current_level;
    int max_level;
} Nexus_V2_HudDepth;

/* ── Gold counter ─────────────────────────────────────────────── */
typedef struct {
    int party_gold;        /* current gold pieces */
    bool visible;
} Nexus_V2_HudGold;

/* ── Action strip icon ──────────────────────────────────────────── */
typedef enum {
    NEXUS_V2_ACTION_ATTACK = 0,
    NEXUS_V2_ACTION_CAST   = 1,
    NEXUS_V2_ACTION_USE    = 2,
    NEXUS_V2_ACTION_DROP   = 3,
    NEXUS_V2_ACTION_MOVE   = 4,
    NEXUS_V2_ACTION_COUNT  = 5
} Nexus_V2_ActionIcon;

typedef struct {
    Nexus_V2_ActionIcon icon;
    bool active;           /* currently highlighted */
    bool hit_flash;        /* set true to trigger one-frame flash */
} Nexus_V2_ActionIconState;

typedef struct {
    Nexus_V2_ActionIconState icons[NEXUS_V2_ACTION_COUNT];
    bool visible;
} Nexus_V2_HudActionStrip;

/* ── Champion mini-bar (HP/St/Ma in status bar) ────────────────────── */
typedef struct {
    int champion_index;   /* 0–3 */
    int hp_pct;           /* 0–100 health percentage */
    int stamina_pct;      /* 0–100 */
    int mana_pct;         /* 0–100 */
    bool leader;          /* TRUE if this champion is party leader */
    bool spell_ready;     /* TRUE if spell is charged */
} Nexus_V2_HudChampionBar;

/* ── Saturn panel indicators ───────────────────────────────────── */
typedef struct {
    bool menu_indicator;   /* Saturn-style menu/hamburger icon */
    bool map_indicator;     /* minimap toggle indicator */
    bool party_indicator;  /* party status icon */
} Nexus_V2_HudPanelIndicators;

/* ── Complete V2 HUD state ──────────────────────────────────────── */
typedef struct {
    Nexus_V2_HudCompass          compass;
    Nexus_V2_HudDepth            depth;
    Nexus_V2_HudGold             gold;
    Nexus_V2_HudActionStrip      action_strip;
    Nexus_V2_HudChampionBar      champion_bars[4];
    Nexus_V2_HudPanelIndicators  panel;
    bool visible;
    uint8_t opacity;       /* 0=invisible, 255=fully opaque */
    bool stats_bar_visible;
    bool hit_flash_active;
    uint8_t hit_flash_timer; /* decrements each render */
} Nexus_V2_HudOverlay;

/* ── Lifecycle ─────────────────────────────────────────────────── */
void nexus_v2_hud_init(Nexus_V2_HudOverlay *h);
void nexus_v2_hud_reset(Nexus_V2_HudOverlay *h);

/* ── Parameter setters ──────────────────────────────────────────── */
void nexus_v2_hud_set_direction(Nexus_V2_HudOverlay *h, int dir);
void nexus_v2_hud_set_level(Nexus_V2_HudOverlay *h, int cur, int max);
void nexus_v2_hud_set_gold(Nexus_V2_HudOverlay *h, int gold_pieces);
void nexus_v2_hud_set_champion_bar(Nexus_V2_HudOverlay *h, int champ_idx,
    int hp_pct, int stamina_pct, int mana_pct, bool leader, bool spell_ready);
void nexus_v2_hud_set_action_active(Nexus_V2_HudOverlay *h, Nexus_V2_ActionIcon icon);
void nexus_v2_hud_trigger_hit_flash(Nexus_V2_HudOverlay *h);
void nexus_v2_hud_toggle(Nexus_V2_HudOverlay *h);
void nexus_v2_hud_set_opacity(Nexus_V2_HudOverlay *h, uint8_t val);

/* ── Geometry constants ─────────────────────────────────────────── */
#define NEXUS_CHAMP_BAR_Y          4
#define NEXUS_CHAMP_BAR_H          8
#define NEXUS_CHAMP_BAR_W         64
#define NEXUS_CHAMP_BAR_X_START    4
#define NEXUS_CHAMP_BAR_SPACING    2

#define NEXUS_ACTION_STRIP_Y  172
#define NEXUS_ACTION_ICON_W   28
#define NEXUS_ACTION_ICONS_X_START 16

/* ── Rendering ───────────────────────────────────────────────────── */
/* nexus_v2_hud_render — draw V2 overlay into 320×200 VGA framebuffer.
 * x200 resolution (VGA 13h modenum).  Opacity 0 = no-op. */
void nexus_v2_hud_render(Nexus_V2_HudOverlay *h, uint8_t *fb, int w, int h_res);

/* ── V1 compatibility seam ─────────────────────────────────────── */
/* Source-lock citation helper */
const char *nexus_v2_hud_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V2_HUD_OVERLAY_H */
