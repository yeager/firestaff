
#include "nexus_v1_projectiles.h"
#include "nexus_v1_movement.h"
#include "nexus_v1_squares.h"
#include <string.h>

void nexus_v1_projectiles_init(Nexus_ProjectileManager* mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
}

int nexus_v1_projectile_spawn(Nexus_ProjectileManager* mgr,
                               enum Nexus_ProjectileType type,
                               int x, int y, int dir,
                               int damage, int speed_ticks,
                               int source_champion) {
    int i;
    if (!mgr || type < 0 || type >= NEXUS_PROJ_COUNT) return -1;
    for (i = 0; i < NEXUS_MAX_PROJECTILES; i++) {
        if (!mgr->slots[i].active) {
            mgr->slots[i].active = 1;
            mgr->slots[i].type = type;
            mgr->slots[i].x = x;
            mgr->slots[i].y = y;
            mgr->slots[i].dir = dir;
            mgr->slots[i].damage = damage;
            mgr->slots[i].speed_ticks = speed_ticks > 0 ? speed_ticks : 2;
            mgr->slots[i].timer = 0;
            mgr->slots[i].source_champion = source_champion;
            mgr->slots[i].source_creature = -1;
            mgr->count++;
            return i;
        }
    }
    return -1;
}

int nexus_v1_projectiles_tick(Nexus_ProjectileManager* mgr,
                               const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                               Nexus_ProjectileHit* out_hits, int max_hits) {
    int i, hits = 0;
    if (!mgr || !squares) return 0;

    for (i = 0; i < NEXUS_MAX_PROJECTILES; i++) {
        Nexus_Projectile* p = &mgr->slots[i];
        int dx, dy, nx, ny, sq;
        if (!p->active) continue;

        p->timer++;
        if (p->timer < p->speed_ticks) continue;
        p->timer = 0;

        nexus_dir_deltas(p->dir, &dx, &dy);
        nx = p->x + dx;
        ny = p->y + dy;

        if (nx < 0 || nx >= NEXUS_MAX_MAP_SIZE ||
            ny < 0 || ny >= NEXUS_MAX_MAP_SIZE) {
            p->active = 0;
            mgr->count--;
            continue;
        }

        sq = nexus_get_square(squares, nx, ny);
        /* The generic square classifier has no door-state input. Stop at an
         * unbound type-8 square rather than passing through a closed door. */
        if (sq == NEXUS_SQUARE_DOOR || !nexus_square_is_passable(sq)) {
            if (out_hits && hits < max_hits) {
                out_hits[hits].hit_x = p->x;
                out_hits[hits].hit_y = p->y;
                out_hits[hits].damage = p->damage;
                out_hits[hits].projectile_type = (int)p->type;
                out_hits[hits].source_champion = p->source_champion;
                out_hits[hits].hit_wall = 1;
                out_hits[hits].slot_index = i;
                hits++;
            }
            p->active = 0;
            mgr->count--;
            continue;
        }

        p->x = nx;
        p->y = ny;

        if (out_hits && hits < max_hits) {
            out_hits[hits].hit_x = nx;
            out_hits[hits].hit_y = ny;
            out_hits[hits].damage = p->damage;
            out_hits[hits].projectile_type = (int)p->type;
            out_hits[hits].source_champion = p->source_champion;
            out_hits[hits].hit_wall = 0;
            out_hits[hits].slot_index = i;
            hits++;
        }
    }
    return hits;
}

int nexus_v1_projectile_count(const Nexus_ProjectileManager* mgr) {
    if (!mgr) return 0;
    return mgr->count;
}
