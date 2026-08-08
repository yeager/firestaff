/* Capture-gated Nexus V1 item-use production adapter.
 * ITEM.IBS proves declarations/icons/material data, not Saturn effects. */

#include "nexus_v1_item_use.h"
#include <string.h>

int nexus_v1_item_can_use(const Nexus_ItemDef *item)
{
    (void)item;
    return 0;
}

Nexus_ItemUseResult nexus_v1_potion_effect(Nexus_V1_Champion *champion,
                                            Nexus_StatusEffects *status,
                                            int attribute)
{
    Nexus_ItemUseResult result;
    memset(&result, 0, sizeof(result));
    result.result = champion ? NEXUS_USE_RESULT_NONE : NEXUS_USE_RESULT_FAILED;
    result.status_applied = -1;
    (void)status;
    (void)attribute;
    return result;
}

Nexus_ItemUseResult nexus_v1_item_use(Nexus_V1_Champion *champion,
                                       Nexus_StatusEffects *status,
                                       const Nexus_ItemDef *item)
{
    Nexus_ItemUseResult result;
    memset(&result, 0, sizeof(result));
    result.result = (champion && item) ? NEXUS_USE_RESULT_NONE
                                       : NEXUS_USE_RESULT_FAILED;
    result.status_applied = -1;
    (void)status;
    return result;
}
