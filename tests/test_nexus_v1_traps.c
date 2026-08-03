
#include <stdio.h>
#include <string.h>
#include "nexus_v1_traps.h"

int main(void) {
    int fail = 0;

    /* Test 1: init */
    {
        Nexus_V1_TrapManager mgr;
        nexus_v1_trap_manager_init(&mgr);
        if (mgr.count != 0) {
            fprintf(stderr, "FAIL: init\n"); fail++;
        } else {
            printf("  Init OK\n");
        }
    }

    /* Test 2: pit trap damages and sets fall position */
    {
        Nexus_V1_TrapManager mgr;
        Nexus_V1_Trap t;
        Nexus_V1_Champion ch;
        Nexus_V1_TrapResult r;
        nexus_v1_trap_manager_init(&mgr);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRAP_PIT;
        t.level = 0; t.x = 5; t.y = 5;
        t.armed = 1;
        t.target_level = 1; t.target_x = 5; t.target_y = 5;
        nexus_v1_trap_add(&mgr, &t);
        memset(&ch, 0, sizeof(ch));
        ch.health = 50;
        r = nexus_v1_trap_trigger(&mgr, &ch, 0, 5, 5);
        if (!r.triggered || r.damage_dealt != NEXUS_PIT_DAMAGE ||
            ch.health != 50 - NEXUS_PIT_DAMAGE ||
            r.fall_level != 1) {
            fprintf(stderr, "FAIL: pit trap dmg=%d hp=%d\n",
                    r.damage_dealt, ch.health); fail++;
        } else {
            printf("  Pit trap OK\n");
        }
    }

    /* Test 3: disarmed trap doesn't trigger */
    {
        Nexus_V1_TrapManager mgr;
        Nexus_V1_Trap t;
        Nexus_V1_TrapResult r;
        nexus_v1_trap_manager_init(&mgr);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRAP_PIT;
        t.level = 0; t.x = 3; t.y = 3;
        t.armed = 0;
        nexus_v1_trap_add(&mgr, &t);
        r = nexus_v1_trap_trigger(&mgr, NULL, 0, 3, 3);
        if (r.triggered) {
            fprintf(stderr, "FAIL: disarmed triggered\n"); fail++;
        } else {
            printf("  Disarmed OK\n");
        }
    }

    /* Test 4: poison dart sets poison flag */
    {
        Nexus_V1_TrapManager mgr;
        Nexus_V1_Trap t;
        Nexus_V1_Champion ch;
        Nexus_V1_TrapResult r;
        nexus_v1_trap_manager_init(&mgr);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRAP_POISON_DART;
        t.level = 0; t.x = 7; t.y = 7;
        t.armed = 1;
        nexus_v1_trap_add(&mgr, &t);
        memset(&ch, 0, sizeof(ch));
        ch.health = 40;
        r = nexus_v1_trap_trigger(&mgr, &ch, 0, 7, 7);
        if (!r.triggered || !r.poisoned ||
            r.damage_dealt != NEXUS_POISON_DART_DAMAGE) {
            fprintf(stderr, "FAIL: poison dart\n"); fail++;
        } else {
            printf("  Poison dart OK\n");
        }
    }

    /* Test 5: pressure plate triggers without damage */
    {
        Nexus_V1_TrapManager mgr;
        Nexus_V1_Trap t;
        Nexus_V1_TrapResult r;
        nexus_v1_trap_manager_init(&mgr);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRAP_PRESSURE_PLATE;
        t.level = 0; t.x = 2; t.y = 2;
        t.armed = 1;
        t.rearm_ticks = 5;
        nexus_v1_trap_add(&mgr, &t);
        r = nexus_v1_trap_trigger(&mgr, NULL, 0, 2, 2);
        if (!r.triggered || r.damage_dealt != 0) {
            fprintf(stderr, "FAIL: pressure plate\n"); fail++;
        } else {
            printf("  Pressure plate OK\n");
        }
    }

    /* Test 6: cooldown prevents re-trigger */
    {
        Nexus_V1_TrapManager mgr;
        Nexus_V1_Trap t;
        Nexus_V1_TrapResult r;
        nexus_v1_trap_manager_init(&mgr);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRAP_POISON_DART;
        t.level = 0; t.x = 1; t.y = 1;
        t.armed = 1;
        t.rearm_ticks = 3;
        nexus_v1_trap_add(&mgr, &t);
        nexus_v1_trap_trigger(&mgr, NULL, 0, 1, 1);
        r = nexus_v1_trap_trigger(&mgr, NULL, 0, 1, 1);
        if (r.triggered) {
            fprintf(stderr, "FAIL: cooldown bypass\n"); fail++;
        } else {
            printf("  Cooldown OK\n");
        }
    }

    /* Test 7: tick decrements cooldown */
    {
        Nexus_V1_TrapManager mgr;
        Nexus_V1_Trap t;
        Nexus_V1_TrapResult r;
        nexus_v1_trap_manager_init(&mgr);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRAP_POISON_DART;
        t.level = 0; t.x = 4; t.y = 4;
        t.armed = 1;
        t.rearm_ticks = 2;
        nexus_v1_trap_add(&mgr, &t);
        nexus_v1_trap_trigger(&mgr, NULL, 0, 4, 4);
        nexus_v1_trap_tick(&mgr);
        nexus_v1_trap_tick(&mgr);
        r = nexus_v1_trap_trigger(&mgr, NULL, 0, 4, 4);
        if (!r.triggered) {
            fprintf(stderr, "FAIL: re-arm after cooldown\n"); fail++;
        } else {
            printf("  Re-arm after cooldown OK\n");
        }
    }

    /* Test 8: find trap */
    {
        Nexus_V1_TrapManager mgr;
        Nexus_V1_Trap t;
        nexus_v1_trap_manager_init(&mgr);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRAP_PIT;
        t.level = 2; t.x = 10; t.y = 15;
        t.armed = 1;
        nexus_v1_trap_add(&mgr, &t);
        if (!nexus_v1_trap_find(&mgr, 2, 10, 15) ||
            nexus_v1_trap_find(&mgr, 0, 0, 0) != NULL) {
            fprintf(stderr, "FAIL: find\n"); fail++;
        } else {
            printf("  Find OK\n");
        }
    }

    /* Test 9: NULL safety */
    {
        nexus_v1_trap_manager_init(NULL);
        nexus_v1_trap_tick(NULL);
        if (nexus_v1_trap_find(NULL, 0, 0, 0) != NULL ||
            nexus_v1_trap_add(NULL, NULL)) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            Nexus_V1_TrapResult r = nexus_v1_trap_trigger(NULL, NULL, 0, 0, 0);
            if (r.triggered) {
                fprintf(stderr, "FAIL: NULL trigger\n"); fail++;
            } else {
                printf("  NULL safety OK\n");
            }
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus trap system verified\n");
    return 0;
}
