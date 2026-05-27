
#include "nexus_v1_creatures.h"
#include "nexus_v1_movement.h"  /* for nexus_get_square */
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

void nexus_v1_creatures_tick(Nexus_V1_CreatureManager *mgr, int party_x, int party_y,
                              const uint8_t squares[32][32], int map_index) {
    int i;
    if (!mgr) return;
    for (i = 0; i < mgr->active_count; i++) {
        Nexus_Creature *c = &mgr->active[i];
        int dist;
        if (!c->alive) continue;
        c->ai_timer++;
        dist = nexus_v1_creature_distance(c->x, c->y, party_x, party_y);

        /* Simple AI: chase if close, patrol if far.
         * Creature speed determines how often it moves.
         * Source: DM1 F0209_GROUP_ProcessEvents29to41 creature AI timeline. */
        if (dist <= 3) {
            c->state = 2; /* chase */
            if (dist <= 1) c->state = 3; /* attack range */
        } else {
            c->state = 1; /* patrol */
        }

        /* Move toward party when chasing — speed-based movement interval.
         * Fast creatures (speed=5) move every tick; slow (speed=1) every 5 ticks.
         * Source: DM1 CREATURE.C creature movement timing. */
        if (c->state == 2 && c->ai_timer % (6 - mgr->types[c->type_index].speed) == 0) {
            int dx = 0, dy = 0;
            if (abs(party_x - c->x) > abs(party_y - c->y)) {
                dx = (party_x > c->x) ? 1 : -1;
            } else {
                dy = (party_y > c->y) ? 1 : -1;
            }

            /* Check if target square is passable before moving */
            if (dx != 0) {
                int sq = nexus_get_square(squares, c->x + dx, c->y);
                if (sq != 0 && sq != 21 && sq != 22) {
                    c->x += dx;
                    /* Update facing to movement direction */
                    if (dx > 0) c->facing = 1; /* East */
                    else if (dx < 0) c->facing = 3; /* West */
                }
            } else if (dy != 0) {
                int sq = nexus_get_square(squares, c->x, c->y + dy);
                if (sq != 0 && sq != 21 && sq != 22) {
                    c->y += dy;
                    if (dy < 0) c->facing = 0; /* North */
                    else if (dy > 0) c->facing = 2; /* South */
                }
            }
        }
    }
}

void nexus_v1_creatures_alert_all(Nexus_V1_CreatureManager *mgr, int level) {
    int i;
    (void)level;  /* STUB: level filtering not yet implemented */
    if (!mgr) return;
    for (i = 0; i < mgr->active_count; i++) {
        if (mgr->active[i].alive) {
            mgr->active[i].state = 2; /* chase — alerted */
            mgr->active[i].ai_timer = 0;
        }
    }
}

int nexus_v1_creature_distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

int nexus_v1_creature_attack(Nexus_V1_CreatureManager *mgr, int creature_idx,
                               int champion_defense, int *out_damage) {
    Nexus_Creature *c;
    int dmg, roll;
    (void)champion_defense;

    if (!mgr || creature_idx < 0 || creature_idx >= mgr->active_count) return 0;
    c = &mgr->active[creature_idx];
    if (!c->alive) return 0;

    /* Attack roll: creature attack vs champion defense.
     * DM1-style: hit if (attack_roll + creature_attack) > champion_defense.
     * Base hit chance: 60% + (creature_attack - champion_defense)/2.
     * Source: DM1 CREATURE.C creature attack resolution. */
    roll = rand() % 100;
    dmg = mgr->types[c->type_index].attack;

    /* Apply defense reduction: champion armor absorbs creature damage.
     * Formula: damage_reduction = defense/2 + random(0..defense/2)
     * Minimum damage = 1 (attacks always deal at least 1 damage).
     * Source: DM1 CREATURE.C F0209 creature attack vs champion (defense wiring;
     *         champion pool ref acquired via mechanics layer get_champion_defense). */
    if (champion_defense > 0) {
        int def_reduce = (champion_defense / 2) + (rand() % (champion_defense / 2 + 1));
        dmg -= def_reduce;
        if (dmg < 1) dmg = 1;
    }
    if (out_damage) *out_damage = dmg;

    /* Hit chance: 70% base + attack bonus */
    return (roll < 70 + dmg / 4) ? 1 : 0;
}

