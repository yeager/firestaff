
#include <stdio.h>
#include <string.h>
#include "nexus_v1_item_use.h"

int main(void) {
    int fail = 0;

    /* ITEM.IBS is a declaration bank, not proof of live action semantics. */
    {
        Nexus_V1_Champion ch;
        Nexus_ItemDef item;
        Nexus_ItemUseResult r;
        memset(&ch, 0, sizeof(ch));
        memset(&item, 0, sizeof(item));
        ch.alive = 1;
        ch.food = 10;
        item.category = NEXUS_ITEM_FOOD;
        item.attribute = 25;
        r = nexus_v1_item_use(&ch, NULL, &item);
        if (r.result != NEXUS_USE_RESULT_NONE || ch.food != 10) {
            fprintf(stderr, "FAIL: unbound food route mutated state: result=%d food=%d\n", r.result, ch.food);
            fail++;
        } else {
            printf("  Food declaration remains no-op without Saturn action trace\n");
        }
    }

    /* Test 2: potion must not infer health from Word36. */
    {
        Nexus_V1_Champion ch;
        Nexus_ItemDef item;
        Nexus_ItemUseResult r;
        memset(&ch, 0, sizeof(ch));
        memset(&item, 0, sizeof(item));
        ch.alive = 1;
        ch.health = 50;
        ch.max_health = 100;
        item.category = NEXUS_ITEM_POTION;
        item.attribute = 30;
        r = nexus_v1_item_use(&ch, NULL, &item);
        if (r.result != NEXUS_USE_RESULT_NONE || ch.health != 50) {
            fprintf(stderr, "FAIL: unbound potion route mutated HP: hp=%d\n", ch.health);
            fail++;
        } else {
            printf("  Potion declaration does not infer health effect\n");
        }
    }

    /* Test 3: potion must not infer mana from Word36. */
    {
        Nexus_V1_Champion ch;
        Nexus_ItemDef item;
        Nexus_ItemUseResult r;
        memset(&ch, 0, sizeof(ch));
        memset(&item, 0, sizeof(item));
        ch.alive = 1;
        ch.mana = 20;
        ch.max_mana = 100;
        item.category = NEXUS_ITEM_POTION;
        item.attribute = 70;
        r = nexus_v1_item_use(&ch, NULL, &item);
        if (r.result != NEXUS_USE_RESULT_NONE || ch.mana != 20) {
            fprintf(stderr, "FAIL: unbound potion route mutated mana: mana=%d\n", ch.mana);
            fail++;
        } else {
            printf("  Potion declaration does not infer mana effect\n");
        }
    }

    /* Test 4: antidote removes poison */
    {
        Nexus_V1_Champion ch;
        Nexus_StatusEffects se;
        Nexus_ItemDef item;
        Nexus_ItemUseResult r;
        memset(&ch, 0, sizeof(ch));
        memset(&item, 0, sizeof(item));
        ch.alive = 1;
        nexus_v1_status_init(&se);
        nexus_v1_status_apply(&se, NEXUS_STATUS_POISON, 100, 10);
        item.category = NEXUS_ITEM_POTION;
        item.attribute = 150;
        r = nexus_v1_item_use(&ch, &se, &item);
        if (r.result != NEXUS_USE_RESULT_NONE ||
            !nexus_v1_status_is_active(&se, NEXUS_STATUS_POISON)) {
            fprintf(stderr, "FAIL: unbound antidote route mutated status\n"); fail++;
        } else {
            printf("  Antidote declaration does not infer status semantics\n");
        }
    }

    /* Test 5: shield potion applies status */
    {
        Nexus_V1_Champion ch;
        Nexus_StatusEffects se;
        Nexus_ItemDef item;
        Nexus_ItemUseResult r;
        memset(&ch, 0, sizeof(ch));
        memset(&item, 0, sizeof(item));
        ch.alive = 1;
        nexus_v1_status_init(&se);
        item.category = NEXUS_ITEM_POTION;
        item.attribute = 215;
        r = nexus_v1_item_use(&ch, &se, &item);
        if (r.result != NEXUS_USE_RESULT_NONE ||
            nexus_v1_status_is_active(&se, NEXUS_STATUS_SHIELD)) {
            fprintf(stderr, "FAIL: unbound shield route mutated status\n"); fail++;
        } else {
            printf("  Shield declaration does not infer status semantics\n");
        }
    }

    /* Test 6: can_use checks */
    {
        Nexus_ItemDef food, weapon;
        memset(&food, 0, sizeof(food));
        memset(&weapon, 0, sizeof(weapon));
        food.category = NEXUS_ITEM_FOOD;
        weapon.category = NEXUS_ITEM_WEAPON;
        if (nexus_v1_item_can_use(&food) || nexus_v1_item_can_use(&weapon)) {
            fprintf(stderr, "FAIL: can_use\n"); fail++;
        } else {
            printf("  can_use: all routes blocked without Saturn action trace\n");
        }
    }

    /* Test 7: NULL safety */
    {
        Nexus_ItemUseResult r = nexus_v1_item_use(NULL, NULL, NULL);
        if (r.result != NEXUS_USE_RESULT_FAILED) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus item use verified\n");
    return 0;
}
