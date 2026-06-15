#ifndef DM1_V1_CHAMPION_PANEL_PORTRAIT_PC34_COMPAT_H
#define DM1_V1_CHAMPION_PANEL_PORTRAIT_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPPOR_CHAMPION_COUNT_PC34 4
#define DM1_V1_CPPOR_STATUS_BOX_SPACING_PC34 69
#define DM1_V1_CPPOR_STATUS_BOX_PORTRAIT_LEFT_MARGIN_PC34 7
#define DM1_V1_CPPOR_PORTRAIT_WIDTH_PC34 32
#define DM1_V1_CPPOR_PORTRAIT_HEIGHT_PC34 29
#define DM1_V1_CPPOR_PORTRAIT_BYTE_WIDTH_PC34 16
#define DM1_V1_CPPOR_SCREEN_PIXEL_WIDTH_PC34 320
#define DM1_V1_CPPOR_ZONE_FIRST_STATUS_BOX_PC34 175
#define DM1_V1_CPPOR_NO_TRANSPARENCY_PC34 -1
#define DM1_V1_CPPOR_HATCH_COLOR_PC34 12

typedef enum DM1_V1_ChampionPanelPortraitOpPc34Compat {
    DM1_V1_CPPOR_OP_NONE_PC34 = 0,
    DM1_V1_CPPOR_OP_GET_STATUS_BOX_ZONE_PC34 = 1,
    DM1_V1_CPPOR_OP_BLIT_CHAMPION_PORTRAIT_PC34 = 2,
    DM1_V1_CPPOR_OP_HATCH_INVISIBILITY_PC34 = 3
} DM1_V1_ChampionPanelPortraitOpPc34Compat;

typedef struct DM1_V1_ChampionPanelPortraitEvidencePc34Compat {
    bool contract_only;
    const char *drawstate_anchor;
    const char *portrait_anchor;
    const char *defs_anchor;
    const char *coord_anchor;
    const char *scope_note;
    const char *no_real_asset_claim;
} DM1_V1_ChampionPanelPortraitEvidencePc34Compat;

typedef struct DM1_V1_ChampionPanelPortraitInputPc34Compat {
    int contract_only;
    int champion_index;
    int inventory_champion_ordinal;
    int current_health;
    int party_invisibility_count;
} DM1_V1_ChampionPanelPortraitInputPc34Compat;

typedef struct DM1_V1_ChampionPanelPortraitResultPc34Compat {
    bool valid;
    bool contract_only;
    bool rejected_champion_index;
    bool rejected_inventory_ordinal;
    bool rejected_invisibility_count;
    bool is_inventory_champion;
    bool is_alive;
    bool should_draw_portrait;
    bool hatches_for_invisibility;
    int champion_index;
    int champion_ordinal;
    int inventory_champion_ordinal;
    int status_box_zone;
    int target_left;
    int target_top;
    int target_right;
    int target_bottom;
    int target_width;
    int target_height;
    int portrait_source_x;
    int portrait_source_y;
    int portrait_width;
    int portrait_height;
    int portrait_byte_width;
    int screen_pixel_width;
    int transparent_color;
    int hatch_color;
    DM1_V1_ChampionPanelPortraitOpPc34Compat operations[3];
    int operation_count;
    const DM1_V1_ChampionPanelPortraitEvidencePc34Compat *evidence;
} DM1_V1_ChampionPanelPortraitResultPc34Compat;

const DM1_V1_ChampionPanelPortraitEvidencePc34Compat *
DM1_V1_ChampionPanelPortrait_EvidencePc34Compat(void);

const char *
DM1_V1_ChampionPanelPortrait_SourceEvidencePc34Compat(void);

void DM1_V1_ChampionPanelPortrait_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelPortraitInputPc34Compat *input);

int DM1_V1_ChampionPanelPortrait_BuildPc34Compat(
    const DM1_V1_ChampionPanelPortraitInputPc34Compat *input,
    DM1_V1_ChampionPanelPortraitResultPc34Compat *out_result);

#ifdef __cplusplus
}
#endif

#endif
