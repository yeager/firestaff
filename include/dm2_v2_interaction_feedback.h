#ifndef FIRESTAFF_DM2_V2_INTERACTION_FEEDBACK_H
#define FIRESTAFF_DM2_V2_INTERACTION_FEEDBACK_H
#include <stdint.h>
#include "dm2_v2_hud_overlay.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ══════════════════════════════════════════════════════════════════════
 * DM2 V2 Interaction Feedback — gate between input events and HUD
 *
 * Phase 3: DM2 V2 enhanced in-game overlay presentation, UI chrome,
 * and interaction feedback.
 *
 * Maps input events (mouse clicks on action strip, champion bars,
 * compass), V1 command responses, and combat events to HUD feedback
 * signals without bypassing or reimplementing V1 command routes.
 *
 * Design:
 *   Each feedback function is a thin gate that:
 *     1. Reads the current interaction context (click position, command ID)
 *     2. Triggers a HUD animation or state update via dm2_v2_hud_* API
 *     3. Returns a hit/not-hit result for the caller
 *
 *   This module does NOT mutate dungeon/champion state — it only
 *   mutates the DM2_V2_HudOverlay state set by dm2_v2_hud_overlay.
 *
 * Click zones mirror DM2 V1 chrome geometry:
 *   - Top 28px:     status bar (champion bars)
 *   - Bottom 28px:  action strip (icon buttons)
 *   - Center:       compass (top-left corner)
 *   - Right 80×144: portrait panel (DM2 different from DM1)
 *
 * Source: SKULL.ASM T560/T520 (DM2 UI click routing)
 *         ReDMCSB COMMAND.C:375-497 (click zone routing)
 *         ReDMCSB CLIKCHAM.C:24-35 (champion bar click handling)
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Hit result ─────────────────────────────────────────────────── */
typedef struct {
    int hit;            /* 1=click matched a zone, 0=miss */
    uint8_t zone_id;    /* 0=status bar, 1=action icon N, 2=compass, 3=portrait */
    uint8_t sub_zone;   /* sub-zone detail (champion index, icon index) */
} DM2_V2_InteractionResult;

/* ── Interaction zones enum ─────────────────────────────────────── */
typedef enum {
    DM2_V2_ZONE_STATUS_BAR  = 0,
    DM2_V2_ZONE_ACTION_ICON  = 1,
    DM2_V2_ZONE_ACTION_ROW  = 2,
    DM2_V2_ZONE_CHAMPION    = 3,
    DM2_V2_ZONE_GOLD_COUNTER = 4,
    DM2_V2_ZONE_COMPASS     = 5,
    DM2_V2_ZONE_PORTRAIT_PANEL = 6,
    DM2_V2_ZONE_MISS        = 255
} DM2_V2_InteractionZone;

/* ── Click routing ──────────────────────────────────────────────── */
/* dm2_v2_hit_test — hit-test a (screen_x, screen_y) against
 * current DM2 UI chrome geometry.
 * Returns DM2_V2_InteractionResult with hit/zone info.
 *
 * Geometry (VGA 320×200):
 *   - Status bar:   y=0..27   (DM2_VP_CHROME_TOP)
 *   - Dungeon view: y=28..171
 *   - Action strip: y=172..199 (DM2_ACTION_STRIP_Y)
 *   - Compass:     x=8..24, y=8..24 (top-left)
 *   - Gold counter: x=260..319, y=176..199
 *   - Portrait panel (indoor only): x=240..319 */
DM2_V2_InteractionResult dm2_v2_hit_test(int screen_x, int screen_y,
    int is_outdoor);

/* ── Feedback triggers ───────────────────────────────────────────── */
/* Each function triggers an appropriate HUD animation and
 * returns a DM2_V2_InteractionResult for the caller's routing. */

/* Action icon click — triggers hit flash on the icon */
DM2_V2_InteractionResult dm2_v2_feedback_action_icon(int icon_index);

/* Champion bar click — highlights that champion's mini-bar */
DM2_V2_InteractionResult dm2_v2_feedback_champion_click(int champ_idx);

/* Combat hit — brief red flash on HP bar */
DM2_V2_InteractionResult dm2_v2_feedback_combat_hit(int champ_idx);

/* Spell cast — mana bar depletes with visual feedback */
DM2_V2_InteractionResult dm2_v2_feedback_spell_cast(int champ_idx);

/* Gold change — gold counter briefly pulses */
DM2_V2_InteractionResult dm2_v2_feedback_gold_change(void);

/* Level change — depth display updates */
DM2_V2_InteractionResult dm2_v2_feedback_level_change(int new_level, int max_level);

/* ── HUD instance binding ────────────────────────────────────────── */
/* Binding call needed by feedback functions — must be called once
 * before interaction feedback functions are used. */
void dm2_v2_interaction_set_hud_instance(DM2_V2_HudOverlay *h);
DM2_V2_HudOverlay *dm2_v2_interaction_get_hud_instance(void);

/* ── Source evidence ─────────────────────────────────────────────── */
const char *dm2_v2_interaction_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V2_INTERACTION_FEEDBACK_H */
