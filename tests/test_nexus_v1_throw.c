
#include <stdio.h>
#include <string.h>
#include "nexus_v1_throw.h"

int main(void) {
    int fail = 0;

    /* Test 1: init */
    {
        Nexus_ThrownItemManager mgr;
        nexus_v1_throw_init(&mgr);
        if (mgr.count != 0) {
            fprintf(stderr, "FAIL: init count=%d\n", mgr.count); fail++;
        } else {
            printf("  Init OK\n");
        }
    }

    /* Test 2: throw profile — weapon */
    {
        Nexus_ItemDef item;
        Nexus_ThrowProfile p;
        memset(&item, 0, sizeof(item));
        item.category = NEXUS_ITEM_WEAPON;
        item.attribute = 20;
        p = nexus_v1_throw_profile(&item);
        if (p.base_damage != 20 || p.speed_ticks != 3) {
            fprintf(stderr, "FAIL: weapon profile dmg=%d spd=%d\n", p.base_damage, p.speed_ticks);
            fail++;
        } else {
            printf("  Throw profile weapon OK\n");
        }
    }

    /* Test 3: throw profile — misc item */
    {
        Nexus_ItemDef item;
        Nexus_ThrowProfile p;
        memset(&item, 0, sizeof(item));
        item.category = NEXUS_ITEM_MISC;
        p = nexus_v1_throw_profile(&item);
        if (p.base_damage != 5) {
            fprintf(stderr, "FAIL: misc profile dmg=%d\n", p.base_damage); fail++;
        } else {
            printf("  Throw profile misc OK\n");
        }
    }

    /* Test 4: throw item spawns projectile and clears slot */
    {
        Nexus_ThrownItemManager mgr;
        Nexus_ProjectileManager proj;
        Nexus_V1_Champion ch;
        Nexus_ItemDef items[2];
        int result;

        nexus_v1_throw_init(&mgr);
        nexus_v1_projectiles_init(&proj);
        memset(&ch, 0, sizeof(ch));
        memset(items, 0, sizeof(items));
        memset(ch.slots, -1, sizeof(ch.slots));

        items[0].category = NEXUS_ITEM_WEAPON;
        items[0].attribute = 25;
        ch.slots[NEXUS_SLOT_WEAPON] = 0;
        ch.strength = 40;

        result = nexus_v1_throw_item(&mgr, &proj, &ch, 0,
            NEXUS_SLOT_WEAPON, items, 5, 5, 0);
        if (!result) {
            fprintf(stderr, "FAIL: throw returned 0\n"); fail++;
        } else if (ch.slots[NEXUS_SLOT_WEAPON] != -1) {
            fprintf(stderr, "FAIL: slot not cleared\n"); fail++;
        } else if (nexus_v1_projectile_count(&proj) != 1) {
            fprintf(stderr, "FAIL: no projectile spawned\n"); fail++;
        } else if (mgr.count != 1) {
            fprintf(stderr, "FAIL: thrown count=%d\n", mgr.count); fail++;
        } else {
            printf("  Throw item: slot cleared, projectile spawned OK\n");
        }
    }

    /* Test 5: on_hit returns item index */
    {
        Nexus_ThrownItemManager mgr;
        int item_idx;
        nexus_v1_throw_init(&mgr);
        mgr.items[0].active = 1;
        mgr.items[0].item_index = 42;
        mgr.items[0].projectile_slot = 3;
        mgr.count = 1;

        item_idx = nexus_v1_throw_on_hit(&mgr, 3);
        if (item_idx != 42 || mgr.count != 0) {
            fprintf(stderr, "FAIL: on_hit item=%d count=%d\n", item_idx, mgr.count); fail++;
        } else {
            printf("  On hit: item recovered OK\n");
        }
    }

    /* Test 6: on_hit for non-thrown returns -1 */
    {
        Nexus_ThrownItemManager mgr;
        nexus_v1_throw_init(&mgr);
        if (nexus_v1_throw_on_hit(&mgr, 5) != -1) {
            fprintf(stderr, "FAIL: on_hit should be -1\n"); fail++;
        } else {
            printf("  On hit non-thrown: -1 OK\n");
        }
    }

    /* Test 7: empty slot returns 0 */
    {
        Nexus_ThrownItemManager mgr;
        Nexus_ProjectileManager proj;
        Nexus_V1_Champion ch;
        Nexus_ItemDef items[1];

        nexus_v1_throw_init(&mgr);
        nexus_v1_projectiles_init(&proj);
        memset(&ch, 0, sizeof(ch));
        memset(items, 0, sizeof(items));
        memset(ch.slots, -1, sizeof(ch.slots));

        if (nexus_v1_throw_item(&mgr, &proj, &ch, 0,
                NEXUS_SLOT_WEAPON, items, 5, 5, 0) != 0) {
            fprintf(stderr, "FAIL: empty slot should fail\n"); fail++;
        } else {
            printf("  Empty slot: throw rejected OK\n");
        }
    }

    /* Test 8: NULL safety */
    {
        nexus_v1_throw_init(NULL);
        if (nexus_v1_throw_on_hit(NULL, 0) != -1 ||
            nexus_v1_throw_is_thrown(NULL, 0) != 0) {
            fprintf(stderr, "FAIL: NULL safety\n"); fail++;
        } else {
            printf("  NULL safety OK\n");
        }
    }

    if (fail) {
        fprintf(stderr, "%d failures\n", fail);
        return 1;
    }
    printf("ok: Nexus throw system verified\n");
    return 0;
}
