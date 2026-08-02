
#include "nexus_v1_throw.h"
#include <string.h>

void nexus_v1_throw_init(Nexus_ThrownItemManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

Nexus_ThrowProfile nexus_v1_throw_profile(const Nexus_ItemDef *item) {
    Nexus_ThrowProfile p;
    p.proj_type = NEXUS_PROJ_ARROW;
    p.base_damage = 10;
    p.speed_ticks = 4;

    if (!item) return p;

    switch (item->category) {
    case NEXUS_ITEM_WEAPON:
        p.base_damage = item->attribute > 0 ? item->attribute : 15;
        p.speed_ticks = 3;
        break;
    case NEXUS_ITEM_MISC:
        p.base_damage = 5;
        p.speed_ticks = 4;
        break;
    default:
        p.base_damage = 3;
        p.speed_ticks = 5;
        break;
    }

    return p;
}

int nexus_v1_throw_item(Nexus_ThrownItemManager *mgr,
                         Nexus_ProjectileManager *proj_mgr,
                         Nexus_V1_Champion *champion,
                         int champion_index,
                         int slot,
                         const Nexus_ItemDef *item_defs,
                         int party_x, int party_y, int party_dir) {
    int item_idx, proj_slot, i;
    Nexus_ThrowProfile profile;
    const Nexus_ItemDef *item;

    if (!mgr || !proj_mgr || !champion || !item_defs) return 0;
    if (slot < 0 || slot >= NEXUS_EQUIP_SLOTS) return 0;

    item_idx = champion->slots[slot];
    if (item_idx < 0) return 0;

    item = &item_defs[item_idx];
    profile = nexus_v1_throw_profile(item);

    proj_slot = nexus_v1_projectile_spawn(proj_mgr,
        profile.proj_type,
        party_x, party_y, party_dir,
        profile.base_damage + champion->strength / 4,
        profile.speed_ticks,
        champion_index);

    if (proj_slot < 0) return 0;

    champion->slots[slot] = -1;

    for (i = 0; i < NEXUS_MAX_THROWN; i++) {
        if (!mgr->items[i].active) {
            mgr->items[i].active = 1;
            mgr->items[i].item_index = item_idx;
            mgr->items[i].projectile_slot = proj_slot;
            mgr->items[i].source_champion = champion_index;
            mgr->count++;
            return 1;
        }
    }

    return 1;
}

int nexus_v1_throw_on_hit(Nexus_ThrownItemManager *mgr, int projectile_slot) {
    int i, item_idx;
    if (!mgr) return -1;

    for (i = 0; i < NEXUS_MAX_THROWN; i++) {
        if (mgr->items[i].active && mgr->items[i].projectile_slot == projectile_slot) {
            item_idx = mgr->items[i].item_index;
            mgr->items[i].active = 0;
            mgr->count--;
            return item_idx;
        }
    }
    return -1;
}

int nexus_v1_throw_is_thrown(const Nexus_ThrownItemManager *mgr, int projectile_slot) {
    int i;
    if (!mgr) return 0;
    for (i = 0; i < NEXUS_MAX_THROWN; i++) {
        if (mgr->items[i].active && mgr->items[i].projectile_slot == projectile_slot)
            return 1;
    }
    return 0;
}
