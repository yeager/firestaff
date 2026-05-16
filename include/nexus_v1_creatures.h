
#ifndef NEXUS_V1_CREATURES_H
#define NEXUS_V1_CREATURES_H
#include <stdint.h>

/* Nexus creature types — loaded from MNS files.
 * Each creature has a DMDF 3D model + stats. */

#define NEXUS_MAX_CREATURE_TYPES 64
#define NEXUS_MAX_ACTIVE_CREATURES 128

typedef struct {
    char name[32];
    char model_file[32];  /* e.g. "SCORPION.MNS" */
    int health, attack, defense, speed;
    int experience_value;
    int model_index;      /* index into engine->models[] */
} Nexus_CreatureType;

typedef struct {
    int type_index;
    int health;
    int x, y;       /* map position */
    int facing;
    int alive;
    int state;       /* 0=idle, 1=patrol, 2=chase, 3=attack, 4=flee */
    int ai_timer;
} Nexus_Creature;

typedef struct {
    Nexus_CreatureType types[NEXUS_MAX_CREATURE_TYPES];
    int type_count;
    Nexus_Creature active[NEXUS_MAX_ACTIVE_CREATURES];
    int active_count;
} Nexus_V1_CreatureManager;

void nexus_v1_creatures_init(Nexus_V1_CreatureManager *mgr);
int nexus_v1_creature_spawn(Nexus_V1_CreatureManager *mgr, int type_idx, int x, int y, int dir);
void nexus_v1_creatures_tick(Nexus_V1_CreatureManager *mgr, int party_x, int party_y);

#endif

