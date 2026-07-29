#ifndef FIRESTAFF_DM2_V1_CHAMPION_HUD_HELPERS_H
#define FIRESTAFF_DM2_V1_CHAMPION_HUD_HELPERS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_CHAMPION_HUD_MAX_SKILL_LEVEL 15u
#define DM2_V1_CHAMPION_HUD_NULL_ITEM 0xffffu

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    int result;
    int dirty;
    const char *symbol;
    const char *source_path;
} DM2_V1_ChampionHudReceipt;

typedef struct {
    uint16_t item_ref;
    int16_t current_value;
    int16_t item_bonus;
    int16_t minimum_value;
    int16_t maximum_value;
} DM2_V1_ProcessItemBonusInput;

typedef struct {
    uint32_t experience;
    uint16_t base_level;
    uint16_t temporary_bonus;
    uint16_t maximum_level;
} DM2_V1_PlayerSkillInput;

typedef struct {
    int16_t current_value;
    int16_t maximum_value;
    int16_t previous_current_value;
    int16_t previous_maximum_value;
    int16_t bar_color;
} DM2_V1_PlayerStatDisplayInput;

typedef struct {
    int16_t current_value;
    int16_t maximum_value;
    uint8_t percent;
    int16_t bar_color;
    uint8_t redraw_value;
    uint8_t redraw_maximum;
    uint8_t redraw_bar;
} DM2_V1_PlayerStatDisplay;

void dm2_v1_champion_hud_receipt_clear(
    DM2_V1_ChampionHudReceipt *receipt);

int16_t dm2_v1_PROCESS_ITEM_BONUS(
    const DM2_V1_ProcessItemBonusInput *input,
    DM2_V1_ChampionHudReceipt *out_receipt);

uint16_t dm2_v1_QUERY_PLAYER_SKILL_LV(
    const DM2_V1_PlayerSkillInput *input,
    DM2_V1_ChampionHudReceipt *out_receipt);

int dm2_v1_REFRESH_PLAYER_STAT_DISP(
    const DM2_V1_PlayerStatDisplayInput *input,
    DM2_V1_PlayerStatDisplay *out_display,
    DM2_V1_ChampionHudReceipt *out_receipt);

/* Bar color queries.
 * Source: SkWinCore.cpp:13194 QUERY_FOOD_WATER_BAR_COLOR
 *         SkWinCore.cpp:13203 QUERY_3STAT_BAR_COLOR
 *
 * Both accept an optional GDAT override (gdat_value >= 0 means GDAT
 * entry was found; < 0 means not found → use default). */

#define DM2_V1_CHAMPION_HUD_DEFAULT_FOOD_COLOR   5   /* COLOR_BROWN */
#define DM2_V1_CHAMPION_HUD_DEFAULT_WATER_COLOR  14  /* COLOR_BLUE  */

typedef struct {
    int valid;
    int16_t color;
    int gdat_override;
} DM2_V1_BarColorReceipt;

int16_t dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(
    int16_t gdat_value,
    int16_t default_color,
    DM2_V1_BarColorReceipt *out_receipt);

int16_t dm2_v1_QUERY_3STAT_BAR_COLOR(
    int16_t gdat_value,
    int16_t default_color,
    DM2_V1_BarColorReceipt *out_receipt);

/* Default per-hero 3-stat bar colors (table1d69d0).
 * Index 0..3 → hero 0..3. */
#define DM2_V1_CHAMPION_HUD_HERO_COUNT 4
extern const int16_t dm2_v1_default_hero_bar_color[DM2_V1_CHAMPION_HUD_HERO_COUNT];

const char *dm2_v1_champion_hud_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CHAMPION_HUD_HELPERS_H */
