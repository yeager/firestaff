#include "nexus_v1_squares.h"
#include "nexus_v1_champions.h"
#include <string.h>
#include <stdio.h>

/* Nexus V1 special square processing.
 * Source: DM1 DUNGEON.C square type dispatch (SQUARE_TYPE() macro),
 * MOVESENS.C F0267/F0268 (move result + square event processing),
 * CLIKMENU.C:264-276 (stairs special cases), docs/nexus_squares.md.
 * Square type encoding matches DM1: lower 5 bits of square byte.
 * ═══════════════════════════════════════════════════════════════════ */

/* Door registry */
static Nexus_SquareDoor g_doors[NEXUS_MAX_DOORS];
static int g_door_count = 0;

/* Teleporter registry */
static Nexus_TeleporterLink g_teleporters[NEXUS_MAX_TELEPORTERS];
static int g_teleporter_count = 0;

/* Stairs registry */
static Nexus_StairsLink g_stairs[NEXUS_MAX_STAIRS];
static int g_stairs_count = 0;

const char *nexus_square_type_name(int type) {
    switch (type) {
    case NEXUS_SQUARE_WALL:      return "Wall";
    case NEXUS_SQUARE_FLOOR:     return "Floor";
    case NEXUS_SQUARE_STAIRS_DN:  return "Stairs Down";
    case NEXUS_SQUARE_STAIRS_UP:  return "Stairs Up";
    case NEXUS_SQUARE_DOOR:       return "Door";
    case NEXUS_SQUARE_TELEPORT:   return "Teleporter";
    case NEXUS_SQUARE_ALARM:      return "Alarm Trap";
    case NEXUS_SQUARE_CHUTE:      return "Chute/Trapdoor";
    case NEXUS_SQUARE_EXIT:       return "Exit";
    case NEXUS_SQUARE_TELEPORT2:  return "Level Teleporter";
    case NEXUS_SQUARE_TELEPORT3:  return "Intra-level Teleporter";
    case NEXUS_SQUARE_WATER:      return "Water";
    case NEXUS_SQUARE_FIRE:       return "Fire";
    default:                       return "Unknown";
    }
}

int nexus_square_is_passable_type(int type) {
    /* Wall (type 0) is the only impassable type.
     * All other types are passable (may have special effects on entry). */
    return type != NEXUS_SQUARE_WALL;
}

int nexus_square_is_wall(int type) {
    return type == NEXUS_SQUARE_WALL;
}

Nexus_SquareInfo nexus_square_info(int type) {
    Nexus_SquareInfo i;
    memset(&i, 0, sizeof(i));
    i.type = type;
    i.passable = nexus_square_is_passable_type(type);
    i.is_door = (type == NEXUS_SQUARE_DOOR);
    i.is_stairs = (type == NEXUS_SQUARE_STAIRS_DN || type == NEXUS_SQUARE_STAIRS_UP);
    i.is_teleporter = (type == NEXUS_SQUARE_TELEPORT || type == NEXUS_SQUARE_TELEPORT2 || type == NEXUS_SQUARE_TELEPORT3);
    i.is_trap = (type == NEXUS_SQUARE_ALARM || type == NEXUS_SQUARE_CHUTE);
    i.is_exit = (type == NEXUS_SQUARE_EXIT);
    i.teleporter_target = -1;
    return i;
}

/* ═══════════════════════════════════════════════════════════════════
 * Door state management
 * DM1: doors are square-type 4. State (open/closed/locked) is tracked
 * separately from the square grid. Opening a door changes its
 * visual representation but not the square type.
 * Source: DM1 viewport rendering (door sprite overlay when closed).
 * Nexus: 3D door geometry switches between open/closed variants
 * based on door state.
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_doors_init(void) {
    g_door_count = 0;
    memset(g_doors, 0, sizeof(g_doors));
}

int nexus_doors_register(int x, int y) {
    int i;
    if (g_door_count >= NEXUS_MAX_DOORS) return -1;
    /* Check if already registered */
    for (i = 0; i < g_door_count; i++) {
        if (g_doors[i].x == x && g_doors[i].y == y) return i;
    }
    i = g_door_count++;
    g_doors[i].x = x;
    g_doors[i].y = y;
    g_doors[i].door_state = NEXUS_DOOR_STATE_CLOSED;
    g_doors[i].key_required = -1;
    g_doors[i].animation_step = 0;
    return i;
}

static int nexus_doors_find(int x, int y) {
    int i;
    for (i = 0; i < g_door_count; i++) {
        if (g_doors[i].x == x && g_doors[i].y == y) return i;
    }
    return -1;
}

int nexus_doors_open(int x, int y) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) idx = nexus_doors_register(x, y);
    if (idx < 0) return -1;
    if (g_doors[idx].door_state == NEXUS_DOOR_STATE_LOCKED) return -1;
    /* If already open or opening, leave it alone. */
    if (g_doors[idx].door_state == NEXUS_DOOR_STATE_OPEN ||
        g_doors[idx].door_state == NEXUS_DOOR_STATE_OPENING) {
        return 0;
    }
    /* Start opening animation from current step (closed=0). */
    g_doors[idx].door_state = NEXUS_DOOR_STATE_OPENING;
    if (g_doors[idx].animation_step == 0)
        g_doors[idx].animation_step = 0;
    return 0;
}

int nexus_doors_close(int x, int y) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) return -1;
    if (g_doors[idx].door_state == NEXUS_DOOR_STATE_LOCKED) return -1;
    /* If already closed or closing, leave it alone. */
    if (g_doors[idx].door_state == NEXUS_DOOR_STATE_CLOSED ||
        g_doors[idx].door_state == NEXUS_DOOR_STATE_CLOSING) {
        return 0;
    }
    /* Start closing animation from current step (open=full). */
    g_doors[idx].door_state = NEXUS_DOOR_STATE_CLOSING;
    if (g_doors[idx].animation_step == 0)
        g_doors[idx].animation_step = NEXUS_DOOR_ANIMATION_STEPS;
    return 0;
}

int nexus_doors_toggle(int x, int y) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) return -1;
    switch (g_doors[idx].door_state) {
    case NEXUS_DOOR_STATE_OPEN:
    case NEXUS_DOOR_STATE_OPENING:
        return nexus_doors_close(x, y);
    case NEXUS_DOOR_STATE_CLOSED:
    case NEXUS_DOOR_STATE_CLOSING:
        return nexus_doors_open(x, y);
    default:
        return -1;
    }
}

int nexus_doors_lock(int x, int y, int key_item_id) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) idx = nexus_doors_register(x, y);
    if (idx < 0) return -1;
    g_doors[idx].door_state = NEXUS_DOOR_STATE_LOCKED;
    g_doors[idx].key_required = key_item_id;
    g_doors[idx].animation_step = 0;
    return 0;
}

int nexus_doors_is_open(int x, int y) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) return 0; /* no record = treat as closed */
    return g_doors[idx].door_state == NEXUS_DOOR_STATE_OPEN;
}

int nexus_doors_is_passable(int x, int y) {
    int idx = nexus_doors_find(x, y);
    /* A type-8 square without a source-owned door record has no proven
     * SDDRVS/DGN state.  Do not turn missing provenance into an open door. */
    if (idx < 0) return 0;
    switch (g_doors[idx].door_state) {
    case NEXUS_DOOR_STATE_OPEN:
        return 1;
    case NEXUS_DOOR_STATE_OPENING:
    case NEXUS_DOOR_STATE_CLOSING:
        return g_doors[idx].animation_step >= NEXUS_DOOR_PASSABLE_STEP_THRESHOLD;
    default:
        return 0;
    }
}

int nexus_doors_can_open(int x, int y, const uint8_t inventory[30]) {
    int idx = nexus_doors_find(x, y);
    int i;

    /* Missing registration is not an unlocked door: SDDRVS/DGN must first
     * supply the state and key relation. */
    if (idx < 0) return 0;
    if (g_doors[idx].door_state != NEXUS_DOOR_STATE_LOCKED) return 1; /* not locked */

    /* Locked door — check for required key */
    int key_required = g_doors[idx].key_required;
    if (key_required < 0) return 1; /* no key required */
    if (!inventory) return 0; /* no inventory = can't open */

    /* Scan inventory for the required key or a generic "Key" (item_id 66).
     * A generic Key can open any locked door.
     * Specific key items (Gold Key=70, Silver Key=71, Skeleton Key=72)
     * open doors that specifically require that key type.
     * Source: DM1 key usage — generic key opens all doors,
     * specific keys only open matching doors. */
    for (i = 0; i < 30; i++) {
        int item_id = inventory[i];
        if (item_id < 0) continue;
        /* Generic key (item 66) opens any locked door */
        if (item_id == 66) return 1;
        /* Specific key matches required key */
        if (item_id == key_required) return 1;
    }

    return 0; /* key not found */
}

/* Advance every animating door by one step.  Opening doors increment their
 * step; closing doors decrement.  When the end of the range is reached the
 * door settles in the matching open/closed state.
 * Source: DM1 viewport door animation (stepped open/close frames).
 * Returns 1 if any door changed step or state this tick. */
int nexus_doors_tick_animation(void) {
    int i;
    int changed = 0;
    for (i = 0; i < g_door_count; i++) {
        Nexus_SquareDoor *d = &g_doors[i];
        if (d->door_state == NEXUS_DOOR_STATE_OPENING) {
            if (d->animation_step < NEXUS_DOOR_ANIMATION_STEPS) {
                d->animation_step++;
                changed = 1;
            }
            if (d->animation_step >= NEXUS_DOOR_ANIMATION_STEPS) {
                d->door_state = NEXUS_DOOR_STATE_OPEN;
            }
        } else if (d->door_state == NEXUS_DOOR_STATE_CLOSING) {
            if (d->animation_step > 0) {
                d->animation_step--;
                changed = 1;
            }
            if (d->animation_step <= 0) {
                d->door_state = NEXUS_DOOR_STATE_CLOSED;
                d->animation_step = 0;
            }
        }
    }
    return changed;
}

int nexus_doors_animation_step(int x, int y) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) return 0;
    return g_doors[idx].animation_step;
}

/* ═══════════════════════════════════════════════════════════════════
 * Teleporter resolution
 * DM1: types 9 and 10 are hardwired teleporters.
 * Nexus: teleporters are scripted via SDDRVS.TSK.
 * V1 implementation: teleporter links are pre-registered from DGN data
 * or from SDDRVS.TSK script actions.
 * Source: DM1 DUNGEON.C teleporter processing, nexus_squares.md.
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_teleporters_init(void) {
    g_teleporter_count = 0;
    memset(g_teleporters, 0, sizeof(g_teleporters));
}

int nexus_teleporters_register(int from_x, int from_y, int to_x, int to_y, int to_level) {
    if (g_teleporter_count >= NEXUS_MAX_TELEPORTERS) return -1;
    int i = g_teleporter_count++;
    g_teleporters[i].from_x = from_x;
    g_teleporters[i].from_y = from_y;
    g_teleporters[i].to_x = to_x;
    g_teleporters[i].to_y = to_y;
    g_teleporters[i].to_level = to_level;
    return i;
}

int nexus_teleporters_resolve(int x, int y, int *out_to_x, int *out_to_y, int *out_to_level) {
    int i;
    for (i = 0; i < g_teleporter_count; i++) {
        if (g_teleporters[i].from_x == x && g_teleporters[i].from_y == y) {
            if (out_to_x) *out_to_x = g_teleporters[i].to_x;
            if (out_to_y) *out_to_y = g_teleporters[i].to_y;
            if (out_to_level) *out_to_level = g_teleporters[i].to_level;
            return 0;
        }
    }
    return -1; /* not found */
}

int nexus_teleporters_count(void) {
    return g_teleporter_count;
}

/* ═══════════════════════════════════════════════════════════════════
 * Stairs resolution
 * DM1: type 2 = stairs down, type 3 = stairs up.
 * Each stairs square links to a specific target on the adjacent level.
 * DM1 stairs target: adjacent level at same (x,y) or shifted.
 * Nexus: stairs scripted via SDDRVS.TSK.
 * Source: DM1 CLIKMENU.C F0364_COMMAND_TakeStairs.
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_stairs_init(void) {
    g_stairs_count = 0;
    memset(g_stairs, 0, sizeof(g_stairs));
}

int nexus_stairs_register(int x, int y, int target_level, int target_x, int target_y, int target_dir) {
    if (g_stairs_count >= NEXUS_MAX_STAIRS) return -1;
    int i = g_stairs_count++;
    g_stairs[i].x = x;
    g_stairs[i].y = y;
    g_stairs[i].target_level = target_level;
    g_stairs[i].target_x = target_x;
    g_stairs[i].target_y = target_y;
    g_stairs[i].target_dir = target_dir;
    return i;
}

int nexus_stairs_resolve(int x, int y, int *out_target_level, int *out_target_x, int *out_target_y, int *out_target_dir) {
    int i;
    for (i = 0; i < g_stairs_count; i++) {
        if (g_stairs[i].x == x && g_stairs[i].y == y) {
            if (out_target_level) *out_target_level = g_stairs[i].target_level;
            if (out_target_x) *out_target_x = g_stairs[i].target_x;
            if (out_target_y) *out_target_y = g_stairs[i].target_y;
            if (out_target_dir) *out_target_dir = g_stairs[i].target_dir;
            return 0;
        }
    }
    /* An unregistered stair has no source-owned destination. Do not invent
     * an adjacent level or reuse the source coordinates as a fake link. */
    if (out_target_level) *out_target_level = -1;
    if (out_target_x) *out_target_x = -1;
    if (out_target_y) *out_target_y = -1;
    if (out_target_dir) *out_target_dir = -1;
    return -1;
}

int nexus_stairs_count(void) {
    return g_stairs_count;
}

/* ═══════════════════════════════════════════════════════════════════
 * Pit / chute resolution
 * Source: DM1 MOVESENS.C F0267/F0268 chute/pit sensor.
 * ═══════════════════════════════════════════════════════════════════ */

static Nexus_PitLink g_pits[NEXUS_MAX_PITS];
static int g_pit_count = 0;

void nexus_pits_init(void) {
    g_pit_count = 0;
    memset(g_pits, 0, sizeof(g_pits));
}

int nexus_pits_register(int x, int y, int target_x, int target_y, int target_level) {
    if (g_pit_count >= NEXUS_MAX_PITS || target_level < 0) return -1;
    g_pits[g_pit_count].x = x;
    g_pits[g_pit_count].y = y;
    g_pits[g_pit_count].target_x = target_x;
    g_pits[g_pit_count].target_y = target_y;
    g_pits[g_pit_count].target_level = target_level;
    return g_pit_count++;
}

int nexus_pits_resolve(int x, int y, int *out_target_x, int *out_target_y, int *out_target_level) {
    int i;
    for (i = 0; i < g_pit_count; i++) {
        if (g_pits[i].x == x && g_pits[i].y == y) {
            if (out_target_x) *out_target_x = g_pits[i].target_x;
            if (out_target_y) *out_target_y = g_pits[i].target_y;
            if (out_target_level) *out_target_level = g_pits[i].target_level;
            return 0;
        }
    }
    return -1;
}

int nexus_pits_count(void) {
    return g_pit_count;
}

/* ═══════════════════════════════════════════════════════════════════
 * Altar registry — Vi altar of rebirth (alcove-based resurrection).
 * Source: DMWeb DGN Structure1Fd docs, Nexus manual translation.
 * Nexus has only the Vi altar; FUL/BRO/GATH are DM1-specific.
 * ═══════════════════════════════════════════════════════════════════ */

static Nexus_Altar g_altars[NEXUS_MAX_ALTARS];
static int g_altar_count = 0;

void nexus_altars_init(void) {
    g_altar_count = 0;
    memset(g_altars, 0, sizeof(g_altars));
}

int nexus_altars_register(int x, int y) {
    return nexus_altars_register_tagged(x, y, NEXUS_ALTAR_TAG_UNKNOWN);
}

int nexus_altars_register_tagged(int x, int y, uint8_t tag) {
    int i;
    if (g_altar_count >= NEXUS_MAX_ALTARS) return -1;
    for (i = 0; i < g_altar_count; i++) {
        if (g_altars[i].x == x && g_altars[i].y == y) {
            if (tag != NEXUS_ALTAR_TAG_UNKNOWN &&
                g_altars[i].tag == NEXUS_ALTAR_TAG_UNKNOWN) {
                g_altars[i].tag = tag;
            }
            return i;
        }
    }
    g_altars[g_altar_count].x = x;
    g_altars[g_altar_count].y = y;
    g_altars[g_altar_count].tag = tag;
    g_altars[g_altar_count].level = 0;
    return g_altar_count++;
}

int nexus_altar_at(int x, int y) {
    int i;
    for (i = 0; i < g_altar_count; i++) {
        if (g_altars[i].x == x && g_altars[i].y == y)
            return 1;
    }
    return 0;
}

int nexus_altar_tag_at(int x, int y) {
    int i;
    for (i = 0; i < g_altar_count; i++) {
        if (g_altars[i].x == x && g_altars[i].y == y)
            return (int)g_altars[i].tag;
    }
    return NEXUS_ALTAR_TAG_UNKNOWN;
}

int nexus_altars_count(void) {
    return g_altar_count;
}

int nexus_altar_resurrect(int x, int y, Nexus_V1_Champion *champion) {
    int tag;
    if (!champion) return -1;
    tag = nexus_altar_tag_at(x, y);
    if (tag != NEXUS_ALTAR_TAG_VI) return 0;
    if (!champion->alive) {
        champion->alive = 1;
        champion->health = champion->max_health;
        champion->stamina = champion->max_stamina;
        champion->mana = champion->max_mana;
        return 1;
    }
    return 0;
}

int nexus_altar_perform_ritual(int x, int y, Nexus_V1_Champion *leader) {
    return nexus_altar_resurrect(x, y, leader);
}

/* Registry counts */
int nexus_doors_count(void) {
    return g_door_count;
}

/* ═══════════════════════════════════════════════════════════════════
 * Square event processing on party entry
 * Source: DM1 MOVESENS.C F0267/F0268, CLIKMENU.C:271-276,
 * docs/nexus_squares.md.
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_square_triggers_on_entry(int type) {
    /* These types trigger events when party steps onto them */
    switch (type) {
    case NEXUS_SQUARE_STAIRS_DN:
    case NEXUS_SQUARE_STAIRS_UP:
    case NEXUS_SQUARE_TELEPORT:
    case NEXUS_SQUARE_TELEPORT2:
    case NEXUS_SQUARE_TELEPORT3:
    case NEXUS_SQUARE_ALARM:
    case NEXUS_SQUARE_CHUTE:
    case NEXUS_SQUARE_EXIT:
    case NEXUS_SQUARE_WATER:
    case NEXUS_SQUARE_FIRE:
        return 1;
    default:
        return 0;
    }
}

Nexus_SquareEvent nexus_process_square_event(int type, int x, int y,
                                               int *out_target_x, int *out_target_y,
                                               int *out_target_level, int *out_target_dir) {
    int tx, ty, tl, td;

    if (out_target_x) *out_target_x = x;
    if (out_target_y) *out_target_y = y;
    if (out_target_level) *out_target_level = -1;
    if (out_target_dir) *out_target_dir = -1;

    switch (type) {
    case NEXUS_SQUARE_FLOOR:
        return NEXUS_EVENT_MOVE_OK;

    case NEXUS_SQUARE_DOOR: {
        /* Inventory ownership belongs to the mechanics caller. That caller
         * performs the source-bound key check before movement; this event
         * stage must not manufacture an empty inventory as a substitute. */
        if (!nexus_doors_is_open(x, y)) {
            if (!nexus_doors_can_open(x, y, NULL))
                return NEXUS_EVENT_DOOR_BLOCKED;
            nexus_doors_open(x, y);
            return NEXUS_EVENT_DOOR_OPEN;
        }
        return NEXUS_EVENT_MOVE_OK;
    }

    case NEXUS_SQUARE_STAIRS_DN:
        if (nexus_stairs_resolve(x, y, &tl, &tx, &ty, &td) == 0) {
            if (out_target_level) *out_target_level = tl;
            if (out_target_x) *out_target_x = tx;
            if (out_target_y) *out_target_y = ty;
            if (out_target_dir) *out_target_dir = td;
        } else {
            /* A stair without a source-owned Structure1F/SDDRVS link has no
             * transition semantics. Do not invent an adjacent-level route. */
            return NEXUS_EVENT_BLOCKED;
        }
        return NEXUS_EVENT_STAIRS_DOWN;

    case NEXUS_SQUARE_STAIRS_UP:
        if (nexus_stairs_resolve(x, y, &tl, &tx, &ty, &td) == 0) {
            if (out_target_level) *out_target_level = tl;
            if (out_target_x) *out_target_x = tx;
            if (out_target_y) *out_target_y = ty;
            if (out_target_dir) *out_target_dir = td;
        } else {
            /* A stair without a source-owned Structure1F/SDDRVS link has no
             * transition semantics. Do not invent an adjacent-level route. */
            return NEXUS_EVENT_BLOCKED;
        }
        return NEXUS_EVENT_STAIRS_UP;

    case NEXUS_SQUARE_TELEPORT:
    case NEXUS_SQUARE_TELEPORT2:
    case NEXUS_SQUARE_TELEPORT3:
        if (nexus_teleporters_resolve(x, y, &tx, &ty, &tl) == 0) {
            if (out_target_x) *out_target_x = tx;
            if (out_target_y) *out_target_y = ty;
            if (out_target_level) *out_target_level = tl;
            return NEXUS_EVENT_TELEPORT;
        }
        return NEXUS_EVENT_MOVE_OK;

    case NEXUS_SQUARE_ALARM:
        /* Alarm: set all creatures to alerted state.
         * Event dispatched to nexus_v1_creatures_alert_all() in
         * nexus_v1_mechanics.c:1096. Source: DM1 MOVESENS.C alarm sensor. */
        return NEXUS_EVENT_ALARM_TRIGGER;

    case NEXUS_SQUARE_CHUTE:
        /* Chute: party is forced only to a registered source-owned target.
         * Source: DM1 MOVESENS.C F0267/F0268 pit/chute sensor. */
        if (nexus_pits_resolve(x, y, &tx, &ty, &tl) == 0) {
            if (out_target_x) *out_target_x = tx;
            if (out_target_y) *out_target_y = ty;
            if (out_target_level) *out_target_level = tl;
        } else {
            /* Do not invent a same-coordinate adjacent-level transition. */
            return NEXUS_EVENT_BLOCKED;
        }
        return NEXUS_EVENT_CHUTE_FALL;

    case NEXUS_SQUARE_EXIT:
        return NEXUS_EVENT_EXIT_REACHED;

    case NEXUS_SQUARE_WATER:
        /* Water crossing is permitted only when the party leader carries a
         * Rope. The passability gate lives in nexus_mechanics_tick(); if we
         * reach this event the crossing was allowed.
         * Source: DM1 MOVESENS.C water square sensor. */
        return NEXUS_EVENT_CROSS_WATER;

    case NEXUS_SQUARE_FIRE:
        /* Fire crossing is permitted only when the party leader carries a
         * Rune of Fire (V1 proxy for fire protection). The passability gate
         * lives in nexus_mechanics_tick().
         * Source: DM1 MOVESENS.C fire square sensor. */
        return NEXUS_EVENT_CROSS_FIRE;

    case NEXUS_SQUARE_WALL:
        return NEXUS_EVENT_BLOCKED;

    default:
        return NEXUS_EVENT_MOVE_OK;
    }
}
