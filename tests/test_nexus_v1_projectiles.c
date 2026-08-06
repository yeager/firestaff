
#include <stdio.h>
#include <string.h>
#include "nexus_v1_projectiles.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_squares.h"

static uint8_t g_map[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];

static void setup_map(void) {
    memset(g_map, 0, sizeof(g_map));
    for (int y = 0; y < 10; y++)
        for (int x = 0; x < 10; x++)
            g_map[y][x] = 0x01;
    g_map[5][5] = 0x00;
}

int main(void) {
    int fail = 0;
    setup_map();

    /* Test 1: spawn and count */
    {
        Nexus_ProjectileManager mgr;
        nexus_v1_projectiles_init(&mgr);
        int idx = nexus_v1_projectile_spawn(&mgr, NEXUS_PROJ_FIREBALL,
                                             3, 3, 0, 20, 1, 0);
        if (idx < 0 || nexus_v1_projectile_count(&mgr) != 1) {
            fprintf(stderr, "FAIL: spawn/count\n"); fail++;
        } else {
            printf("  Spawn fireball at (3,3) dir=N: slot=%d OK\n", idx);
        }
    }

    /* Test 2: projectile moves forward each tick */
    {
        Nexus_ProjectileManager mgr;
        Nexus_ProjectileHit hits[4];
        nexus_v1_projectiles_init(&mgr);
        nexus_v1_projectile_spawn(&mgr, NEXUS_PROJ_LIGHTNING,
                                   5, 8, 0, 15, 1, 0);
        int n = nexus_v1_projectiles_tick(&mgr, g_map, hits, 4);
        if (n != 1 || hits[0].hit_x != 5 || hits[0].hit_y != 7) {
            fprintf(stderr, "FAIL: move forward hit_y=%d (exp 7)\n",
                    n > 0 ? hits[0].hit_y : -1);
            fail++;
        } else {
            printf("  Lightning moves N from (5,8) to (5,7) OK\n");
        }
    }

    /* Test 3: projectile hits wall */
    {
        Nexus_ProjectileManager mgr;
        Nexus_ProjectileHit hits[4];
        nexus_v1_projectiles_init(&mgr);
        nexus_v1_projectile_spawn(&mgr, NEXUS_PROJ_FIREBALL,
                                   5, 6, 0, 10, 1, 0);
        nexus_v1_projectiles_tick(&mgr, g_map, hits, 4);
        int n = nexus_v1_projectiles_tick(&mgr, g_map, hits, 4);
        if (nexus_v1_projectile_count(&mgr) != 0) {
            fprintf(stderr, "FAIL: projectile should be removed after wall hit\n");
            fail++;
        } else {
            printf("  Fireball hits wall at (5,5), removed OK\n");
        }
    }

    /* Test 4: speed_ticks delays movement */
    {
        Nexus_ProjectileManager mgr;
        Nexus_ProjectileHit hits[4];
        nexus_v1_projectiles_init(&mgr);
        nexus_v1_projectile_spawn(&mgr, NEXUS_PROJ_POISON_CLOUD,
                                   3, 3, 1, 10, 3, 0);
        int n1 = nexus_v1_projectiles_tick(&mgr, g_map, hits, 4);
        int n2 = nexus_v1_projectiles_tick(&mgr, g_map, hits, 4);
        int n3 = nexus_v1_projectiles_tick(&mgr, g_map, hits, 4);
        if (n1 != 0 || n2 != 0 || n3 != 1) {
            fprintf(stderr, "FAIL: speed_ticks=3 delay: n1=%d n2=%d n3=%d\n",
                    n1, n2, n3);
            fail++;
        } else {
            printf("  Poison cloud speed_ticks=3: moves on tick 3 OK\n");
        }
    }

    /* Test 5: an unbound door is a projectile collision, not a passability
       fallback through the generic non-wall classifier. */
    {
        Nexus_ProjectileManager mgr;
        Nexus_ProjectileHit hits[4];
        nexus_v1_projectiles_init(&mgr);
        g_map[5][5] = NEXUS_SQUARE_DOOR;
        nexus_v1_projectile_spawn(&mgr, NEXUS_PROJ_FIREBALL,
                                   5, 6, 0, 10, 1, 0);
        nexus_v1_projectiles_tick(&mgr, g_map, hits, 4);
        if (nexus_v1_projectile_count(&mgr) != 0 || !hits[0].hit_wall) {
            fprintf(stderr, "FAIL: projectile passed through unbound door\n");
            fail++;
        } else {
            printf("  Fireball stops at unbound door, no placeholder passage OK\n");
        }
        g_map[5][5] = 0x00;
    }

    /* Test 6: max projectiles */
    {
        Nexus_ProjectileManager mgr;
        nexus_v1_projectiles_init(&mgr);
        for (int i = 0; i < NEXUS_MAX_PROJECTILES; i++)
            nexus_v1_projectile_spawn(&mgr, NEXUS_PROJ_ARROW,
                                       1, 1, 0, 5, 1, 0);
        int overflow = nexus_v1_projectile_spawn(&mgr, NEXUS_PROJ_ARROW,
                                                  1, 1, 0, 5, 1, 0);
        if (overflow != -1) {
            fprintf(stderr, "FAIL: overflow not rejected\n"); fail++;
        } else {
            printf("  Max projectiles overflow rejected OK\n");
        }
    }

    /* Test 7: NULL safety */
    {
        nexus_v1_projectiles_init(NULL);
        nexus_v1_projectile_spawn(NULL, NEXUS_PROJ_FIREBALL, 0, 0, 0, 0, 0, 0);
        nexus_v1_projectiles_tick(NULL, g_map, NULL, 0);
        if (nexus_v1_projectile_count(NULL) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus projectile system verified\n");
    return 0;
}
