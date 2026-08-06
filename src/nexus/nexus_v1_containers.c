
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
    /* Retail LEV*.DGN currently supplies item/location records, not a
     * proven container owner, content chain, key dispatch or Saturn loot
     * writeback. Do not let the old DM1-shaped helper manufacture a chest
     * or promote caller-supplied item IDs into Nexus gameplay. */
    (void)mgr; (void)type; (void)map_x; (void)map_y;
    (void)locked; (void)key_id;
    return -1;
}

int nexus_v1_container_add_item(Nexus_ContainerManager *mgr,
    int container_idx, int item_id) {
    (void)mgr; (void)container_idx; (void)item_id;
    return -1;
}

int nexus_v1_container_find_at(const Nexus_ContainerManager *mgr,
    int map_x, int map_y) {
    (void)mgr; (void)map_x; (void)map_y;
    return -1;
}

int nexus_v1_container_open(Nexus_ContainerManager *mgr,
    int container_idx, int held_key_id) {
    (void)mgr; (void)container_idx; (void)held_key_id;
    return 0;
}

int nexus_v1_container_take(Nexus_ContainerManager *mgr,
    int container_idx, int slot) {
    (void)mgr; (void)container_idx; (void)slot;
    return -1;
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
