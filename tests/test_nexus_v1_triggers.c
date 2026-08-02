
#include <stdio.h>
#include "nexus_v1_triggers.h"

int main(void) {
    int fail = 0;

    /* Test 1: init */
    {
        Nexus_TriggerManager mgr;
        nexus_v1_triggers_init(&mgr);
        if (nexus_v1_triggers_count(&mgr) != 0) {
            fprintf(stderr, "FAIL: init count\n"); fail++;
        } else {
            printf("  Init: count=0 OK\n");
        }
    }

    /* Test 2: register floor plate */
    {
        Nexus_TriggerManager mgr;
        nexus_v1_triggers_init(&mgr);
        int idx = nexus_v1_trigger_register(&mgr, 5, 5, -1, 5, 8,
            NEXUS_TRIGGER_FLOOR_PLATE, NEXUS_TRIGGER_ACT_TOGGLE_DOOR, 0);
        if (idx < 0 || nexus_v1_triggers_count(&mgr) != 1) {
            fprintf(stderr, "FAIL: register\n"); fail++;
        } else {
            printf("  Register floor plate at (5,5)->door(5,8) OK\n");
        }
    }

    /* Test 3: step on floor plate activates */
    {
        Nexus_TriggerManager mgr;
        nexus_v1_triggers_init(&mgr);
        nexus_v1_trigger_register(&mgr, 3, 3, -1, 3, 7,
            NEXUS_TRIGGER_FLOOR_PLATE, NEXUS_TRIGGER_ACT_TOGGLE_DOOR, 0);
        int fired = nexus_v1_triggers_on_step(&mgr, 3, 3);
        if (fired != 1 || !mgr.triggers[0].activated) {
            fprintf(stderr, "FAIL: step on\n"); fail++;
        } else {
            printf("  Step on (3,3): 1 trigger fired OK\n");
        }
    }

    /* Test 4: step off deactivates plate */
    {
        Nexus_TriggerManager mgr;
        nexus_v1_triggers_init(&mgr);
        nexus_v1_trigger_register(&mgr, 4, 4, -1, 4, 8,
            NEXUS_TRIGGER_FLOOR_PLATE, NEXUS_TRIGGER_ACT_OPEN_DOOR, 0);
        nexus_v1_triggers_on_step(&mgr, 4, 4);
        int deact = nexus_v1_triggers_off_step(&mgr, 4, 4);
        if (deact != 1 || mgr.triggers[0].activated) {
            fprintf(stderr, "FAIL: step off\n"); fail++;
        } else {
            printf("  Step off (4,4): deactivated OK\n");
        }
    }

    /* Test 5: wall switch toggles */
    {
        Nexus_TriggerManager mgr;
        nexus_v1_triggers_init(&mgr);
        nexus_v1_trigger_register(&mgr, 2, 2, 0, 2, 5,
            NEXUS_TRIGGER_WALL_SWITCH, NEXUS_TRIGGER_ACT_TOGGLE_DOOR, 0);
        int f1 = nexus_v1_triggers_wall_click(&mgr, 2, 2, 0);
        if (f1 != 1 || !mgr.triggers[0].activated) {
            fprintf(stderr, "FAIL: wall click on\n"); fail++;
        } else {
            int f2 = nexus_v1_triggers_wall_click(&mgr, 2, 2, 0);
            if (f2 != 1 || mgr.triggers[0].activated) {
                fprintf(stderr, "FAIL: wall click toggle off\n"); fail++;
            } else {
                printf("  Wall switch toggle on/off OK\n");
            }
        }
    }

    /* Test 6: once-only trigger */
    {
        Nexus_TriggerManager mgr;
        nexus_v1_triggers_init(&mgr);
        nexus_v1_trigger_register(&mgr, 6, 6, -1, 6, 9,
            NEXUS_TRIGGER_FLOOR_PLATE, NEXUS_TRIGGER_ACT_ALARM, 1);
        nexus_v1_triggers_on_step(&mgr, 6, 6);
        nexus_v1_triggers_off_step(&mgr, 6, 6);
        int f2 = nexus_v1_triggers_on_step(&mgr, 6, 6);
        if (f2 != 0) {
            fprintf(stderr, "FAIL: once-only fired again\n"); fail++;
        } else {
            printf("  Once-only plate: second step ignored OK\n");
        }
    }

    /* Test 7: wrong position no-op */
    {
        Nexus_TriggerManager mgr;
        nexus_v1_triggers_init(&mgr);
        nexus_v1_trigger_register(&mgr, 1, 1, -1, 1, 5,
            NEXUS_TRIGGER_FLOOR_PLATE, NEXUS_TRIGGER_ACT_TOGGLE_DOOR, 0);
        int f = nexus_v1_triggers_on_step(&mgr, 2, 2);
        if (f != 0) {
            fprintf(stderr, "FAIL: wrong position triggered\n"); fail++;
        } else {
            printf("  Wrong position: no trigger OK\n");
        }
    }

    /* Test 8: get action and target */
    {
        Nexus_TriggerManager mgr;
        int tx, ty;
        nexus_v1_triggers_init(&mgr);
        nexus_v1_trigger_register(&mgr, 7, 7, 2, 10, 12,
            NEXUS_TRIGGER_WALL_SWITCH, NEXUS_TRIGGER_ACT_TELEPORT, 0);
        if (nexus_v1_trigger_get_action(&mgr, 0) != NEXUS_TRIGGER_ACT_TELEPORT) {
            fprintf(stderr, "FAIL: get action\n"); fail++;
        } else {
            nexus_v1_trigger_get_target(&mgr, 0, &tx, &ty);
            if (tx != 10 || ty != 12) {
                fprintf(stderr, "FAIL: get target\n"); fail++;
            } else {
                printf("  Get action=TELEPORT target=(10,12) OK\n");
            }
        }
    }

    /* Test 9: NULL safety */
    {
        nexus_v1_triggers_init(NULL);
        nexus_v1_triggers_on_step(NULL, 0, 0);
        nexus_v1_triggers_wall_click(NULL, 0, 0, 0);
        if (nexus_v1_triggers_count(NULL) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus trigger system verified\n");
    return 0;
}
