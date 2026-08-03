
#ifndef NEXUS_V1_PROJECTILES_H
#define NEXUS_V1_PROJECTILES_H

/* Nexus V1 projectile system — spell and thrown item projectiles.
 * Projectiles travel forward on the dungeon grid at a fixed speed
 * until they hit a wall or creature, then apply their effect.
 * Source: DM1 OBJECTMAN.C F0258 projectile movement,
 *         PROJECTIL.C F0218 projectile creation + collision,
 *         ReDMCSB COMMAND.C spell projectile dispatch. */

#include <stdint.h>
#include "nexus_v1_dungeon.h"
#include "nexus_v1_rasterizer.h"

#define NEXUS_MAX_PROJECTILES 16

typedef struct {
    int active;
    enum Nexus_ProjectileType type;
    int x, y;
    int dir;
    int damage;
    int speed_ticks;
    int timer;
    int source_champion;
    int source_creature;
} Nexus_Projectile;

typedef struct {
    Nexus_Projectile slots[NEXUS_MAX_PROJECTILES];
    int count;
} Nexus_ProjectileManager;

void nexus_v1_projectiles_init(Nexus_ProjectileManager* mgr);

int nexus_v1_projectile_spawn(Nexus_ProjectileManager* mgr,
                               enum Nexus_ProjectileType type,
                               int x, int y, int dir,
                               int damage, int speed_ticks,
                               int source_champion);

/* Tick all active projectiles. Returns number of hits this tick.
 * Projectiles that hit a wall are removed. Projectiles that hit
 * a creature apply damage via the creature manager (caller handles). */
typedef struct {
    int hit_x, hit_y;
    int damage;
    int projectile_type;
    int source_champion;
    int hit_wall;
    int slot_index;
} Nexus_ProjectileHit;

int nexus_v1_projectiles_tick(Nexus_ProjectileManager* mgr,
                               const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                               Nexus_ProjectileHit* out_hits, int max_hits);

int nexus_v1_projectile_count(const Nexus_ProjectileManager* mgr);

#endif
