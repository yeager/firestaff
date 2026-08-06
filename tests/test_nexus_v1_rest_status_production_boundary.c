#include "nexus_v1_rest.h"
#include "nexus_v1_status.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_RestState rest;
    Nexus_RestState rest_before;
    Nexus_StatusEffects status;
    Nexus_StatusEffects status_before;
    Nexus_V1_ChampionPool pool;
    Nexus_V1_ChampionPool pool_before;

    memset(&rest, 0x6a, sizeof(rest));
    memset(&status, 0x3c, sizeof(status));
    memset(&pool, 0x51, sizeof(pool));
    rest_before = rest;
    status_before = status;
    pool_before = pool;

    nexus_v1_rest_init(&rest);
    nexus_v1_rest_start(&rest);
    nexus_v1_rest_tick(&rest, &pool);
    nexus_v1_rest_interrupt(&rest);
    nexus_v1_rest_stop(&rest);
    nexus_v1_status_init(&status);
    nexus_v1_status_apply(&status, NEXUS_STATUS_POISON, 60, 10);
    nexus_v1_status_tick(&status);
    nexus_v1_status_remove(&status, NEXUS_STATUS_POISON);

    if (memcmp(&rest, &rest_before, sizeof(rest)) != 0 ||
        memcmp(&status, &status_before, sizeof(status)) != 0 ||
        memcmp(&pool, &pool_before, sizeof(pool)) != 0 ||
        nexus_v1_rest_is_resting(&rest) != 0 ||
        nexus_v1_rest_all_full(&pool) != 0 ||
        nexus_v1_status_is_active(&status, NEXUS_STATUS_POISON) != 0 ||
        nexus_v1_status_poison_damage(&status) != 0) {
        fprintf(stderr, "FAIL: production Nexus rest/status route mutated or exposed inferred state\n");
        return 1;
    }

    puts("PASS: production Nexus rest/status route is fail-closed");
    return 0;
}
