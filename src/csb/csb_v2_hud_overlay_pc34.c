/*
 * CSB V2 HUD compatibility state.
 *
 * Rendering is deliberately no-draw. PC3.4 HUD pixels are owned by C017/C040
 * and Atari ST HUD pixels by C232 plus its package graphics. No host-drawn
 * compass, status bar, text or action icon may substitute for those surfaces.
 */

#include "csb_v2_hud_overlay_pc34.h"

#include <string.h>

void csb_v2_hud_init(CSB_V2_HudOverlay *h)
{
    if (!h) return;
    memset(h, 0, sizeof(*h));
    h->compass.animated = true;
    h->depth.current_level = 1;
    h->depth.max_level = 10;
    h->gold.visible = true;
    h->visible = true;
    h->opacity = 255;
    h->stats_bar_visible = true;
    h->action_strip.visible = true;
    h->chaos.visible = true;
}

void csb_v2_hud_reset(CSB_V2_HudOverlay *h)
{
    csb_v2_hud_init(h);
}

void csb_v2_hud_set_direction(CSB_V2_HudOverlay *h, int dir)
{
    if (!h) return;
    if (dir < 0) dir = 0;
    if (dir > 3) dir = 3;
    h->compass.direction = dir;
    h->compass.needle_angle = (float)dir * 90.0f;
}

void csb_v2_hud_set_level(CSB_V2_HudOverlay *h, int cur, int max)
{
    if (!h) return;
    if (cur < 0) cur = 0;
    if (max <= 0) max = 1;
    h->depth.current_level = cur;
    h->depth.max_level = max;
}

void csb_v2_hud_set_gold(CSB_V2_HudOverlay *h, int gold_pieces)
{
    if (!h) return;
    h->gold.party_gold = gold_pieces;
    h->gold.visible = true;
}

void csb_v2_hud_set_champion_bar(CSB_V2_HudOverlay *h, int champ_idx,
                                 int hp_pct, int stamina_pct, int mana_pct,
                                 bool leader, bool spell_ready)
{
    if (!h || champ_idx < 0 || champ_idx >= 4) return;
    h->champion_bars[champ_idx].champion_index = champ_idx;
    h->champion_bars[champ_idx].hp_pct = hp_pct;
    h->champion_bars[champ_idx].stamina_pct = stamina_pct;
    h->champion_bars[champ_idx].mana_pct = mana_pct;
    h->champion_bars[champ_idx].leader = leader;
    h->champion_bars[champ_idx].spell_ready = spell_ready;
}

void csb_v2_hud_set_action_active(CSB_V2_HudOverlay *h,
                                  CSB_V2_ActionIcon icon)
{
    int i;
    if (!h) return;
    for (i = 0; i < CSB_V2_ACTION_COUNT; ++i) {
        h->action_strip.icons[i].active = (i == (int)icon);
    }
}

void csb_v2_hud_trigger_hit_flash(CSB_V2_HudOverlay *h)
{
    if (!h) return;
    h->hit_flash_active = true;
    h->hit_flash_timer = 6;
}

void csb_v2_hud_toggle(CSB_V2_HudOverlay *h)
{
    if (h) h->visible = !h->visible;
}

void csb_v2_hud_set_opacity(CSB_V2_HudOverlay *h, uint8_t val)
{
    if (h) h->opacity = val;
}

void csb_v2_hud_set_chaos_active(CSB_V2_HudOverlay *h, bool active,
                                 int power_runes)
{
    if (!h) return;
    if (power_runes < 0) power_runes = 0;
    if (power_runes > 3) power_runes = 3;
    h->chaos.chaos_active = active;
    h->chaos.power_rune_count = power_runes;
    h->chaos.visible = true;
}

void csb_v2_hud_render(CSB_V2_HudOverlay *h, uint8_t *fb, int w, int h_res)
{
    (void)h;
    (void)fb;
    (void)w;
    (void)h_res;
}

const char *csb_v2_hud_source_evidence(void)
{
    return
        "CSB V2 HUD compatibility state is no-draw; source-owned HUD pixels\n"
        "are composed by PC3.4 C017/C040 or Atari ST C232.\n"
        "Source: CSBWin/Viewport.cpp and CSBWin/Graphics.cpp.\n"
        "Source: ReDMCSB PANEL.C F0354 and DUNGEON.C F0260.\n"
        "Source: ReDMCSB COMMAND.C action feedback and DISPLAY.C timing; "
        "CSBWin/Chaos.cpp retains chaos state ownership.\n";
}
