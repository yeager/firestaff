
#ifndef NEXUS_V1_ITEM_USE_H
#define NEXUS_V1_ITEM_USE_H

/* Nexus V1 item-use ABI boundary.
 * ITEM.IBS proves declaration/icon/material data only. The Saturn action
 * consumer is not authenticated, so the production helpers below remain
 * no-op until an event/action trace binds their semantics. */

#include "nexus_v1_champions.h"
#include "nexus_v1_inventory.h"
#include "nexus_v1_status.h"

#define NEXUS_USE_RESULT_NONE      0
#define NEXUS_USE_RESULT_CONSUMED  1
#define NEXUS_USE_RESULT_EQUIPPED  2
#define NEXUS_USE_RESULT_FAILED    3
#define NEXUS_USE_RESULT_THROWN    4

typedef struct {
    int result;           /* NEXUS_USE_RESULT_* */
    int health_restored;
    int mana_restored;
    int stamina_restored;
    int food_restored;
    int water_restored;
    int status_applied;   /* NEXUS_STATUS_* or -1 */
    int status_duration;
    int status_strength;
} Nexus_ItemUseResult;

/* Attempt an item use without mutating state. Returns NONE while the
 * Saturn action consumer is unbound, or FAILED for invalid arguments. */
Nexus_ItemUseResult nexus_v1_item_use(Nexus_V1_Champion *champion,
                                       Nexus_StatusEffects *status,
                                       const Nexus_ItemDef *item);

/* No item is advertised as usable until Saturn action semantics are bound. */
int nexus_v1_item_can_use(const Nexus_ItemDef *item);

/* Legacy ABI-shaped helper. It is intentionally no-op: ITEM.IBS Word36 is
 * not an authenticated potion-effect encoding. */
Nexus_ItemUseResult nexus_v1_potion_effect(Nexus_V1_Champion *champion,
                                            Nexus_StatusEffects *status,
                                            int attribute);

#endif
