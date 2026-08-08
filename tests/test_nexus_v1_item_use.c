
#include <stdio.h>
#include <string.h>
#include "nexus_v1_item_use.h"

int main(void) {
    int fail = 0;

    /* ITEM.IBS does not authenticate a Saturn action/effect consumer. */
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
        if (r.result != NEXUS_USE_RESULT_NONE || ch.food != 10) {
            fprintf(stderr, "FAIL: food use was not fail-closed: result=%d food=%d\n", r.result, ch.food);
            fail++;
        } else {
            printf("  Food use remains fail-closed\n");
        }
    }

    /* Potion magnitude is not a proven ITEM.IBS field. */
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
            fprintf(stderr, "FAIL: health potion mutated state: hp=%d\n", ch.health);
            fail++;
        } else {
            printf("  Health potion remains fail-closed\n");
        }
    }

    /* A guessed potion cap must not be applied. */
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
        if (r.result != NEXUS_USE_RESULT_NONE || ch.health != 90) {
            fprintf(stderr, "FAIL: health potion cap: hp=%d\n", ch.health);
            fail++;
        } else {
            printf("  Health potion does not mutate champion state\n");
        }
    }

    /* No item is advertised as usable before Saturn action capture. */
    {
        Nexus_ItemDef food, weapon, potion;
        memset(&food, 0, sizeof(food));
        memset(&weapon, 0, sizeof(weapon));
        memset(&potion, 0, sizeof(potion));
        food.category = NEXUS_ITEM_FOOD;
        weapon.category = NEXUS_ITEM_WEAPON;
        potion.category = NEXUS_ITEM_POTION;
        if (nexus_v1_item_can_use(&food) || nexus_v1_item_can_use(&weapon) ||
            nexus_v1_item_can_use(&potion)) {
            fprintf(stderr, "FAIL: can_use\n"); fail++;
        } else {
            printf("  can_use: all item effects remain closed\n");
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
