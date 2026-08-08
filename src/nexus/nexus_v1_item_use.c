
#include "nexus_v1_item_use.h"
#include <string.h>

int nexus_v1_item_can_use(const Nexus_ItemDef *item) {
    /* ITEM.IBS is declaration/icon/material evidence only.  No Saturn
     * action trace has authenticated a usable-item producer or consumer. */
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

    if (!champion) {
        r.result = NEXUS_USE_RESULT_FAILED;
        return r;
    }
    (void)status;
    (void)attribute;
    /* The Word36 value is not an authenticated Saturn effect encoding. */
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

    (void)status;
    /* Do not infer food/potion/scroll effects from ITEM.IBS. */
    return r;
}
