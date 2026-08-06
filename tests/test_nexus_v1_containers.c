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

    /* No retail DGN container owner/content chain has been authenticated. */
    index = nexus_v1_container_register(&manager, NEXUS_CONTAINER_CHEST,
                                        3, 5, 0, -1);
    expect(index == -1 && manager.count == 0,
           "unproven container registration is blocked");
    expect(nexus_v1_container_add_item(&manager, index, 0x42) == -1,
           "caller-supplied container loot is blocked");
    expect(nexus_v1_container_find_at(&manager, 3, 5) == -1,
           "container lookup has no unproven owner");
    expect(!nexus_v1_container_open(&manager, index, -1),
           "container open remains blocked");
    expect(nexus_v1_container_take(&manager, index, 0) == -1,
           "container loot take remains blocked");
    expect(!nexus_v1_container_is_open(&manager, index),
           "blocked container cannot become open");
    expect(nexus_v1_container_item_count(&manager, index) == 0,
           "blocked container has no synthetic contents");

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
    puts("ok: Nexus container provenance gate verified");
    return 0;
}
