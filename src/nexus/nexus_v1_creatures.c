
#include "nexus_v1_creatures.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Known Nexus creature types from MNS files */
static const struct { const char *name; const char *mns; int hp; int atk; int def; int spd; int xp; } g_creature_defs[] = {
    {"Scorpion",     "SCORPION.MNS",  30, 15, 5,  3, 25},
    {"Mummy",        "MUMMY.MNS",     50, 20, 8,  2, 40},
    {"Dragon",       "DRAGON.MNS",   200, 60, 30, 4, 200},
    {"Skeleton",     "SKELETON.MNS",  35, 18, 6,  3, 30},
    {"Ghost",        "GHOST.MNS",     25, 12, 2,  5, 20},
    {"Worm",         "WORM.MNS",      80, 25, 10, 2, 60},
    {"Golem",        "GOLEM.MNS",    120, 35, 20, 1, 100},
    {"Spider",       "SPIDER.MNS",    20, 10, 3,  4, 15},
    {NULL, NULL, 0, 0, 0, 0, 0}
};

void nexus_v1_creatures_init(Nexus_V1_CreatureManager *mgr) {
    int i;
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
    for (i = 0; g_creature_defs[i].name; i++) {
        Nexus_CreatureType *t = &mgr->types[i];
        strncpy(t->name, g_creature_defs[i].name, 31);
        strncpy(t->model_file, g_creature_defs[i].mns, 31);
        t->health = g_creature_defs[i].hp;
        t->attack = g_creature_defs[i].atk;
        t->defense = g_creature_defs[i].def;
        t->speed = g_creature_defs[i].spd;
        t->experience_value = g_creature_defs[i].xp;
        t->model_index = -1;
        mgr->type_count++;
    }
}

int nexus_v1_creature_spawn(Nexus_V1_CreatureManager *mgr, int type_idx, int x, int y, int dir) {
    Nexus_Creature *c;
    if (!mgr || type_idx < 0 || type_idx >= mgr->type_count) return -1;
    if (mgr->active_count >= NEXUS_MAX_ACTIVE_CREATURES) return -1;
    c = &mgr->active[mgr->active_count];
    c->type_index = type_idx;
    c->health = mgr->types[type_idx].health;
    c->x = x; c->y = y; c->facing = dir;
    c->alive = 1; c->state = 1; /* patrol */
    c->ai_timer = 0;
    mgr->active_count++;
    return mgr->active_count - 1;
}

void nexus_v1_creatures_tick(Nexus_V1_CreatureManager *mgr, int party_x, int party_y) {
    int i;
    if (!mgr) return;
    for (i = 0; i < mgr->active_count; i++) {
        Nexus_Creature *c = &mgr->active[i];
        int dist;
        if (!c->alive) continue;
        c->ai_timer++;
        dist = abs(c->x - party_x) + abs(c->y - party_y);

        /* Simple AI: chase if close, patrol if far */
        if (dist <= 3) {
            c->state = 2; /* chase */
            if (dist <= 1) c->state = 3; /* attack range */
        } else {
            c->state = 1; /* patrol */
        }

        /* Move toward party when chasing (every N ticks based on speed) */
        if (c->state == 2 && c->ai_timer % (6 - mgr->types[c->type_index].speed) == 0) {
            if (abs(party_x - c->x) > abs(party_y - c->y)) {
                c->x += (party_x > c->x) ? 1 : -1;
            } else {
                c->y += (party_y > c->y) ? 1 : -1;
            }
        }
    }
}

