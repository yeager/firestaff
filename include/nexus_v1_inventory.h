#ifndef NEXUS_V1_INVENTORY_H
#define NEXUS_V1_INVENTORY_H

#include <stdint.h>

/* Nexus V1 inventory system.
 * 30 slots per champion (vs DM1's 12-slot grid).
 * Items stored as indices into a global item encyclopedia.
 * Supports: pickup from floor, drop to floor, equip/unequip,
 * swap between slots, cursor-held item. */

/* Item category enum — mirrors FS_ITEM_CAT_* but Nexus-specific */
typedef enum {
    NEXUS_ITEM_WEAPON = 0,
    NEXUS_ITEM_ARMOR,
    NEXUS_ITEM_POTION,
    NEXUS_ITEM_SCROLL,
    NEXUS_ITEM_CONTAINER,
    NEXUS_ITEM_MISC,
    NEXUS_ITEM_KEY,
    NEXUS_ITEM_SPELL_RUNE,
    NEXUS_ITEM_COUNT
} Nexus_ItemCategory;

/* Item definition — shared global catalog (Nexus uses same items as DM1) */
typedef struct {
    const char      *name;
    Nexus_ItemCategory category;
    int             weight;      /* encumbrance units */
    int             attack;      /* weapon power (0 if not weapon) */
    int             defense;     /* armor value (0 if not armor) */
    int             flags;      /* NEXUS_ITEMF_* */
} Nexus_ItemDef;

/* Equipment slot encoding */
typedef enum {
    NEXUS_SLOT_NONE      = 0,
    NEXUS_SLOT_HEAD      = 1,
    NEXUS_SLOT_TORSO     = 2,
    NEXUS_SLOT_LEGS      = 3,
    NEXUS_SLOT_FEET      = 4,
    NEXUS_SLOT_HANDS     = 5,
    NEXUS_SLOT_WEAPON    = 6,
    NEXUS_SLOT_SHIELD    = 7,
    NEXUS_SLOT_RING1     = 8,
    NEXUS_SLOT_RING2     = 9,
    NEXUS_SLOT_AMULET    = 10,
    NEXUS_SLOT_INVENTORY = 99  /* marker for inventory grid */
} Nexus_EquipSlot;

#define NEXUS_INVENTORY_SLOTS  30
#define NEXUS_EQUIP_SLOTS      11  /* head..amulet, plus 2 rings */
#define NEXUS_MAX_CARRY_WEIGHT 400 /* max carry weight before speed penalty */

#define NEXUS_ITEMF_CONSUMABLE  0x0001
#define NEXUS_ITEMF_EQUIPPABLE   0x0002
#define NEXUS_ITEMF_STACKABLE    0x0004
#define NEXUS_ITEMF_KEY          0x0008
#define NEXUS_ITEMF_NO_DROP      0x0010  /* quest items */

typedef struct {
    int item_id;          /* index into global item catalog, -1 = empty */
    int quantity;          /* stack count, 1 for non-stackable */
} Nexus_InventorySlot;

/* Floor item — dropped/chest item in the dungeon */
typedef struct {
    int item_id;
    int quantity;
    int x, y;             /* dungeon position */
    int on_ground;        /* 1 = on ground, 0 = in container */
} Nexus_FloorItem;

/* Cursor state — item currently held by player */
typedef struct {
    int item_id;
    int quantity;
    int source_type;      /* 0=none, 1=inventory, 2=floor */
    int source_idx;       /* champion slot index or floor item index */
} Nexus_CursorItem;

/* ═══════════════════════════════════════════════════════════════════
 * Global item catalog — mirrors DM1 item system.
 * Source: firestaff_item_encyclopedia.c (shared), nexus_items.md
 * ═══════════════════════════════════════════════════════════════════ */
extern const Nexus_ItemDef g_nexus_items[];
int nexus_itemdef_count(void);
const Nexus_ItemDef *nexus_itemdef_get(int id);
const char *nexus_itemdef_category_name(Nexus_ItemCategory cat);

/* ═══════════════════════════════════════════════════════════════════
 * Inventory management
 * ═══════════════════════════════════════════════════════════════════ */

/* Init an inventory to empty */
void nexus_inventory_init(Nexus_InventorySlot *inv, int count);

/* Get item in slot (returns item def or NULL) */
const Nexus_ItemDef *nexus_inventory_get(const Nexus_InventorySlot *inv, int slot);

/* Add item to first available slot. Returns slot index or -1. */
int nexus_inventory_add(Nexus_InventorySlot *inv, int count, int item_id, int qty);

/* Remove item from slot */
void nexus_inventory_remove(Nexus_InventorySlot *inv, int slot);

/* Move item within inventory */
int nexus_inventory_move(Nexus_InventorySlot *inv, int from, int to);

/* Get total weight of inventory */
int nexus_inventory_weight(const Nexus_InventorySlot *inv, int count);

/* Count items of a given category in inventory */
int nexus_inventory_count_category(const Nexus_InventorySlot *inv, int count, Nexus_ItemCategory cat);

/* Find first slot containing item_id (or -1) */
int nexus_inventory_find(const Nexus_InventorySlot *inv, int count, int item_id);

/* Equip item from inventory slot. Returns slot that was cleared or -1. */
int nexus_inventory_equip(Nexus_InventorySlot *inv, int slot,
                           int weapon_slot, int shield_slot,
                           int ring1_slot, int ring2_slot,
                           int head_slot, int torso_slot,
                           int legs_slot, int feet_slot, int hands_slot,
                           int amulet_slot);

/* Unequip and return to inventory. Returns slot index or -1. */
int nexus_inventory_unequip(Nexus_InventorySlot *inv, int count,
                             int equip_slot,
                             int weapon_slot, int shield_slot,
                             int ring1_slot, int ring2_slot,
                             int head_slot, int torso_slot,
                             int legs_slot, int feet_slot, int hands_slot,
                             int amulet_slot);

/* Cursor item helpers */
void nexus_cursor_clear(Nexus_CursorItem *cursor);
int nexus_cursor_can_pickup(const Nexus_CursorItem *cursor);
int nexus_cursor_pickup(Nexus_CursorItem *cursor, int item_id, int qty,
                         int source_type, int source_idx);
int nexus_cursor_place(Nexus_CursorItem *cursor);

/* Floor item management */
void nexus_floor_init(void);
int nexus_floor_drop(int x, int y, int item_id, int qty);
int nexus_floor_pickup(int floor_idx, int *out_item_id, int *out_qty);
int nexus_floor_count_at(int x, int y);
int nexus_floor_get_at(int x, int y, int idx, int *out_item_id, int *out_qty);

#endif /* NEXUS_V1_INVENTORY_H */