
#include <stdio.h>
#include <string.h>
#include "nexus_v1_hunger.h"

static void make_party(Nexus_V1_ChampionPool *pool) {
    memset(pool, 0, sizeof(*pool));
    pool->champion_count = 1;
    pool->party_count = 1;
    pool->party[0] = 0;
    pool->champions[0].alive = 1;
    pool->champions[0].health = 100;
    pool->champions[0].max_health = 100;
    pool->champions[0].food = 50;
    pool->champions[0].water = 50;
}

int main(void) {
    int fail = 0;

    /* Test 1: init zeroes all fields */
    {
        Nexus_HungerState hs;
        nexus_v1_hunger_init(&hs);
        if (hs.food_accumulator != 0 || hs.water_accumulator != 0 ||
            hs.tick_counter != 0 || hs.encumbrance != 0) {
            fprintf(stderr, "FAIL: init\n"); fail++;
        } else {
            printf("  Init OK\n");
        }
    }

    /* Test 2: food drains after ACCUM_CYCLE ticks (accumulator model) */
    {
        Nexus_HungerState hs;
        Nexus_V1_ChampionPool pool;
        int i;
        nexus_v1_hunger_init(&hs);
        make_party(&pool);
        for (i = 0; i < NEXUS_HUNGER_ACCUM_CYCLE; i++)
            nexus_v1_hunger_tick(&hs, &pool);
        if (pool.champions[0].food >= 50) {
            fprintf(stderr, "FAIL: food not drained after %d ticks, food=%d\n",
                    NEXUS_HUNGER_ACCUM_CYCLE, pool.champions[0].food);
            fail++;
        } else {
            printf("  Food drain: 50->%d after %d ticks OK\n",
                   pool.champions[0].food, NEXUS_HUNGER_ACCUM_CYCLE);
        }
    }

    /* Test 3: water drains using threshold model */
    {
        Nexus_HungerState hs;
        Nexus_V1_ChampionPool pool;
        int i;
        nexus_v1_hunger_init(&hs);
        make_party(&pool);
        for (i = 0; i < NEXUS_HUNGER_ACCUM_CYCLE; i++)
            nexus_v1_hunger_tick(&hs, &pool);
        if (pool.champions[0].water != 50 - NEXUS_WATER_THRESHOLD) {
            fprintf(stderr, "FAIL: water=%d expected %d\n",
                    pool.champions[0].water, 50 - NEXUS_WATER_THRESHOLD);
            fail++;
        } else {
            printf("  Water drain: 50->%d OK\n", pool.champions[0].water);
        }
    }

    /* Test 4: starvation damages health */
    {
        Nexus_HungerState hs;
        Nexus_V1_ChampionPool pool;
        int i, mask;
        nexus_v1_hunger_init(&hs);
        make_party(&pool);
        pool.champions[0].food = 0;
        for (i = 0; i < NEXUS_HUNGER_ACCUM_CYCLE; i++)
            mask = nexus_v1_hunger_tick(&hs, &pool);
        if (pool.champions[0].health != 100 - NEXUS_STARVATION_DAMAGE) {
            fprintf(stderr, "FAIL: starvation hp=%d\n", pool.champions[0].health);
            fail++;
        } else if (!(mask & 1)) {
            fprintf(stderr, "FAIL: starvation mask\n"); fail++;
        } else {
            printf("  Starvation damage: hp->%d OK\n", pool.champions[0].health);
        }
    }

    /* Test 5: dehydration damages health */
    {
        Nexus_HungerState hs;
        Nexus_V1_ChampionPool pool;
        int i, mask;
        nexus_v1_hunger_init(&hs);
        make_party(&pool);
        pool.champions[0].water = 0;
        for (i = 0; i < NEXUS_HUNGER_ACCUM_CYCLE; i++)
            mask = nexus_v1_hunger_tick(&hs, &pool);
        if (pool.champions[0].health != 100 - NEXUS_DEHYDRATION_DAMAGE) {
            fprintf(stderr, "FAIL: dehydration hp=%d\n", pool.champions[0].health);
            fail++;
        } else if (!(mask & (1 << 4))) {
            fprintf(stderr, "FAIL: dehydration mask=%d\n", mask); fail++;
        } else {
            printf("  Dehydration damage: hp->%d OK\n", pool.champions[0].health);
        }
    }

    /* Test 6: dead champions skipped */
    {
        Nexus_HungerState hs;
        Nexus_V1_ChampionPool pool;
        int i;
        nexus_v1_hunger_init(&hs);
        make_party(&pool);
        pool.champions[0].alive = 0;
        pool.champions[0].food = 50;
        for (i = 0; i < NEXUS_HUNGER_ACCUM_CYCLE; i++)
            nexus_v1_hunger_tick(&hs, &pool);
        if (pool.champions[0].food != 50) {
            fprintf(stderr, "FAIL: dead champion food changed\n"); fail++;
        } else {
            printf("  Dead champion skipped OK\n");
        }
    }

    /* Test 7: NULL safety */
    {
        nexus_v1_hunger_init(NULL);
        if (nexus_v1_hunger_tick(NULL, NULL) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    /* Test 8: food drain formula varies with encumbrance */
    {
        Nexus_HungerState hs;
        Nexus_V1_ChampionPool pool;
        int i, food_normal, food_encumbered;
        nexus_v1_hunger_init(&hs);
        make_party(&pool);
        pool.champions[0].food = 200;
        for (i = 0; i < NEXUS_HUNGER_ACCUM_CYCLE; i++)
            nexus_v1_hunger_tick(&hs, &pool);
        food_normal = pool.champions[0].food;

        nexus_v1_hunger_init(&hs);
        make_party(&pool);
        pool.champions[0].food = 200;
        hs.encumbrance = 8;
        for (i = 0; i < NEXUS_HUNGER_ACCUM_CYCLE; i++)
            nexus_v1_hunger_tick(&hs, &pool);
        food_encumbered = pool.champions[0].food;

        if (food_encumbered >= food_normal) {
            fprintf(stderr, "FAIL: encumbrance should drain more food (%d vs %d)\n",
                    food_encumbered, food_normal);
            fail++;
        } else {
            printf("  Encumbrance drain: normal=%d encumbered=%d OK\n",
                   food_normal, food_encumbered);
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus hunger/thirst accumulator system verified\n");
    return 0;
}
