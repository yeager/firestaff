
#include <stdio.h>
#include <string.h>
#include "nexus_v1_item_use.h"

int main(void) {
    int fail = 0;

    /* Food restores food and stamina. */
    {
        Nexus_V1_Champion ch;
        Nexus_ItemDef item;
        Nexus_ItemUseResult r;
        memset(&ch, 0, sizeof(ch));
        memset(&item, 0, sizeof(item));
        ch.alive = 1;
        ch.food = 10;
        ch.stamina = 50;
        ch.max_stamina = 200;
        item.category = NEXUS_ITEM_FOOD;
        item.attribute = 25;
        r = nexus_v1_item_use(&ch, NULL, &item);
        if (r.result != NEXUS_USE_RESULT_CONSUMED || ch.food != 510) {
            fprintf(stderr, "FAIL: food use: result=%d food=%d\n", r.result, ch.food);
            fail++;
        } else {
            printf("  Food restores food and stamina\n");
        }
    }

    /* Potion with positive attribute restores health. */
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
            printf("  Health potion restores HP\n");
        }
    }

    /* Potion with positive attribute restores health (capped at max). */
    {
        Nexus_V1_Champion ch;
        Nexus_ItemDef item;
        Nexus_ItemUseResult r;
        memset(&ch, 0, sizeof(ch));
        memset(&item, 0, sizeof(item));
        ch.alive = 1;
        ch.health = 90;
        ch.max_health = 100;
        item.category = NEXUS_ITEM_POTION;
        item.attribute = 50;
        r = nexus_v1_item_use(&ch, NULL, &item);
        if (r.result != NEXUS_USE_RESULT_CONSUMED || ch.health != 100) {
            fprintf(stderr, "FAIL: health potion cap: hp=%d\n", ch.health);
            fail++;
        } else {
            printf("  Health potion caps at max_health\n");
        }
    }

    /* can_use returns 1 for food and potion, 0 for weapon. */
    {
        Nexus_ItemDef food, weapon, potion;
        memset(&food, 0, sizeof(food));
        memset(&weapon, 0, sizeof(weapon));
        memset(&potion, 0, sizeof(potion));
        food.category = NEXUS_ITEM_FOOD;
        weapon.category = NEXUS_ITEM_WEAPON;
        potion.category = NEXUS_ITEM_POTION;
        if (!nexus_v1_item_can_use(&food) || nexus_v1_item_can_use(&weapon) ||
            !nexus_v1_item_can_use(&potion)) {
            fprintf(stderr, "FAIL: can_use\n"); fail++;
        } else {
            printf("  can_use: food/potion usable, weapon not\n");
        }
    }

    /* NULL safety */
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
