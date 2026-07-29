#include "dm2_v1_champion_stat_bridge.h"
#include "dm2_v1_champion_hud_helpers.h"

#include <string.h>

static uint8_t compute_bar_pct(int16_t cur, int16_t max)
{
    if (max <= 0) return 0;
    if (cur <= 0) return 0;
    if (cur >= max) return 100;
    return (uint8_t)(((int)cur * 100) / (int)max);
}

int dm2_v1_champion_stat_bridge_compute(
    const DM2_V1_ChampionStatInput *inputs,
    const DM2_V1_ChampionStatPrev *prev,
    int champion_count,
    int16_t gdat_bar_color_override,
    DM2_V1_ChampionStatBridgeReceipt *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (!inputs || champion_count <= 0 ||
        champion_count > DM2_V1_CHAMPION_STAT_BRIDGE_MAX_HEROES) {
        return 0;
    }

    out->valid = 1;
    out->champion_count = champion_count;

    for (int i = 0; i < champion_count; i++) {
        const DM2_V1_ChampionStatInput *in = &inputs[i];
        DM2_V1_ChampionStatOutput *c = &out->champions[i];

        c->is_alive = in->cur_hp > 0;
        c->is_leader = in->is_leader;
        c->spell_ready = in->spell_ready;

        if (!c->is_alive) {
            continue;
        }

        c->hp_pct = compute_bar_pct(in->cur_hp, in->max_hp);
        c->stamina_pct = compute_bar_pct(in->cur_stamina, in->max_stamina);

        int16_t effective_max_mp = in->max_mp;
        if (in->cur_mp > effective_max_mp)
            effective_max_mp = in->cur_mp;
        c->mana_pct = compute_bar_pct(in->cur_mp, effective_max_mp);

        int16_t default_color = (i < DM2_V1_CHAMPION_HUD_HERO_COUNT)
            ? dm2_v1_default_hero_bar_color[i] : 7;
        int16_t bar_color = dm2_v1_QUERY_3STAT_BAR_COLOR(
            gdat_bar_color_override, default_color, NULL);
        c->hp_bar_color = bar_color;
        c->stamina_bar_color = bar_color;
        c->mana_bar_color = bar_color;

        if (prev) {
            c->redraw_hp = (in->cur_hp != prev[i].prev_hp) ||
                           (in->max_hp != prev[i].prev_max_hp);
            c->redraw_stamina = (in->cur_stamina != prev[i].prev_stamina) ||
                                (in->max_stamina != prev[i].prev_max_stamina);
            c->redraw_mana = (in->cur_mp != prev[i].prev_mp) ||
                             (in->max_mp != prev[i].prev_max_mp);
        } else {
            c->redraw_hp = 1;
            c->redraw_stamina = 1;
            c->redraw_mana = 1;
        }
    }

    return 1;
}

const char *dm2_v1_champion_stat_bridge_source_evidence(void)
{
    return "skproject SKULLWIN/c_gui_draw.cpp DM2_DRAW_PLAYER_3STAT_HEALTH_BAR:167 "
           "DM2_DRAW_PLAYER_3STAT_TEXT:260; "
           "bridges V1 hero stats to V2 HUD runtime percentages.";
}
