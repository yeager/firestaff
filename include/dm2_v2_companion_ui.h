
#ifndef FIRESTAFF_DM2_V2_COMPANION_UI_H
#define FIRESTAFF_DM2_V2_COMPANION_UI_H
#include <stdint.h>

/* DM2 V2.2 Companion UI — enhanced NPC companion display.
 * V1: companion stats shown in basic text.
 * V2.2: companion portrait, health bar, loyalty meter, action icons. */

typedef struct {
    int companion_id;
    float health_bar_width;
    float loyalty_bar_width;
    uint32_t portrait_frame;
    int action_icon; /* 0=follow, 1=guard, 2=attack */
} DM2_V2_CompanionDisplay;

void dm2_v2_companion_ui_update(DM2_V2_CompanionDisplay *d, int companion_id,
    float health_pct, float loyalty_pct, int action);
const char *dm2_v2_companion_ui_source_evidence(void);
#endif

