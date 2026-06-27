#ifndef FIRESTAFF_THERON_V2_HUD_OVERLAY_PC34_H
#define FIRESTAFF_THERON_V2_HUD_OVERLAY_PC34_H
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ══════════════════════════════════════════════════════════════════════
 * Theron V2 HUD Overlay — presentation-only enhanced UI chrome
 *
 * Phase 3 (initial seed): Theron V2 enhanced in-game overlay
 * presentation, UI chrome, and interaction feedback.
 *
 * Scope of this initial seed:
 *   - Data-free overlay module: draws into a Theron V1 indexed
 *     framebuffer (PC Engine native 256x224, TQR_FB_W × TQR_FB_H)
 *     without touching any V1 champion / world / dungeon state.
 *   - Phase 3 overlay elements mirror the DM2 + CSB HUD overlay
 *     surface but are Theron-specific:
 *
 *       V2.0 / V2.1 (data-free):
 *         - Compass rose (4-way cardinal indicator, top-bar)
 *         - Quest item counter  (Theron "items collected / total")
 *         - Dungeon progression "N/7"  (Theron has 7 dungeons)
 *         - Relic counter   (Theron has 7 relic goals, DMWeb)
 *         - Spell-rune ready indicator (Theron's primary spell mechanic)
 *         - 4 champion mini-bars  (HP / Stamina / Mana)
 *         - Action strip icons    (Attack / Cast / Use / Drop / Move)
 *
 *       V2.2 interaction feedback (data-free):
 *         - Hit flash on action icons
 *         - Low-health pulse on champion bars
 *         - Smooth compass needle interpolation
 *
 *   - Preserves V1 UI chrome: the existing theron_v1_ui_chrome top-bar
 *     (y=0..23), right panel (x=224..320), bottom panel (y=184..240)
 *     remains untouched.  The HUD overlay paints V2 chrome on top of
 *     those zones only when v2PresentationEnabled=1; otherwise the
 *     V1 chrome is the only chrome on screen (V1 source-locked).
 *
 * Architecture:
 *   This module is deliberately presentation-only: it draws optional
 *   overlay elements into the supplied framebuffer and does NOT mutate
 *   dungeon, champion, companion, command, save, or world runtime state.
 *
 * Source-lock anchors:
 *   THQUEST.ASM T400  tile bank loading
 *   THQUEST.ASM T520  party placement / start position
 *   THQUEST.ASM T560  dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T600  UI overlay zones
 *   THQUEST.ASM T700  timers / world tick
 *   THQUEST.ASM T800  champion persistence + inventory reset
 *   THQUEST.ASM T900  object database / thing list / rune magic
 *   HuC6260 / HuC6270 datasheet (PC Engine VDC + VCE)
 *   ReDMCSB COMMAND.C F0359 (LoadGameSettings, M12 menu selection)
 *   ReDMCSB PANEL.C F0354 (champion status-box drawing, sibling pattern)
 *   ReDMCSB DUNGEON.C F0260 (stat-bar refresh timing, sibling pattern)
 *   dmweb Theron overview page (7 dungeons, 7 relic goals, rune magic)
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *
 * Companion modules (sibling V2 wire-up):
 *   - theron_v2_phase_gate_pc34.c       V2 phase-gate decision (gates this)
 *   - theron_v2_presentation_mode_pc34.c  V2 mode selector (V1/V20/V21/V22)
 *   - theron_v22_modern_assets_pc34.c   V2.2 modern-asset manifest (sibling pattern)
 *   - theron_v1_ui_chrome.c             V1 chrome (top-bar / right / bottom / msg)
 *   - csb_v2_hud_overlay_pc34.c         CSB Phase 3 sibling (mirror reference)
 *   - dm2_v2_hud_overlay.c              DM2 Phase 3 sibling (mirror reference)
 *
 * Out of scope for this initial seed:
 *   - Real bitmap art assets (procedural rectangle / 5x5 digit / 3x5
 *     letter fallback only; per dm2_v2_hud_overlay.c pattern this is
 *     the documented data-free baseline that gets superseded by
 *     finished PBR art later, just like the DM2 HUD widget gate).
 *   - Theron-specific touch / controller / smooth movement / lighting
 *     (tracked under separate Phase 4 / 5 / 6 TODOs).
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Compass ─────────────────────────────────────────────────────── */
typedef struct
{
    int direction;       /* 0=N, 1=E, 2=S, 3=W (V1 cardinal, matches Theron leader_dir) */
    float needle_angle;  /* smooth animated angle (degrees) */
    bool animated;
} Theron_V2_HudCompass;

/* ── Quest item counter (top-bar, replaces/augments V1 top-bar count) ── */
typedef struct
{
    int collected;       /* items collected so far */
    int total;           /* items required to clear dungeon */
    bool visible;
} Theron_V2_HudQuestItems;

/* ── Dungeon progression indicator (Theron has 7 dungeons per DMWeb) ── */
typedef struct
{
    int current_dungeon; /* 1..7 */
    int total_dungeons;  /* 7 (Theron fixed) */
} Theron_V2_HudDungeonProgress;

/* ── Relic goal counter (Theron has 7 relic goals per DMWeb) ──────── */
typedef struct
{
    int relics_found;    /* 0..7 */
    int relics_required; /* 7 (Theron fixed) */
    bool visible;
} Theron_V2_HudRelicCounter;

/* ── Spell-rune ready indicator (Theron's primary spell mechanic) ── */
typedef struct
{
    bool rune_ready;     /* TRUE if at least one rune is charged */
    bool spell_charging; /* TRUE while a spell is mid-cast */
    int rune_index;      /* 0..3 rune glyph highlight (-1 if none) */
    bool visible;
} Theron_V2_HudRuneIndicator;

/* ── Action strip icon ──────────────────────────────────────────── */
typedef enum
{
    THERON_V2_ACTION_ATTACK = 0,
    THERON_V2_ACTION_CAST   = 1,
    THERON_V2_ACTION_USE    = 2,
    THERON_V2_ACTION_DROP   = 3,
    THERON_V2_ACTION_MOVE   = 4,
    THERON_V2_ACTION_COUNT  = 5
} Theron_V2_ActionIcon;

typedef struct
{
    Theron_V2_ActionIcon icon;
    bool active;       /* currently highlighted */
    bool hit_flash;    /* set true to trigger one-frame flash */
} Theron_V2_ActionIconState;

typedef struct
{
    Theron_V2_ActionIconState icons[THERON_V2_ACTION_COUNT];
    bool visible;
} Theron_V2_HudActionStrip;

/* ── Champion mini-bar (HP / Stamina / Mana) ───────────────────────── */
/* Theron has 4 champion slots: Theron (leader) + 3 companions.
 * Per dm1_v2_hud_overlay.c mirror; matches theron_v1_ui_chrome.c
 * bottom-panel slot geometry. */
typedef struct
{
    int champion_index;   /* 0..3 (0 = Theron / leader) */
    int hp_pct;           /* 0..100 health percentage */
    int stamina_pct;      /* 0..100 */
    int mana_pct;         /* 0..100 */
    bool leader;          /* TRUE if this champion is party leader */
    bool spell_ready;     /* TRUE if spell is charged */
} Theron_V2_HudChampionBar;

/* ── Complete V2 HUD state ────────────────────────────────────────── */
typedef struct
{
    Theron_V2_HudCompass          compass;
    Theron_V2_HudQuestItems       quest_items;
    Theron_V2_HudDungeonProgress  dungeon_progress;
    Theron_V2_HudRelicCounter     relic_counter;
    Theron_V2_HudRuneIndicator    rune_indicator;
    Theron_V2_HudActionStrip      action_strip;
    Theron_V2_HudChampionBar      champion_bars[4];
    bool visible;
    uint8_t opacity;        /* 0=invisible, 255=fully opaque */
    bool stats_bar_visible;
    bool top_bar_visible;
    bool hit_flash_active;
    uint8_t hit_flash_timer; /* decrements each render */
} Theron_V2_HudOverlay;

/* ── Lifecycle ─────────────────────────────────────────────────────── */
void theron_v2_hud_init(Theron_V2_HudOverlay *h);
void theron_v2_hud_reset(Theron_V2_HudOverlay *h);

/* ── Parameter setters ─────────────────────────────────────────────── */
void theron_v2_hud_set_direction(Theron_V2_HudOverlay *h, int dir);
void theron_v2_hud_set_quest_items(Theron_V2_HudOverlay *h, int collected, int total);
void theron_v2_hud_set_dungeon_progress(Theron_V2_HudOverlay *h, int current_dungeon, int total_dungeons);
void theron_v2_hud_set_relics(Theron_V2_HudOverlay *h, int found, int required);
void theron_v2_hud_set_rune_indicator(Theron_V2_HudOverlay *h, bool rune_ready,
                                      bool spell_charging, int rune_index);
void theron_v2_hud_set_champion_bar(Theron_V2_HudOverlay *h, int champ_idx,
                                    int hp_pct, int stamina_pct, int mana_pct,
                                    bool leader, bool spell_ready);
void theron_v2_hud_set_action_active(Theron_V2_HudOverlay *h, Theron_V2_ActionIcon icon);
void theron_v2_hud_trigger_hit_flash(Theron_V2_HudOverlay *h);
void theron_v2_hud_toggle(Theron_V2_HudOverlay *h);
void theron_v2_hud_set_opacity(Theron_V2_HudOverlay *h, uint8_t val);

/* ── Geometry constants (also used by hit-test in sibling modules) ─── */
/* Top-bar zone (y=0..23) - V2 chrome overlays the V1 top-bar
 * (theron_v1_ui_chrome.c TR_UI_TOPBAR = 1U << 0). */
#define THERON_V2_HUD_TOPBAR_H       24
#define THERON_V2_HUD_TOPBAR_W       TQR_FB_W   /* 256 (PC Engine native) */
#define THERON_V2_HUD_COMPASS_CX     16
#define THERON_V2_HUD_COMPASS_CY     12
#define THERON_V2_HUD_QUEST_X        64
#define THERON_V2_HUD_QUEST_Y        4
#define THERON_V2_HUD_DUNGEON_X      160
#define THERON_V2_HUD_DUNGEON_Y      4
#define THERON_V2_HUD_RELIC_X        220
#define THERON_V2_HUD_RELIC_Y        4

/* Champion mini-bar slots (bottom panel, y=184..204). */
#define THERON_V2_CHAMP_BAR_Y        184
#define THERON_V2_CHAMP_BAR_H        8
#define THERON_V2_CHAMP_BAR_W        60
#define THERON_V2_CHAMP_BAR_X_START  4
#define THERON_V2_CHAMP_BAR_SPACING  2

/* Action strip (bottom-most row, above V1 bottom panel). */
#define THERON_V2_ACTION_STRIP_Y     208
#define THERON_V2_ACTION_STRIP_H     14
#define THERON_V2_ACTION_ICON_W      28
#define THERON_V2_ACTION_ICONS_X_START 16

/* ── Rendering ─────────────────────────────────────────────────────── */
/* theron_v2_hud_render — draw V2 overlay into a 256x224 indexed
 * framebuffer (PC Engine native).  Opacity 0 = no-op.  Visible=0
 * no-op.  This function never reads or writes V1 champion / world
 * state; it only consumes the Theron_V2_HudOverlay presentation
 * snapshot. */
void theron_v2_hud_render(Theron_V2_HudOverlay *h, uint8_t *fb, int w, int h_res);

/* ── Source-lock citation helper ───────────────────────────────────── */
const char *theron_v2_hud_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_HUD_OVERLAY_PC34_H */
