
#include <stdio.h>
#include <string.h>
#include "nexus_v1_automap.h"

int main(void) {
    int fail = 0;

    /* Test 1: init clears all state */
    {
        Nexus_Automap map;
        nexus_v1_automap_init(&map);
        if (map.map_open != 0 || map.current_level != 0) {
            fprintf(stderr, "FAIL: init state\n"); fail++;
        } else {
            printf("  Init: map_open=0 current_level=0 OK\n");
        }
    }

    /* Test 2: reveal and query */
    {
        Nexus_Automap map;
        nexus_v1_automap_init(&map);
        nexus_v1_automap_reveal(&map, 0, 5, 5);
        if (!nexus_v1_automap_is_explored(&map, 0, 5, 5)) {
            fprintf(stderr, "FAIL: reveal not visible\n"); fail++;
        } else if (nexus_v1_automap_is_explored(&map, 0, 6, 6)) {
            fprintf(stderr, "FAIL: unexplored square visible\n"); fail++;
        } else {
            printf("  Reveal (0,5,5): explored=1, (0,6,6): explored=0 OK\n");
        }
    }

    /* Test 3: reveal radius */
    {
        Nexus_Automap map;
        nexus_v1_automap_init(&map);
        nexus_v1_automap_reveal_radius(&map, 2, 10, 10, 1);
        int count = nexus_v1_automap_explored_count(&map, 2);
        if (count != 9) {
            fprintf(stderr, "FAIL: radius=1 count=%d (exp 9)\n", count); fail++;
        } else {
            printf("  Reveal radius=1 at (10,10): 9 squares OK\n");
        }
    }

    /* Test 4: toggle */
    {
        Nexus_Automap map;
        nexus_v1_automap_init(&map);
        nexus_v1_automap_toggle(&map);
        if (!map.map_open) {
            fprintf(stderr, "FAIL: toggle on\n"); fail++;
        } else {
            nexus_v1_automap_toggle(&map);
            if (map.map_open) {
                fprintf(stderr, "FAIL: toggle off\n"); fail++;
            } else {
                printf("  Toggle on/off OK\n");
            }
        }
    }

    /* Test 5: boundary safety */
    {
        Nexus_Automap map;
        nexus_v1_automap_init(&map);
        nexus_v1_automap_reveal(&map, -1, 0, 0);
        nexus_v1_automap_reveal(&map, 0, -1, 0);
        nexus_v1_automap_reveal(&map, NEXUS_AUTOMAP_MAX_LEVELS, 0, 0);
        nexus_v1_automap_reveal(&map, 0, NEXUS_MAX_MAP_SIZE, 0);
        if (nexus_v1_automap_is_explored(&map, -1, 0, 0) ||
            nexus_v1_automap_is_explored(&map, 0, -1, 0)) {
            fprintf(stderr, "FAIL: boundary check\n"); fail++;
        } else {
            printf("  Boundary safety OK\n");
        }
    }

    /* Test 6: NULL safety */
    {
        nexus_v1_automap_init(NULL);
        nexus_v1_automap_reveal(NULL, 0, 0, 0);
        nexus_v1_automap_toggle(NULL);
        if (nexus_v1_automap_is_explored(NULL, 0, 0, 0) != 0 ||
            nexus_v1_automap_explored_count(NULL, 0) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    /* Test 7: multi-level isolation */
    {
        Nexus_Automap map;
        nexus_v1_automap_init(&map);
        nexus_v1_automap_reveal(&map, 0, 3, 3);
        nexus_v1_automap_reveal(&map, 5, 7, 7);
        if (!nexus_v1_automap_is_explored(&map, 0, 3, 3) ||
            nexus_v1_automap_is_explored(&map, 5, 3, 3) ||
            !nexus_v1_automap_is_explored(&map, 5, 7, 7)) {
            fprintf(stderr, "FAIL: multi-level isolation\n"); fail++;
        } else {
            printf("  Multi-level isolation OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus automap verified\n");
    return 0;
}
