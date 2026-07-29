#ifndef FIRESTAFF_DM2_V1_CHAMPION_STAT_BRIDGE_H
#define FIRESTAFF_DM2_V1_CHAMPION_STAT_BRIDGE_H

/*
 * DM2 Champion Stat Bridge — connects V1 hero stat data to V2 HUD runtime.
 *
 * Takes raw hero stats (curHP/maxHP/curStamina/maxStamina/curMP/maxMP),
 * computes bar percentages via REFRESH_PLAYER_STAT_DISP, resolves bar
 * colors via QUERY_3STAT_BAR_COLOR/default_hero_bar_color, and packages
 * the result for dm2_v2_hud_runtime_set_champion().
 *
 * Source: skproject SKULLWIN/c_gui_draw.cpp:167 DM2_DRAW_PLAYER_3STAT_HEALTH_BAR
 *         skproject SKULLWIN/c_gui_draw.cpp:260 DM2_DRAW_PLAYER_3STAT_TEXT
 *         skproject SKWIN/SkWinCore.cpp:14573 REFRESH_PLAYER_STAT_DISP
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_CHAMPION_STAT_BRIDGE_MAX_HEROES 4

typedef struct {
    int16_t cur_hp;
    int16_t max_hp;
    int16_t cur_stamina;
    int16_t max_stamina;
    int16_t cur_mp;
    int16_t max_mp;
    int16_t food;
    int16_t water;
    int16_t poison;
    uint8_t poisoned;
    uint8_t is_leader;
    uint8_t spell_ready;
} DM2_V1_ChampionStatInput;

typedef struct {
    uint8_t hp_pct;
    uint8_t stamina_pct;
    uint8_t mana_pct;
    int16_t hp_bar_color;
    int16_t stamina_bar_color;
    int16_t mana_bar_color;
    uint8_t is_alive;
    uint8_t is_leader;
    uint8_t spell_ready;
    uint8_t redraw_hp;
    uint8_t redraw_stamina;
    uint8_t redraw_mana;
} DM2_V1_ChampionStatOutput;

typedef struct {
    int valid;
    int champion_count;
    DM2_V1_ChampionStatOutput champions[DM2_V1_CHAMPION_STAT_BRIDGE_MAX_HEROES];
} DM2_V1_ChampionStatBridgeReceipt;

typedef struct {
    int16_t prev_hp;
    int16_t prev_max_hp;
    int16_t prev_stamina;
    int16_t prev_max_stamina;
    int16_t prev_mp;
    int16_t prev_max_mp;
} DM2_V1_ChampionStatPrev;

int dm2_v1_champion_stat_bridge_compute(
    const DM2_V1_ChampionStatInput *inputs,
    const DM2_V1_ChampionStatPrev *prev,
    int champion_count,
    int16_t gdat_bar_color_override,
    DM2_V1_ChampionStatBridgeReceipt *out);

const char *dm2_v1_champion_stat_bridge_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
