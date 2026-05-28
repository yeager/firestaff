#include "dm2_v2_interaction_feedback.h"
#include <string.h>

/* ══════════════════════════════════════════════════════════════════════
 * DM2 V2 Interaction Feedback — gate between input events and HUD
 *
 * Phase 3: DM2 V2 enhanced in-game overlay presentation, UI chrome,
 * and interaction feedback.
 *
 * Maps input events to HUD feedback signals without bypassing V1 routes.
 * All functions are deliberately thin gates: they read interaction
 * context and call dm2_v2_hud_* APIs but do NOT mutate dungeon state.
 *
 * Source: SKULL.ASM T560/T520 (DM2 UI click routing)
 *         ReDMCSB COMMAND.C:375-497 (click zone routing)
 *         ReDMCSB CLIKCHAM.C:24-35 (champion bar click handling)
 *         ReDMCSB PANEL.C (champion status box rendering)
 * ══════════════════════════════════════════════════════════════════════ */

/* Shared HUD state — initialised by dm2_v2_hud_init().
 * The game loop maintains the single canonical instance. */
static DM2_V2_HudOverlay *s_hud_instance = NULL;

void dm2_v2_interaction_set_hud_instance(DM2_V2_HudOverlay *h) {
    s_hud_instance = h;
}

DM2_V2_HudOverlay *dm2_v2_interaction_get_hud_instance(void) {
    return s_hud_instance;
}

/* ── Click geometry constants ──────────────────────────────────── */
#define DM2_ZONE_STATUS_Y0      0
#define DM2_ZONE_STATUS_Y1     27
#define DM2_ZONE_ACTION_Y0    172
#define DM2_ZONE_ACTION_Y1    199
#define DM2_ZONE_COMPASS_X0      8
#define DM2_ZONE_COMPASS_X1     24
#define DM2_ZONE_COMPASS_Y0      8
#define DM2_ZONE_COMPASS_Y1     24
#define DM2_ZONE_GOLD_X0       260
#define DM2_ZONE_PORTRAIT_X0   240

DM2_V2_InteractionResult dm2_v2_hit_test(int screen_x, int screen_y,
    int is_outdoor)
{
    DM2_V2_InteractionResult r;
    memset(&r, 0, sizeof(r));

    /* Outside viewport */
    if (screen_x < 0 || screen_x >= 320 || screen_y < 0 || screen_y >= 200) {
        r.zone_id = DM2_V2_ZONE_MISS;
        return r;
    }

    /* Status bar (top 28px) */
    if (screen_y <= DM2_ZONE_STATUS_Y1) {
        if (screen_x >= DM2_ZONE_COMPASS_X0 && screen_x <= DM2_ZONE_COMPASS_X1 &&
            screen_y >= DM2_ZONE_COMPASS_Y0 && screen_y <= DM2_ZONE_COMPASS_Y1) {
            r.hit = 1;
            r.zone_id = DM2_V2_ZONE_COMPASS;
            return r;
        }
        if (screen_x >= DM2_ZONE_GOLD_X0 && screen_y <= DM2_ZONE_ACTION_Y1) {
            r.hit = 1;
            r.zone_id = DM2_V2_ZONE_GOLD_COUNTER;
            return r;
        }
        r.hit = 1;
        r.zone_id = DM2_V2_ZONE_CHAMPION;
        r.sub_zone = (uint8_t)((screen_x - DM2_CHAMP_BAR_X_START) /
            (DM2_CHAMP_BAR_W + DM2_CHAMP_BAR_SPACING));
        if (r.sub_zone > 3) r.sub_zone = 3;
        return r;
    }

    /* Action strip (bottom 28px): icons only for valid slots. Remaining
     * strip pixels still belong to the action row chrome; the right-side
     * portrait panel is indoor-only dungeon-area geometry, not a bottom
     * strip owner. */
    if (screen_y >= DM2_ZONE_ACTION_Y0) {
        if (screen_x >= DM2_ACTION_ICONS_X_START) {
            int icon_slot = (screen_x - DM2_ACTION_ICONS_X_START) /
                (DM2_ACTION_ICON_W + 4);
            if (icon_slot >= 0 && icon_slot < DM2_V2_ACTION_COUNT) {
                r.hit = 1;
                r.zone_id = DM2_V2_ZONE_ACTION_ICON;
                r.sub_zone = (uint8_t)icon_slot;
                return r;
            }
        }
        r.hit = 1;
        r.zone_id = DM2_V2_ZONE_ACTION_ROW;
        return r;
    }

    /* Portrait panel (indoor only, right 80px at x=240..319) */
    if (!is_outdoor && screen_x >= DM2_ZONE_PORTRAIT_X0) {
        /* Only in dungeon area (y=28..171); action strip y=172+ is handled above. */
        if (screen_y >= DM2_ZONE_STATUS_Y1 && screen_y < DM2_ZONE_ACTION_Y0) {
            r.hit = 1;
            r.zone_id = DM2_V2_ZONE_PORTRAIT_PANEL;
            return r;
        }
    }

    /* Dungeon area — no overlay zone */
    if (screen_y < DM2_ZONE_ACTION_Y0) {
        r.zone_id = DM2_V2_ZONE_MISS;
        return r;
    }

    r.zone_id = DM2_V2_ZONE_MISS;
    return r;
}

/* ── Feedback triggers ─────────────────────────────────────────── */

/* Action icon click */
DM2_V2_InteractionResult dm2_v2_feedback_action_icon(int icon_index) {
    DM2_V2_InteractionResult r;
    memset(&r, 0, sizeof(r));

    if (icon_index < 0 || icon_index >= DM2_V2_ACTION_COUNT) {
        r.zone_id = DM2_V2_ZONE_MISS;
        return r;
    }

    if (s_hud_instance) {
        dm2_v2_hud_set_action_active(s_hud_instance, (DM2_V2_ActionIcon)icon_index);
        dm2_v2_hud_trigger_hit_flash(s_hud_instance);
    }

    r.hit = 1;
    r.zone_id = DM2_V2_ZONE_ACTION_ICON;
    r.sub_zone = (uint8_t)icon_index;
    return r;
}

/* Champion bar click */
DM2_V2_InteractionResult dm2_v2_feedback_champion_click(int champ_idx) {
    DM2_V2_InteractionResult r;
    memset(&r, 0, sizeof(r));

    if (champ_idx < 0 || champ_idx >= 4) {
        r.zone_id = DM2_V2_ZONE_MISS;
        return r;
    }

    if (s_hud_instance) {
        /* Highlight leader selection for this champion */
        for (int i = 0; i < 4; i++) {
            DM2_V2_HudChampionBar *cb = &s_hud_instance->champion_bars[i];
            cb->leader = (i == champ_idx);
        }
    }

    r.hit = 1;
    r.zone_id = DM2_V2_ZONE_CHAMPION;
    r.sub_zone = (uint8_t)champ_idx;
    return r;
}

/* Combat hit — brief red flash on HP bar */
DM2_V2_InteractionResult dm2_v2_feedback_combat_hit(int champ_idx) {
    DM2_V2_InteractionResult r;
    memset(&r, 0, sizeof(r));

    if (s_hud_instance && champ_idx >= 0 && champ_idx < 4) {
        DM2_V2_HudChampionBar *cb = &s_hud_instance->champion_bars[champ_idx];
        /* Reduce HP visual — actual damage is V1 route's job */
        cb->hp_pct = (cb->hp_pct > 10) ? (cb->hp_pct - 10) : 0;
        dm2_v2_hud_trigger_hit_flash(s_hud_instance);
    }

    r.hit = 1;
    r.zone_id = DM2_V2_ZONE_CHAMPION;
    r.sub_zone = (uint8_t)champ_idx;
    return r;
}

/* Spell cast — mana bar visual feedback */
DM2_V2_InteractionResult dm2_v2_feedback_spell_cast(int champ_idx) {
    DM2_V2_InteractionResult r;
    memset(&r, 0, sizeof(r));

    if (s_hud_instance && champ_idx >= 0 && champ_idx < 4) {
        DM2_V2_HudChampionBar *cb = &s_hud_instance->champion_bars[champ_idx];
        cb->spell_ready = false;
        cb->mana_pct = (cb->mana_pct > 15) ? (cb->mana_pct - 15) : 0;
        dm2_v2_hud_trigger_hit_flash(s_hud_instance);
    }

    r.hit = 1;
    r.zone_id = DM2_V2_ZONE_CHAMPION;
    r.sub_zone = (uint8_t)champ_idx;
    return r;
}

/* Gold change — gold counter pulse */
DM2_V2_InteractionResult dm2_v2_feedback_gold_change(void) {
    DM2_V2_InteractionResult r;
    memset(&r, 0, sizeof(r));

    if (s_hud_instance) {
        dm2_v2_hud_trigger_hit_flash(s_hud_instance);
    }

    r.hit = 1;
    r.zone_id = DM2_V2_ZONE_GOLD_COUNTER;
    return r;
}

/* Level change */
DM2_V2_InteractionResult dm2_v2_feedback_level_change(int new_level, int max_level) {
    DM2_V2_InteractionResult r;
    memset(&r, 0, sizeof(r));

    if (s_hud_instance) {
        dm2_v2_hud_set_level(s_hud_instance, new_level, max_level);
    }

    r.hit = 1;
    r.zone_id = DM2_V2_ZONE_COMPASS;
    return r;
}

const char *dm2_v2_interaction_source_evidence(void) {
    return
        "DM2 V2.0/V2.1 interaction feedback:\n"
        "  Source: SKULL.ASM T560/T520 (DM2 UI click routing)\n"
        "  Source: ReDMCSB COMMAND.C:375-497 (click zone routes)\n"
        "  Source: ReDMCSB CLIKCHAM.C:24-35 (F0367 champion box click)\n"
        "  Source: ReDMCSB PANEL.C F0354 champion status box\n"
        "DM2 V2.2: combat/skill feedback gates (hit flash, HP pulse)\n"
        "  Source: ReDMCSB DISPLAY.C animation timing (2 Hz pulse)\n"
        "  Source: SKULLWIN c_gui_vp.cpp UI hit-test layout\n";
}
