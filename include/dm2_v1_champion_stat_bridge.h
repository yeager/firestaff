#ifndef FIRESTAFF_DM2_V1_CHAMPION_STAT_BRIDGE_H
#define FIRESTAFF_DM2_V1_CHAMPION_STAT_BRIDGE_H

/*
 * DM2 Champion Stat Bridge — converts source c_hero stat data for a HUD.
 *
 * Takes raw hero stats (curHP/maxHP/curStamina/maxStamina/curMP/maxMP),
 * computes bar percentages via REFRESH_PLAYER_STAT_DISP, requires the
 * source GDAT/palette-owned bar colour, and packages
 * the result for a source-bound V1/V2 HUD consumer.
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
    uint16_t max_hp;
    uint16_t cur_stamina;
    uint16_t max_stamina;
    uint16_t cur_mp;
    uint16_t max_mp;
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
    uint16_t prev_max_hp;
    uint16_t prev_stamina;
    uint16_t prev_max_stamina;
    uint16_t prev_mp;
    uint16_t prev_max_mp;
} DM2_V1_ChampionStatPrev;

int dm2_v1_champion_stat_bridge_compute(
    const DM2_V1_ChampionStatInput *inputs,
    const DM2_V1_ChampionStatPrev *prev,
    int champion_count,
    /* Required source GDAT colour; a negative value blocks the HUD receipt. */
    int16_t gdat_bar_color_override,
    DM2_V1_ChampionStatBridgeReceipt *out);

const char *dm2_v1_champion_stat_bridge_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
