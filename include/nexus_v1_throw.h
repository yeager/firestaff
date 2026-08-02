
#ifndef NEXUS_V1_THROW_H
#define NEXUS_V1_THROW_H

/* Nexus V1 thrown items — champions throw weapons/items as projectiles.
 * When a thrown projectile hits a wall or creature, the item drops
 * to the ground at the impact position.
 * Source: DM1 OBJECTMAN.C F0258, COMMAND.C throw action,
 *         ReDMCSB COMMAND.C throw mechanics. */

#include "nexus_v1_projectiles.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_inventory.h"

#define NEXUS_MAX_THROWN 16

typedef struct {
    int active;
    int item_index;
    int projectile_slot;
    int source_champion;
} Nexus_ThrownItem;

typedef struct {
    Nexus_ThrownItem items[NEXUS_MAX_THROWN];
    int count;
} Nexus_ThrownItemManager;

void nexus_v1_throw_init(Nexus_ThrownItemManager *mgr);

/* Throw the item in a champion's equipment slot. Returns 1 on success.
 * Removes item from slot, spawns a projectile. */
int nexus_v1_throw_item(Nexus_ThrownItemManager *mgr,
                         Nexus_ProjectileManager *proj_mgr,
                         Nexus_V1_Champion *champion,
                         int champion_index,
                         int slot,
                         const Nexus_ItemDef *item_defs,
                         int party_x, int party_y, int party_dir);

/* Map item category to projectile type and base damage. */
typedef struct {
    enum Nexus_ProjectileType proj_type;
    int base_damage;
    int speed_ticks;
} Nexus_ThrowProfile;

Nexus_ThrowProfile nexus_v1_throw_profile(const Nexus_ItemDef *item);

/* After a projectile hits, find and remove matching thrown item record.
 * Returns the item_index to drop on the ground, or -1 if not a thrown item. */
int nexus_v1_throw_on_hit(Nexus_ThrownItemManager *mgr, int projectile_slot);

/* Check if a projectile slot is tracked as a thrown item. */
int nexus_v1_throw_is_thrown(const Nexus_ThrownItemManager *mgr, int projectile_slot);

#endif
