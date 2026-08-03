
#include <stdio.h>
#include <string.h>
#include "nexus_v1_encumbrance.h"

int main(void) {
    int fail = 0;

    /* Test 1: unloaded champion — base speed */
    {
        Nexus_V1_Champion ch;
        memset(&ch, 0, sizeof(ch));
        ch.max_load = 200;
        ch.load = 0;
        if (nexus_v1_encumbrance_move_ticks(&ch) != NEXUS_BASE_MOVE_TICKS) {
            fprintf(stderr, "FAIL: unloaded ticks=%d\n",
                    nexus_v1_encumbrance_move_ticks(&ch)); fail++;
        } else {
            printf("  Unloaded: %d ticks OK\n", NEXUS_BASE_MOVE_TICKS);
        }
    }

    /* Test 2: heavy load — slower */
    {
        Nexus_V1_Champion ch;
        int ticks;
        memset(&ch, 0, sizeof(ch));
        ch.max_load = 200;
        ch.load = 180;
        ticks = nexus_v1_encumbrance_move_ticks(&ch);
        if (ticks <= NEXUS_BASE_MOVE_TICKS) {
            fprintf(stderr, "FAIL: heavy load ticks=%d\n", ticks); fail++;
        } else {
            printf("  Heavy load: %d ticks OK\n", ticks);
        }
    }

    /* Test 3: overloaded detection */
    {
        Nexus_V1_Champion ch;
        memset(&ch, 0, sizeof(ch));
        ch.max_load = 100;
        ch.load = 101;
        if (!nexus_v1_encumbrance_overloaded(&ch)) {
            fprintf(stderr, "FAIL: overloaded\n"); fail++;
        } else {
            printf("  Overloaded detection OK\n");
        }
    }

    /* Test 4: stamina cost increases with load */
    {
        Nexus_V1_Champion light, heavy, overloaded;
        memset(&light, 0, sizeof(light));
        memset(&heavy, 0, sizeof(heavy));
        memset(&overloaded, 0, sizeof(overloaded));
        light.max_load = 200; light.load = 50;
        heavy.max_load = 200; heavy.load = 160;
        overloaded.max_load = 200; overloaded.load = 250;
        if (nexus_v1_encumbrance_stamina_cost(&light) != 1 ||
            nexus_v1_encumbrance_stamina_cost(&heavy) != 2 ||
            nexus_v1_encumbrance_stamina_cost(&overloaded) != NEXUS_OVERLOADED_STAMINA_DRAIN) {
            fprintf(stderr, "FAIL: stamina costs %d/%d/%d\n",
                    nexus_v1_encumbrance_stamina_cost(&light),
                    nexus_v1_encumbrance_stamina_cost(&heavy),
                    nexus_v1_encumbrance_stamina_cost(&overloaded));
            fail++;
        } else {
            printf("  Stamina cost: 1/2/%d OK\n", NEXUS_OVERLOADED_STAMINA_DRAIN);
        }
    }

    /* Test 5: load ratio */
    {
        Nexus_V1_Champion ch;
        memset(&ch, 0, sizeof(ch));
        ch.max_load = 200;
        ch.load = 100;
        if (nexus_v1_encumbrance_ratio(&ch) != 50) {
            fprintf(stderr, "FAIL: ratio=%d\n", nexus_v1_encumbrance_ratio(&ch)); fail++;
        } else {
            printf("  Load ratio: 50%% OK\n");
        }
    }

    /* Test 6: recalc max_load from strength */
    {
        Nexus_V1_Champion ch;
        memset(&ch, 0, sizeof(ch));
        ch.strength = 30;
        ch.wounds = 0;
        nexus_v1_encumbrance_recalc_max_load(&ch);
        if (ch.max_load != (30 << 3) + 100) {
            fprintf(stderr, "FAIL: max_load=%d expected %d\n",
                    ch.max_load, (30 << 3) + 100); fail++;
        } else {
            printf("  Recalc: str=30 -> max_load=%d OK\n", ch.max_load);
        }
    }

    /* Test 7: wounds reduce max_load */
    {
        Nexus_V1_Champion ch;
        int full_load;
        memset(&ch, 0, sizeof(ch));
        ch.strength = 30;
        ch.wounds = 0;
        nexus_v1_encumbrance_recalc_max_load(&ch);
        full_load = ch.max_load;
        ch.wounds = 0xF;
        nexus_v1_encumbrance_recalc_max_load(&ch);
        if (ch.max_load >= full_load) {
            fprintf(stderr, "FAIL: wounds didn't reduce max_load=%d\n", ch.max_load);
            fail++;
        } else {
            printf("  Wounds: max_load %d->%d OK\n", full_load, ch.max_load);
        }
    }

    /* Test 8: NULL safety */
    {
        if (nexus_v1_encumbrance_move_ticks(NULL) != NEXUS_BASE_MOVE_TICKS ||
            nexus_v1_encumbrance_stamina_cost(NULL) != 1 ||
            nexus_v1_encumbrance_overloaded(NULL) ||
            nexus_v1_encumbrance_ratio(NULL) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            nexus_v1_encumbrance_recalc_max_load(NULL);
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus encumbrance system verified\n");
    return 0;
}
