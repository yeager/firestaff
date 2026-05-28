#ifndef FIRESTAFF_DM2_V2_HUD_OVERLAY_H
#define FIRESTAFF_DM2_V2_HUD_OVERLAY_H
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════════
 * DM2 V2 HUD Overlay — presentation-only enhanced UI chrome
 *
 * Phase 3: DM2 V2 enhanced in-game overlay presentation, UI chrome,
 * and interaction feedback.
 *
 * Architecture:
 *   This module is deliberately presentation-only: it draws optional
 *   overlay elements into the supplied framebuffer and does NOT mutate
 *   dungeon, champion, companion, or command runtime state.
 *
 * V2.0/V2.1 overlay elements:
 *   - Compass rose (4-way directional indicator)
 *   - Dungeon depth counter  (e.g. "4/10")
 *   - Gold piece counter    (DM2-specific, unlike DM1)
 *   - Health/stamina/mana mini-bars for party champions
 *   - Action strip icons    (Attack/Cast/Use/Drop/Move)
 *
 * V2.2 interaction feedback:
 *   - Hit flash on action icons
 *   - Low-health pulse on champion bars
 *   - Compass smooth rotation between cardinal directions
 *
 * Source: SKULL.ASM T560 (DM2 HUD rendering)
 *         SKULLWIN/SKWIN/c_gui_vp.cpp (DM2 UI chrome layout)
 *         ReDMCSB PANEL.C / DUNGEON.C (champion status refresh)
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Compass ─────────────────────────────────────────────────────── */
typedef struct {
    int direction;         /* 0=N, 1=E, 2=S, 3=W (V1 cardinal) */
    float needle_angle;    /* smooth animated angle (degrees) */
    bool animated;
} DM2_V2_HudCompass;

/* ── Depth indicator ───────────────────────────────────────────── */
typedef struct {
    int current_level;
    int max_level;
} DM2_V2_HudDepth;

/* ── Gold counter ─────────────────────────────────────────────── */
typedef struct {
    int party_gold;        /* current gold pieces */
    bool visible;
} DM2_V2_HudGold;

/* ── Action strip icon ──────────────────────────────────────────── */
typedef enum {
    DM2_V2_ACTION_ATTACK = 0,
    DM2_V2_ACTION_CAST   = 1,
    DM2_V2_ACTION_USE    = 2,
    DM2_V2_ACTION_DROP   = 3,
    DM2_V2_ACTION_MOVE   = 4,
    DM2_V2_ACTION_COUNT  = 5
} DM2_V2_ActionIcon;

typedef struct {
    DM2_V2_ActionIcon icon;
    bool active;           /* currently highlighted */
    bool hit_flash;        /* set true to trigger one-frame flash */
} DM2_V2_ActionIconState;

typedef struct {
    DM2_V2_ActionIconState icons[DM2_V2_ACTION_COUNT];
    bool visible;
} DM2_V2_HudActionStrip;

/* ── Champion mini-bar (V1 champion HP/St/Ma in status bar) ──────── */
typedef struct {
    int champion_index;   /* 0–3 */
    int hp_pct;           /* 0–100 health percentage */
    int stamina_pct;      /* 0–100 */
    int mana_pct;         /* 0–100 */
    bool leader;          /* TRUE if this champion is party leader */
    bool spell_ready;     /* TRUE if spell is charged */
} DM2_V2_HudChampionBar;

/* ── Complete V2 HUD state ──────────────────────────────────────── */
typedef struct {
    DM2_V2_HudCompass        compass;
    DM2_V2_HudDepth          depth;
    DM2_V2_HudGold           gold;
    DM2_V2_HudActionStrip    action_strip;
    DM2_V2_HudChampionBar    champion_bars[4];
    bool visible;
    uint8_t opacity;       /* 0=invisible, 255=fully opaque */
    bool stats_bar_visible;
    bool hit_flash_active;
    uint8_t hit_flash_timer; /* decrements each render */
} DM2_V2_HudOverlay;

/* ── Lifecycle ─────────────────────────────────────────────────── */
void dm2_v2_hud_init(DM2_V2_HudOverlay *h);
void dm2_v2_hud_reset(DM2_V2_HudOverlay *h);

/* ── Parameter setters ──────────────────────────────────────────── */
void dm2_v2_hud_set_direction(DM2_V2_HudOverlay *h, int dir);
void dm2_v2_hud_set_level(DM2_V2_HudOverlay *h, int cur, int max);
void dm2_v2_hud_set_gold(DM2_V2_HudOverlay *h, int gold_pieces);
void dm2_v2_hud_set_champion_bar(DM2_V2_HudOverlay *h, int champ_idx,
    int hp_pct, int stamina_pct, int mana_pct, bool leader, bool spell_ready);
void dm2_v2_hud_set_action_active(DM2_V2_HudOverlay *h, DM2_V2_ActionIcon icon);
void dm2_v2_hud_trigger_hit_flash(DM2_V2_HudOverlay *h);
void dm2_v2_hud_toggle(DM2_V2_HudOverlay *h);
void dm2_v2_hud_set_opacity(DM2_V2_HudOverlay *h, uint8_t val);

/* ── Geometry constants (also used by interaction_feedback) ─────── */
#define DM2_CHAMP_BAR_Y          4
#define DM2_CHAMP_BAR_H          8
#define DM2_CHAMP_BAR_W         64
#define DM2_CHAMP_BAR_X_START    4
#define DM2_CHAMP_BAR_SPACING    2

#define DM2_ACTION_STRIP_Y  172
#define DM2_ACTION_ICON_W   28
#define DM2_ACTION_ICONS_X_START 16

/* ── Rendering ───────────────────────────────────────────────────── */
/* dm2_v2_hud_render — draw V2 overlay into 320×200 VGA framebuffer.
 * x200 resolution (VGA 13h modenum).  Opacity 0 = no-op. */
void dm2_v2_hud_render(DM2_V2_HudOverlay *h, uint8_t *fb, int w, int h_res);

/* ── V1 compatibility seam ─────────────────────────────────────── */
/* Source-lock citation helper */
const char *dm2_v2_hud_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_HUD_OVERLAY_H */
