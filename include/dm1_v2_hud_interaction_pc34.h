#ifndef FIRESTAFF_DM1_V2_HUD_INTERACTION_PC34_H
#define FIRESTAFF_DM1_V2_HUD_INTERACTION_PC34_H

#include "touch_click_zone_matrix_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum M11_V2_HudTouchKind {
    M11_V2_HUD_TOUCH_NONE_PC34 = 0,
    M11_V2_HUD_TOUCH_CHAMPION_FOCUS_PC34 = 1,
    M11_V2_HUD_TOUCH_CHAMPION_HAND_PC34 = 2,
    M11_V2_HUD_TOUCH_ACTION_PARENT_PC34 = 3,
    M11_V2_HUD_TOUCH_ACTION_ROW_PC34 = 4,
    M11_V2_HUD_TOUCH_ACTION_ICON_PC34 = 5
} M11_V2_HudTouchKind;

typedef struct M11_V2_HudTouchResult {
    int hit;
    M11_V2_HudTouchKind kind;
    unsigned int commandId;
    unsigned int zoneIndex;
    unsigned int championIndex;
    unsigned int slotIndex;
    int screenX;
    int screenY;
    const char* groupName;
    const char* sourceEvidence;
} M11_V2_HudTouchResult;

int v2_hud_interaction_dispatch_screen_click(int screenX,
                                             int screenY,
                                             unsigned int buttonMask,
                                             M11_V2_HudTouchResult* outResult);
int v2_hud_interaction_dispatch_scaled_click(int physicalX,
                                             int physicalY,
                                             int surfaceW,
                                             int surfaceH,
                                             unsigned int buttonMask,
                                             M11_V2_HudTouchResult* outResult);
unsigned int v2_hud_interaction_source_lock_ok(void);
const char* v2_hud_interaction_get_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
