/*
 * Production seam for the Theron V2 HUD overlay.
 *
 * The former overlay contains procedural compass, text, rune and champion
 * bar pixels.  Track 02 has not yielded an authenticated HUD/font/widget
 * bank, so production must expose state setters without drawing anything.
 * The complete implementation remains available to focused fixture tests.
 */

#include "theron_v2_hud_overlay_pc34.h"

#include <string.h>

void theron_v2_hud_init(Theron_V2_HudOverlay *h)
{
    if (h) memset(h, 0, sizeof(*h));
}

void theron_v2_hud_reset(Theron_V2_HudOverlay *h)
{
    theron_v2_hud_init(h);
}

void theron_v2_hud_set_direction(Theron_V2_HudOverlay *h, int dir)
{ (void)h; (void)dir; }
void theron_v2_hud_set_quest_items(Theron_V2_HudOverlay *h, int collected, int total)
{ (void)h; (void)collected; (void)total; }
void theron_v2_hud_set_dungeon_progress(Theron_V2_HudOverlay *h, int current_dungeon, int total_dungeons)
{ (void)h; (void)current_dungeon; (void)total_dungeons; }
void theron_v2_hud_set_relics(Theron_V2_HudOverlay *h, int found, int required)
{ (void)h; (void)found; (void)required; }
void theron_v2_hud_set_rune_indicator(Theron_V2_HudOverlay *h, bool rune_ready,
                                      bool spell_charging, int rune_index)
{ (void)h; (void)rune_ready; (void)spell_charging; (void)rune_index; }
void theron_v2_hud_set_champion_bar(Theron_V2_HudOverlay *h, int champ_idx,
                                    int hp_pct, int stamina_pct, int mana_pct,
                                    bool leader, bool spell_ready)
{ (void)h; (void)champ_idx; (void)hp_pct; (void)stamina_pct; (void)mana_pct;
  (void)leader; (void)spell_ready; }
void theron_v2_hud_set_action_active(Theron_V2_HudOverlay *h, Theron_V2_ActionIcon icon)
{ (void)h; (void)icon; }
void theron_v2_hud_trigger_hit_flash(Theron_V2_HudOverlay *h)
{ (void)h; }
void theron_v2_hud_toggle(Theron_V2_HudOverlay *h)
{ (void)h; }
void theron_v2_hud_set_opacity(Theron_V2_HudOverlay *h, uint8_t val)
{ (void)h; (void)val; }

Theron_V2_HudSeedGate theron_v2_hud_seed_from_v1_world(
    Theron_V2_HudOverlay *out,
    const Theron_V1_World *world,
    int v2PresentationEnabled)
{
    (void)world;
    (void)v2PresentationEnabled;
    theron_v2_hud_init(out);
    return THERON_V2_HUD_SEED_V1_SKIPPED;
}

const char *theron_v2_hud_seed_gate_name(Theron_V2_HudSeedGate gate)
{
    switch (gate) {
    case THERON_V2_HUD_SEED_INVALID: return "INVALID";
    case THERON_V2_HUD_SEED_V1_SKIPPED: return "V1_SKIPPED";
    case THERON_V2_HUD_SEED_V2_READY: return "V2_READY";
    default: return "UNKNOWN";
    }
}

void theron_v2_hud_render(Theron_V2_HudOverlay *h, uint8_t *fb, int w, int h_res)
{ (void)h; (void)fb; (void)w; (void)h_res; }

const char *theron_v2_hud_source_evidence(void)
{
    return "Track 02 HUD/font/widget bank not decoded; production draw blocked";
}
