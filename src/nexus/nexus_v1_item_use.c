
#include "nexus_v1_item_use.h"
#include <string.h>

int nexus_v1_item_can_use(const Nexus_ItemDef *item) {
    if (!item) return 0;
    if (item->flags & NEXUS_ITEMF_CONSUMABLE) return 1;
    if (item->category == NEXUS_ITEM_FOOD) return 1;
    if (item->category == NEXUS_ITEM_POTION) return 1;
    if (item->category == NEXUS_ITEM_SCROLL) return 1;
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

    if (attribute > 0) {
        champion->health += attribute;
        if (champion->health > champion->max_health)
            champion->health = champion->max_health;
        r.health_restored = attribute;
    } else if (attribute < 0) {
        champion->mana += (-attribute);
        if (champion->mana > champion->max_mana)
            champion->mana = champion->max_mana;
        r.mana_restored = -attribute;
    }

    r.result = NEXUS_USE_RESULT_CONSUMED;
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

    switch (item->category) {
    case NEXUS_ITEM_FOOD:
        champion->food += 500;
        if (champion->food > 2048) champion->food = 2048;
        champion->stamina += 100;
        if (champion->stamina > champion->max_stamina)
            champion->stamina = champion->max_stamina;
        r.food_restored = 500;
        r.stamina_restored = 100;
        r.result = NEXUS_USE_RESULT_CONSUMED;
        break;
    case NEXUS_ITEM_POTION:
        r = nexus_v1_potion_effect(champion, status, item->attribute);
        break;
    case NEXUS_ITEM_SCROLL:
        r.result = NEXUS_USE_RESULT_CONSUMED;
        break;
    default:
        r.result = NEXUS_USE_RESULT_FAILED;
        break;
    }
    return r;
}
