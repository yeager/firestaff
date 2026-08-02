
#include <stdio.h>
#include <string.h>
#include "nexus_v1_spawner.h"

int main(void) {
    int fail = 0;

    /* Test 1: init */
    {
        Nexus_SpawnerManager mgr;
        nexus_v1_spawners_init(&mgr);
        if (mgr.count != 0) {
            fprintf(stderr, "FAIL: init\n"); fail++;
        } else {
            printf("  Init OK\n");
        }
    }

    /* Test 2: add spawner */
    {
        Nexus_SpawnerManager mgr;
        int idx;
        nexus_v1_spawners_init(&mgr);
        idx = nexus_v1_spawner_add(&mgr, 10, 15, 3, 5, 100, 0);
        if (idx < 0 || mgr.count != 1) {
            fprintf(stderr, "FAIL: add idx=%d\n", idx); fail++;
        } else if (mgr.spawners[idx].creature_type != 5 ||
                   mgr.spawners[idx].respawn_delay != 100) {
            fprintf(stderr, "FAIL: add data\n"); fail++;
        } else {
            printf("  Add spawner OK\n");
        }
    }

    /* Test 3: no spawn without death notification */
    {
        Nexus_SpawnerManager mgr;
        Nexus_SpawnEvent events[4];
        int n, i;
        nexus_v1_spawners_init(&mgr);
        nexus_v1_spawner_add(&mgr, 5, 5, 0, 2, 10, 0);
        mgr.spawners[0].spawned_creature = 7;
        for (i = 0; i < 20; i++)
            n = nexus_v1_spawners_tick(&mgr, events, 4);
        if (n != 0) {
            fprintf(stderr, "FAIL: spawned without death\n"); fail++;
        } else {
            printf("  No spawn while creature alive OK\n");
        }
    }

    /* Test 4: spawn after death + delay */
    {
        Nexus_SpawnerManager mgr;
        Nexus_SpawnEvent events[4];
        int idx, n, i;
        nexus_v1_spawners_init(&mgr);
        idx = nexus_v1_spawner_add(&mgr, 8, 8, 1, 3, 5, 0);
        mgr.spawners[idx].spawned_creature = 12;
        nexus_v1_spawner_on_creature_death(&mgr, 12);
        if (mgr.spawners[idx].timer != 5) {
            fprintf(stderr, "FAIL: timer not set=%d\n", mgr.spawners[idx].timer); fail++;
        } else {
            for (i = 0; i < 4; i++)
                nexus_v1_spawners_tick(&mgr, events, 4);
            n = nexus_v1_spawners_tick(&mgr, events, 4);
            if (n != 1 || events[0].creature_type != 3 ||
                events[0].x != 8 || events[0].y != 8) {
                fprintf(stderr, "FAIL: spawn event n=%d\n", n); fail++;
            } else {
                printf("  Spawn after death+delay OK\n");
            }
        }
    }

    /* Test 5: max_spawns limit */
    {
        Nexus_SpawnerManager mgr;
        Nexus_SpawnEvent events[4];
        int idx, n, i;
        nexus_v1_spawners_init(&mgr);
        idx = nexus_v1_spawner_add(&mgr, 3, 3, 0, 1, 2, 1);
        mgr.spawners[idx].timer = 2;
        for (i = 0; i < 2; i++)
            nexus_v1_spawners_tick(&mgr, events, 4);
        if (mgr.spawners[idx].spawn_count != 1) {
            fprintf(stderr, "FAIL: first spawn count=%d\n", mgr.spawners[idx].spawn_count);
            fail++;
        } else {
            mgr.spawners[idx].timer = 2;
            for (i = 0; i < 3; i++)
                n = nexus_v1_spawners_tick(&mgr, events, 4);
            if (n != 0) {
                fprintf(stderr, "FAIL: should not spawn past max\n"); fail++;
            } else {
                printf("  Max spawns limit OK\n");
            }
        }
    }

    /* Test 6: unlimited spawns when max_spawns=0 */
    {
        Nexus_SpawnerManager mgr;
        Nexus_SpawnEvent events[4];
        int idx, spawns = 0, round;
        nexus_v1_spawners_init(&mgr);
        idx = nexus_v1_spawner_add(&mgr, 1, 1, 0, 0, 1, 0);
        for (round = 0; round < 5; round++) {
            mgr.spawners[idx].timer = 1;
            if (nexus_v1_spawners_tick(&mgr, events, 4) > 0)
                spawns++;
        }
        if (spawns != 5) {
            fprintf(stderr, "FAIL: unlimited spawns=%d\n", spawns); fail++;
        } else {
            printf("  Unlimited spawns OK\n");
        }
    }

    /* Test 7: NULL safety */
    {
        Nexus_SpawnEvent events[1];
        nexus_v1_spawners_init(NULL);
        nexus_v1_spawner_on_creature_death(NULL, 0);
        if (nexus_v1_spawner_add(NULL, 0, 0, 0, 0, 0, 0) != -1 ||
            nexus_v1_spawners_tick(NULL, events, 1) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus spawner system verified\n");
    return 0;
}
