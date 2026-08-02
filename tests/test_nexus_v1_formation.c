
#include <stdio.h>
#include <string.h>
#include "nexus_v1_formation.h"

int main(void) {
    int fail = 0;

    /* Test 1: init assigns positions 0-3 */
    {
        Nexus_Formation f;
        nexus_v1_formation_init(&f, 4);
        if (f.positions[0] != 0 || f.positions[1] != 1 ||
            f.positions[2] != 2 || f.positions[3] != 3) {
            fprintf(stderr, "FAIL: init positions\n"); fail++;
        } else {
            printf("  Init: positions 0-3 OK\n");
        }
    }

    /* Test 2: front row detection */
    {
        Nexus_Formation f;
        nexus_v1_formation_init(&f, 4);
        if (!nexus_v1_formation_is_front(&f, 0) ||
            !nexus_v1_formation_is_front(&f, 1) ||
            nexus_v1_formation_is_front(&f, 2) ||
            nexus_v1_formation_is_front(&f, 3)) {
            fprintf(stderr, "FAIL: front row\n"); fail++;
        } else {
            printf("  Front row: slots 0,1=front, 2,3=rear OK\n");
        }
    }

    /* Test 3: swap positions */
    {
        Nexus_Formation f;
        nexus_v1_formation_init(&f, 4);
        nexus_v1_formation_swap(&f, 0, 3);
        if (!nexus_v1_formation_is_front(&f, 3) ||
            nexus_v1_formation_is_front(&f, 0)) {
            fprintf(stderr, "FAIL: swap\n"); fail++;
        } else {
            printf("  Swap: slot 0<->3 OK\n");
        }
    }

    /* Test 4: slot_at lookup */
    {
        Nexus_Formation f;
        nexus_v1_formation_init(&f, 4);
        if (nexus_v1_formation_slot_at(&f, NEXUS_POS_FRONT_LEFT, 4) != 0 ||
            nexus_v1_formation_slot_at(&f, NEXUS_POS_REAR_RIGHT, 4) != 3) {
            fprintf(stderr, "FAIL: slot_at\n"); fail++;
        } else {
            printf("  Slot at position OK\n");
        }
    }

    /* Test 5: melee target favors front row */
    {
        Nexus_Formation f;
        int front_hits = 0, i, target;
        nexus_v1_formation_init(&f, 4);
        for (i = 0; i < 100; i++) {
            target = nexus_v1_formation_melee_target(&f, 4, i);
            if (nexus_v1_formation_is_front(&f, target))
                front_hits++;
        }
        if (front_hits < 60) {
            fprintf(stderr, "FAIL: melee front_hits=%d (expected ~75)\n", front_hits);
            fail++;
        } else {
            printf("  Melee targets: %d%% front row OK\n", front_hits);
        }
    }

    /* Test 6: melee works with only rear (all front dead) */
    {
        Nexus_Formation f;
        int target;
        nexus_v1_formation_init(&f, 2);
        f.positions[0] = NEXUS_POS_REAR_LEFT;
        f.positions[1] = NEXUS_POS_REAR_RIGHT;
        target = nexus_v1_formation_melee_target(&f, 2, 50);
        if (target < 0 || target >= 2) {
            fprintf(stderr, "FAIL: rear-only melee target=%d\n", target); fail++;
        } else {
            printf("  Melee rear-only: target=%d OK\n", target);
        }
    }

    /* Test 7: ranged target is uniform */
    {
        int target = nexus_v1_formation_ranged_target(4, 7);
        if (target != 3) {
            fprintf(stderr, "FAIL: ranged target=%d\n", target); fail++;
        } else {
            printf("  Ranged target: 7%%4=%d OK\n", target);
        }
    }

    /* Test 8: 2-member party */
    {
        Nexus_Formation f;
        nexus_v1_formation_init(&f, 2);
        if (!nexus_v1_formation_is_front(&f, 0) ||
            !nexus_v1_formation_is_front(&f, 1)) {
            fprintf(stderr, "FAIL: 2-member\n"); fail++;
        } else {
            printf("  2-member party: both front OK\n");
        }
    }

    /* Test 9: NULL safety */
    {
        nexus_v1_formation_init(NULL, 4);
        nexus_v1_formation_swap(NULL, 0, 1);
        if (nexus_v1_formation_is_front(NULL, 0) ||
            nexus_v1_formation_slot_at(NULL, 0, 4) != -1) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus formation system verified\n");
    return 0;
}
