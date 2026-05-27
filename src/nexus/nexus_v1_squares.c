#include "nexus_v1_squares.h"
#include <string.h>
#include <stdio.h>

/* Nexus V1 special square processing.
 * Source: DM1 DUNGEON.C square type dispatch (SQUARE_TYPE() macro),
 * MOVESENS.C F0267/F0268 (move result + square event processing),
 * CLIKMENU.C:264-276 (stairs special cases), docs/nexus_squares.md.
 * Square type encoding matches DM1: lower 5 bits of square byte.
 * ═══════════════════════════════════════════════════════════════════ */

/* Door registry */
static Nexus_Door g_doors[NEXUS_MAX_DOORS];
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
    g_doors[idx].door_state = NEXUS_DOOR_STATE_OPEN;
    return 0;
}

int nexus_doors_close(int x, int y) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) return -1;
    g_doors[idx].door_state = NEXUS_DOOR_STATE_CLOSED;
    return 0;
}

int nexus_doors_toggle(int x, int y) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) return -1;
    if (g_doors[idx].door_state == NEXUS_DOOR_STATE_OPEN)
        g_doors[idx].door_state = NEXUS_DOOR_STATE_CLOSED;
    else if (g_doors[idx].door_state == NEXUS_DOOR_STATE_CLOSED)
        g_doors[idx].door_state = NEXUS_DOOR_STATE_OPEN;
    return 0;
}

int nexus_doors_lock(int x, int y, int key_item_id) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) idx = nexus_doors_register(x, y);
    if (idx < 0) return -1;
    g_doors[idx].door_state = NEXUS_DOOR_STATE_LOCKED;
    g_doors[idx].key_required = key_item_id;
    return 0;
}

int nexus_doors_is_open(int x, int y) {
    int idx = nexus_doors_find(x, y);
    if (idx < 0) return 0; /* no record = treat as closed */
    return g_doors[idx].door_state == NEXUS_DOOR_STATE_OPEN;
}

int nexus_doors_can_open(int x, int y, const uint8_t inventory[30]) {
    int idx = nexus_doors_find(x, y);
    int i;

    if (idx < 0) return 1; /* unregistered door = always passable */
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
    /* Fallback: same coords, +1/-1 level */
    if (out_target_x) *out_target_x = x;
    if (out_target_y) *out_target_y = y;
    return -1;
}

int nexus_stairs_count(void) {
    return g_stairs_count;
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
        return 1;
    default:
        return 0;
    }
}

Nexus_SquareEvent nexus_process_square_event(int type, int x, int y,
                                               int *out_target_x, int *out_target_y,
                                               int *out_target_level, int *out_target_dir) {
    int tx, ty, tl;

    if (out_target_x) *out_target_x = x;
    if (out_target_y) *out_target_y = y;
    if (out_target_level) *out_target_level = -1;
    if (out_target_dir) *out_target_dir = -1;

    switch (type) {
    case NEXUS_SQUARE_FLOOR:
        return NEXUS_EVENT_MOVE_OK;

    case NEXUS_SQUARE_DOOR: {
        if (!nexus_doors_is_open(x, y)) {
            /* Get champion pool inventory to check for key.
             * If no key is required, door always opens.
             * If key is required, check party leader's inventory.
             * Source: DM1 door processing — key check against required key. */
            const uint8_t *inv = NULL;
            uint8_t dummy_inv[30] = {[0 ... 29] = (uint8_t)-1};
            /* NOTE: engine doesn't have a direct pointer here.
             * Door-can-open check is delegated to the mechanics tick
             * which has engine access. The can_open function also
             * returns 1 for unregistered doors (always passable). */
            (void)inv;
            if (nexus_doors_can_open(x, y, dummy_inv) == 0) {
                /* Locked and no key in empty inventory — blocked */
                return NEXUS_EVENT_DOOR_BLOCKED;
            }
            nexus_doors_open(x, y);
            return NEXUS_EVENT_DOOR_OPEN;
        }
        return NEXUS_EVENT_MOVE_OK;
    }

    case NEXUS_SQUARE_STAIRS_DN:
        if (nexus_stairs_resolve(x, y, &tl, &tx, &ty, NULL) == 0) {
            if (out_target_level) *out_target_level = tl;
            if (out_target_x) *out_target_x = tx;
            if (out_target_y) *out_target_y = ty;
        } else {
            /* Default: same coords, level+1 */
            if (out_target_level) *out_target_level = -1; /* signal level down */
            if (out_target_x) *out_target_x = x;
            if (out_target_y) *out_target_y = y;
        }
        return NEXUS_EVENT_STAIRS_DOWN;

    case NEXUS_SQUARE_STAIRS_UP:
        if (nexus_stairs_resolve(x, y, &tl, &tx, &ty, NULL) == 0) {
            if (out_target_level) *out_target_level = tl;
            if (out_target_x) *out_target_x = tx;
            if (out_target_y) *out_target_y = ty;
        } else {
            if (out_target_level) *out_target_level = -1; /* signal level up */
            if (out_target_x) *out_target_x = x;
            if (out_target_y) *out_target_y = y;
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
         * Implementation: creature manager receives alert signal.
         * V1 stub: alarm event returned, actual alert dispatch in
         * creature tick. Source: DM1 MOVESENS.C alarm sensor. */
        return NEXUS_EVENT_ALARM_TRIGGER;

    case NEXUS_SQUARE_CHUTE:
        /* Chute: party forced to next level.
         * Default: same coords, level+1. */
        if (out_target_level) *out_target_level = -1;
        return NEXUS_EVENT_CHUTE_FALL;

    case NEXUS_SQUARE_EXIT:
        return NEXUS_EVENT_EXIT_REACHED;

    case NEXUS_SQUARE_WALL:
        return NEXUS_EVENT_BLOCKED;

    default:
        return NEXUS_EVENT_MOVE_OK;
    }
}