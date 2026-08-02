
#include <stdio.h>
#include <string.h>
#include "nexus_v1_action_timer.h"

int main(void) {
    int fail = 0;

    /* Test 1: init — all ready */
    {
        Nexus_ActionTimers t;
        nexus_v1_action_timers_init(&t);
        if (!nexus_v1_action_is_ready(&t, 0) || !nexus_v1_action_is_ready(&t, 3)) {
            fprintf(stderr, "FAIL: init not ready\n"); fail++;
        } else {
            printf("  Init: all ready OK\n");
        }
    }

    /* Test 2: cooldown makes champion not ready */
    {
        Nexus_ActionTimers t;
        nexus_v1_action_timers_init(&t);
        nexus_v1_action_start_cooldown(&t, 0, NEXUS_BASE_MELEE_COOLDOWN, NULL);
        if (nexus_v1_action_is_ready(&t, 0)) {
            fprintf(stderr, "FAIL: should not be ready during cooldown\n"); fail++;
        } else {
            printf("  Cooldown: not ready OK\n");
        }
    }

    /* Test 3: cooldown expires after ticks */
    {
        Nexus_ActionTimers t;
        int i;
        nexus_v1_action_timers_init(&t);
        nexus_v1_action_start_cooldown(&t, 0, NEXUS_BASE_MELEE_COOLDOWN, NULL);
        for (i = 0; i < NEXUS_BASE_MELEE_COOLDOWN; i++)
            nexus_v1_action_timers_tick(&t);
        if (!nexus_v1_action_is_ready(&t, 0)) {
            fprintf(stderr, "FAIL: should be ready after cooldown rem=%d\n",
                    nexus_v1_action_remaining(&t, 0));
            fail++;
        } else {
            printf("  Cooldown expires: ready after %d ticks OK\n",
                   NEXUS_BASE_MELEE_COOLDOWN);
        }
    }

    /* Test 4: dexterity reduces cooldown */
    {
        Nexus_ActionTimers t;
        Nexus_V1_Champion ch;
        nexus_v1_action_timers_init(&t);
        memset(&ch, 0, sizeof(ch));
        ch.dexterity = 40;
        nexus_v1_action_start_cooldown(&t, 1, NEXUS_BASE_MELEE_COOLDOWN, &ch);
        if (nexus_v1_action_remaining(&t, 1) >= NEXUS_BASE_MELEE_COOLDOWN) {
            fprintf(stderr, "FAIL: dex should reduce cooldown rem=%d\n",
                    nexus_v1_action_remaining(&t, 1));
            fail++;
        } else {
            printf("  Dexterity reduces cooldown: %d->%d OK\n",
                   NEXUS_BASE_MELEE_COOLDOWN, nexus_v1_action_remaining(&t, 1));
        }
    }

    /* Test 5: load increases cooldown */
    {
        Nexus_ActionTimers t;
        Nexus_V1_Champion ch;
        nexus_v1_action_timers_init(&t);
        memset(&ch, 0, sizeof(ch));
        ch.load = 200;
        ch.max_load = 200;
        nexus_v1_action_start_cooldown(&t, 2, NEXUS_BASE_MELEE_COOLDOWN, &ch);
        if (nexus_v1_action_remaining(&t, 2) <= NEXUS_BASE_MELEE_COOLDOWN) {
            fprintf(stderr, "FAIL: load should increase cooldown rem=%d\n",
                    nexus_v1_action_remaining(&t, 2));
            fail++;
        } else {
            printf("  Load increases cooldown: %d->%d OK\n",
                   NEXUS_BASE_MELEE_COOLDOWN, nexus_v1_action_remaining(&t, 2));
        }
    }

    /* Test 6: fraction for HUD */
    {
        Nexus_ActionTimers t;
        int frac;
        nexus_v1_action_timers_init(&t);
        nexus_v1_action_start_cooldown(&t, 0, 20, NULL);
        frac = nexus_v1_action_fraction(&t, 0);
        if (frac != 255) {
            fprintf(stderr, "FAIL: initial fraction=%d\n", frac); fail++;
        } else {
            int i;
            for (i = 0; i < 10; i++) nexus_v1_action_timers_tick(&t);
            frac = nexus_v1_action_fraction(&t, 0);
            if (frac < 100 || frac > 140) {
                fprintf(stderr, "FAIL: half fraction=%d\n", frac); fail++;
            } else {
                printf("  Fraction: 255->%d at half OK\n", frac);
            }
        }
    }

    /* Test 7: minimum cooldown clamp */
    {
        Nexus_ActionTimers t;
        Nexus_V1_Champion ch;
        nexus_v1_action_timers_init(&t);
        memset(&ch, 0, sizeof(ch));
        ch.dexterity = 255;
        nexus_v1_action_start_cooldown(&t, 0, 4, &ch);
        if (nexus_v1_action_remaining(&t, 0) < 4) {
            fprintf(stderr, "FAIL: below minimum %d\n",
                    nexus_v1_action_remaining(&t, 0));
            fail++;
        } else {
            printf("  Minimum clamp: %d OK\n", nexus_v1_action_remaining(&t, 0));
        }
    }

    /* Test 8: NULL safety */
    {
        nexus_v1_action_timers_init(NULL);
        nexus_v1_action_timers_tick(NULL);
        if (nexus_v1_action_is_ready(NULL, 0) ||
            nexus_v1_action_remaining(NULL, 0) != 0 ||
            nexus_v1_action_fraction(NULL, 0) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus action timer verified\n");
    return 0;
}
