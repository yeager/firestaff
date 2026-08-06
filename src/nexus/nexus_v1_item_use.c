
#include "nexus_v1_item_use.h"
#include <string.h>

int nexus_v1_item_can_use(const Nexus_ItemDef *item) {
    /* ITEM.IBS identifies declarations and icon/material references only.
     * Nexus action dispatch and item semantics are not authenticated yet;
     * never advertise the inherited DM1 categories as usable in production. */
    (void)item;
    return 0;
}

Nexus_ItemUseResult nexus_v1_potion_effect(Nexus_V1_Champion *champion,
                                            Nexus_StatusEffects *status,
                                            int attribute) {
    Nexus_ItemUseResult r;
    memset(&r, 0, sizeof(r));
    r.result = NEXUS_USE_RESULT_NONE;
    r.status_applied = -1;

    /* ITEM.IBS Word36 is not an authenticated effect magnitude. Keep this
     * legacy-shaped helper available for ABI compatibility, but do not turn
     * a declaration field into DM1 potion semantics without Saturn trace
     * evidence for the action consumer. */
    (void)champion;
    (void)status;
    (void)attribute;
    return r;
}

Nexus_ItemUseResult nexus_v1_item_use(Nexus_V1_Champion *champion,
                                       Nexus_StatusEffects *status,
                                       const Nexus_ItemDef *item) {
    Nexus_ItemUseResult r;
    memset(&r, 0, sizeof(r));
    r.result = NEXUS_USE_RESULT_NONE;
    r.status_applied = -1;

    if (!champion || !item) {
        r.result = NEXUS_USE_RESULT_FAILED;
        return r;
    }
    /* No Saturn action/event consumer is bound. In particular, do not
     * consume inventory, alter champion state, or invoke inherited DM1
     * food/potion/scroll semantics from ITEM.IBS declaration bytes. */
    (void)status;
    (void)item;
    return r;
}
