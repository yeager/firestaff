#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_MOUTH_EYE_POISON_WARNING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_MOUTH_EYE_POISON_WARNING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPMEP_CHAMPION_COUNT 4
#define DM1_V1_CPMEP_STATISTIC_COUNT 6

#define DM1_V1_CPMEP_COLOR_RED_FLASH_DARK 0x04
#define DM1_V1_CPMEP_COLOR_RED_FLASH_LIT  0x0C
#define DM1_V1_CPMEP_COLOR_DARKEST_GRAY   12

#define DM1_V1_CPMEP_GFX_SLOT_NORMAL      33
#define DM1_V1_CPMEP_GFX_SLOT_WOUNDED     34
#define DM1_V1_CPMEP_GFX_FOOD_LABEL       30
#define DM1_V1_CPMEP_GFX_WATER_LABEL      31
#define DM1_V1_CPMEP_GFX_POISONED_LABEL   32

#define DM1_V1_CPMEP_ZONE_FOOD            500
#define DM1_V1_CPMEP_ZONE_WATER           501
#define DM1_V1_CPMEP_ZONE_POISONED        502
#define DM1_V1_CPMEP_ZONE_MOUTH           545
#define DM1_V1_CPMEP_ZONE_EYE             546

typedef struct DM1_V1_ChampionPanelMouthEyePoisonWarningEvidencePc34Compat {
    int contract_only;
    const char *inventory_overlay_anchor;
    const char *panel_warning_anchor;
    const char *inventory_swap_anchor;
    const char *video_primitive_anchor;
    const char *defs_anchor;
} DM1_V1_ChampionPanelMouthEyePoisonWarningEvidencePc34Compat;

typedef struct DM1_V1_ChampionPanelMouthEyePoisonWarningChampionPc34Compat {
    int current_health;
    int food;
    int water;
    int poison_event_count;
    int statistic_current[DM1_V1_CPMEP_STATISTIC_COUNT];
    int statistic_maximum[DM1_V1_CPMEP_STATISTIC_COUNT];
} DM1_V1_ChampionPanelMouthEyePoisonWarningChampionPc34Compat;

typedef struct DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat {
    int contract_only;
    int inventory_open;
    int leader_champion_index;
    int active_inventory_champion_index;
    DM1_V1_ChampionPanelMouthEyePoisonWarningChampionPc34Compat
        champions[DM1_V1_CPMEP_CHAMPION_COUNT];
} DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat;

typedef struct DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat {
    int valid;
    int contract_only;
    int selected_champion_index;
    int leader_index_switched_to_inventory_champion;
    int follows_inventory_champion_not_party_leader;

    int mouth_border_drawn;
    int mouth_warning_border_drawn;
    int mouth_zone;
    int mouth_border_graphic;
    int mouth_transparent_color;

    int eye_border_drawn;
    int eye_warning_border_drawn;
    int eye_zone;
    int eye_border_graphic;
    int eye_transparent_color;
    int first_low_statistic_index;

    int panel_drawn;
    int food_label_graphic;
    int food_label_zone;
    int food_warning_border_drawn;
    int food_warning_border_flashes;
    int food_warning_palette_dark;
    int food_warning_palette_lit;
    int food_bar_color;

    int water_label_graphic;
    int water_label_zone;
    int water_warning_border_drawn;
    int water_warning_border_flashes;
    int water_warning_palette_dark;
    int water_warning_palette_lit;
    int water_bar_color;

    int poison_label_graphic;
    int poison_label_zone;
    int poison_label_drawn;
    int poison_warning_border_drawn;
    int poison_warning_border_flashes;
    int poison_warning_palette_dark;
    int poison_warning_palette_lit;
} DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat;

const DM1_V1_ChampionPanelMouthEyePoisonWarningEvidencePc34Compat *
DM1_V1_ChampionPanelMouthEyePoisonWarning_EvidencePc34Compat(void);

void DM1_V1_ChampionPanelMouthEyePoisonWarning_InitStatePc34Compat(
    DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat *state);

int DM1_V1_ChampionPanelMouthEyePoisonWarning_BuildPc34Compat(
    const DM1_V1_ChampionPanelMouthEyePoisonWarningStatePc34Compat *state,
    DM1_V1_ChampionPanelMouthEyePoisonWarningResultPc34Compat *out_result);

#ifdef __cplusplus
}
#endif

#endif
