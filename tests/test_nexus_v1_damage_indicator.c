
#include <stdio.h>
#include <string.h>
#include "nexus_v1_damage_indicator.h"

int main(void) {
    int fail = 0;

    /* Test 1: init */
    {
        Nexus_DamageDisplay dd;
        nexus_v1_damage_display_init(&dd);
        if (nexus_v1_damage_display_active(&dd, 0) != 0) {
            fprintf(stderr, "FAIL: init\n"); fail++;
        } else {
            printf("  Init OK\n");
        }
    }

    /* Test 2: add damage indicator */
    {
        Nexus_DamageDisplay dd;
        nexus_v1_damage_display_init(&dd);
        nexus_v1_damage_display_add(&dd, 0, 15, NEXUS_DMG_TAKEN);
        if (nexus_v1_damage_display_active(&dd, 0) != 1) {
            fprintf(stderr, "FAIL: add\n"); fail++;
        } else {
            printf("  Add indicator OK\n");
        }
    }

    /* Test 3: portrait flash on damage taken */
    {
        Nexus_DamageDisplay dd;
        nexus_v1_damage_display_init(&dd);
        nexus_v1_damage_display_add(&dd, 1, 10, NEXUS_DMG_TAKEN);
        if (!nexus_v1_damage_display_flash(&dd, 1)) {
            fprintf(stderr, "FAIL: flash\n"); fail++;
        } else {
            printf("  Portrait flash OK\n");
        }
    }

    /* Test 4: no flash on heal */
    {
        Nexus_DamageDisplay dd;
        nexus_v1_damage_display_init(&dd);
        nexus_v1_damage_display_add(&dd, 2, 20, NEXUS_DMG_HEALED);
        if (nexus_v1_damage_display_flash(&dd, 2)) {
            fprintf(stderr, "FAIL: flash on heal\n"); fail++;
        } else {
            printf("  No flash on heal OK\n");
        }
    }

    /* Test 5: indicator expires */
    {
        Nexus_DamageDisplay dd;
        int i;
        nexus_v1_damage_display_init(&dd);
        nexus_v1_damage_display_add(&dd, 0, 5, NEXUS_DMG_DEALT);
        for (i = 0; i < NEXUS_DAMAGE_DISPLAY_TICKS; i++)
            nexus_v1_damage_display_tick(&dd);
        if (nexus_v1_damage_display_active(&dd, 0) != 0) {
            fprintf(stderr, "FAIL: not expired\n"); fail++;
        } else {
            printf("  Indicator expires OK\n");
        }
    }

    /* Test 6: flash expires */
    {
        Nexus_DamageDisplay dd;
        int i;
        nexus_v1_damage_display_init(&dd);
        nexus_v1_damage_display_add(&dd, 3, 8, NEXUS_DMG_TAKEN);
        for (i = 0; i < 8; i++)
            nexus_v1_damage_display_tick(&dd);
        if (nexus_v1_damage_display_flash(&dd, 3)) {
            fprintf(stderr, "FAIL: flash not expired\n"); fail++;
        } else {
            printf("  Flash expires OK\n");
        }
    }

    /* Test 7: multiple indicators per champion */
    {
        Nexus_DamageDisplay dd;
        nexus_v1_damage_display_init(&dd);
        nexus_v1_damage_display_add(&dd, 0, 5, NEXUS_DMG_TAKEN);
        nexus_v1_damage_display_add(&dd, 0, 10, NEXUS_DMG_TAKEN);
        nexus_v1_damage_display_add(&dd, 1, 7, NEXUS_DMG_DEALT);
        if (nexus_v1_damage_display_active(&dd, 0) != 2 ||
            nexus_v1_damage_display_active(&dd, 1) != 1) {
            fprintf(stderr, "FAIL: multiple\n"); fail++;
        } else {
            printf("  Multiple indicators OK\n");
        }
    }

    /* Test 8: NULL safety */
    {
        nexus_v1_damage_display_init(NULL);
        nexus_v1_damage_display_add(NULL, 0, 5, NEXUS_DMG_TAKEN);
        nexus_v1_damage_display_tick(NULL);
        if (nexus_v1_damage_display_active(NULL, 0) != 0 ||
            nexus_v1_damage_display_flash(NULL, 0) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus damage indicator verified\n");
    return 0;
}
