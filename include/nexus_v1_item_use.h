
#ifndef NEXUS_V1_ITEM_USE_H
#define NEXUS_V1_ITEM_USE_H

/* Nexus V1 item use effects — consumable item actions.
 * Source: DM1 COMMAND.C F0412 item use dispatch, CHAMPION.C food/water/potion.
 * Nexus ITEM.IBS attribute field (Word36) carries the effect magnitude. */

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

/* Use a consumable item on a champion.
 * Checks item category and applies effect based on type:
 *   FOOD: restores food (attribute = amount)
 *   POTION: heals health or restores mana (attribute = amount)
 *   SCROLL: not consumed here (handled by spell system)
 * Returns result describing what happened. */
Nexus_ItemUseResult nexus_v1_item_use(Nexus_V1_Champion *champion,
                                       Nexus_StatusEffects *status,
                                       const Nexus_ItemDef *item);

/* Check if an item can be used (is consumable). */
int nexus_v1_item_can_use(const Nexus_ItemDef *item);

/* Apply a potion effect. Sub-types based on item attribute:
 *   0-49:   health potion (restores attribute HP)
 *   50-99:  mana potion (restores attribute-50 MP)
 *   100-149: stamina potion (restores attribute-100 SP)
 *   150-199: antidote (removes poison)
 *   200-249: shield potion (applies shield status)
 *   250+:    haste potion (applies haste status) */
Nexus_ItemUseResult nexus_v1_potion_effect(Nexus_V1_Champion *champion,
                                            Nexus_StatusEffects *status,
                                            int attribute);

#endif
