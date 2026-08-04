
#include <stdio.h>
#include <string.h>
#include "nexus_v1_encumbrance.h"

int main(void) {
    int fail = 0;

    /* Test 1: unloaded champion — min ticks (DM.BIN 0x02C2EE: min=2) */
    {
        Nexus_V1_Champion ch;
        int ticks;
        memset(&ch, 0, sizeof(ch));
        ch.max_load = 200;
        ch.load = 0;
        ticks = nexus_v1_encumbrance_move_ticks(&ch);
        if (ticks < NEXUS_MIN_MOVE_TICKS || ticks > NEXUS_MAX_MOVE_TICKS) {
            fprintf(stderr, "FAIL: unloaded ticks=%d out of [2,31]\n", ticks); fail++;
        } else {
            printf("  Unloaded: %d ticks OK\n", ticks);
        }
    }

    /* Test 2: heavy load — slower than unloaded */
    {
        Nexus_V1_Champion ch;
        int light_ticks, heavy_ticks;
        memset(&ch, 0, sizeof(ch));
        ch.max_load = 200;
        ch.load = 0;
        light_ticks = nexus_v1_encumbrance_move_ticks(&ch);
        ch.load = 180;
        heavy_ticks = nexus_v1_encumbrance_move_ticks(&ch);
        if (heavy_ticks <= light_ticks) {
            fprintf(stderr, "FAIL: heavy=%d not > light=%d\n", heavy_ticks, light_ticks); fail++;
        } else {
            printf("  Heavy load: %d > %d ticks OK\n", heavy_ticks, light_ticks);
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
        /* DM.BIN 0x02A93A: tiered stamina cost.
         * light (<62.5%): 2, medium (>=62.5%): 3, overloaded: (excess*4)/max+4 */
        light.max_load = 200; light.load = 50;       /* 25% -> cost 2 */
        heavy.max_load = 200; heavy.load = 160;       /* 80% -> cost 3 */
        overloaded.max_load = 200; overloaded.load = 250; /* excess=50 -> (50*4)/200+4 = 5 */
        if (nexus_v1_encumbrance_stamina_cost(&light) != NEXUS_STAMINA_COST_LIGHT ||
            nexus_v1_encumbrance_stamina_cost(&heavy) != NEXUS_STAMINA_COST_MEDIUM ||
            nexus_v1_encumbrance_stamina_cost(&overloaded) != 5) {
            fprintf(stderr, "FAIL: stamina costs %d/%d/%d\n",
                    nexus_v1_encumbrance_stamina_cost(&light),
                    nexus_v1_encumbrance_stamina_cost(&heavy),
                    nexus_v1_encumbrance_stamina_cost(&overloaded));
            fail++;
        } else {
            printf("  Stamina cost: 2/3/5 OK (DM.BIN tiered)\n");
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
        if (nexus_v1_encumbrance_move_ticks(NULL) != 8 ||
            nexus_v1_encumbrance_stamina_cost(NULL) != NEXUS_STAMINA_COST_LIGHT ||
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
