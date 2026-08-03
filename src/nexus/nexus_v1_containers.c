
#include "nexus_v1_containers.h"
#include <string.h>

void nexus_v1_container_manager_init(Nexus_ContainerManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
    {
        int i, j;
        for (i = 0; i < NEXUS_MAX_CONTAINERS; i++)
            for (j = 0; j < NEXUS_CONTAINER_SLOTS; j++)
                mgr->containers[i].items[j] = -1;
    }
}

int nexus_v1_container_register(Nexus_ContainerManager *mgr,
    int type, int map_x, int map_y,
    int locked, int key_id) {
    Nexus_Container *c;
    int j;
    if (!mgr || mgr->count >= NEXUS_MAX_CONTAINERS) return -1;
    c = &mgr->containers[mgr->count];
    memset(c, 0, sizeof(*c));
    c->active = 1;
    c->type = type;
    c->map_x = map_x;
    c->map_y = map_y;
    c->locked = locked;
    c->key_id = key_id;
    for (j = 0; j < NEXUS_CONTAINER_SLOTS; j++)
        c->items[j] = -1;
    return mgr->count++;
}

int nexus_v1_container_add_item(Nexus_ContainerManager *mgr,
    int container_idx, int item_id) {
    Nexus_Container *c;
    int j;
    if (!mgr || container_idx < 0 || container_idx >= mgr->count) return -1;
    c = &mgr->containers[container_idx];
    for (j = 0; j < NEXUS_CONTAINER_SLOTS; j++) {
        if (c->items[j] < 0) {
            c->items[j] = item_id;
            c->item_count++;
            return j;
        }
    }
    return -1;
}

int nexus_v1_container_find_at(const Nexus_ContainerManager *mgr,
    int map_x, int map_y) {
    int i;
    if (!mgr) return -1;
    for (i = 0; i < mgr->count; i++) {
        if (mgr->containers[i].active &&
            mgr->containers[i].map_x == map_x &&
            mgr->containers[i].map_y == map_y)
            return i;
    }
    return -1;
}

int nexus_v1_container_open(Nexus_ContainerManager *mgr,
    int container_idx, int held_key_id) {
    Nexus_Container *c;
    if (!mgr || container_idx < 0 || container_idx >= mgr->count) return 0;
    c = &mgr->containers[container_idx];
    if (!c->active) return 0;
    if (c->opened) return 1;
    if (c->locked && c->key_id >= 0 && held_key_id != c->key_id) return 0;
    c->opened = 1;
    c->locked = 0;
    return 1;
}

int nexus_v1_container_take(Nexus_ContainerManager *mgr,
    int container_idx, int slot) {
    Nexus_Container *c;
    int item;
    if (!mgr || container_idx < 0 || container_idx >= mgr->count) return -1;
    c = &mgr->containers[container_idx];
    if (!c->opened) return -1;
    if (slot < 0 || slot >= NEXUS_CONTAINER_SLOTS) return -1;
    if (c->items[slot] < 0) return -1;
    item = c->items[slot];
    c->items[slot] = -1;
    c->item_count--;
    return item;
}

int nexus_v1_container_is_open(const Nexus_ContainerManager *mgr,
    int container_idx) {
    if (!mgr || container_idx < 0 || container_idx >= mgr->count) return 0;
    return mgr->containers[container_idx].opened;
}

int nexus_v1_container_item_count(const Nexus_ContainerManager *mgr,
    int container_idx) {
    if (!mgr || container_idx < 0 || container_idx >= mgr->count) return 0;
    return mgr->containers[container_idx].item_count;
}
