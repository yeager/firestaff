
#include "nexus_v1_item_use.h"
#include <string.h>

int nexus_v1_item_can_use(const Nexus_ItemDef *item) {
    if (!item) return 0;
    if (item->flags & NEXUS_ITEMF_CONSUMABLE) return 1;
    if (item->category == NEXUS_ITEM_FOOD) return 1;
    if (item->category == NEXUS_ITEM_POTION) return 1;
    return 0;
}

Nexus_ItemUseResult nexus_v1_potion_effect(Nexus_V1_Champion *champion,
                                            Nexus_StatusEffects *status,
                                            int attribute) {
    Nexus_ItemUseResult r;
    memset(&r, 0, sizeof(r));
    r.result = NEXUS_USE_RESULT_CONSUMED;
    r.status_applied = -1;

    if (!champion) { r.result = NEXUS_USE_RESULT_FAILED; return r; }

    if (attribute < 50) {
        int heal = attribute > 0 ? attribute : 10;
        champion->health += heal;
        if (champion->health > champion->max_health)
            champion->health = champion->max_health;
        r.health_restored = heal;
    } else if (attribute < 100) {
        int mana = attribute - 50;
        if (mana <= 0) mana = 10;
        champion->mana += mana;
        if (champion->mana > champion->max_mana)
            champion->mana = champion->max_mana;
        r.mana_restored = mana;
    } else if (attribute < 150) {
        int stam = attribute - 100;
        if (stam <= 0) stam = 10;
        champion->stamina += stam;
        if (champion->stamina > champion->max_stamina)
            champion->stamina = champion->max_stamina;
        r.stamina_restored = stam;
    } else if (attribute < 200) {
        if (status)
            nexus_v1_status_remove(status, NEXUS_STATUS_POISON);
        r.status_applied = NEXUS_STATUS_POISON;
    } else if (attribute < 250) {
        int str = attribute - 200;
        if (str <= 0) str = 10;
        if (status)
            nexus_v1_status_apply(status, NEXUS_STATUS_SHIELD, 300, str);
        r.status_applied = NEXUS_STATUS_SHIELD;
        r.status_duration = 300;
        r.status_strength = str;
    } else {
        int str = attribute - 250;
        if (str <= 0) str = 5;
        if (status)
            nexus_v1_status_apply(status, NEXUS_STATUS_HASTE, 200, str);
        r.status_applied = NEXUS_STATUS_HASTE;
        r.status_duration = 200;
        r.status_strength = str;
    }
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
    case NEXUS_ITEM_FOOD: {
        int amount = item->attribute > 0 ? item->attribute : 5;
        champion->food += amount;
        if (champion->food > 255) champion->food = 255;
        r.food_restored = amount;
        r.result = NEXUS_USE_RESULT_CONSUMED;
        break;
    }
    case NEXUS_ITEM_POTION:
        return nexus_v1_potion_effect(champion, status, item->attribute);
    case NEXUS_ITEM_SCROLL:
        r.result = NEXUS_USE_RESULT_NONE;
        break;
    default:
        if (item->flags & NEXUS_ITEMF_CONSUMABLE) {
            r.result = NEXUS_USE_RESULT_CONSUMED;
        } else {
            r.result = NEXUS_USE_RESULT_FAILED;
        }
        break;
    }
    return r;
}
