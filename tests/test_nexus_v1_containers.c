
#include <stdio.h>
#include <string.h>
#include "nexus_v1_containers.h"

static int g_fail;
static void expect(int c, const char *m) {
    if (!c) { fprintf(stderr, "FAIL: %s\n", m); g_fail++; }
}

int main(void) {
    /* Test 1: init */
    {
        Nexus_ContainerManager mgr;
        nexus_v1_container_manager_init(&mgr);
        expect(mgr.count == 0, "init count 0");
    }

    /* Test 2: register */
    {
        Nexus_ContainerManager mgr;
        int idx;
        nexus_v1_container_manager_init(&mgr);
        idx = nexus_v1_container_register(&mgr, NEXUS_CONTAINER_CHEST, 3, 5, 0, -1);
        expect(idx == 0, "first idx 0");
        expect(mgr.containers[0].type == NEXUS_CONTAINER_CHEST, "type chest");
    }

    /* Test 3: add and take items */
    {
        Nexus_ContainerManager mgr;
        int ci, slot, item;
        nexus_v1_container_manager_init(&mgr);
        ci = nexus_v1_container_register(&mgr, NEXUS_CONTAINER_CHEST, 1, 1, 0, -1);
        slot = nexus_v1_container_add_item(&mgr, ci, 0x42);
        expect(slot == 0, "item in slot 0");
        nexus_v1_container_add_item(&mgr, ci, 0x43);
        expect(nexus_v1_container_item_count(&mgr, ci) == 2, "2 items");

        nexus_v1_container_open(&mgr, ci, -1);
        item = nexus_v1_container_take(&mgr, ci, 0);
        expect(item == 0x42, "took first item");
        expect(nexus_v1_container_item_count(&mgr, ci) == 1, "1 item left");
    }

    /* Test 4: locked chest */
    {
        Nexus_ContainerManager mgr;
        int ci;
        nexus_v1_container_manager_init(&mgr);
        ci = nexus_v1_container_register(&mgr, NEXUS_CONTAINER_CHEST, 2, 2, 1, 7);
        expect(!nexus_v1_container_open(&mgr, ci, -1), "no key = locked");
        expect(!nexus_v1_container_open(&mgr, ci, 5), "wrong key = locked");
        expect(nexus_v1_container_open(&mgr, ci, 7), "right key = open");
        expect(nexus_v1_container_is_open(&mgr, ci), "is open");
    }

    /* Test 5: find at position */
    {
        Nexus_ContainerManager mgr;
        nexus_v1_container_manager_init(&mgr);
        nexus_v1_container_register(&mgr, NEXUS_CONTAINER_CRATE, 4, 6, 0, -1);
        expect(nexus_v1_container_find_at(&mgr, 4, 6) == 0, "found");
        expect(nexus_v1_container_find_at(&mgr, 0, 0) == -1, "not found");
    }

    /* Test 6: take from closed container */
    {
        Nexus_ContainerManager mgr;
        int ci;
        nexus_v1_container_manager_init(&mgr);
        ci = nexus_v1_container_register(&mgr, NEXUS_CONTAINER_CHEST, 1, 1, 0, -1);
        nexus_v1_container_add_item(&mgr, ci, 0xAA);
        expect(nexus_v1_container_take(&mgr, ci, 0) == -1, "cant take from closed");
    }

    /* Test 7: take from empty slot */
    {
        Nexus_ContainerManager mgr;
        int ci;
        nexus_v1_container_manager_init(&mgr);
        ci = nexus_v1_container_register(&mgr, NEXUS_CONTAINER_CHEST, 1, 1, 0, -1);
        nexus_v1_container_open(&mgr, ci, -1);
        expect(nexus_v1_container_take(&mgr, ci, 0) == -1, "empty slot");
    }

    /* Test 8: unlocked chest opens immediately */
    {
        Nexus_ContainerManager mgr;
        int ci;
        nexus_v1_container_manager_init(&mgr);
        ci = nexus_v1_container_register(&mgr, NEXUS_CONTAINER_BARREL, 1, 1, 0, -1);
        expect(nexus_v1_container_open(&mgr, ci, -1), "unlocked opens");
    }

    /* Test 9: NULL safety */
    {
        nexus_v1_container_manager_init(NULL);
        expect(nexus_v1_container_register(NULL, 0, 0, 0, 0, 0) == -1, "NULL reg");
        expect(nexus_v1_container_add_item(NULL, 0, 0) == -1, "NULL add");
        expect(nexus_v1_container_find_at(NULL, 0, 0) == -1, "NULL find");
        expect(!nexus_v1_container_open(NULL, 0, 0), "NULL open");
        expect(nexus_v1_container_take(NULL, 0, 0) == -1, "NULL take");
        expect(!nexus_v1_container_is_open(NULL, 0), "NULL is_open");
        expect(nexus_v1_container_item_count(NULL, 0) == 0, "NULL count");
    }

    if (g_fail) {
        fprintf(stderr, "%d failures\n", g_fail);
        return 1;
    }
    printf("ok: Nexus container system verified\n");
    return 0;
}
