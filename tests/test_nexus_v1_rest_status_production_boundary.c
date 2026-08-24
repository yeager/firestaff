#include "nexus_v1_rest.h"
#include "nexus_v1_status.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    Nexus_RestState rest;
    Nexus_StatusEffects status;

    memset(&rest, 0, sizeof(rest));
    nexus_v1_rest_init(&rest);
    nexus_v1_rest_start(&rest);
    nexus_v1_rest_interrupt(&rest);
    if (nexus_v1_rest_is_resting(&rest) || rest.interrupted) {
        fprintf(stderr, "FAIL: production rest route mutated state\n");
        return 1;
    }

    memset(&status, 0, sizeof(status));
    nexus_v1_status_init(&status);
    nexus_v1_status_apply(&status, NEXUS_STATUS_POISON, 60, 10);
    nexus_v1_status_tick(&status);
    nexus_v1_status_remove(&status, NEXUS_STATUS_POISON);
    if (nexus_v1_status_is_active(&status, NEXUS_STATUS_POISON) ||
        status.ticks[NEXUS_STATUS_POISON] != 0 ||
        status.strength[NEXUS_STATUS_POISON] != 0) {
        fprintf(stderr, "FAIL: production status route mutated state\n");
        return 1;
    }

    puts("PASS: production Nexus rest/status boundary remains fail-closed");
    return 0;
}
