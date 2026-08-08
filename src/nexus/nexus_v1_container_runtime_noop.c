/* Capture-gated Nexus V1 container/loot production adapter.
 * DGN does not authenticate container contents, key dispatch, or writeback. */

#include "nexus_v1_containers.h"
#include <string.h>

void nexus_v1_container_manager_init(Nexus_ContainerManager *manager)
{
    if (manager) memset(manager, 0, sizeof(*manager));
}

int nexus_v1_container_register(Nexus_ContainerManager *manager, int type,
                                int map_x, int map_y, int locked, int key_id)
{
    (void)manager; (void)type; (void)map_x; (void)map_y;
    (void)locked; (void)key_id;
    return -1;
}

int nexus_v1_container_add_item(Nexus_ContainerManager *manager,
                                int container_idx, int item_id)
{
    (void)manager; (void)container_idx; (void)item_id;
    return -1;
}

int nexus_v1_container_find_at(const Nexus_ContainerManager *manager,
                               int map_x, int map_y)
{
    (void)manager; (void)map_x; (void)map_y;
    return -1;
}

int nexus_v1_container_open(Nexus_ContainerManager *manager,
                            int container_idx, int held_key_id)
{
    (void)manager; (void)container_idx; (void)held_key_id;
    return 0;
}

int nexus_v1_container_take(Nexus_ContainerManager *manager,
                            int container_idx, int slot)
{
    (void)manager; (void)container_idx; (void)slot;
    return -1;
}

int nexus_v1_container_is_open(const Nexus_ContainerManager *manager,
                               int container_idx)
{
    (void)manager; (void)container_idx;
    return 0;
}

int nexus_v1_container_item_count(const Nexus_ContainerManager *manager,
                                  int container_idx)
{
    (void)manager; (void)container_idx;
    return 0;
}
