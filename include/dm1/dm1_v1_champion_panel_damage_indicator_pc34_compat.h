#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_DAMAGE_INDICATOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_DAMAGE_INDICATOR_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPDI_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPDI_GFX_DAMAGE_SMALL_PC34 15
#define DM1_V1_CPDI_GFX_DAMAGE_BIG_PC34 16
#define DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34 167
#define DM1_V1_CPDI_ZONE_DAMAGE_BIG_FIRST_PC34 179
#define DM1_V1_CPDI_COLOR_TRANSPARENT_FLESH_PC34 10
#define DM1_V1_CPDI_COLOR_TEXT_WHITE_PC34 15
#define DM1_V1_CPDI_COLOR_TEXT_RED_PC34 8
#define DM1_V1_CPDI_FORMAT_PADDING_OFF_PC34 0
#define DM1_V1_CPDI_FORMAT_WIDTH_PC34 3
#define DM1_V1_CPDI_DAMAGE_MAX_PC34 32767

typedef enum DM1_V1_ChampionPanelDamageIndicatorOpPc34Compat {
    DM1_V1_CPDI_OP_NONE_PC34 = 0,
    DM1_V1_CPDI_OP_ENABLE_MOUSE_UPDATE_PC34 = 1,
    DM1_V1_CPDI_OP_BLIT_DAMAGE_GRAPHIC_PC34 = 2,
    DM1_V1_CPDI_OP_PRINT_CENTERED_TEXT_PC34 = 3,
    DM1_V1_CPDI_OP_REDRAW_CHAMPION_STATE_PC34 = 4,
    DM1_V1_CPDI_OP_DISABLE_MOUSE_UPDATE_PC34 = 5
} DM1_V1_ChampionPanelDamageIndicatorOpPc34Compat;

typedef struct DM1_V1_ChampionPanelDamageIndicatorEvidencePc34Compat {
    bool contract_only;
    const char *draw_function_anchor;
    const char *pipeline_caller_anchor;
    const char *defs_graphics_anchor;
    const char *defs_colors_anchor;
    const char *defs_zones_anchor;
    const char *defs_prototype_anchor;
    const char *base_blit_anchor;
    const char *panel_inventory_anchor;
    const char *ordinal_macro_anchor;
    const char *scope_note;
    const char *no_real_asset_claim;
} DM1_V1_ChampionPanelDamageIndicatorEvidencePc34Compat;

typedef struct DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat {
    int champion_index;
    int inventory_champion_ordinal;
    int damage;
} DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat;

typedef struct DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat {
    bool valid;
    bool contract_only;
    bool rejected_null_output;
    bool rejected_champion_index;
    bool rejected_damage;
    bool is_inventory_champion;
    int champion_index;
    int champion_ordinal;
    int inventory_champion_ordinal;
    int graphic_index;
    int base_zone_index;
    int zone_index;
    int transparent_color;
    int text_color;
    int text_background_color;
    int integer_padding_enabled;
    int integer_padding_width;
    int redraw_champion_index;
    int damage;
    char damage_text[8];
    DM1_V1_ChampionPanelDamageIndicatorOpPc34Compat operations[5];
    int operation_count;
    const DM1_V1_ChampionPanelDamageIndicatorEvidencePc34Compat *evidence;
} DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat;

const DM1_V1_ChampionPanelDamageIndicatorEvidencePc34Compat *
DM1_V1_ChampionPanelDamageIndicator_EvidencePc34Compat(void);

const char *
DM1_V1_ChampionPanelDamageIndicator_SourceEvidencePc34Compat(void);

void DM1_V1_ChampionPanelDamageIndicator_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat *input);

int DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(
    const DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat *input,
    DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat *out_result);

#ifdef __cplusplus
}
#endif

#endif
