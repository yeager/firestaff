
#include <stdio.h>
#include <string.h>
#include "nexus_v1_rest.h"
#include "nexus_v1_champions.h"

static void make_party(Nexus_V1_ChampionPool *pool) {
    memset(pool, 0, sizeof(*pool));
    pool->champion_count = 1;
    pool->party_count = 1;
    pool->party[0] = 0;
    pool->champions[0].alive = 1;
    pool->champions[0].health = 50;
    pool->champions[0].max_health = 100;
    pool->champions[0].stamina = 30;
    pool->champions[0].max_stamina = 100;
    pool->champions[0].mana = 20;
    pool->champions[0].max_mana = 80;
}

int main(void) {
    int fail = 0;

    /* Test 1: init not resting */
    {
        Nexus_RestState rs;
        nexus_v1_rest_init(&rs);
        if (nexus_v1_rest_is_resting(&rs)) {
            fprintf(stderr, "FAIL: init resting\n"); fail++;
        } else {
            printf("  Init: not resting OK\n");
        }
    }

    /* Test 2: start/stop rest */
    {
        Nexus_RestState rs;
        nexus_v1_rest_init(&rs);
        nexus_v1_rest_start(&rs);
        if (!nexus_v1_rest_is_resting(&rs)) {
            fprintf(stderr, "FAIL: start rest\n"); fail++;
        } else {
            nexus_v1_rest_stop(&rs);
            if (nexus_v1_rest_is_resting(&rs)) {
                fprintf(stderr, "FAIL: stop rest\n"); fail++;
            } else {
                printf("  Start/stop rest OK\n");
            }
        }
    }

    /* Test 3: resting regenerates stamina */
    {
        Nexus_RestState rs;
        Nexus_V1_ChampionPool pool;
        int i;
        nexus_v1_rest_init(&rs);
        make_party(&pool);
        nexus_v1_rest_start(&rs);
        for (i = 0; i < NEXUS_REST_REGEN_TICKS; i++)
            nexus_v1_rest_tick(&rs, &pool);
        if (pool.champions[0].stamina <= 30) {
            fprintf(stderr, "FAIL: stamina not restored: %d\n",
                    pool.champions[0].stamina);
            fail++;
        } else {
            printf("  Rest regen: stamina 30->%d OK\n",
                   pool.champions[0].stamina);
        }
    }

    /* Test 4: resting regenerates mana */
    {
        Nexus_RestState rs;
        Nexus_V1_ChampionPool pool;
        int i;
        nexus_v1_rest_init(&rs);
        make_party(&pool);
        nexus_v1_rest_start(&rs);
        for (i = 0; i < NEXUS_REST_REGEN_TICKS; i++)
            nexus_v1_rest_tick(&rs, &pool);
        if (pool.champions[0].mana <= 20) {
            fprintf(stderr, "FAIL: mana not restored\n"); fail++;
        } else {
            printf("  Rest regen: mana 20->%d OK\n",
                   pool.champions[0].mana);
        }
    }

    /* Test 5: interrupt stops rest */
    {
        Nexus_RestState rs;
        nexus_v1_rest_init(&rs);
        nexus_v1_rest_start(&rs);
        nexus_v1_rest_interrupt(&rs);
        if (nexus_v1_rest_is_resting(&rs) || !rs.interrupted) {
            fprintf(stderr, "FAIL: interrupt\n"); fail++;
        } else {
            printf("  Interrupt: rest stopped, interrupted=1 OK\n");
        }
    }

    /* Test 6: auto-stop when all full */
    {
        Nexus_RestState rs;
        Nexus_V1_ChampionPool pool;
        nexus_v1_rest_init(&rs);
        make_party(&pool);
        pool.champions[0].health = 100;
        pool.champions[0].stamina = 100;
        pool.champions[0].mana = 80;
        nexus_v1_rest_start(&rs);
        nexus_v1_rest_tick(&rs, &pool);
        if (nexus_v1_rest_is_resting(&rs)) {
            fprintf(stderr, "FAIL: should auto-stop when full\n"); fail++;
        } else {
            printf("  Auto-stop when all full OK\n");
        }
    }

    /* Test 7: NULL safety */
    {
        nexus_v1_rest_init(NULL);
        nexus_v1_rest_start(NULL);
        nexus_v1_rest_tick(NULL, NULL);
        if (nexus_v1_rest_is_resting(NULL) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus rest system verified\n");
    return 0;
}
