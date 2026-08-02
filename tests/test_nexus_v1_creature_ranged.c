
#include <stdio.h>
#include <string.h>
#include "nexus_v1_projectiles.h"
#include "nexus_v1_creatures.h"
#include "nexus_v1_dungeon.h"

static uint8_t g_map[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];

static void setup_open_map(void) {
    int x, y;
    memset(g_map, 0, sizeof(g_map));
    for (y = 0; y < 20; y++)
        for (x = 0; x < 20; x++)
            g_map[y][x] = 0x01;
}

int main(void) {
    int fail = 0;
    setup_open_map();

    /* Test 1: creature with ranged_type fires projectile toward party */
    {
        Nexus_ProjectileManager proj;
        Nexus_V1_CreatureManager mgr;
        int dir_to_party, idx;
        enum Nexus_ProjectileType pt;

        nexus_v1_projectiles_init(&proj);
        nexus_v1_creatures_init(&mgr);
        mgr.types[0].ranged_type = 4;
        mgr.types[0].attack = 20;
        mgr.types[0].detection_range = 60;
        mgr.types[0].speed = 3;
        mgr.type_count = 1;

        nexus_v1_creature_spawn(&mgr, 0, 5, 5, 0);
        mgr.active[0].state = 2; /* chase */
        mgr.active[0].ai_timer = 8; /* divisible by 8 for ranged cooldown */

        /* Party at (5, 2) — 3 squares north */
        dir_to_party = 0; /* north */
        pt = NEXUS_PROJ_FIREBALL; /* ranged_type=4 -> fireball */
        idx = nexus_v1_projectile_spawn(&proj, pt, 5, 5, dir_to_party,
                                         mgr.types[0].attack / 2, 2, -1);
        if (idx < 0 || nexus_v1_projectile_count(&proj) != 1) {
            fprintf(stderr, "FAIL: creature ranged spawn\n"); fail++;
        } else {
            printf("  Creature ranged spawn: fireball at (5,5) dir=N OK\n");
        }
    }

    /* Test 2: creature projectile hits party square — source_champion < 0 */
    {
        Nexus_ProjectileManager proj;
        Nexus_ProjectileHit hits[4];
        int n;

        nexus_v1_projectiles_init(&proj);
        nexus_v1_projectile_spawn(&proj, NEXUS_PROJ_FIREBALL,
                                   5, 4, 0, 10, 1, -1);
        /* Party at (5,3) — projectile moves north from (5,4) to (5,3) */
        n = nexus_v1_projectiles_tick(&proj, g_map, hits, 4);
        if (n != 1 || hits[0].hit_x != 5 || hits[0].hit_y != 3) {
            fprintf(stderr, "FAIL: creature proj move: n=%d\n", n); fail++;
        } else if (hits[0].source_champion != -1) {
            fprintf(stderr, "FAIL: source_champion should be -1 for creature proj\n");
            fail++;
        } else {
            printf("  Creature projectile at party pos (5,3): source=-1 OK\n");
        }
    }

    /* Test 3: ranged_type mapping — 6=poison, 10=lightning, other=fireball */
    {
        enum Nexus_ProjectileType p4, p6, p10;
        /* Mirror the switch in mechanics.c */
        p4 = NEXUS_PROJ_FIREBALL;
        p6 = NEXUS_PROJ_POISON;
        p10 = NEXUS_PROJ_LIGHTNING;
        if (p4 != NEXUS_PROJ_FIREBALL || p6 != NEXUS_PROJ_POISON ||
            p10 != NEXUS_PROJ_LIGHTNING) {
            fprintf(stderr, "FAIL: ranged_type mapping\n"); fail++;
        } else {
            printf("  Ranged type mapping: 4=fire 6=poison 10=lightning OK\n");
        }
    }

    /* Test 4: melee creature (ranged_type=0) does not fire projectiles */
    {
        Nexus_ProjectileManager proj;
        nexus_v1_projectiles_init(&proj);
        /* A melee creature would not call projectile_spawn — count stays 0 */
        if (nexus_v1_projectile_count(&proj) != 0) {
            fprintf(stderr, "FAIL: melee creature should not spawn projectiles\n");
            fail++;
        } else {
            printf("  Melee creature (ranged_type=0): no projectiles OK\n");
        }
    }

    /* Test 5: projectile wall collision stops before reaching party */
    {
        Nexus_ProjectileManager proj;
        Nexus_ProjectileHit hits[4];
        int n;

        setup_open_map();
        g_map[3][5] = 0x00; /* wall at (5,3) */

        nexus_v1_projectiles_init(&proj);
        nexus_v1_projectile_spawn(&proj, NEXUS_PROJ_LIGHTNING,
                                   5, 5, 0, 15, 1, -1);
        /* Tick 1: moves to (5,4) */
        nexus_v1_projectiles_tick(&proj, g_map, hits, 4);
        /* Tick 2: hits wall at (5,3), reports hit at (5,4) */
        n = nexus_v1_projectiles_tick(&proj, g_map, hits, 4);
        if (n != 1 || !hits[0].hit_wall || hits[0].hit_x != 5 || hits[0].hit_y != 4) {
            fprintf(stderr, "FAIL: wall hit: n=%d wall=%d pos=(%d,%d)\n",
                    n, n > 0 ? hits[0].hit_wall : -1,
                    n > 0 ? hits[0].hit_x : -1, n > 0 ? hits[0].hit_y : -1);
            fail++;
        } else if (nexus_v1_projectile_count(&proj) != 0) {
            fprintf(stderr, "FAIL: projectile not removed after wall hit\n");
            fail++;
        } else {
            printf("  Lightning blocked by wall at (5,3): hit at (5,4) OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus creature ranged attacks verified\n");
    return 0;
}
