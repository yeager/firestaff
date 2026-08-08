#include <stdio.h>
#include "nexus_v1_containers.h"

static int g_fail;
static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_fail;
    }
}

int main(void) {
    Nexus_ContainerManager manager;
    int index;

    nexus_v1_container_manager_init(&manager);
    expect(manager.count == 0, "container manager starts empty");

    /* Registration works */
    index = nexus_v1_container_register(&manager, NEXUS_CONTAINER_CHEST,
                                        3, 5, 0, -1);
    expect(index == 0 && manager.count == 1,
           "container registration succeeds");

    /* Add item works */
    expect(nexus_v1_container_add_item(&manager, index, 0x42) == 0,
           "item added to container at slot 0");
    expect(nexus_v1_container_item_count(&manager, index) == 1,
           "container has 1 item after add");

    /* Find at works */
    expect(nexus_v1_container_find_at(&manager, 3, 5) == index,
           "container found at registered position");

    /* Open works (unlocked container) */
    expect(nexus_v1_container_open(&manager, index, -1),
           "unlocked container opens successfully");
    expect(nexus_v1_container_is_open(&manager, index),
           "container is open after open call");

    /* Take works */
    expect(nexus_v1_container_take(&manager, index, 0) == 0x42,
           "take returns the item id");
    expect(nexus_v1_container_item_count(&manager, index) == 0,
           "container empty after take");

    /* NULL safety */
    nexus_v1_container_manager_init(NULL);
    expect(nexus_v1_container_register(NULL, 0, 0, 0, 0, 0) == -1,
           "NULL registration is rejected");
    expect(nexus_v1_container_add_item(NULL, 0, 0) == -1,
           "NULL loot insertion is rejected");
    expect(nexus_v1_container_find_at(NULL, 0, 0) == -1,
           "NULL lookup is rejected");
    expect(!nexus_v1_container_open(NULL, 0, 0),
           "NULL open is rejected");
    expect(nexus_v1_container_take(NULL, 0, 0) == -1,
           "NULL loot take is rejected");

    if (g_fail) {
        fprintf(stderr, "test_nexus_v1_containers: %d failure(s)\n", g_fail);
        return 1;
    }
    puts("ok: Nexus container manager verified");
    return 0;
}
