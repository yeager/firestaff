#include "nexus_v1_rest.h"
#include "nexus_v1_status.h"

#include <stdio.h>

int main(void)
{
    Nexus_RestState rest;
    Nexus_StatusEffects status;

    /* Rest: start should set resting flag */
    nexus_v1_rest_init(&rest);
    nexus_v1_rest_start(&rest);
    if (!nexus_v1_rest_is_resting(&rest)) {
        fprintf(stderr, "FAIL: rest_start should set resting state\n");
        return 1;
    }
    nexus_v1_rest_interrupt(&rest);
    if (nexus_v1_rest_is_resting(&rest)) {
        fprintf(stderr, "FAIL: rest_interrupt should clear resting state\n");
        return 1;
    }
    if (!rest.interrupted) {
        fprintf(stderr, "FAIL: rest_interrupt should set interrupted flag\n");
        return 1;
    }

    /* Status: apply should activate effect */
    nexus_v1_status_init(&status);
    nexus_v1_status_apply(&status, NEXUS_STATUS_POISON, 60, 10);
    if (!nexus_v1_status_is_active(&status, NEXUS_STATUS_POISON)) {
        fprintf(stderr, "FAIL: status_apply should activate poison\n");
        return 1;
    }
    if (status.ticks[NEXUS_STATUS_POISON] != 60) {
        fprintf(stderr, "FAIL: poison ticks should be 60, got %d\n",
                status.ticks[NEXUS_STATUS_POISON]);
        return 1;
    }
    if (status.strength[NEXUS_STATUS_POISON] != 10) {
        fprintf(stderr, "FAIL: poison strength should be 10, got %d\n",
                status.strength[NEXUS_STATUS_POISON]);
        return 1;
    }

    /* Tick should decrement */
    nexus_v1_status_tick(&status);
    if (status.ticks[NEXUS_STATUS_POISON] != 59) {
        fprintf(stderr, "FAIL: poison ticks should be 59 after one tick, got %d\n",
                status.ticks[NEXUS_STATUS_POISON]);
        return 1;
    }

    /* Remove should clear */
    nexus_v1_status_remove(&status, NEXUS_STATUS_POISON);
    if (nexus_v1_status_is_active(&status, NEXUS_STATUS_POISON)) {
        fprintf(stderr, "FAIL: status_remove should deactivate poison\n");
        return 1;
    }
    if (status.ticks[NEXUS_STATUS_POISON] != 0) {
        fprintf(stderr, "FAIL: ticks should be 0 after remove, got %d\n",
                status.ticks[NEXUS_STATUS_POISON]);
        return 1;
    }

    puts("PASS: production Nexus rest/status gameplay verification");
    return 0;
}
