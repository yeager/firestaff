
#include "nexus_v1_spawner.h"
#include <string.h>

void nexus_v1_spawners_init(Nexus_SpawnerManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

int nexus_v1_spawner_add(Nexus_SpawnerManager *mgr,
                          int x, int y, int level,
                          int creature_type,
                          int respawn_delay,
                          int max_spawns) {
    int i;
    if (!mgr) return -1;
    for (i = 0; i < NEXUS_MAX_SPAWNERS; i++) {
        if (!mgr->spawners[i].active) {
            mgr->spawners[i].active = 1;
            mgr->spawners[i].x = x;
            mgr->spawners[i].y = y;
            mgr->spawners[i].level = level;
            mgr->spawners[i].creature_type = creature_type;
            mgr->spawners[i].respawn_delay = respawn_delay > 0 ? respawn_delay : NEXUS_SPAWNER_DEFAULT_DELAY;
            mgr->spawners[i].timer = 0;
            mgr->spawners[i].spawned_creature = -1;
            mgr->spawners[i].max_spawns = max_spawns;
            mgr->spawners[i].spawn_count = 0;
            mgr->count++;
            return i;
        }
    }
    return -1;
}

void nexus_v1_spawner_on_creature_death(Nexus_SpawnerManager *mgr,
                                         int creature_index) {
    int i;
    if (!mgr) return;
    for (i = 0; i < NEXUS_MAX_SPAWNERS; i++) {
        if (mgr->spawners[i].active &&
            mgr->spawners[i].spawned_creature == creature_index) {
            mgr->spawners[i].spawned_creature = -1;
            mgr->spawners[i].timer = mgr->spawners[i].respawn_delay;
        }
    }
}

int nexus_v1_spawners_tick(Nexus_SpawnerManager *mgr,
                            Nexus_SpawnEvent *out_events, int max_events) {
    int i, event_count = 0;
    if (!mgr || !out_events || max_events <= 0) return 0;

    for (i = 0; i < NEXUS_MAX_SPAWNERS && event_count < max_events; i++) {
        Nexus_Spawner *s = &mgr->spawners[i];
        if (!s->active) continue;
        if (s->spawned_creature >= 0) continue;
        if (s->max_spawns > 0 && s->spawn_count >= s->max_spawns) continue;

        if (s->timer > 0) {
            s->timer--;
            if (s->timer <= 0) {
                out_events[event_count].spawner_index = i;
                out_events[event_count].creature_type = s->creature_type;
                out_events[event_count].x = s->x;
                out_events[event_count].y = s->y;
                out_events[event_count].level = s->level;
                s->spawn_count++;
                event_count++;
            }
        }
    }
    return event_count;
}
