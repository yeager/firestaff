#include "nexus_v1_rest.h"
#include "nexus_v1_status.h"

#include <stdio.h>

int main(void)
{
    Nexus_RestState rest;
    Nexus_StatusEffects status;

    nexus_v1_rest_init(&rest);
    nexus_v1_rest_start(&rest);
    if (nexus_v1_rest_is_resting(&rest)) {
        fprintf(stderr, "FAIL: production rest route mutated state\n");
        return 1;
    }
    nexus_v1_rest_interrupt(&rest);

    nexus_v1_status_init(&status);
    nexus_v1_status_apply(&status, NEXUS_STATUS_POISON, 60, 10);
    if (nexus_v1_status_is_active(&status, NEXUS_STATUS_POISON)) {
        fprintf(stderr, "FAIL: production status route mutated state\n");
        return 1;
    }
    nexus_v1_status_tick(&status);
    if (status.ticks[NEXUS_STATUS_POISON] != 0) {
        fprintf(stderr, "FAIL: production status tick mutated state\n");
        return 1;
    }
    nexus_v1_status_remove(&status, NEXUS_STATUS_POISON);
    if (nexus_v1_status_is_active(&status, NEXUS_STATUS_POISON)) {
        fprintf(stderr, "FAIL: production status remove mutated state\n");
        return 1;
    }

    puts("PASS: production Nexus rest/status route is fail-closed");
    return 0;
}
