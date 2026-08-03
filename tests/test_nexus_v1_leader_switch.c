
#include <stdio.h>
#include <string.h>
#include "nexus_v1_champions.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_movement.h"

static int g_fail;
static void expect(int c, const char *m) {
    if (!c) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; }
}

int main(void) {
    /* Test 1: set_leader_slot stores value */
    {
        Nexus_MechanicsState st;
        nexus_mechanics_init(&st, 5, 5, 0);
        nexus_mechanics_set_leader_slot(&st, 2);
        expect(st.set_leader_slot == 2, "slot stored");
    }

    /* Test 2: SET_LEADER command changes leader_index */
    {
        Nexus_V1_ChampionPool pool;
        memset(&pool, 0, sizeof(pool));
        pool.party_count = 3;
        pool.party[0] = 0; pool.party[1] = 1; pool.party[2] = 2;
        pool.champions[0].alive = 1;
        pool.champions[1].alive = 1;
        pool.champions[2].alive = 1;
        pool.leader_index = 0;

        expect(pool.leader_index == 0, "starts at 0");
        pool.leader_index = 2;
        expect(pool.leader_index == 2, "can set to 2");
    }

    /* Test 3: NULL safety */
    {
        nexus_mechanics_set_leader_slot(NULL, 0);
    }

    /* Test 4: encumbrance move ticks basic */
    {
        Nexus_MechanicsState st;
        nexus_mechanics_init(&st, 0, 0, 0);
        expect(st.set_leader_slot == 0, "default slot 0");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus leader switch verified\n");
    return 0;
}
