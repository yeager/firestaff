
#include <stdio.h>
#include <string.h>
#include "nexus_v1_item_use.h"

int main(void) {
    int fail = 0;

    /* Test 1: food restores food stat */
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
        if (r.result != NEXUS_USE_RESULT_CONSUMED || ch.food != 35) {
            fprintf(stderr, "FAIL: food use: result=%d food=%d\n", r.result, ch.food);
            fail++;
        } else {
            printf("  Food (attr=25): food 10->35 OK\n");
        }
    }

    /* Test 2: health potion */
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
        if (r.result != NEXUS_USE_RESULT_CONSUMED || ch.health != 80) {
            fprintf(stderr, "FAIL: health potion: hp=%d\n", ch.health);
            fail++;
        } else {
            printf("  Health potion (attr=30): hp 50->80 OK\n");
        }
    }

    /* Test 3: mana potion (attribute 50-99) */
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
        if (r.result != NEXUS_USE_RESULT_CONSUMED || ch.mana != 40) {
            fprintf(stderr, "FAIL: mana potion: mana=%d\n", ch.mana);
            fail++;
        } else {
            printf("  Mana potion (attr=70): mana 20->40 OK\n");
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
        if (r.result != NEXUS_USE_RESULT_CONSUMED ||
            nexus_v1_status_is_active(&se, NEXUS_STATUS_POISON)) {
            fprintf(stderr, "FAIL: antidote\n"); fail++;
        } else {
            printf("  Antidote: poison removed OK\n");
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
        if (r.result != NEXUS_USE_RESULT_CONSUMED ||
            !nexus_v1_status_is_active(&se, NEXUS_STATUS_SHIELD)) {
            fprintf(stderr, "FAIL: shield potion\n"); fail++;
        } else {
            printf("  Shield potion (attr=215): shield active OK\n");
        }
    }

    /* Test 6: can_use checks */
    {
        Nexus_ItemDef food, weapon;
        memset(&food, 0, sizeof(food));
        memset(&weapon, 0, sizeof(weapon));
        food.category = NEXUS_ITEM_FOOD;
        weapon.category = NEXUS_ITEM_WEAPON;
        if (!nexus_v1_item_can_use(&food) || nexus_v1_item_can_use(&weapon)) {
            fprintf(stderr, "FAIL: can_use\n"); fail++;
        } else {
            printf("  can_use: food=yes weapon=no OK\n");
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
