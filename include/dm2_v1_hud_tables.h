#ifndef FIRESTAFF_DM2_V1_HUD_TABLES_H
#define FIRESTAFF_DM2_V1_HUD_TABLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t gdat_category;
    int16_t button_id;
    int16_t click_target;
    int8_t  action_type;
} DM2_V1_HudButtonDesc;

typedef struct {
    int8_t  flags;
    int8_t  param;
    int16_t rect_id;
} DM2_V1_HudPanelEntry;

typedef struct {
    int8_t  gdat_flag;
    int8_t  param;
    int16_t icon_id;
} DM2_V1_HudActionIcon;

#define DM2_V1_HUD_BUTTON_COUNT     62
#define DM2_V1_HUD_CLICKMAP_COUNT   83
#define DM2_V1_HUD_PANEL_COUNT      76
#define DM2_V1_HUD_ACTION_ICON_COUNT 10

extern const DM2_V1_HudButtonDesc dm2_v1_hud_button_desc[DM2_V1_HUD_BUTTON_COUNT];
extern const int8_t dm2_v1_hud_clickmap[DM2_V1_HUD_CLICKMAP_COUNT];
extern const DM2_V1_HudPanelEntry dm2_v1_hud_panel_layout[DM2_V1_HUD_PANEL_COUNT];
extern const DM2_V1_HudActionIcon dm2_v1_hud_action_icons[DM2_V1_HUD_ACTION_ICON_COUNT];

const char *dm2_v1_hud_tables_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
