
#ifndef NEXUS_V1_SPAWNER_H
#define NEXUS_V1_SPAWNER_H

/* Nexus V1 creature spawner — timed creature respawning at dungeon locations.
 * Spawners activate after a delay when their creature dies, producing
 * a new creature of the same type at the spawn position.
 * Source: DM1 DUNGEON.C creature generator things,
 *         ReDMCSB OBJECTMAN.C F0258 generator processing,
 *         DM.BIN yam\dungeon.c creature generator tick. */

#include "nexus_v1_creatures.h"

#define NEXUS_MAX_SPAWNERS 32
#define NEXUS_SPAWNER_DEFAULT_DELAY 300

typedef struct {
    int active;
    int x, y, level;
    int creature_type;
    int respawn_delay;
    int timer;
    int spawned_creature;
    int max_spawns;
    int spawn_count;
} Nexus_Spawner;

typedef struct {
    Nexus_Spawner spawners[NEXUS_MAX_SPAWNERS];
    int count;
} Nexus_SpawnerManager;

void nexus_v1_spawners_init(Nexus_SpawnerManager *mgr);

/* Register a spawner. Returns index or -1. */
int nexus_v1_spawner_add(Nexus_SpawnerManager *mgr,
                          int x, int y, int level,
                          int creature_type,
                          int respawn_delay,
                          int max_spawns);

/* Notify that a creature died — starts the respawn timer
 * for any spawner that produced it. */
void nexus_v1_spawner_on_creature_death(Nexus_SpawnerManager *mgr,
                                         int creature_index);

/* Tick spawners. Returns bitmask of spawner indices that spawned this tick.
 * Caller must create the actual creature via the creature manager. */
typedef struct {
    int spawner_index;
    int creature_type;
    int x, y, level;
} Nexus_SpawnEvent;

int nexus_v1_spawners_tick(Nexus_SpawnerManager *mgr,
                            Nexus_SpawnEvent *out_events, int max_events);

#endif
