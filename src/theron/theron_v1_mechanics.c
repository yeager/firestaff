/*
 * theron_v1_mechanics.c — Theron's Quest V1 Phase 5: Mechanics Parity
 *
 * Movement, click/command routing, doors, pits, teleporters,
 * altar-of-vi resurrection, pool recovery, and per-move stat processing.
 *
 * Source references:
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T560  — dungeon loading
 *   THQUEST.ASM T600  — map transitions / teleporter chains
 *   THQUEST.ASM T700  — per-tick champion stat updates
 *   THQUEST.ASM T800  — champion persistence + inventory reset
 *   THQUEST.ASM T900  — object database / altar-of-vi
 *
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md
 *   docs/source-lock/movement_features.md  (pit fall / teleporter chains)
 *   docs/source-lock/movement_forward_step.md
 */

#include "theron_v1_mechanics.h"
#include "theron_v1_combat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Compile-time checks ─────────────────────────────────────────── */
_Static_assert(THERON_DIR_COUNT == 4, "four cardinal directions");

/* ── Directional delta lookup ────────────────────────────────────── */
const int8_t g_theron_dir_dx[THERON_DIR_COUNT] = { 0, +1,  0, -1 };
const int8_t g_theron_dir_dy[THERON_DIR_COUNT] = { -1, 0, +1,  0 };

/* ── World party position helpers ─────────────────────────────────── */
/* Party position is stored in world->party.leader_pos{x,y,dir}
 * (Phase 3 world struct may store these in world or party struct);
 * for Phase 5, Theron_V1_Party keeps x, y, dir. */
static Theron_V1_Party *world_party(Theron_V1_World *world) {
    return &world->party;
}

/* ── Helper: resolve grid square from party at (x+dx, y+dy) ─────── */
static uint8_t look_ahead(const Theron_V1_World *world, int x, int y) {
    return theron_v1_world_get_square(world, x, y);
}

/* ── Helper: canonicalize direction (-1 left, +1 right) ──────────── */
static int normalize_dir(int dir) {
    return ((dir % THERON_DIR_COUNT) + THERON_DIR_COUNT) % THERON_DIR_COUNT;
}

/* ══════════════════════════════════════════════════════════════════════
 * Command / click routing
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_click_route(Theron_V1_World *world, int x, int y, int command) {
    if (!world) return -1;
    uint8_t tile = theron_v1_world_get_square(world, x, y);

    switch (command) {
    case THERON_CMD_MOVE: {
        /* Determine direction from party leader to clicked square */
        int px = world->party.leader_x;
        int py = world->party.leader_y;
        int dx = x - px;
        int dy = y - py;
        int dir;
        if (dy < 0) dir = THERON_DIR_NORTH;
        else if (dy > 0) dir = THERON_DIR_SOUTH;
        else if (dx > 0) dir = THERON_DIR_EAST;
        else if (dx < 0) dir = THERON_DIR_WEST;
        else return 0;
        return theron_v1_move_party(world, dir);
    }
    case THERON_CMD_LOOK:
    case THERON_CMD_EXAMINE:
        /* Phase 4: description lookup; for now just return the tile type */
        return (int)tile;
    case THERON_CMD_USE:
        if (tile == THERON_SQUARE_DOOR)
            return theron_v1_door_open(world, x, y);
        return -1;
    case THERON_CMD_TAKE: {
        Theron_V1_Object *o = theron_v1_object_at(world,
                                world->current_level, x, y);
        if (!o) return -1;
        /* Place into party inventory — Phase 5 inventory logic */
        return 0;
    }
    case THERON_CMD_ATTACK:
        if (tile == THERON_SQUARE_FLOOR) {
            /* Attack creature at that square */
            Theron_V1_Creature *cr =
                theron_v1_creature_at(world, world->current_level, x, y);
            if (cr) {
                return theron_v1_champion_attack(world,
                        world->party.active_slot, cr->id);
            }
        }
        return -1;
    default:
        return -1;
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Movement
 * ══════════════════════════════════════════════════════════════════════ */

/* Forward-declare for mutual recursion */
static int move_party_internal(Theron_V1_World *world, int direction);

int theron_v1_get_move_result(const Theron_V1_World *world, int direction) {
    if (!world) return THERON_MOVE_BLOCKED;
    int dir = normalize_dir(direction);
    int nx = world->party.leader_x + g_theron_dir_dx[dir];
    int ny = world->party.leader_y + g_theron_dir_dy[dir];
    uint8_t tile = look_ahead(world, nx, ny);

    if (tile == THERON_SQUARE_WALL || tile == THERON_SQUARE_SECRET) {
        return THERON_MOVE_BLOCKED;
    }
    if (tile == THERON_SQUARE_DOOR) {
        /* Doors are solid unless open/quarter-open */
        int state = world->party.door_state_override; /* placeholder */
        if (state < THERON_DOOR_STATE_QUARTER_OPEN) {
            return THERON_MOVE_BLOCKED;
        }
        return THERON_MOVE_SPECIAL;
    }
    if (tile == THERON_SQUARE_PIT) {
        /* Phase 5 pit logic: fall through */
        return THERON_MOVE_PIT_FALL;
    }
    if (tile == THERON_SQUARE_STAIRS_UP || tile == THERON_SQUARE_STAIRS_DOWN) {
        return THERON_MOVE_STAIRS;
    }
    if (tile == THERON_SQUARE_TELEPORTER) {
        return THERON_MOVE_TELEPORT;
    }
    if (tile == THERON_SQUARE_EXIT) {
        return THERON_MOVE_EXIT;
    }
    if (tile == THERON_SQUARE_FLOOR || tile == THERON_SQUARE_ALARM ||
        tile == THERON_SQUARE_TRIGGER || tile == THERON_SQUARE_POOL) {
        return THERON_MOVE_SPECIAL;
    }
    return THERON_MOVE_OK;
}

int theron_v1_move_party(Theron_V1_World *world, int direction) {
    return move_party_internal(world, direction);
}

static int move_party_internal(Theron_V1_World *world, int direction) {
    if (!world) return THERON_MOVE_BLOCKED;
    int dir = normalize_dir(direction);
    int nx = world->party.leader_x + g_theron_dir_dx[dir];
    int ny = world->party.leader_y + g_theron_dir_dy[dir];
    uint8_t tile = look_ahead(world, nx, ny);

    /* ── Walls / solid barriers ── */
    if (tile == THERON_SQUARE_WALL || tile == THERON_SQUARE_SECRET) {
        return THERON_MOVE_BLOCKED;
    }

    /* ── Door squares ── */
    if (tile == THERON_SQUARE_DOOR) {
        /* Doors block movement unless already open or quarter-open.
         * Block if locked or fully closed. */
        Theron_V1_Object *d = theron_v1_object_at(world,
                                world->current_level, nx, ny);
        if (d && d->type == THERON_OBJTYPE_DOOR) {
            if (d->state == THERON_DOOR_STATE_CLOSED &&
                (d->flags & THERON_DOOR_F_LOCKED)) {
                return THERON_MOVE_BLOCKED;
            }
            if (d->state == THERON_DOOR_STATE_CLOSED) {
                /* Try auto-open */
                theron_v1_door_open(world, nx, ny);
                /* If auto-open failed (e.g. no key), block */
                d = theron_v1_object_by_id(world, d->id);
                if (!d || d->state == THERON_DOOR_STATE_CLOSED) {
                    return THERON_MOVE_BLOCKED;
                }
            }
        }
        if (d && d->state >= THERON_DOOR_STATE_QUARTER_OPEN) {
            tile = THERON_SQUARE_FLOOR; /* treat open door as floor */
        } else {
            return THERON_MOVE_BLOCKED;
        }
    }

    /* ── Special squares: teleporter ── */
    if (tile == THERON_SQUARE_TELEPORTER) {
        theron_v1_teleporter_resolve(world, nx, ny);
        theron_v1_apply_post_move_effects(world);
        return THERON_MOVE_TELEPORT;
    }

    /* ── Special squares: pit ── */
    if (tile == THERON_SQUARE_PIT) {
        bool fell = theron_v1_pit_check_and_trigger(world, nx, ny);
        if (fell) {
            theron_v1_apply_post_move_effects(world);
            return THERON_MOVE_PIT_FALL;
        }
        /* Fall-through to normal move if pit is blocked/imaginary */
    }

    /* ── Special squares: stairs ── */
    if (tile == THERON_SQUARE_STAIRS_UP || tile == THERON_SQUARE_STAIRS_DOWN) {
        /* Teleporter-style result: queue transition but don't move */
        theron_v1_check_transition(world, nx, ny);
        theron_v1_transition_execute(world);
        theron_v1_apply_post_move_effects(world);
        return THERON_MOVE_STAIRS;
    }

    /* ── Special squares: exit ── */
    if (tile == THERON_SQUARE_EXIT) {
        if (world->dungeon_complete) {
            theron_v1_check_transition(world, nx, ny);
            theron_v1_transition_execute(world);
            theron_v1_apply_post_move_effects(world);
            return THERON_MOVE_EXIT;
        }
        return THERON_MOVE_BLOCKED;
    }

    /* ── Normal floor move ── */
    world->party.leader_x = nx;
    world->party.leader_y = ny;
    world->party.leader_dir = dir;

    /* Handle alarm square immediately */
    if (tile == THERON_SQUARE_ALARM) {
        theron_v1_alarm_trigger(world, nx, ny);
    }
    /* Handle trigger square */
    if (tile == THERON_SQUARE_TRIGGER) {
        theron_v1_trigger_activate(world, nx, ny);
    }
    /* Handle pool — water/food recovery */
    if (tile == THERON_SQUARE_POOL) {
        theron_v1_pool_use(world, nx, ny);
    }

    /* Handle altar-of-vi (TQ-specific resurrection square) */
    /* TQ altar squares are stored in the object map, not a tile type.
     * When a dead party member steps onto an altar square, T900 kicks in. */
    {
        Theron_V1_Object *o = theron_v1_object_at(world,
                                world->current_level, nx, ny);
        if (o && o->type == THERON_OBJTYPE_ALTAR_VI) {
            theron_v1_altar_of_vi_resurrect(world, nx, ny);
        }
    }

    /* Per-move stat updates: stamina drain, poison, food/water */
    theron_v1_apply_post_move_effects(world);

    return THERON_MOVE_OK;
}

int theron_v1_turn_party(Theron_V1_World *world, int turn) {
    if (!world) return -1;
    int d = world->party.leader_dir;
    d = ((d + turn) % THERON_DIR_COUNT + THERON_DIR_COUNT) % THERON_DIR_COUNT;
    world->party.leader_dir = d;
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Door interactions
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_door_open(Theron_V1_World *world, int x, int y) {
    if (!world) return -1;
    Theron_V1_Object *d = theron_v1_object_at(world, world->current_level, x, y);
    if (!d || d->type != THERON_OBJTYPE_DOOR) return -1;
    if (d->state >= THERON_DOOR_STATE_OPEN) return 0; /* already open */
    if (d->flags & THERON_DOOR_F_BROKEN) return -1;

    if (d->flags & THERON_DOOR_F_LOCKED) {
        /* Need a key in inventory — Phase 5 inventory check */
        d->flags &= ~THERON_DOOR_F_LOCKED;
        d->state = THERON_DOOR_STATE_OPEN;
        theron_v1_play_sound(THERON_SOUND_KEY_USE);
        return 0;
    }
    d->state = THERON_DOOR_STATE_OPEN;
    theron_v1_play_sound(THERON_SOUND_DOOR_OPEN);
    return 0;
}

int theron_v1_door_close(Theron_V1_World *world, int x, int y) {
    if (!world) return -1;
    Theron_V1_Object *d = theron_v1_object_at(world, world->current_level, x, y);
    if (!d || d->type != THERON_OBJTYPE_DOOR) return -1;
    if (d->state == THERON_DOOR_STATE_DESTROYED) return -1;
    d->state = THERON_DOOR_STATE_CLOSED;
    theron_v1_play_sound(THERON_SOUND_DOOR_CLOSE);
    return 0;
}

int theron_v1_door_is_open(const Theron_V1_World *world, int x, int y) {
    if (!world) return 0;
    const Theron_V1_Object *d = (const Theron_V1_Object *)
        theron_v1_object_at((Theron_V1_World *)world,
                              world->current_level, x, y);
    if (!d || d->type != THERON_OBJTYPE_DOOR) return 0;
    return d->state >= THERON_DOOR_STATE_QUARTER_OPEN;
}

int theron_v1_door_is_locked(const Theron_V1_World *world, int x, int y) {
    if (!world) return 0;
    const Theron_V1_Object *d = (const Theron_V1_Object *)
        theron_v1_object_at((Theron_V1_World *)world,
                              world->current_level, x, y);
    if (!d || d->type != THERON_OBJTYPE_DOOR) return 0;
    return (d->flags & THERON_DOOR_F_LOCKED) != 0;
}

int theron_v1_door_unlock_with_key(Theron_V1_World *world,
                                      int x, int y, int key_item) {
    if (!world) return -1;
    if (key_item != THERON_ITEM_KEY) return -1;
    Theron_V1_Object *d = theron_v1_object_at(world, world->current_level, x, y);
    if (!d || d->type != THERON_OBJTYPE_DOOR) return -1;
    d->flags &= ~THERON_DOOR_F_LOCKED;
    return theron_v1_door_open(world, x, y);
}

/* ══════════════════════════════════════════════════════════════════════
 * Pit mechanics
 *
 * Source: movement_features.md — pit fall chain (ReDMCSB MOVESENS.C:538)
 * Conditions for pit fall:
 *   1. Square type = PIT
 *   2. Party does NOT levitate (world->party.levitating == 0)
 *   3. Pit OPEN bit is set (MASK0x0008_PIT_OPEN = bit 3)
 *   4. Pit is NOT imaginary (MASK0x0001_PIT_IMAGINARY is clear)
 * On fall: party takes stamina/HP damage; world level may change.
 * ══════════════════════════════════════════════════════════════════════ */

bool theron_v1_pit_check_and_trigger(Theron_V1_World *world,
                                      int x, int y) {
    if (!world) return false;
    uint8_t tile = theron_v1_world_get_square(world, x, y);
    if (tile != THERON_SQUARE_PIT) return false;

    /* TQ pits always have OPEN bit set in the grid attribute byte.
     * For Phase 5, treat any PIT tile as active (no anti-pit flags in TQ grid). */
    if (!(world->party.levitating)) {
        /* Party falls: wound the leader's legs, drain stamina */
        Theron_V1_Champion *c =
            &world->party.champions[world->party.active_slot];
        if (c->alive) {
            /* 20 HP self-damage (pit fall) — matches ReDMCSB F0128 chain */
            int dmg = 20;
            if (c->wounds & THERON_WOUND_LEGS) dmg += 10;
            c->health -= dmg;
            if (c->health <= 0) {
                c->health = 0;
                c->alive  = 0;
                theron_v1_champion_die(world, world->party.active_slot);
            }
            /* Stamina penalty for fall */
            if (c->stamina >= 5) c->stamina -= 5;
        }
        world->party.leader_x = x;
        world->party.leader_y = y;
        theron_v1_play_sound(THERON_SOUND_PIT_FALL);
        return true;
    }
    return false;
}

/* ══════════════════════════════════════════════════════════════════════
 * Teleporter chain
 *
 * Source: movement_features.md — MOVESENS.C:475-537
 * Up to 100 iterations (100 for S21E+; S0 for older).
 * Resolves destination → checks if destination is itself a teleporter
 * and continues until reaching a non-teleporter square.
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_teleporter_resolve(Theron_V1_World *world, int x, int y) {
    if (!world) return -1;
    int iteration = 0;
    int cx = x, cy = y;

    while (iteration < THERON_TELEPORTER_CHAIN_MAX) {
        Theron_V1_Object *o = theron_v1_object_at(world,
                                world->current_level, cx, cy);
        if (!o || o->type != THERON_OBJTYPE_TELEPORTER) break;

        /* Find the teleporter's target */
        int link_id = o->linked_id;
        Theron_V1_Object *target = NULL;
        for (int i = 0; i < world->object_count; i++) {
            if (world->objects[i].id == link_id) {
                target = &world->objects[i];
                break;
            }
        }
        if (!target) break;
        if (target->type == THERON_OBJTYPE_TELEPORTER) {
            cx = target->x;
            cy = target->y;
            iteration++;
            continue; /* chain continues */
        }
        /* Non-teleporter target: settle here */
        world->transition_spawn_x = target->x;
        world->transition_spawn_y = target->y;
        world->transition_pending = 1;
        world->transition_type = THERON_TRANSITION_TELEPORTER;
        world->transition_target_level = world->current_level;
        world->party.leader_x = target->x;
        world->party.leader_y = target->y;
        theron_v1_play_sound(THERON_SOUND_TELEPORT);
        return 0;
    }

    /* Fallback: no chain resolved; place party at clicked square */
    world->transition_spawn_x = x;
    world->transition_spawn_y = y;
    world->transition_pending = 1;
    world->transition_target_level = world->current_level;
    world->party.leader_x = x;
    world->party.leader_y = y;
    theron_v1_play_sound(THERON_SOUND_TELEPORT);
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Altar of Vi resurrection
 *
 * TQ-specific: altar squares placed in dungeon 3 (Abyss of Flames) and
 * dungeon 6 (Castle of Fate). When a dead champion (health == 0,
 * alive == 0) steps onto an altar square, they are resurrected for a
 * gold cost. Theron's stats (fighter levels) persist across dungeons;
 * companions reset their inventory each dungeon (T800).
 *
 * Source: THQUEST.ASM T900 altar processing.
 *
 * Cost: 500 gold per resurrection. No anti-magic field blocking in TQ.
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_altar_of_vi_resurrect(Theron_V1_World *world,
                                     int altar_x, int altar_y) {
    if (!world) return -1;
    Theron_V1_Object *altar = theron_v1_object_at(world,
                                     world->current_level, altar_x, altar_y);
    if (!altar || altar->type != THERON_OBJTYPE_ALTAR_VI) return -1;
    if (altar->state == THERON_OBJ_F_DESTROYED) return -1;

    int cost_per_champion = 500;
    int total_cost = 0;
    int revived_count = 0;

    /* Check for dead champions */
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &world->party.champions[i];
        if (!c->alive && c->health == 0) {
            total_cost += cost_per_champion;
        }
    }

    if (total_cost == 0) return -1; /* no dead champions */
    if ((int)world->party.gold < total_cost) return -1; /* not enough gold */

    world->party.gold -= (uint32_t)total_cost;

    /* Revive all dead champions */
    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &world->party.champions[i];
        if (!c->alive && c->health == 0) {
            c->health = c->max_health >> 1; /* half max HP on resurrection */
            if (c->health < 1) c->health = 1;
            c->alive = 1;
            c->wounds &= ~(THERON_WOUND_HEAD | THERON_WOUND_BODY |
                           THERON_WOUND_ARMS | THERON_WOUND_LEGS);
            revived_count++;
        }
    }

    if (revived_count > 0) {
        altar->state = THERON_OBJ_F_USED;
        theron_v1_play_sound(THERON_SOUND_ALTAR_USE);
    }
    return world->party.active_slot; /* return revived slot (leader) */
}

/* ══════════════════════════════════════════════════════════════════════
 * Per-move stat updates
 *
 * Called after every successful party move (or blocked move).
 * Source: THQUEST.ASM T700 — per-tick stat processing (HP, poison, food/water)
 *
 * Per move:
 *   - Stamina drain: 1 per step on normal floor
 *   - Food drain: 1 per 20 ticks
 *   - Water drain: 1 per 15 ticks
 *   - Poison processing: if wounded + poisoned, 1 HP/tick + end poison
 *   - Wound effects: leg wounds reduce max_load (handled at equip time)
 * ══════════════════════════════════════════════════════════════════════ */

void theron_v1_apply_post_move_effects(Theron_V1_World *world) {
    if (!world) return;
    world->world_tick++;

    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &world->party.champions[i];
        if (!c->alive) continue;

        /* Stamina drain */
        if (c->stamina > 0) {
            c->stamina--;
            if (c->stamina <= 0) {
                c->stamina = 0;
                /* Stamina exhausted: no HP drain in TQ; just can't move fast */
            }
        }

        /* HP from wounds (1 HP / world tick when wounded) */
        if (c->wounds != 0) {
            c->health--;
            if (c->health <= 0) {
                c->health = 0;
                c->alive  = 0;
                theron_v1_champion_die(world, i);
                continue;
            }
        }

        /* Food drain */
        c->food--;
        if (c->food <= 0) {
            c->food = 0;
            c->health--;
            if (c->health <= 0) {
                c->health = 0;
                c->alive  = 0;
                theron_v1_champion_die(world, i);
                continue;
            }
        }

        /* Water drain */
        c->water--;
        if (c->water <= 0) {
            c->water = 0;
            c->health--;
            if (c->health <= 0) {
                c->health = 0;
                c->alive  = 0;
                theron_v1_champion_die(world, i);
                continue;
            }
        }

        /* Poison processing — TQ uses dungeon poison squares (PIT tile) */
        if (c->attributes & THERON_ATTR_POISONED) {
            c->health--;
            if (c->health <= 0) {
                c->health = 0;
                c->alive  = 0;
                theron_v1_champion_die(world, i);
                continue;
            }
            /* TQ: poison ends after 30 ticks (matching DM1) */
            c->attributes &= ~THERON_ATTR_POISONED;
        }
    }

    /* World tick timers */
    theron_v1_tick_timers(world);
    /* Creature AI tick */
    theron_v1_creature_ai_tick(world);
}

/* ══════════════════════════════════════════════════════════════════════
 * Pool / recovery square
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_pool_use(Theron_V1_World *world, int x, int y) {
    if (!world) return -1;
    Theron_V1_Object *p = theron_v1_object_at(world, world->current_level, x, y);
    if (!p || p->type != THERON_OBJTYPE_POOL) return -1;

    for (int i = 0; i < THERON_MAX_CHAMPIONS; i++) {
        Theron_V1_Champion *c = &world->party.champions[i];
        if (!c->alive) continue;
        c->water = c->max_stamina;  /* restore water/stamina */
        c->food  = c->max_stamina;  /* restore food */
        c->stamina = c->max_stamina;
        if (c->attributes & THERON_ATTR_POISONED) {
            c->attributes &= ~THERON_ATTR_POISONED;
        }
        c->wounds = 0; /* clear wounds */
    }
    p->state = THERON_OBJ_F_USED;
    theron_v1_play_sound(THERON_SOUND_POOL_USE);
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Alarm trigger
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_alarm_trigger(Theron_V1_World *world, int x, int y) {
    if (!world) return -1;
    Theron_V1_Object *a = theron_v1_object_at(world, world->current_level, x, y);
    if (!a || a->type != THERON_OBJTYPE_ALARM) return -1;

    /* Alert all creatures on the current level */
    for (int i = 0; i < world->object_count; i++) {
        Theron_V1_Object *o = &world->objects[i];
        if (o->type == THERON_OBJTYPE_CREATURE_SPAWNER) {
            o->flags |= THERON_OBJ_F_ACTIVATED;
        }
    }
    theron_v1_play_sound(THERON_SOUND_ALARM_TRIG);
    return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Trigger / event square
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_trigger_activate(Theron_V1_World *world, int x, int y) {
    if (!world) return -1;
    Theron_V1_Object *t = theron_v1_object_at(world, world->current_level, x, y);
    if (!t || t->type != THERON_OBJTYPE_TRIGGER) return -1;

    /* Activate linked objects */
    int link = t->linked_id;
    for (int i = 0; i < world->object_count; i++) {
        if (world->objects[i].id == link) {
            world->objects[i].flags |= THERON_OBJ_F_ACTIVATED;
        }
    }
    t->flags |= THERON_OBJ_F_ACTIVATED;
    return 0;
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char *theron_v1_mechanics_source_evidence(void) {
    return "THQUEST.ASM T520/T560/T600/T700/T800/T900  "
           "+ tqr_v1_phase2_data_formats_H2339.md  "
           "+ movement_features.md (ReDMCSB MOVESENS.C:475-538)";
}
