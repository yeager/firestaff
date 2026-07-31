#ifndef FIRESTAFF_CSB_V2_HUD_OVERLAY_PC34_H
#define FIRESTAFF_CSB_V2_HUD_OVERLAY_PC34_H
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* CSB V2 HUD compatibility state. It does not draw pixels: PC3.4 C017/C040
 * and Atari ST C232 own the authentic HUD surfaces. The values below are
 * retained for input/runtime compatibility only and cannot authorise a
 * generated overlay. ReDMCSB: PANEL.C F0354, DUNGEON.C F0260, COMMAND.C. */

/* ── Compass ─────────────────────────────────────────────────────── */
typedef struct {
    int direction;         /* 0=N, 1=E, 2=S, 3=W (V1 cardinal) */
    float needle_angle;    /* smooth animated angle (degrees) */
    bool animated;
} CSB_V2_HudCompass;

/* ── Depth indicator ───────────────────────────────────────────── */
typedef struct {
    int current_level;
    int max_level;
} CSB_V2_HudDepth;

/* ── Gold counter ─────────────────────────────────────────────── */
typedef struct {
    int party_gold;        /* current gold pieces */
    bool visible;
} CSB_V2_HudGold;

/* ── Action strip icon ──────────────────────────────────────────── */
typedef enum {
    CSB_V2_ACTION_ATTACK = 0,
    CSB_V2_ACTION_CAST   = 1,
    CSB_V2_ACTION_USE    = 2,
    CSB_V2_ACTION_DROP   = 3,
    CSB_V2_ACTION_MOVE   = 4,
    CSB_V2_ACTION_COUNT  = 5
} CSB_V2_ActionIcon;

typedef struct {
    CSB_V2_ActionIcon icon;
    bool active;           /* currently highlighted */
    bool hit_flash;        /* set true to trigger one-frame flash */
} CSB_V2_ActionIconState;

typedef struct {
    CSB_V2_ActionIconState icons[CSB_V2_ACTION_COUNT];
    bool visible;
} CSB_V2_HudActionStrip;

/* ── Champion mini-bar (HP/St/Ma in status bar) ────────────────────── */
typedef struct {
    int champion_index;   /* 0–3 */
    int hp_pct;           /* 0–100 health percentage */
    int stamina_pct;      /* 0–100 */
    int mana_pct;         /* 0–100 */
    bool leader;          /* TRUE if this champion is party leader */
    bool spell_ready;     /* TRUE if spell is charged */
} CSB_V2_HudChampionBar;

/* ── Chaos magic indicator (CSB-specific) ─────────────────────── */
typedef struct {
    bool chaos_active;    /* DSA chaos magic currently active */
    int power_rune_count; /* number of power runes charged (0–3) */
    bool visible;
} CSB_V2_HudChaosIndicator;

/* ── Complete V2 HUD state ──────────────────────────────────────── */
typedef struct {
    CSB_V2_HudCompass        compass;
    CSB_V2_HudDepth depth;
    CSB_V2_HudGold           gold;
    CSB_V2_HudActionStrip    action_strip;
    CSB_V2_HudChampionBar    champion_bars[4];
    CSB_V2_HudChaosIndicator chaos;
    bool visible;
    uint8_t opacity;       /* 0=invisible, 255=fully opaque */
    bool stats_bar_visible;
    bool hit_flash_active;
    uint8_t hit_flash_timer; /* decrements each render */
} CSB_V2_HudOverlay;

/* ── Lifecycle ─────────────────────────────────────────────────── */
void csb_v2_hud_init(CSB_V2_HudOverlay *h);
void csb_v2_hud_reset(CSB_V2_HudOverlay *h);

/* ── Parameter setters ──────────────────────────────────────────── */
void csb_v2_hud_set_direction(CSB_V2_HudOverlay *h, int dir);
void csb_v2_hud_set_level(CSB_V2_HudOverlay *h, int cur, int max);
void csb_v2_hud_set_gold(CSB_V2_HudOverlay *h, int gold_pieces);
void csb_v2_hud_set_champion_bar(CSB_V2_HudOverlay *h, int champ_idx,
    int hp_pct, int stamina_pct, int mana_pct, bool leader, bool spell_ready);
void csb_v2_hud_set_action_active(CSB_V2_HudOverlay *h, CSB_V2_ActionIcon icon);
void csb_v2_hud_trigger_hit_flash(CSB_V2_HudOverlay *h);
void csb_v2_hud_toggle(CSB_V2_HudOverlay *h);
void csb_v2_hud_set_opacity(CSB_V2_HudOverlay *h, uint8_t val);
void csb_v2_hud_set_chaos_active(CSB_V2_HudOverlay *h, bool active, int power_runes);

/* ── Geometry constants ─────────────────────────────────────────── */
#define CSB_CHAMP_BAR_Y          4
#define CSB_CHAMP_BAR_H          8
#define CSB_CHAMP_BAR_W         64
#define CSB_CHAMP_BAR_X_START    4
#define CSB_CHAMP_BAR_SPACING    2

#define CSB_ACTION_STRIP_Y 172
#define CSB_ACTION_ICON_W   28
#define CSB_ACTION_ICONS_X_START 16

/* ── Rendering ───────────────────────────────────────────────────── */
/* Compatibility render entry. It is always no-draw: generated HUD chrome
 * cannot replace PC3.4 C017/C040 or Atari ST C232 source surfaces. */
void csb_v2_hud_render(CSB_V2_HudOverlay *h, uint8_t *fb, int w, int h_res);

/* ── V1 compatibility seam ─────────────────────────────────────── */
/* Source-lock citation helper */
const char *csb_v2_hud_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V2_HUD_OVERLAY_PC34_H */
