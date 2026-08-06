
#ifndef NEXUS_V1_CONTAINERS_H
#define NEXUS_V1_CONTAINERS_H

/* Nexus V1 container provenance seam.
 *
 * Retail LEV*.DGN currently exposes item/location records, but no
 * authenticated Saturn container owner, content chain, key dispatch or loot
 * writeback. The former DM1-shaped implementation is retained only as a
 * fail-closed API boundary until the real consumer is captured. */

#include <stdint.h>

#define NEXUS_MAX_CONTAINERS 64
#define NEXUS_CONTAINER_SLOTS 8

enum {
    NEXUS_CONTAINER_CHEST      = 0,
    NEXUS_CONTAINER_CRATE      = 1,
    NEXUS_CONTAINER_SARCOPHAGUS = 2,
    NEXUS_CONTAINER_BARREL     = 3
};

typedef struct {
    int active;
    int type;
    int map_x, map_y;
    int locked;
    int key_id;
    int opened;
    int items[NEXUS_CONTAINER_SLOTS];
    int item_count;
} Nexus_Container;

typedef struct {
    Nexus_Container containers[NEXUS_MAX_CONTAINERS];
    int count;
} Nexus_ContainerManager;

void nexus_v1_container_manager_init(Nexus_ContainerManager *mgr);

int nexus_v1_container_register(Nexus_ContainerManager *mgr,
    int type, int map_x, int map_y,
    int locked, int key_id);

int nexus_v1_container_add_item(Nexus_ContainerManager *mgr,
    int container_idx, int item_id);

int nexus_v1_container_find_at(const Nexus_ContainerManager *mgr,
    int map_x, int map_y);

/* Open a container. Returns 1 on success, 0 if locked/not found.
 * held_key_id: key the party holds, or -1 for none. */
int nexus_v1_container_open(Nexus_ContainerManager *mgr,
    int container_idx, int held_key_id);

/* Take an item from an opened container. Returns item_id or -1. */
int nexus_v1_container_take(Nexus_ContainerManager *mgr,
    int container_idx, int slot);

int nexus_v1_container_is_open(const Nexus_ContainerManager *mgr,
    int container_idx);

int nexus_v1_container_item_count(const Nexus_ContainerManager *mgr,
    int container_idx);

#endif
