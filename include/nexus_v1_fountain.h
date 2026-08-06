
#ifndef NEXUS_V1_FOUNTAIN_H
#define NEXUS_V1_FOUNTAIN_H

/* Nexus V1 fountain/well provenance seam. No retail fountain record,
 * effect magnitude, or Saturn action dispatcher is currently authenticated;
 * registration and drinking therefore remain fail-closed/no-op. */

#include "nexus_v1_champions.h"

#define NEXUS_MAX_FOUNTAINS 32

enum {
    NEXUS_FOUNTAIN_WATER  = 0,
    NEXUS_FOUNTAIN_HEALTH = 1,
    NEXUS_FOUNTAIN_MANA   = 2,
    NEXUS_FOUNTAIN_POISON = 3
};

typedef struct {
    int active;
    int type;
    int map_x, map_y;
    int uses_remaining;
    int restore_amount;
} Nexus_Fountain;

typedef struct {
    Nexus_Fountain fountains[NEXUS_MAX_FOUNTAINS];
    int count;
} Nexus_FountainManager;

void nexus_v1_fountain_manager_init(Nexus_FountainManager *mgr);

int nexus_v1_fountain_register(Nexus_FountainManager *mgr,
    int type, int map_x, int map_y,
    int uses, int restore_amount);

int nexus_v1_fountain_find_at(const Nexus_FountainManager *mgr,
    int map_x, int map_y);

/* Drink from an admitted fountain. Current production implementation is
 * always no-op until the original Saturn action/effect consumer is bound. */
int nexus_v1_fountain_drink(Nexus_FountainManager *mgr,
    int fountain_idx, Nexus_V1_Champion *champion);

int nexus_v1_fountain_uses_left(const Nexus_FountainManager *mgr,
    int fountain_idx);

#endif
