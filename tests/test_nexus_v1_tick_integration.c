
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nexus_v1_engine.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_status.h"
#include "nexus_v1_hunger.h"
#include "nexus_v1_messages.h"
#include "nexus_v1_damage_indicator.h"

static int g_fail;

static void expect(int cond, const char *msg) {
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); g_fail++; }
}

static Nexus_V1_Engine *create_minimal_engine(void) {
    int i;
    Nexus_V1_Engine *e = calloc(1, sizeof(*e));
    if (!e) return NULL;
    e->initialized = 1;
    e->game.game_started = 1;
    nexus_v1_champions_init(&e->champions);
    nexus_v1_champion_recruit(&e->champions, 0);
    e->champions.champions[0].health = 100;
    e->champions.champions[0].max_health = 100;
    e->champions.champions[0].alive = 1;
    e->champions.champions[0].food = 100;
    e->champions.champions[0].water = 100;
    nexus_v1_hunger_init(&e->hunger);
    nexus_v1_messages_init(&e->messages);
    nexus_v1_damage_display_init(&e->damage_display);
    nexus_v1_action_timers_init(&e->action_timers);
    nexus_v1_spawners_init(&e->spawners);
    nexus_v1_formation_init(&e->formation, 1);
    for (i = 0; i < 4; i++)
        nexus_v1_status_init(&e->champion_status[i]);
    return e;
}

int main(void) {
    /* Test 1: tick increments counter */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        nexus_v1_tick(e);
        expect(e->game.tick_count == 1, "tick counter incremented");
        free(e);
    }

    /* Test 2: poison ticks reduce health */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int old_hp;
        nexus_v1_status_apply(&e->champion_status[0],
                              NEXUS_STATUS_POISON, 10, 8);
        old_hp = e->champions.champions[0].health;
        nexus_v1_tick(e);
        expect(e->champions.champions[0].health < old_hp,
               "poison tick reduced health");
        free(e);
    }

    /* Test 3: message queue ticks */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        nexus_v1_message_push_ex(&e->messages, "Test message", 2, 0);
        nexus_v1_tick(e);
        nexus_v1_tick(e);
        expect(e->messages.count == 0,
               "message expired after ticks");
        free(e);
    }

    /* Test 4: multiple ticks don't crash */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        int i;
        for (i = 0; i < 100; i++)
            nexus_v1_tick(e);
        expect(e->game.tick_count == 100, "100 ticks completed");
        free(e);
    }

    /* Test 5: dead champion not poisoned further */
    {
        Nexus_V1_Engine *e = create_minimal_engine();
        e->champions.champions[0].health = 1;
        e->champions.champions[0].alive = 1;
        nexus_v1_status_apply(&e->champion_status[0],
                              NEXUS_STATUS_POISON, 50, 20);
        nexus_v1_tick(e);
        expect(e->champions.champions[0].health == 0 &&
               e->champions.champions[0].alive == 0,
               "champion dies from poison");
        nexus_v1_tick(e);
        expect(e->champions.champions[0].health == 0,
               "dead champion not poisoned further");
        free(e);
    }

    /* Test 6: NULL engine doesn't crash */
    {
        nexus_v1_tick(NULL);
        expect(1, "NULL engine safe");
    }

    /* Test 7: uninitialized engine doesn't tick */
    {
        Nexus_V1_Engine *e = calloc(1, sizeof(*e));
        e->initialized = 0;
        nexus_v1_tick(e);
        expect(e->game.tick_count == 0, "uninitialized engine no tick");
        free(e);
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus tick integration verified\n");
    return 0;
}
