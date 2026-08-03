
#include <stdio.h>
#include <string.h>
#include "nexus_v1_level_transition.h"

int main(void) {
    int fail = 0;

    /* Test 1: init */
    {
        Nexus_V1_TransitionTable table;
        nexus_v1_transition_table_init(&table);
        if (table.count != 0) {
            fprintf(stderr, "FAIL: init count=%d\n", table.count); fail++;
        } else {
            printf("  Init OK\n");
        }
    }

    /* Test 2: add and find stairs down */
    {
        Nexus_V1_TransitionTable table;
        Nexus_V1_Transition t;
        const Nexus_V1_Transition *found;
        nexus_v1_transition_table_init(&table);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRANSITION_STAIRS_DOWN;
        t.src_level = 0; t.src_x = 5; t.src_y = 10;
        t.dst_level = 1; t.dst_x = 3; t.dst_y = 7; t.dst_dir = 2;
        nexus_v1_transition_add(&table, &t);
        found = nexus_v1_transition_find(&table, 0, 5, 10);
        if (!found || found->dst_level != 1 || found->dst_x != 3) {
            fprintf(stderr, "FAIL: stairs down find\n"); fail++;
        } else {
            printf("  Stairs down OK\n");
        }
    }

    /* Test 3: find returns NULL for missing */
    {
        Nexus_V1_TransitionTable table;
        nexus_v1_transition_table_init(&table);
        if (nexus_v1_transition_find(&table, 0, 5, 10) != NULL) {
            fprintf(stderr, "FAIL: should return NULL\n"); fail++;
        } else {
            printf("  Missing returns NULL OK\n");
        }
    }

    /* Test 4: apply writes outputs */
    {
        Nexus_V1_Transition t;
        int level = -1, x = -1, y = -1, dir = -1;
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRANSITION_TELEPORTER;
        t.dst_level = 3; t.dst_x = 12; t.dst_y = 8; t.dst_dir = 1;
        if (!nexus_v1_transition_apply(&t, &level, &x, &y, &dir) ||
            level != 3 || x != 12 || y != 8 || dir != 1) {
            fprintf(stderr, "FAIL: apply %d %d %d %d\n",
                    level, x, y, dir); fail++;
        } else {
            printf("  Apply OK\n");
        }
    }

    /* Test 5: NONE kind rejected */
    {
        Nexus_V1_TransitionTable table;
        Nexus_V1_Transition t;
        nexus_v1_transition_table_init(&table);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRANSITION_NONE;
        if (nexus_v1_transition_add(&table, &t) || table.count != 0) {
            fprintf(stderr, "FAIL: NONE kind accepted\n"); fail++;
        } else {
            printf("  NONE kind rejected OK\n");
        }
    }

    /* Test 6: capacity limit */
    {
        Nexus_V1_TransitionTable table;
        Nexus_V1_Transition t;
        int i;
        nexus_v1_transition_table_init(&table);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRANSITION_STAIRS_UP;
        for (i = 0; i < NEXUS_MAX_TRANSITIONS; ++i) {
            t.src_x = i;
            nexus_v1_transition_add(&table, &t);
        }
        t.src_x = 999;
        if (nexus_v1_transition_add(&table, &t) ||
            table.count != NEXUS_MAX_TRANSITIONS) {
            fprintf(stderr, "FAIL: overflow count=%d\n", table.count); fail++;
        } else {
            printf("  Capacity limit OK\n");
        }
    }

    /* Test 7: multiple transitions on different levels */
    {
        Nexus_V1_TransitionTable table;
        Nexus_V1_Transition t;
        const Nexus_V1_Transition *f;
        nexus_v1_transition_table_init(&table);
        memset(&t, 0, sizeof(t));
        t.kind = NEXUS_TRANSITION_STAIRS_DOWN;
        t.src_level = 0; t.src_x = 1; t.src_y = 1;
        t.dst_level = 1; t.dst_x = 2; t.dst_y = 2; t.dst_dir = 0;
        nexus_v1_transition_add(&table, &t);
        t.src_level = 1; t.src_x = 2; t.src_y = 2;
        t.dst_level = 2; t.dst_x = 3; t.dst_y = 3; t.dst_dir = 1;
        nexus_v1_transition_add(&table, &t);
        f = nexus_v1_transition_find(&table, 1, 2, 2);
        if (!f || f->dst_level != 2) {
            fprintf(stderr, "FAIL: multi-level find\n"); fail++;
        } else {
            printf("  Multi-level find OK\n");
        }
    }

    /* Test 8: NULL safety */
    {
        nexus_v1_transition_table_init(NULL);
        if (nexus_v1_transition_find(NULL, 0, 0, 0) != NULL ||
            nexus_v1_transition_add(NULL, NULL) ||
            nexus_v1_transition_apply(NULL, NULL, NULL, NULL, NULL)) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus level transition system verified\n");
    return 0;
}
