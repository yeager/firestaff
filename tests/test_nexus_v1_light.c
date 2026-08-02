
#include <stdio.h>
#include "nexus_v1_light.h"

int main(void) {
    int fail = 0;

    /* Test 1: init starts at max light */
    {
        Nexus_LightState ls;
        nexus_v1_light_init(&ls);
        if (ls.level != NEXUS_LIGHT_MAX) {
            fprintf(stderr, "FAIL: init level=%d (exp %d)\n", ls.level, NEXUS_LIGHT_MAX);
            fail++;
        } else {
            printf("  Init: level=%d OK\n", ls.level);
        }
    }

    /* Test 2: light decays after NEXUS_LIGHT_DECAY_TICKS */
    {
        Nexus_LightState ls;
        int i;
        nexus_v1_light_init(&ls);
        for (i = 0; i < NEXUS_LIGHT_DECAY_TICKS; i++)
            nexus_v1_light_tick(&ls);
        if (ls.level != NEXUS_LIGHT_MAX - 1) {
            fprintf(stderr, "FAIL: decay level=%d (exp %d)\n",
                    ls.level, NEXUS_LIGHT_MAX - 1);
            fail++;
        } else {
            printf("  Decay after %d ticks: level=%d OK\n",
                   NEXUS_LIGHT_DECAY_TICKS, ls.level);
        }
    }

    /* Test 3: torch prevents decay */
    {
        Nexus_LightState ls;
        int i;
        nexus_v1_light_init(&ls);
        nexus_v1_light_torch_on(&ls, 500);
        for (i = 0; i < NEXUS_LIGHT_DECAY_TICKS * 2; i++)
            nexus_v1_light_tick(&ls);
        if (ls.level < 12) {
            fprintf(stderr, "FAIL: torch should prevent decay, level=%d\n", ls.level);
            fail++;
        } else {
            printf("  Torch prevents decay: level=%d OK\n", ls.level);
        }
    }

    /* Test 4: torch burns out */
    {
        Nexus_LightState ls;
        int i;
        nexus_v1_light_init(&ls);
        nexus_v1_light_set(&ls, 5);
        nexus_v1_light_torch_on(&ls, 10);
        for (i = 0; i < 10; i++)
            nexus_v1_light_tick(&ls);
        if (ls.torch_active) {
            fprintf(stderr, "FAIL: torch should burn out\n"); fail++;
        } else {
            printf("  Torch burns out after 10 ticks OK\n");
        }
    }

    /* Test 5: FUL spell sets max light */
    {
        Nexus_LightState ls;
        nexus_v1_light_init(&ls);
        nexus_v1_light_set(&ls, 3);
        nexus_v1_light_ful_spell(&ls, 2, 200);
        if (ls.level != NEXUS_LIGHT_MAX || !ls.ful_active) {
            fprintf(stderr, "FAIL: FUL spell level=%d active=%d\n",
                    ls.level, ls.ful_active);
            fail++;
        } else {
            printf("  FUL spell: level=%d active=1 OK\n", ls.level);
        }
    }

    /* Test 6: light_add clamps */
    {
        Nexus_LightState ls;
        nexus_v1_light_init(&ls);
        nexus_v1_light_add(&ls, 10);
        if (ls.level != NEXUS_LIGHT_MAX) {
            fprintf(stderr, "FAIL: add overflow level=%d\n", ls.level);
            fail++;
        }
        nexus_v1_light_set(&ls, 2);
        nexus_v1_light_add(&ls, -5);
        if (ls.level != NEXUS_LIGHT_MIN) {
            fprintf(stderr, "FAIL: add underflow level=%d\n", ls.level);
            fail++;
        } else {
            printf("  Light add with clamping OK\n");
        }
    }

    /* Test 7: NULL safety */
    {
        nexus_v1_light_init(NULL);
        nexus_v1_light_tick(NULL);
        nexus_v1_light_set(NULL, 5);
        nexus_v1_light_torch_on(NULL, 100);
        nexus_v1_light_ful_spell(NULL, 1, 100);
        if (nexus_v1_light_get(NULL) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus light system verified\n");
    return 0;
}
