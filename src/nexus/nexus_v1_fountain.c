
#include "nexus_v1_fountain.h"
#include <string.h>

void nexus_v1_fountain_manager_init(Nexus_FountainManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

int nexus_v1_fountain_register(Nexus_FountainManager *mgr,
    int type, int map_x, int map_y,
    int uses, int restore_amount) {
    Nexus_Fountain *f;
    if (!mgr || mgr->count >= NEXUS_MAX_FOUNTAINS) return -1;
    f = &mgr->fountains[mgr->count];
    f->active = 1;
    f->type = type;
    f->map_x = map_x;
    f->map_y = map_y;
    f->uses_remaining = uses;
    f->restore_amount = restore_amount;
    return mgr->count++;
}

int nexus_v1_fountain_find_at(const Nexus_FountainManager *mgr,
    int map_x, int map_y) {
    int i;
    if (!mgr) return -1;
    for (i = 0; i < mgr->count; i++) {
        if (mgr->fountains[i].active &&
            mgr->fountains[i].map_x == map_x &&
            mgr->fountains[i].map_y == map_y)
            return i;
    }
    return -1;
}

int nexus_v1_fountain_drink(Nexus_FountainManager *mgr,
    int fountain_idx, Nexus_V1_Champion *champion) {
    Nexus_Fountain *f;
    if (!mgr || !champion) return 0;
    if (fountain_idx < 0 || fountain_idx >= mgr->count) return 0;
    f = &mgr->fountains[fountain_idx];
    if (!f->active || f->uses_remaining == 0) return 0;

    switch (f->type) {
    case NEXUS_FOUNTAIN_WATER:
        champion->water += f->restore_amount;
        if (champion->water > 1500) champion->water = 1500;
        break;
    case NEXUS_FOUNTAIN_HEALTH:
        champion->health += f->restore_amount;
        if (champion->health > champion->max_health)
            champion->health = champion->max_health;
        break;
    case NEXUS_FOUNTAIN_MANA:
        champion->mana += f->restore_amount;
        if (champion->mana > champion->max_mana)
            champion->mana = champion->max_mana;
        break;
    case NEXUS_FOUNTAIN_POISON:
        champion->health -= f->restore_amount;
        if (champion->health < 0) champion->health = 0;
        break;
    default:
        return 0;
    }

    if (f->uses_remaining > 0)
        f->uses_remaining--;
    return 1;
}

int nexus_v1_fountain_uses_left(const Nexus_FountainManager *mgr,
    int fountain_idx) {
    if (!mgr || fountain_idx < 0 || fountain_idx >= mgr->count) return 0;
    return mgr->fountains[fountain_idx].uses_remaining;
}
