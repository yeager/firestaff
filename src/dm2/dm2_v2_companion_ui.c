
#include "dm2_v2_companion_ui.h"

void dm2_v2_companion_ui_update(DM2_V2_CompanionDisplay *d, int companion_id,
    float health_pct, float loyalty_pct, int action)
{
    if (!d) return;
    d->companion_id = companion_id;
    d->health_bar_width = health_pct < 0 ? 0 : (health_pct > 1 ? 1 : health_pct);
    d->loyalty_bar_width = loyalty_pct < 0 ? 0 : (loyalty_pct > 1 ? 1 : loyalty_pct);
    d->action_icon = action;
}

const char *dm2_v2_companion_ui_source_evidence(void) {
    return "DM2 V2.2: companion portrait, health/loyalty bars, action icons\n";
}

