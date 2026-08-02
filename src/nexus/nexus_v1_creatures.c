
#include "nexus_v1_creatures.h"
#include "nexus_v1_movement.h"  /* for nexus_get_square */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Local FNV-1a64 (mirrors nexus_v1_dungeon.c nexus_v1_fnv1a64) used for
 * MNS model metadata fingerprints and actor-mesh signatures. */
static uint64_t creatures_fnv1a64(const uint8_t *data, long size) {
    uint64_t h = 1469598103934665603ULL;
    long i;
    if (!data || size <= 0) return 0;
    for (i = 0; i < size; i++) {
        h ^= data[i];
        h *= 1099511628211ULL;
    }
    return h;
}

/* Real Nexus creature roster — 30 MNS filenames from DM.BIN 0x0385F0.
 * AI function pointers from DM.BIN 0x0383A8 (yam\crenet.c dispatch).
 * Stats sourced from RLOWFIX.BIN CRET section (96 bytes per creature). */
static const struct { const char *name; const char *mns; uint32_t ai_func; } g_creature_defs[] = {
    {"Ant Man",        "ANTMAN.MNS",   0xFFFFFFFF},
    {"Mummy",          "MUMMY.MNS",   0x060478D0},
    {"Ghost",          "GHOST.MNS",   0x06048374},
    {"Vexirk",         "VEXIRK.MNS",  0x060479E0},
    {"Giggler",        "GIGGLER.MNS", 0x06047B30},
    {"Golem",          "GOLEM.MNS",   0x06048378},
    {"Hell Hound",     "H_HOUND.MNS", 0x060479D0},
    {"Gold Dragon",    "D_GOLD.MNS",  0x060477C8},
    {"Silver Dragon",  "D_SILVER.MNS",0x06048388},
    {"Red Dragon",     "D_RED.MNS",   0x060479C8},
    {"Last Monster",   "LAS_MON.MNS", 0x06047C78},
    {"Oitu",           "OITU.MNS",    0x0604837E},
    {"Rat",            "RAT.MNS",     0x060479B8},
    {"Rock Pile",      "ROCKPILE.MNS",0x060477C8},
    {"Screamer",       "SCREAMER.MNS",0x06048388},
    {"Scorpion",       "SCORPION.MNS",0x060479A8},
    {"Floor Snake",    "SN_FLOOR.MNS",0x06047B90},
    {"Wall Snake",     "SN_WALL.MNS", 0x0604838A},
    {"Skeleton Sword", "S_SWORD.MNS", 0x060479B0},
    {"Skeleton Shield","S_SHIELD.MNS",0x06047870},
    {"Worm",           "WORM.MNS",    0x06048390},
    {"Green Dragon",   "GRN_DRA.MNS", 0x060479C0},
    {"Red Drake",      "RED_DRA.MNS", 0x06047928},
    {"Dragon Zombie",  "DRA_ZOM.MNS", 0x0604839C},
    {"Mini Dragon",    "MINI_DRA.MNS",0x060479D8},
    {"Borketh",        "BORKETH.MNS", 0x06047BC0},
    {"Chaos",          "CHAOS.MNS",   0x060483A0},
    {"Lord Rib",       "LORD_RIB.MNS",0x00000000},
    {"Big Worm",       "BIGWORM.MNS", 0x06047870},
    {"Obake",          "OBAKE.MNS",   0x06047888},
    {NULL, NULL, 0}
};

void nexus_v1_creatures_init(Nexus_V1_CreatureManager *mgr) {
    int i;
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
    for (i = 0; g_creature_defs[i].name; i++) {
        Nexus_CreatureType *t = &mgr->types[i];
        strncpy(t->name, g_creature_defs[i].name, 31);
        strncpy(t->model_file, g_creature_defs[i].mns, 31);
        t->model_index = -1;
        t->ai_func_ptr = g_creature_defs[i].ai_func;
        mgr->type_count++;
    }
}

void nexus_v1_creatures_init_production(Nexus_V1_CreatureManager *mgr) {
    if (!mgr) return;
    /* Seed the roster from the fixture table (names, MNS files, AI func
     * pointers), then override stats from RLOWFIX.BIN CRET section when
     * the data file is available.  CRET stats are Saturn-owned — the
     * fixture table health/attack/defense/speed values are only used as
     * fallback when RLOWFIX.BIN is absent. */
    nexus_v1_creatures_init(mgr);
}

int nexus_v1_creature_spawn_on_level(Nexus_V1_CreatureManager *mgr, int type_idx, int x, int y, int dir, int level) {
    Nexus_Creature *c;
    if (!mgr || type_idx < 0 || type_idx >= mgr->type_count) return -1;
    /* ReDMCSB: GROUP.C F0183 lines ~414-424 refuses to assign an active-group
     * slot after the active pool is exhausted, and the actor-type reference
     * is a hard reject (a bad type index would read garbage stats from the
     * type table). Malformed spatial fields from external Nexus DGN actor
     * records, however, are clamped/normalized below rather than dropped,
     * so a single bad coordinate does not silently lose an otherwise-valid
     * actor and the AI grid index stays in bounds. See
     * tests/test_nexus_v1_dgn_actor_slot_bounds.c and
     * probes/nexus/firestaff_nexus_v1_mechanics_parity_probe.c
     * probe_dgn_actor_refs() for the clamp/reject contract. */
    if (mgr->active_count >= NEXUS_MAX_ACTIVE_CREATURES) return -1;
    /* ReDMCSB: GROUP.C F0183 lines 389-432 caps active group slots before
     * writing G0375_ps_ActiveGroups. Nexus DGN actor records can be malformed
     * while fixture parsing is still being hardened, so preserve that fixed
     * pool boundary and clamp spatial fields before AI indexes the 64x64 grid. */
    if (x < 0) x = 0;
    if (x >= NEXUS_MAX_MAP_SIZE) x = NEXUS_MAX_MAP_SIZE - 1;
    if (y < 0) y = 0;
    if (y >= NEXUS_MAX_MAP_SIZE) y = NEXUS_MAX_MAP_SIZE - 1;
    dir %= 4;
    if (dir < 0) dir += 4;
    c = &mgr->active[mgr->active_count];
    c->type_index = type_idx;
    c->health = mgr->types[type_idx].health;
    c->x = x; c->y = y; c->facing = dir;
    c->alive = 1; c->state = 1; /* patrol */
    c->ai_timer = 0;
    c->level = level;
    c->actor_ref_bound = 0;
    c->hidden = 0;
    c->structure3_model_index = 0;
    c->model_signature = 0;
    c->wander_target_x = -1;
    c->wander_target_y = -1;
    c->wander_timer = 0;
    mgr->active_count++;
    return mgr->active_count - 1;
}

int nexus_v1_creature_spawn(Nexus_V1_CreatureManager *mgr, int type_idx, int x, int y, int dir) {
    return nexus_v1_creature_spawn_on_level(mgr, type_idx, x, y, dir, 0);
}

void nexus_v1_creatures_tick(Nexus_V1_CreatureManager *mgr, int party_x, int party_y,
                              const uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE],
                              int map_index) {
    int i;
    if (!mgr) return;
    for (i = 0; i < mgr->active_count; i++) {
        Nexus_Creature *c = &mgr->active[i];
        int dist;
        if (!c->alive) continue;
        /* Creatures only act on their own level. */
        if (c->level != map_index) continue;
        /* Fail-closed: actors without a proven type binding (type_index
         * < 0) have no source-locked stats, and hidden actors (Structure1B
         * invisible-by-default bit, DMWeb: the Grey Lord on LEV1.DGN) wait
         * for a reveal trigger whose semantics are not yet source-locked.
         * Neither may move or attack. */
        if (c->type_index < 0 || c->hidden) continue;

        /* Alarm override: while a level alarm is active, all living creatures
         * on this level remain in chase state regardless of distance.
         * Source: DM1 MOVESENS.C F0277 — ALARM sets creature alert=255. */
        if (mgr->alarm_timer > 0 && c->level == map_index) {
            c->state = 2; /* chase */
        }

        c->ai_timer++;
        dist = nexus_v1_creature_distance(c->x, c->y, party_x, party_y);

        /* Simple AI: chase if close, patrol/wander if far.
         * Creature speed determines how often it moves.
         * Source: DM1 F0209_GROUP_ProcessEvents29to41 creature AI timeline;
         *         CREATURE.C idle wander (small random moves when not alert). */
        if (dist <= 3) {
            c->state = 2; /* chase */
            if (dist <= 1) c->state = 3; /* attack range */
            c->wander_target_x = -1;
            c->wander_target_y = -1;
        } else if (c->state != 2 || mgr->alarm_timer <= 0) {
            /* Only drop back to patrol when not alarm-chasing. */
            c->state = 1; /* patrol */
            if (c->wander_timer <= 0) {
                /* Pick a new nearby wander target within a 3-square radius.
                 * Source: DM1 CREATURE.C idle movement — creatures shuffle
                 * around their home area when not chasing the party. */
                int wx = c->x + (rand() % 7) - 3;
                int wy = c->y + (rand() % 7) - 3;
                if (wx < 0) wx = 0;
                if (wx >= NEXUS_MAX_MAP_SIZE) wx = NEXUS_MAX_MAP_SIZE - 1;
                if (wy < 0) wy = 0;
                if (wy >= NEXUS_MAX_MAP_SIZE) wy = NEXUS_MAX_MAP_SIZE - 1;
                c->wander_target_x = wx;
                c->wander_target_y = wy;
                c->wander_timer = 20 + (rand() % 20);
            }
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

        /* Wander when patrolling: move slowly toward the wander target.
         * Source: DM1 CREATURE.C idle wander (small random moves). */
        if (c->state == 1 && c->wander_target_x >= 0 && c->wander_target_y >= 0 &&
            c->ai_timer % (6 - mgr->types[c->type_index].speed) == 0) {
            int dx = 0, dy = 0;
            if (abs(c->wander_target_x - c->x) > abs(c->wander_target_y - c->y)) {
                dx = (c->wander_target_x > c->x) ? 1 : -1;
            } else {
                dy = (c->wander_target_y > c->y) ? 1 : -1;
            }
            if (dx != 0) {
                int sq = nexus_get_square(squares, c->x + dx, c->y);
                if (sq != 0 && sq != 21 && sq != 22) {
                    c->x += dx;
                    if (dx > 0) c->facing = 1;
                    else c->facing = 3;
                }
            } else if (dy != 0) {
                int sq = nexus_get_square(squares, c->x, c->y + dy);
                if (sq != 0 && sq != 21 && sq != 22) {
                    c->y += dy;
                    if (dy < 0) c->facing = 0;
                    else c->facing = 2;
                }
            }
            /* If we reached the target (or got stuck), pick a new one soon. */
            if (c->x == c->wander_target_x && c->y == c->wander_target_y) {
                c->wander_target_x = -1;
                c->wander_target_y = -1;
                c->wander_timer = 0;
            }
        }
        if (c->state == 1 && c->wander_timer > 0) c->wander_timer--;
    }
    if (mgr->alarm_timer > 0) mgr->alarm_timer--;
}

void nexus_v1_creatures_alert_all(Nexus_V1_CreatureManager *mgr, int level) {
    int i;
    if (!mgr) return;
    /* Alarm alert persists for a bounded number of ticks so creatures stay
     * chasing even when the party moves out of normal detection range.
     * Source: DM1 MOVESENS.C F0277 — ALARM sets creature alert=255. */
    mgr->alarm_timer = 60;
    for (i = 0; i < mgr->active_count; i++) {
        /* Fail-closed: unbound/hidden actors cannot chase (no source-locked
         * stats / no proven reveal trigger). */
        if (mgr->active[i].alive && mgr->active[i].level == level &&
                mgr->active[i].type_index >= 0 && !mgr->active[i].hidden) {
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
    /* Fail-closed: unbound actors have no source-locked attack stats and
     * hidden actors have not been revealed by a proven trigger. */
    if (c->type_index < 0 || c->hidden) return 0;

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

void nexus_v1_creatures_reset_active(Nexus_V1_CreatureManager *mgr) {
    if (!mgr) return;
    /* ReDMCSB: GROUP.C F0183 — the active-group pool is per-map; roster
     * types and evidence-gated bindings survive the level transition. */
    memset(mgr->active, 0, sizeof(mgr->active));
    mgr->active_count = 0;
    mgr->alarm_timer = 0;
    mgr->real_actor_spawn_count = 0;
}

int nexus_v1_creature_bind_mns_metadata(Nexus_V1_CreatureManager *mgr,
                                        int type_idx, const char *path) {
    FILE *file;
    long size;
    uint8_t *bytes;
    Nexus_CreatureType *t;

    if (!mgr || type_idx < 0 || type_idx >= mgr->type_count || !path) return 0;
    t = &mgr->types[type_idx];
    t->mns_bound = 0;
    t->mns_size = 0;
    t->mns_fnv1a64 = 0;

    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
            fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);

    /* Fail-closed: only an authenticated DMDF container may bind.
     * Source: nexus_v1_dmdf_model.c nexus_v1_dmdf_is_valid()
     * (magic 0x444D4446 "DMDF", >= 32-byte floor). */
    if (size < 32 || bytes[0] != 'D' || bytes[1] != 'M' ||
            bytes[2] != 'D' || bytes[3] != 'F') {
        free(bytes);
        return 0;
    }
    t->mns_size = (uint32_t)size;
    t->mns_fnv1a64 = creatures_fnv1a64(bytes, size);
    t->mns_bound = 1;
    free(bytes);
    return 1;
}

int nexus_v1_creature_bind_actor_model(Nexus_V1_CreatureManager *mgr,
                                       uint64_t model_signature,
                                       uint8_t structure3_model_index,
                                       int type_idx) {
    Nexus_V1_CreatureActorBinding *b;
    int i;
    if (!mgr || type_idx < 0 || type_idx >= mgr->type_count) return -1;
    /* Replace an existing binding for the same model identity. */
    for (i = 0; i < mgr->actor_binding_count; i++) {
        b = &mgr->actor_bindings[i];
        if (b->model_signature == model_signature &&
                b->structure3_model_index == structure3_model_index) {
            b->type_index = type_idx;
            return 0;
        }
    }
    if (mgr->actor_binding_count >= NEXUS_MAX_ACTOR_BINDINGS) return -1;
    b = &mgr->actor_bindings[mgr->actor_binding_count++];
    b->model_signature = model_signature;
    b->structure3_model_index = structure3_model_index;
    b->type_index = type_idx;
    return 0;
}

int nexus_v1_creature_actor_type_for(const Nexus_V1_CreatureManager *mgr,
                                     uint64_t model_signature,
                                     uint8_t structure3_model_index) {
    int i;
    int index_match = -1;
    if (!mgr) return -1;
    for (i = 0; i < mgr->actor_binding_count; i++) {
        const Nexus_V1_CreatureActorBinding *b = &mgr->actor_bindings[i];
        if (b->structure3_model_index != structure3_model_index) continue;
        if (b->model_signature == model_signature) return b->type_index;
        if (model_signature == 0 && index_match < 0) index_match = b->type_index;
    }
    return index_match;
}

int nexus_v1_creature_spawn_actor(Nexus_V1_CreatureManager *mgr,
                                  int x, int y, int dir, int level,
                                  int hidden, uint8_t structure3_model_index,
                                  uint64_t model_signature) {
    Nexus_Creature *c;
    int type_idx;
    if (!mgr) return -1;
    /* ReDMCSB: GROUP.C F0183 lines 389-432 caps active group slots before
     * writing G0375_ps_ActiveGroups; the same fixed pool boundary applies
     * to real DGN actor spawns.  Spatial fields are clamped exactly as in
     * nexus_v1_creature_spawn_on_level so a malformed record cannot push
     * the AI grid index out of bounds. */
    if (mgr->active_count >= NEXUS_MAX_ACTIVE_CREATURES) return -1;
    if (x < 0) x = 0;
    if (x >= NEXUS_MAX_MAP_SIZE) x = NEXUS_MAX_MAP_SIZE - 1;
    if (y < 0) y = 0;
    if (y >= NEXUS_MAX_MAP_SIZE) y = NEXUS_MAX_MAP_SIZE - 1;
    dir %= 4;
    if (dir < 0) dir += 4;

    type_idx = nexus_v1_creature_actor_type_for(mgr, model_signature,
                                                structure3_model_index);
    c = &mgr->active[mgr->active_count];
    memset(c, 0, sizeof(*c));
    c->type_index = type_idx;
    c->health = (type_idx >= 0) ? mgr->types[type_idx].health : 0;
    c->x = x; c->y = y; c->facing = dir;
    c->alive = 1;
    /* Unbound and hidden actors stay idle (fail-closed); proven visible
     * actors start in patrol like any roster spawn.
     * Source: DM1 CREATURE.C — creatures start non-alerted. */
    c->state = (type_idx >= 0 && !hidden) ? 1 : 0;
    c->ai_timer = 0;
    c->level = level;
    c->actor_ref_bound = 1;
    c->hidden = hidden ? 1 : 0;
    c->structure3_model_index = structure3_model_index;
    c->model_signature = model_signature;
    c->wander_target_x = -1;
    c->wander_target_y = -1;
    c->wander_timer = 0;
    mgr->active_count++;
    mgr->real_actor_spawn_count++;
    return mgr->active_count - 1;
}

int nexus_v1_creatures_load_cret(Nexus_V1_CreatureManager *mgr,
                                 const char *rlowfix_path) {
    FILE *file;
    long fsize;
    uint8_t *buf;
    int bound = 0, i;
    uint32_t cret_off, table_off;

    if (!mgr || !rlowfix_path) return 0;
    file = fopen(rlowfix_path, "rb");
    if (!file) return 0;
    if (fseek(file, 0, SEEK_END) != 0 || (fsize = ftell(file)) <= 0 ||
            fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    buf = (uint8_t *)malloc((size_t)fsize);
    if (!buf || fread(buf, 1, (size_t)fsize, file) != (size_t)fsize) {
        free(buf);
        fclose(file);
        return 0;
    }
    fclose(file);

    /* Scan for "CRET" magic followed by section-index 0 (the creature stat
     * table).  The RES directory at the start of the file also contains
     * "CRET" tags as pointers; the real data section has the 8-byte header
     * "CRET" + uint32 index (0 for stats) and is located deeper in the file
     * (offset > 0x100). */
    cret_off = 0;
    for (i = 0; i + 8 <= fsize; i++) {
        if (buf[i] == 'C' && buf[i+1] == 'R' && buf[i+2] == 'E' && buf[i+3] == 'T' &&
            buf[i+4] == 0 && buf[i+5] == 0 && buf[i+6] == 0 && buf[i+7] == 0 &&
            i > 0x100) {
            cret_off = (uint32_t)i;
            break;
        }
    }
    if (cret_off == 0) { free(buf); return 0; }

    table_off = cret_off + 8; /* skip "CRET\0\0\0\0" header */
    if (table_off + NEXUS_CRET_COUNT * NEXUS_CRET_RECORD_SIZE > (uint32_t)fsize) {
        free(buf);
        return 0;
    }

    for (i = 0; i < NEXUS_CRET_COUNT && i < mgr->type_count; i++) {
        Nexus_CreatureType *t = &mgr->types[i];
        const uint8_t *rec = buf + table_off + i * NEXUS_CRET_RECORD_SIZE;
        memcpy(t->cret_raw, rec, NEXUS_CRET_RECORD_SIZE);
        t->health  = rec[NEXUS_CRET_OFF_HP];
        t->attack  = rec[NEXUS_CRET_OFF_DAMAGE];
        t->defense = rec[NEXUS_CRET_OFF_ARMOR];
        t->speed   = rec[NEXUS_CRET_OFF_SPEED];
        t->cret_bound = 1;
        bound++;
    }

    free(buf);
    return bound;
}

int nexus_v1_creature_rebind_unbound(Nexus_V1_CreatureManager *mgr) {
    int i;
    int resolved = 0;
    if (!mgr) return 0;
    for (i = 0; i < mgr->active_count; i++) {
        Nexus_Creature *c = &mgr->active[i];
        int type_idx;
        if (!c->alive || !c->actor_ref_bound || c->type_index >= 0) continue;
        type_idx = nexus_v1_creature_actor_type_for(mgr, c->model_signature,
                                                    c->structure3_model_index);
        if (type_idx < 0) continue;
        c->type_index = type_idx;
        c->health = mgr->types[type_idx].health;
        if (!c->hidden && c->state == 0) c->state = 1; /* patrol */
        resolved++;
    }
    return resolved;
}
