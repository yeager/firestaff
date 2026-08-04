
#include <stdio.h>
#include "nexus_v1_status.h"

int main(void) {
    int fail = 0;

    /* Test 1: init clears all effects */
    {
        Nexus_StatusEffects se;
        nexus_v1_status_init(&se);
        int i, any = 0;
        for (i = 0; i < NEXUS_STATUS_COUNT; i++)
            if (se.active[i]) any = 1;
        if (any) {
            fprintf(stderr, "FAIL: init not cleared\n"); fail++;
        } else {
            printf("  Init: all effects inactive OK\n");
        }
    }

    /* Test 2: apply and query */
    {
        Nexus_StatusEffects se;
        nexus_v1_status_init(&se);
        nexus_v1_status_apply(&se, NEXUS_STATUS_POISON, 100, 8);
        if (!nexus_v1_status_is_active(&se, NEXUS_STATUS_POISON) ||
            nexus_v1_status_strength(&se, NEXUS_STATUS_POISON) != 8) {
            fprintf(stderr, "FAIL: apply/query\n"); fail++;
        } else {
            printf("  Apply poison str=8 dur=100: active OK\n");
        }
    }

    /* Test 3: tick expires effect */
    {
        Nexus_StatusEffects se;
        int i, expired;
        nexus_v1_status_init(&se);
        nexus_v1_status_apply(&se, NEXUS_STATUS_SHIELD, 5, 10);
        for (i = 0; i < 4; i++)
            nexus_v1_status_tick(&se);
        if (!nexus_v1_status_is_active(&se, NEXUS_STATUS_SHIELD)) {
            fprintf(stderr, "FAIL: expired too early\n"); fail++;
        } else {
            expired = nexus_v1_status_tick(&se);
            if (nexus_v1_status_is_active(&se, NEXUS_STATUS_SHIELD)) {
                fprintf(stderr, "FAIL: not expired after 5 ticks\n"); fail++;
            } else if (!(expired & (1 << NEXUS_STATUS_SHIELD))) {
                fprintf(stderr, "FAIL: expired bitmask wrong\n"); fail++;
            } else {
                printf("  Shield expires after 5 ticks OK\n");
            }
        }
    }

    /* Test 4: poison damage */
    {
        Nexus_StatusEffects se;
        nexus_v1_status_init(&se);
        nexus_v1_status_apply(&se, NEXUS_STATUS_POISON, 50, 12);
        int dmg = nexus_v1_status_poison_damage(&se);
        if (dmg != 3) {
            fprintf(stderr, "FAIL: poison dmg=%d (exp 3)\n", dmg); fail++;
        } else {
            printf("  Poison str=12: damage=3 OK\n");
        }
    }

    /* Test 5: poison damage minimum 1 */
    {
        Nexus_StatusEffects se;
        nexus_v1_status_init(&se);
        nexus_v1_status_apply(&se, NEXUS_STATUS_POISON, 50, 1);
        int dmg = nexus_v1_status_poison_damage(&se);
        if (dmg != 1) {
            fprintf(stderr, "FAIL: min poison dmg=%d\n", dmg); fail++;
        } else {
            printf("  Poison str=1: damage=1 (minimum) OK\n");
        }
    }

    /* Test 6: defense bonus from shield */
    {
        Nexus_StatusEffects se;
        nexus_v1_status_init(&se);
        if (nexus_v1_status_defense_bonus(&se) != 0) {
            fprintf(stderr, "FAIL: defense without shield\n"); fail++;
        }
        nexus_v1_status_apply(&se, NEXUS_STATUS_SHIELD, 100, 15);
        if (nexus_v1_status_defense_bonus(&se) != 15) {
            fprintf(stderr, "FAIL: defense bonus\n"); fail++;
        } else {
            printf("  Shield defense bonus=15 OK\n");
        }
    }

    /* Test 7: remove effect */
    {
        Nexus_StatusEffects se;
        nexus_v1_status_init(&se);
        nexus_v1_status_apply(&se, NEXUS_STATUS_HASTE, 100, 5);
        nexus_v1_status_remove(&se, NEXUS_STATUS_HASTE);
        if (nexus_v1_status_is_active(&se, NEXUS_STATUS_HASTE)) {
            fprintf(stderr, "FAIL: remove\n"); fail++;
        } else {
            printf("  Remove haste OK\n");
        }
    }

    /* Test 8: XP init */
    {
        Nexus_Experience xp;
        nexus_v1_xp_init(&xp);
        if (xp.xp[0] != 0 || xp.level[0] != 0) {
            fprintf(stderr, "FAIL: xp init\n"); fail++;
        } else {
            printf("  XP init: all zero OK\n");
        }
    }

    /* Test 9: XP add and levelup */
    {
        Nexus_Experience xp;
        nexus_v1_xp_init(&xp);
        nexus_v1_xp_add(&xp, NEXUS_XP_CLASS_FIGHTER, 10240);
        int leveled = nexus_v1_xp_check_levelup(&xp, NEXUS_XP_CLASS_FIGHTER);
        if (leveled != 1 || xp.level[NEXUS_XP_CLASS_FIGHTER] != 1) {
            fprintf(stderr, "FAIL: levelup: leveled=%d level=%d\n",
                    leveled, xp.level[NEXUS_XP_CLASS_FIGHTER]); fail++;
        } else {
            printf("  Fighter 10240 XP: level 0->1 OK\n");
        }
    }

    /* Test 10: multi-level up */
    {
        Nexus_Experience xp;
        nexus_v1_xp_init(&xp);
        nexus_v1_xp_add(&xp, NEXUS_XP_CLASS_WIZARD, 30720);
        int leveled = nexus_v1_xp_check_levelup(&xp, NEXUS_XP_CLASS_WIZARD);
        if (leveled < 3 || xp.level[NEXUS_XP_CLASS_WIZARD] < 3) {
            fprintf(stderr, "FAIL: multi-level: leveled=%d level=%d\n",
                    leveled, xp.level[NEXUS_XP_CLASS_WIZARD]); fail++;
        } else {
            printf("  Wizard 30720 XP: level 0->%d (%d levels) OK\n",
                   xp.level[NEXUS_XP_CLASS_WIZARD], leveled);
        }
    }

    /* Test 11: XP threshold table */
    {
        if (nexus_v1_xp_threshold(0) != 0 || nexus_v1_xp_threshold(1) != 10240 ||
            nexus_v1_xp_threshold(2) != 20480) {
            fprintf(stderr, "FAIL: xp thresholds\n"); fail++;
        } else {
            printf("  XP thresholds: L0=0 L1=10240 L2=20480 OK\n");
        }
    }

    /* Test 12: NULL safety */
    {
        nexus_v1_status_init(NULL);
        nexus_v1_status_apply(NULL, 0, 10, 5);
        nexus_v1_status_tick(NULL);
        nexus_v1_xp_init(NULL);
        nexus_v1_xp_add(NULL, 0, 100);
        if (nexus_v1_status_is_active(NULL, 0) != 0 ||
            nexus_v1_xp_check_levelup(NULL, 0) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus status effects and experience verified\n");
    return 0;
}
