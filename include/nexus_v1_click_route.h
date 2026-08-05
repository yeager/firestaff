#ifndef NEXUS_V1_CLICK_ROUTE_H
#define NEXUS_V1_CLICK_ROUTE_H

/* Nexus V1 mouse click-route dispatch.
 *
 * Converts high-level UI click targets (inventory slots, world squares,
 * doors, floor items) into the same movement/action commands the keyboard
 * path uses.  This keeps M11/UI code from poking directly into mechanics
 * state; instead a click becomes one or more queued commands consumed by
 * nexus_mechanics_tick().
 *
 * Source: DM1 COMMAND.C mouse/click dispatch (inventory use, viewport click
 *         to move/open/interact), CLIKMENU.C F0366 command queue,
 *         CHAMPION.C F0309 equipment slot layout.
 */

#include "nexus_v1_engine.h"
#include "nexus_v1_mechanics.h"

/* Kind of UI element the player clicked */
typedef enum {
    NEXUS_CLICK_TARGET_NONE = 0,
    NEXUS_CLICK_TARGET_INVENTORY_SLOT,  /* champion backpack slot */
    NEXUS_CLICK_TARGET_EQUIPMENT_SLOT,  /* champion equipment slot */
    NEXUS_CLICK_TARGET_WORLD_SQUARE,    /* dungeon grid coordinate */
    NEXUS_CLICK_TARGET_DOOR_SQUARE,     /* specific door in the dungeon */
    NEXUS_CLICK_TARGET_FLOOR_ITEM       /* item lying on the floor */
} Nexus_ClickTargetKind;

/* High-level click target.  Only the fields relevant to |kind| are read. */
typedef struct {
    Nexus_ClickTargetKind kind;
    int champion_index;  /* INVENTORY_SLOT / EQUIPMENT_SLOT */
    int slot;            /* INVENTORY_SLOT: 0..29; EQUIPMENT_SLOT: NEXUS_SLOT_* */
    int x, y;            /* WORLD_SQUARE / DOOR_SQUARE / FLOOR_ITEM position */
    int floor_item_idx;  /* FLOOR_ITEM: index from nexus_floor_get_at() */
} Nexus_ClickTarget;

/* Result of attempting to dispatch a click */
typedef enum {
    NEXUS_CLICK_RESULT_OK = 0,
    NEXUS_CLICK_RESULT_INVALID,     /* NULL argument or bad target */
    NEXUS_CLICK_RESULT_NO_ACTION,   /* empty slot / no actionable target */
    NEXUS_CLICK_RESULT_BLOCKED,     /* target not reachable/interactable */
    NEXUS_CLICK_RESULT_NO_PATH      /* movement target is a wall, etc. */
} Nexus_ClickResult;

/* Constructor helpers */
Nexus_ClickTarget nexus_click_target_inventory_slot(int champion_index, int slot);
Nexus_ClickTarget nexus_click_target_equipment_slot(int champion_index, int slot);
Nexus_ClickTarget nexus_click_target_world_square(int x, int y);
Nexus_ClickTarget nexus_click_target_door_square(int x, int y);
Nexus_ClickTarget nexus_click_target_floor_item(int x, int y, int floor_item_idx);

/* Dispatch a click target into the mechanics input queue.
 * Returns immediately; the caller must still call nexus_mechanics_tick()
 * to consume the queued command(s).
 *
 * Inventory slot: pushes NEXUS_CMD_USE_ITEM for usable items.
 * World square:   pushes turn / forward / strafe commands to move toward it.
 * Door square:    pushes turn + forward; the step logic opens the door.
 * Floor item:     pushes movement if not on the square, else NEXUS_CMD_INTERACT
 *                 to pick the item up into the leader's inventory.
 */
Nexus_ClickResult nexus_click_route_dispatch(Nexus_MechanicsState *st,
                                             Nexus_V1_Engine *engine,
                                             const Nexus_ClickTarget *target);

#endif /* NEXUS_V1_CLICK_ROUTE_H */
