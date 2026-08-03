
#include <stdio.h>
#include <string.h>
#include "nexus_v1_gameover.h"

static int g_fail;
static void expect(int c, const char *m) {
    if (!c) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; }
}

int main(void) {
    /* Test 1: init */
    {
        Nexus_GameOverState gs;
        nexus_v1_gameover_init(&gs);
        expect(gs.state == NEXUS_GAMEOVER_NONE, "init state is NONE");
        expect(!nexus_v1_gameover_is_active(&gs), "not active after init");
    }

    /* Test 2: all alive — no gameover */
    {
        Nexus_GameOverState gs;
        Nexus_V1_ChampionPool pool;
        nexus_v1_gameover_init(&gs);
        memset(&pool, 0, sizeof(pool));
        pool.party_count = 2;
        pool.party[0] = 0;
        pool.party[1] = 1;
        pool.champions[0].alive = 1;
        pool.champions[1].alive = 1;
        expect(nexus_v1_gameover_check(&gs, &pool) == NEXUS_GAMEOVER_NONE,
               "all alive = no gameover");
    }

    /* Test 3: all dead — defeat */
    {
        Nexus_GameOverState gs;
        Nexus_V1_ChampionPool pool;
        nexus_v1_gameover_init(&gs);
        memset(&pool, 0, sizeof(pool));
        pool.party_count = 2;
        pool.party[0] = 0;
        pool.party[1] = 1;
        pool.champions[0].alive = 0;
        pool.champions[1].alive = 0;
        expect(nexus_v1_gameover_check(&gs, &pool) == NEXUS_GAMEOVER_DEFEAT,
               "all dead = defeat");
        expect(nexus_v1_gameover_is_active(&gs), "active after defeat");
    }

    /* Test 4: one alive — no gameover */
    {
        Nexus_GameOverState gs;
        Nexus_V1_ChampionPool pool;
        nexus_v1_gameover_init(&gs);
        memset(&pool, 0, sizeof(pool));
        pool.party_count = 2;
        pool.party[0] = 0;
        pool.party[1] = 1;
        pool.champions[0].alive = 0;
        pool.champions[1].alive = 1;
        expect(nexus_v1_gameover_check(&gs, &pool) == NEXUS_GAMEOVER_NONE,
               "one alive = no gameover");
    }

    /* Test 5: victory */
    {
        Nexus_GameOverState gs;
        nexus_v1_gameover_init(&gs);
        nexus_v1_gameover_victory(&gs);
        expect(gs.state == NEXUS_GAMEOVER_VICTORY, "victory state");
        expect(nexus_v1_gameover_is_active(&gs), "active after victory");
    }

    /* Test 6: tick delay */
    {
        Nexus_GameOverState gs;
        int i, ready = 0;
        nexus_v1_gameover_init(&gs);
        gs.state = NEXUS_GAMEOVER_DEFEAT;
        gs.ticks_elapsed = 0;
        for (i = 0; i < 100; i++) {
            if (nexus_v1_gameover_tick(&gs)) { ready = 1; break; }
        }
        expect(ready, "tick delay elapsed");
        expect(gs.ticks_elapsed >= gs.delay_ticks, "elapsed >= delay");
    }

    /* Test 7: reset */
    {
        Nexus_GameOverState gs;
        nexus_v1_gameover_init(&gs);
        gs.state = NEXUS_GAMEOVER_DEFEAT;
        nexus_v1_gameover_reset(&gs);
        expect(gs.state == NEXUS_GAMEOVER_NONE, "reset clears state");
        expect(!nexus_v1_gameover_is_active(&gs), "not active after reset");
    }

    /* Test 8: NULL safety */
    {
        nexus_v1_gameover_init(NULL);
        expect(nexus_v1_gameover_check(NULL, NULL) == NEXUS_GAMEOVER_NONE,
               "NULL check safe");
        nexus_v1_gameover_victory(NULL);
        expect(nexus_v1_gameover_tick(NULL) == 0, "NULL tick safe");
        nexus_v1_gameover_reset(NULL);
        expect(!nexus_v1_gameover_is_active(NULL), "NULL is_active safe");
    }

    /* Test 9: empty party — no gameover */
    {
        Nexus_GameOverState gs;
        Nexus_V1_ChampionPool pool;
        nexus_v1_gameover_init(&gs);
        memset(&pool, 0, sizeof(pool));
        pool.party_count = 0;
        expect(nexus_v1_gameover_check(&gs, &pool) == NEXUS_GAMEOVER_NONE,
               "empty party = no gameover");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus gameover system verified\n");
    return 0;
}
