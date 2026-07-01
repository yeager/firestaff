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

/*
 * F0320 PC34 MEDIA009 box-geometry / text-stride constants.
 *
 * Source: ReDMCSB CHAMPION.C F0320:1744-1775 (PC34 MEDIA009 branch).
 * - DEFS.H:2471-2472: C016_BYTE_WIDTH = 16, C024_BYTE_WIDTH = 24.
 * - DEFS.H:2157:      C69_CHAMPION_STATUS_BOX_SPACING = 69.
 * - DEFS.H:2086/2093: C08_COLOR_RED, C15_COLOR_WHITE.
 * - DEFS.H:2088:      C10_COLOR_FLESH.
 * - F0320:1745: AL0969_i_X = championIndex * C69_CHAMPION_STATUS_BOX_SPACING.
 * - F0320:1746/1761: M770_BOX_TOP = 0 (both branches).
 * - F0320:1748-1749: inventory bottom = 28, blit byte_width = C016_BYTE_WIDTH.
 * - F0320:1749:       inventory box.left = AL0969_i_X + 7, right = + 31.
 * - F0320:1762:       non-inventory bottom = 6, blit byte_width = C024_BYTE_WIDTH.
 * - F0320:1763:       non-inventory box.left = AL0969_i_X, right = + 47.
 * - F0320:1751-1758:  inventory 1/2/3-digit x-stride adds 21/18/15.
 * - F0320:1758:       inventory text Y = 16.
 * - F0320:1765-1772:  non-inventory 1/2/3-digit x-stride adds 19/16/13.
 * - F0320:1773:       non-inventory text Y = 5.
 * - F0320:1775:       F0053_TEXT_PrintToLogicalScreen(x, y, C15, C08, F0288).
 */
#define DM1_V1_CPDI_BOX_TOP_PC34 0
#define DM1_V1_CPDI_BOX_BOTTOM_INVENTORY_PC34 28
#define DM1_V1_CPDI_BOX_BOTTOM_NONINVENTORY_PC34 6
#define DM1_V1_CPDI_BOX_LEFT_OFFSET_INVENTORY_PC34 7
#define DM1_V1_CPDI_BOX_RIGHT_OFFSET_INVENTORY_PC34 31
#define DM1_V1_CPDI_BOX_LEFT_OFFSET_NONINVENTORY_PC34 0
#define DM1_V1_CPDI_BOX_RIGHT_OFFSET_NONINVENTORY_PC34 47
#define DM1_V1_CPDI_BOX_BYTE_WIDTH_INVENTORY_PC34 16
#define DM1_V1_CPDI_BOX_BYTE_WIDTH_NONINVENTORY_PC34 24
#define DM1_V1_CPDI_TEXT_Y_INVENTORY_PC34 16
#define DM1_V1_CPDI_TEXT_Y_NONINVENTORY_PC34 5
#define DM1_V1_CPDI_TEXT_X_STRIDE_1DIGIT_INVENTORY_PC34 21
#define DM1_V1_CPDI_TEXT_X_STRIDE_2DIGIT_INVENTORY_PC34 18
#define DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_INVENTORY_PC34 15
#define DM1_V1_CPDI_TEXT_X_STRIDE_1DIGIT_NONINVENTORY_PC34 19
#define DM1_V1_CPDI_TEXT_X_STRIDE_2DIGIT_NONINVENTORY_PC34 16
#define DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_NONINVENTORY_PC34 13
#define DM1_V1_CPDI_TEXT_X_STRIDE_NO_DAMAGE_PC34 0
#define DM1_V1_CPDI_CHAMPION_X_STRIDE_PC34 69
#define DM1_V1_CPDI_DIGIT_COUNT_NO_DAMAGE_PC34 0
#define DM1_V1_CPDI_DIGIT_COUNT_1_PC34 1
#define DM1_V1_CPDI_DIGIT_COUNT_2_PC34 2
#define DM1_V1_CPDI_DIGIT_COUNT_3_PC34 3
#define DM1_V1_CPDI_DIGIT_THRESHOLD_1_PC34 10
#define DM1_V1_CPDI_DIGIT_THRESHOLD_2_PC34 100
#define DM1_V1_CPDI_DIGIT_THRESHOLD_3_PC34 1000

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
    /*
     * F0320 PC34 MEDIA009 box-geometry / text-stride fields
     * (CHAMPION.C F0320:1744-1775).
     *
     * damage == 0 short-circuits in F0320:1736 (continue), so the
     * centered F0053 text print never runs; the gate still records
     * digit_count = 0 and all offsets/stride = 0 in that case so a
     * downstream checker can confirm the early-return.
     */
    int damage_box_top;
    int damage_box_bottom;
    int damage_box_left_offset;
    int damage_box_right_offset;
    int damage_box_byte_width;
    int damage_text_x_offset;
    int damage_text_y;
    int damage_digit_count;
    int champion_x_base;
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
